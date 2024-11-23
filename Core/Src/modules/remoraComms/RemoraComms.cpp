#include "../remoraComms/RemoraComms.h"
#include <cstdio>

RemoraComms::RemoraComms(volatile rxData_t* ptrRxData, volatile txData_t* ptrTxData, SPI_TypeDef* spiType) :
    ptrRxData(ptrRxData),
    ptrTxData(ptrTxData),
    spiType(spiType)
{
    this->spiHandle.Instance = this->spiType;
    this->irq = EXTI4_IRQn;

    // the constructor is called before any DMA and cache setup
    // don't do stuff here
}


void RemoraComms::init()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

    if(this->spiHandle.Instance == SPI1)
    {
    	// Interrupt pin is the NSS pin
        // Configure GPIO pin : PA_4

        __HAL_RCC_GPIOC_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        interruptPtr = new ModuleInterrupt(this->irq, this);
    	HAL_NVIC_SetPriority(this->irq, 5, 0);
        HAL_NVIC_EnableIRQ(this->irq);


        printf("Initialising SPI1 slave\n");

        this->spiHandle.Init.Mode           		= SPI_MODE_SLAVE;
        this->spiHandle.Init.Direction      		= SPI_DIRECTION_2LINES;
        this->spiHandle.Init.DataSize       		= SPI_DATASIZE_8BIT;
        this->spiHandle.Init.CLKPolarity    		= SPI_POLARITY_LOW;
        this->spiHandle.Init.CLKPhase       		= SPI_PHASE_1EDGE;
        this->spiHandle.Init.NSS            		= SPI_NSS_HARD_INPUT;
        this->spiHandle.Init.FirstBit       		= SPI_FIRSTBIT_MSB;
        this->spiHandle.Init.TIMode         		= SPI_TIMODE_DISABLE;
        this->spiHandle.Init.CRCCalculation 		= SPI_CRCCALCULATION_DISABLE;
        this->spiHandle.Init.CRCPolynomial  		= 0x0;
        this->spiHandle.Init.NSSPMode 				= SPI_NSS_PULSE_DISABLE;
        this->spiHandle.Init.NSSPolarity 			= SPI_NSS_POLARITY_LOW;
        this->spiHandle.Init.FifoThreshold 			= SPI_FIFO_THRESHOLD_01DATA;
        this->spiHandle.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
        this->spiHandle.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
        this->spiHandle.Init.MasterSSIdleness 		= SPI_MASTER_SS_IDLENESS_00CYCLE;
        this->spiHandle.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
        this->spiHandle.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
        this->spiHandle.Init.MasterKeepIOState 		= SPI_MASTER_KEEP_IO_STATE_DISABLE;
        this->spiHandle.Init.IOSwap 				= SPI_IO_SWAP_DISABLE;

        HAL_SPI_Init(&this->spiHandle);

    	// Peripheral clock enable
    	__HAL_RCC_SPI1_CLK_ENABLE();

		printf("Initialising GPIO for SPI\n");

	    __HAL_RCC_GPIOA_CLK_ENABLE();
	    /**SPI1 GPIO Configuration
	    PA4     ------> SPI1_NSS
	    PA5     ------> SPI1_SCK
	    PA6     ------> SPI1_MISO
	    PA7     ------> SPI1_MOSI
	    */
    	GPIO_InitStruct = {0};
	    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        printf("Initialising DMA for SPI\n");

        this->hdma_spi_rx.Instance 					= DMA1_Stream1;
        this->hdma_spi_rx.Init.Request 				= DMA_REQUEST_SPI1_RX;
        this->hdma_spi_rx.Init.Direction 			= DMA_PERIPH_TO_MEMORY;
        this->hdma_spi_rx.Init.PeriphInc 			= DMA_PINC_DISABLE;
        this->hdma_spi_rx.Init.MemInc 				= DMA_MINC_ENABLE;
        this->hdma_spi_rx.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE;
        this->hdma_spi_rx.Init.MemDataAlignment 	= DMA_MDATAALIGN_BYTE;
        this->hdma_spi_rx.Init.Mode 				= DMA_CIRCULAR;
        this->hdma_spi_rx.Init.Priority 			= DMA_PRIORITY_LOW;
        this->hdma_spi_rx.Init.FIFOMode 			= DMA_FIFOMODE_DISABLE;

        HAL_DMA_Init(&this->hdma_spi_rx);
        __HAL_LINKDMA(&this->spiHandle, hdmarx, this->hdma_spi_rx);

        this->hdma_spi_tx.Instance 					= DMA1_Stream0;
        this->hdma_spi_tx.Init.Request 				= DMA_REQUEST_SPI1_TX;
        this->hdma_spi_tx.Init.Direction 			= DMA_MEMORY_TO_PERIPH;
        this->hdma_spi_tx.Init.PeriphInc 			= DMA_PINC_DISABLE;
        this->hdma_spi_tx.Init.MemInc 				= DMA_MINC_ENABLE;
        this->hdma_spi_tx.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE;
        this->hdma_spi_tx.Init.MemDataAlignment 	= DMA_MDATAALIGN_BYTE;
        this->hdma_spi_tx.Init.Mode 				= DMA_CIRCULAR;
        this->hdma_spi_tx.Init.Priority 			= DMA_PRIORITY_LOW;
        this->hdma_spi_tx.Init.FIFOMode 			= DMA_FIFOMODE_DISABLE;

        HAL_DMA_Init(&this->hdma_spi_tx);
        __HAL_LINKDMA(&this->spiHandle, hdmatx, this->hdma_spi_tx);
    }

	// Interrupt pin is the NSS pin
    // Configure GPIO pin : PA_4
