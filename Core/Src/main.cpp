
/*
Remora firmware for LinuxCNC
Copyright (C) 2022  Scott Alford (aka scotta)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#define BOARD 			"STM32H7xx"
#define MAJOR_VERSION 	0
#define MINOR_VERSION	1
#define PATCH			0



#include "main.h"
#include "fatfs.h"

#include <stdio.h>
#include <cstring>
#include <sys/errno.h>

#include "configuration.h"
#include "remora.h"

// libraries
#include "lib/ArduinoJson6/ArduinoJson.h"

// drivers
#include "drivers/pin/pin.h"

// interrupts
#include "interrupt/irqHandlers.h"
#include "interrupt/interrupt.h"

// threads
#include "thread/pruThread.h"
#include "thread/createThreads.h"

// modules
#include "modules/remoraComms/RemoraComms.h"


/***********************************************************************
*                STRUCTURES AND GLOBAL VARIABLES                       *
************************************************************************/

// state machine
enum State {
    ST_SETUP = 0,
    ST_START,
    ST_IDLE,
    ST_RUNNING,
    ST_STOP,
    ST_RESET,
    ST_WDRESET
};

uint8_t resetCnt;
uint32_t base_freq = PRU_BASEFREQ;
uint32_t servo_freq = PRU_SERVOFREQ;

// boolean
volatile bool PRUreset;
bool configError = false;
bool threadsRunning = false;

// pointers to objects with global scope
pruThread* servoThread;
pruThread* baseThread;
pruThread* commsThread;

RxPingPongBuffer rxPingPongBuffer;
TxPingPongBuffer txPingPongBuffer;

// unions for RX and TX data
__attribute__((aligned(32))) volatile rxData_t rxData;	// TODO: remove
__attribute__((aligned(32))) volatile txData_t txData;	// TODO: remove

// pointers to data
rxData_t* pruRxData;
volatile rxData_t*  ptrRxData = &rxData;	// TODO: remove
volatile txData_t*  ptrTxData = &txData;	// TODO: remove
volatile int32_t* 	ptrTxHeader;
volatile bool*    	ptrPRUreset;
volatile int32_t* 	ptrJointFreqCmd[JOINTS];
volatile int32_t* 	ptrJointFeedback[JOINTS];
volatile uint8_t* 	ptrJointEnable;
volatile float*   	ptrSetPoint[VARIABLES];
volatile float*   	ptrProcessVariable[VARIABLES];
volatile uint16_t* 	ptrInputs;
volatile uint16_t* 	ptrOutputs;

// JSONconfig file stuff
std::string strJson;
DynamicJsonDocument doc(JSON_BUFF_SIZE);
JsonObject thread;
JsonObject module;


/***********************************************************************
        OBJECTS etc
************************************************************************/

RemoraComms* comms = new RemoraComms(ptrRxData, ptrTxData, SPI1);



/***********************************************************************
        PLATFORM SPECIFIC
************************************************************************/

SD_HandleTypeDef hsd1;
UART_HandleTypeDef huart1;

void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SDMMC1_SD_Init(void);



// re-target printf to UART1 by redeclaring week function in syscalls.c
extern "C" {
	int __io_putchar(int ch)
	{
	  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
	  return ch;
	}
}



void readJsonConfig()
{
	uint32_t bytesread; // bytes read count


    printf("\n1. Reading JSON configuration file\n");

    // Try to mount the file system
    printf("	Mounting the file system... \n");
    if(f_mount(&SDFatFS, (TCHAR const*)SDPath, 0) != FR_OK)
	{
    	printf("	Failed to mount SD card\n\r");
    	Error_Handler();
	}
    else
    {
		//Open file for reading
		if(f_open(&SDFile, "config.txt", FA_READ) != FR_OK)
		{
			printf("	Failed to open JSON config file\n");
			Error_Handler();
		}
		else
		{
			int32_t length = f_size(&SDFile);
			printf("	JSON config file lenght = %2ld\n", length);

			__attribute__((aligned(32))) char rtext[length];
			if(f_read(&SDFile, rtext, length, (UINT *)&bytesread) != FR_OK)
			{
				printf("	JSON config file read FAILURE\n");
			}
			else
			{
				printf("	JSON config file read SUCCESS!\n");
				// put JSON char array into std::string
				strJson.reserve(length + 1);
			    for (int i = 0; i < length; i++) {
			    	strJson = strJson + rtext[i];
			    }

			    // Remove comments from next line to print out the JSON config file
			    printf("\n%s\n", strJson.c_str());
			}

			f_close(&SDFile);
		}
    }
}