/*
    GPIO_InitStruct = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    interruptPtr = new ModuleInterrupt(this->irq, this);
	HAL_NVIC_SetPriority(this->irq, 5, 0);
    HAL_NVIC_EnableIRQ(this->irq);
*/
}

void RemoraComms::start()
{
    this->ptrTxData->header = PRU_DATA;
    SCB_CleanDCache_by_Addr((uint32_t*)(((uint32_t)this->ptrTxData->txBuffer) & ~(uint32_t)0x1F), BUFFER_ALIGNED_SIZE);
    SCB_InvalidateDCache_by_Addr((uint32_t*)(((uint32_t)this->spiRxBuffer.rxBuffer) & ~(uint32_t)0x1F), BUFFER_ALIGNED_SIZE);
    HAL_SPI_TransmitReceive_DMA(&this->spiHandle, (uint8_t *)this->ptrTxData->txBuffer, (uint8_t *)this->spiRxBuffer.rxBuffer, SPI_BUFF_SIZE);
}


void RemoraComms::handleInterrupt()
{
	//printf("RemoraComms interrupt\n");

    SCB_CleanDCache_by_Addr((uint32_t*)(((uint32_t)this->ptrTxData->txBuffer) & ~(uint32_t)0x1F), BUFFER_ALIGNED_SIZE);
	SCB_InvalidateDCache_by_Addr((uint32_t*)(((uint32_t)this->spiRxBuffer.rxBuffer) & ~(uint32_t)0x1F), BUFFER_ALIGNED_SIZE);

	switch (this->spiRxBuffer.header)
	{
	  case PRU_READ:
		this->SPIdata = true;
		this->rejectCnt = 0;
		// READ so do nothing with the received data
		break;

	  case PRU_WRITE:
		this->SPIdata = true;
		this->rejectCnt = 0;

		// ensure an atomic access to the rxBuffer
		// disable thread interrupts
		__disable_irq();
		for (int i = 0; i < SPI_BUFF_SIZE; i++)
		{
			this->ptrRxData->rxBuffer[i] = this->spiRxBuffer.rxBuffer[i];
		}
		// re-enable thread interrupts
		__enable_irq();
		break;

	  default:
		this->rejectCnt++;
		if (this->rejectCnt > 5)
		{
			this->SPIdataError = true;
		}
		// reset SPI somehow
	}

    HAL_SPI_TransmitReceive_DMA(&this->spiHandle, (uint8_t *)this->ptrTxData->txBuffer, (uint8_t *)this->spiRxBuffer.rxBuffer, SPI_BUFF_SIZE);
}


void RemoraComms::update()
{
	// unused for RemoraComms
}