void setup()
{
    printf("\n2. Setting up SPI DMA and threads\n");

     // initialise the Remora comms
    comms->init();
    comms->start();
}


void deserialiseJSON()
{
    printf("\n3. Parsing json configuration file\n");

    const char *json = strJson.c_str();

    // parse the json configuration file
    DeserializationError error = deserializeJson(doc, json);

    printf("	Config deserialisation - ");

    switch (error.code())
    {
        case DeserializationError::Ok:
            printf("Deserialization succeeded\n");
            break;
        case DeserializationError::InvalidInput:
            printf("Invalid input!\n");
            configError = true;
            break;
        case DeserializationError::NoMemory:
            printf("Not enough memory\n");
            configError = true;
            break;
        default:
            printf("Deserialization failed: ");
            printf(error.c_str());
            printf("\n");
            configError = true;
            break;
    }
}

void configThreads()
{
    if (configError) return;

    printf("\n4. Configuring threads\n");

    JsonArray Threads = doc["Threads"];

    // create objects from JSON data
    for (JsonArray::iterator it=Threads.begin(); it!=Threads.end(); ++it)
    {
        thread = *it;

        const char* configor = thread["Thread"];
        uint32_t    freq = thread["Frequency"];

        if (!strcmp(configor,"Base"))
        {
            base_freq = freq;
            printf("	Setting BASE thread frequency to %lu\n", base_freq);
        }
        else if (!strcmp(configor,"Servo"))
        {
            servo_freq = freq;
            printf("	Setting SERVO thread frequency to %lu\n", servo_freq);
        }
    }
}


int main(void)
{
	MPU_Config();
	SCB->VTOR = 0x08000000;
	HAL_Init();
	SystemClock_Config();
	PeriphCommonClock_Config();

	// Enable caches
	SCB_InvalidateICache();
	SCB_EnableICache();
	SCB_InvalidateDCache();
	SCB_EnableDCache();

	/* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();

	MX_GPIO_Init(); // used for SD card detect
	MX_USART1_UART_Init();
	MX_SDMMC1_SD_Init();		// uncomment #define ENABLE_SD_DMA_CACHE_MAINTENANCE  1 in sd_diskio.c
	MX_FATFS_Init();


	txPingPongBuffer.txBuffers[0].header = PRU_DATA;
	txPingPongBuffer.txBuffers[1].header = PRU_DATA;

	enum State currentState;
	enum State prevState;

	currentState = ST_SETUP;
	prevState = ST_RESET;

    comms->setStatus(false);
    comms->setError(false);
    resetCnt = 0;

	printf("\nRemora version %d.%d.%d for %s starting\n\n", MAJOR_VERSION, MINOR_VERSION, PATCH, BOARD);

	while (1)
	{
		switch(currentState){
			          case ST_SETUP:
			              // do setup tasks
			              if (currentState != prevState)
			              {
			                  printf("\n## Entering SETUP state\n\n");
			              }
			              prevState = currentState;

			              readJsonConfig();
			              setup();
			              deserialiseJSON();
			              configThreads();
			              createThreads();
			              //debugThreadHigh();
			              //loadModules();
			              //debugThreadLow();

			              currentState = ST_START;
			              break;

			          case ST_START:
			              // do start tasks
			              if (currentState != prevState)
			              {
			                  printf("\n## Entering START state\n");
			              }
			              prevState = currentState;

			              if (!threadsRunning)
			              {
			                  // Start the threads
			                  printf("\nStarting the BASE thread\n");
			                  baseThread->startThread();

			                  printf("\nStarting the SERVO thread\n");
			                  servoThread->startThread();

			                  threadsRunning = true;
			              }

			              currentState = ST_IDLE;

			              break;


			          case ST_IDLE:
			              // do something when idle
			              if (currentState != prevState)
			              {
			                  printf("\n## Entering IDLE state\n");
			              }
			              prevState = currentState;

			              //wait for data before changing to running state
			              if (comms->getStatus())
			              {
			                  currentState = ST_RUNNING;
			              }

			              break;

			          case ST_RUNNING:
			              // do running tasks
			              if (currentState != prevState)
			              {
			                  printf("\n## Entering RUNNING state\n");
			              }
			              prevState = currentState;


			              // check to see if there there has been SPI errors
			              if (comms->getError())
			              {
			                  printf("Communication data error\n");
			                  comms->setError(false);
			              }

			              if (comms->getStatus())
			              {
			                  // SPI data received by DMA
			                  resetCnt = 0;
			                  comms->setStatus(false);
			              }
			              else
			              {
			                  // no data received by DMA
			                  resetCnt++;
			              }

			              if (resetCnt > SPI_ERR_MAX)
			              {
			                  // reset threshold reached, reset the PRU
			                  printf("   Communication data error limit reached, resetting\n");
			                  resetCnt = 0;
			                  currentState = ST_RESET;
			              }

			              if (PRUreset)
			              {
			                  currentState = ST_WDRESET;
			              }

			              break;

			          case ST_STOP:
			              // do stop tasks
			              if (currentState != prevState)
			              {
			                  printf("\n## Entering STOP state\n");
			              }
			              prevState = currentState;


			              currentState = ST_STOP;
			              break;

			          case ST_RESET:
			              // do reset tasks
			              if (currentState != prevState)
			              {
			                  printf("\n## Entering RESET state\n");
			              }
			              prevState = currentState;

			              // set all of the rxData buffer to 0
			              // rxData.rxBuffer is volatile so need to do this the long way. memset cannot be used for volatile

			              pruRxData = getCurrentRxBuffer(&rxPingPongBuffer);

			              printf("   Resetting rxBuffer\n");
			              {
			            	  SCB_InvalidateDCache_by_Addr((uint32_t*)(((uint32_t)pruRxData->rxBuffer) & ~(uint32_t)0x1F), BUFFER_ALIGNED_SIZE);
			                  int n = sizeof(pruRxData->rxBuffer);
			                  while(n-- > 0)
			                  {
			                      pruRxData->rxBuffer[n] = 0;
			                  }
			              }

			              currentState = ST_IDLE;
			              break;

			          case ST_WDRESET:
			        	  // force a reset
			        	  HAL_NVIC_SystemReset();
			              break;
			  }

	}
}



void initRxPingPongBuffer(RxPingPongBuffer* buffer) {
    buffer->currentRxBuffer = 0;
}

void initTxPingPongBuffer(TxPingPongBuffer* buffer) {
    buffer->currentTxBuffer = 0;
}

void swapRxBuffers(RxPingPongBuffer* buffer) {
    buffer->currentRxBuffer = 1 - buffer->currentRxBuffer;
}

void swapTxBuffers(TxPingPongBuffer* buffer) {
    buffer->currentTxBuffer = 1 - buffer->currentTxBuffer;
}

rxData_t* getCurrentRxBuffer(RxPingPongBuffer* buffer) {
    return &buffer->rxBuffers[buffer->currentRxBuffer];
}

txData_t* getCurrentTxBuffer(TxPingPongBuffer* buffer) {
    return &buffer->txBuffers[buffer->currentTxBuffer];
}

rxData_t* getAltRxBuffer(RxPingPongBuffer* buffer) {
    return &buffer->rxBuffers[1 - buffer->currentRxBuffer];
}

txData_t* getAltTxBuffer(TxPingPongBuffer* buffer) {
    return &buffer->txBuffers[1 - buffer->currentTxBuffer];
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC|RCC_PERIPHCLK_SPI1;
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 12;
  PeriphClkInitStruct.PLL2.PLL2P = 1;
  PeriphClkInitStruct.PLL2.PLL2Q = 10;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL2;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_ENABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 8;
  /* USER CODE BEGIN SDMMC1_Init 2 */
  if (HAL_SD_Init(&hsd1) != HAL_OK)
  {
      printf("SD_Init error\n");
  }
  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = PC_BAUD;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin : PC4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

	printf("error\n\r");

  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
