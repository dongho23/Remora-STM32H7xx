#include "../remoraComms/RemoraComms.h"
#include <cstdio>


RemoraComms::RemoraComms(volatile rxData_t* ptrRxData, volatile txData_t* ptrTxData, SPI_TypeDef* spiType) :
    ptrRxData(ptrRxData),
    ptrTxData(ptrTxData),
    spiType(spiType)
{
    this->spiHandle.Instance = this->spiType;

    this->irqNss = EXTI4_IRQn;
    this->irqDMAtx = DMA1_Stream0_IRQn;
    this->irqDMArx = DMA1_Stream1_IRQn;

    // the constructor is called before any DMA and cache setup
    // don't do stuff here

    this->pin1 = new Pin("PE_11", OUTPUT);
    this->pin2 = new Pin("PE_12", OUTPUT);
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

        //NssInterrupt = new ModuleInterrupt(this->irqNss, this, static_cast<void (Module::*)()>(&RemoraComms::handleNssInterrupt));
    	//HAL_NVIC_SetPriority(this->irqNss, 6, 0);
        //HAL_NVIC_EnableIRQ(this->irqNss);


        printf("	Initialising SPI1 slave\n");

        this->spiHandle.Init.Mode           		= SPI_MODE_SLAVE;
        this->spiHandle.Init.Direction      		= SPI_DIRECTION_2LINES;
        this->spiHandle.Init.DataSize       		= SPI_DATASIZE_8BIT;
        this->spiHandle.Init.CLKPolarity    		= SPI_POLARITY_LOW;
        this->spiHandle.Init.CLKPhase       		= SPI_PHASE_1EDGE;
        //this->spiHandle.Init.NSS            		= SPI_NSS_HARD_INPUT;
        this->spiHandle.Init.NSS            		= SPI_NSS_SOFT;
        this->spiHandle.Init.FirstBit       		= SPI_FIRSTBIT_MSB;
        this->spiHandle.Init.TIMode         		= SPI_TIMODE_DISABLE;
        this->spiHandle.Init.CRCCalculation 		= SPI_CRCCALCULATION_DISABLE;
        this->spiHandle.Init.CRCPolynomial  		= 0x0;
        this->spiHandle.Init.NSSPMode 				= SPI_NSS_PULSE_DISABLE;
        //this->spiHandle.Init.NSSPMode 				= SPI_NSS_PULSE_ENABLE;
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

		printf("	Initialising GPIO for SPI\n");

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

        printf("	Initialising DMA for SPI\n");

        this->hdma_spi_rx.Instance 					= DMA1_Stream1;
        this->hdma_spi_rx.Init.Request 				= DMA_REQUEST_SPI1_RX;
        this->hdma_spi_rx.Init.Direction 			= DMA_PERIPH_TO_MEMORY;
        this->hdma_spi_rx.Init.PeriphInc 			= DMA_PINC_DISABLE;
        this->hdma_spi_rx.Init.MemInc 				= DMA_MINC_ENABLE;
        this->hdma_spi_rx.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE;
        this->hdma_spi_rx.Init.MemDataAlignment 	= DMA_MDATAALIGN_BYTE;
        this->hdma_spi_rx.Init.Mode 				= DMA_CIRCULAR;
        //this->hdma_spi_rx.Init.Mode 				= DMA_NORMAL;
        this->hdma_spi_rx.Init.Priority 			= DMA_PRIORITY_LOW;
        this->hdma_spi_rx.Init.FIFOMode 			= DMA_FIFOMODE_DISABLE;

        HAL_DMA_Init(&this->hdma_spi_rx);
        __HAL_LINKDMA(&this->spiHandle, hdmarx, this->hdma_spi_rx);

        //dmaRxInterrupt = new ModuleInterrupt(this->irqDMArx, this, static_cast<void (Module::*)()>(&RemoraComms::handleRxInterrupt));
    	//HAL_NVIC_SetPriority(this->irqDMArx, 5, 0);
        //HAL_NVIC_EnableIRQ(this->irqDMArx);

        this->hdma_spi_tx.Instance 					= DMA1_Stream0;
        this->hdma_spi_tx.Init.Request 				= DMA_REQUEST_SPI1_TX;
        this->hdma_spi_tx.Init.Direction 			= DMA_MEMORY_TO_PERIPH;
        this->hdma_spi_tx.Init.PeriphInc 			= DMA_PINC_DISABLE;
        this->hdma_spi_tx.Init.MemInc 				= DMA_MINC_ENABLE;
        this->hdma_spi_tx.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE;
        this->hdma_spi_tx.Init.MemDataAlignment 	= DMA_MDATAALIGN_BYTE;
        this->hdma_spi_tx.Init.Mode 				= DMA_CIRCULAR;
        //this->hdma_spi_tx.Init.Mode 				= DMA_NORMAL;
        this->hdma_spi_tx.Init.Priority 			= DMA_PRIORITY_LOW;
        this->hdma_spi_tx.Init.FIFOMode 			= DMA_FIFOMODE_DISABLE;

        HAL_DMA_Init(&this->hdma_spi_tx);
        __HAL_LINKDMA(&this->spiHandle, hdmatx, this->hdma_spi_tx);

        //dmaTxInterrupt = new ModuleInterrupt(this->irqDMAtx, this, static_cast<void (Module::*)()>(&RemoraComms::handleTxInterrupt));
        //HAL_NVIC_SetPriority(this->irqDMAtx, 4, 0);	// TX needs to be a higher priority than RX
        //HAL_NVIC_EnableIRQ(this->irqDMAtx);
    }
}

void RemoraComms::start()
{
    NssInterrupt = new ModuleInterrupt(this->irqNss, this, static_cast<void (Module::*)()>(&RemoraComms::handleNssInterrupt));
	HAL_NVIC_SetPriority(this->irqNss, 6, 0);
    HAL_NVIC_EnableIRQ(this->irqNss);

    dmaRxInterrupt = new ModuleInterrupt(this->irqDMArx, this, static_cast<void (Module::*)()>(&RemoraComms::handleRxInterrupt));
	HAL_NVIC_SetPriority(this->irqDMArx, 5, 0);
    HAL_NVIC_EnableIRQ(this->irqDMArx);

    dmaTxInterrupt = new ModuleInterrupt(this->irqDMAtx, this, static_cast<void (Module::*)()>(&RemoraComms::handleTxInterrupt));
    HAL_NVIC_SetPriority(this->irqDMAtx, 4, 0);	// TX needs to be a higher priority than RX
    HAL_NVIC_EnableIRQ(this->irqDMAtx);

	txPingPongBuffer.txBuffers[0].header =  PRU_DATA;
	txPingPongBuffer.txBuffers[1].header =  PRU_DATA;

	txPingPongBuffer.currentTxBuffer = 0;
	rxPingPongBuffer.currentRxBuffer = 0;

	//SCB_CleanDCache_by_Addr((uint32_t*)(((uint32_t)&txPingPongBuffer.txBuffers[0]) & ~(uint32_t)0x1F), BUFFER_ALIGNED_SIZE);
	//SCB_CleanDCache_by_Addr((uint32_t*)(((uint32_t)&txPingPongBuffer.txBuffers[1]) & ~(uint32_t)0x1F), BUFFER_ALIGNED_SIZE);

	//printf("txBuffers[0].header = 0x%x\n", txPingPongBuffer.txBuffers[0].header);
	//printf("txBuffers[1].header = 0x%x\n", txPingPongBuffer.txBuffers[1].header);


    // Prepare the header in each Tx buffer
    // Current buffer: used by Remora modules
	//txBuffer = getCurrentTxBuffer(&txPingPongBuffer);
    //txBuffer->header = PRU_DATA;
    //SCB_CleanDCache_by_Addr((uint32_t*)(((uint32_t)txBuffer->txBuffer) & ~(uint32_t)0x1F), BUFFER_ALIGNED_SIZE);

    // Alternate buffer: used by DMA comms
    //txBuffer = getAltTxBuffer(&txPingPongBuffer);
    //txBuffer->header = PRU_DATA;
    //SCB_CleanDCache_by_Addr((uint32_t*)(((uint32_t)txBuffer->txBuffer) & ~(uint32_t)0x1F), BUFFER_ALIGNED_SIZE);

	txBuffer = getAltTxBuffer(&txPingPongBuffer);
	rxBuffer = getAltRxBuffer(&rxPingPongBuffer);

    //SCB_InvalidateDCache_by_Addr((uint32_t*)(((uint32_t)rxBuffer->rxBuffer) & ~(uint32_t)0x1F), BUFFER_ALIGNED_SIZE);
    this->spiHandle.Lock = HAL_UNLOCKED;
    this->dmaStatus = HAL_SPI_TransmitReceive_DMA(&this->spiHandle, (uint8_t *)txBuffer->txBuffer, (uint8_t *)rxBuffer->rxBuffer, SPI_BUFF_SIZE);

    printf("%d\n", this->dmaStatus);
}


void RemoraComms::swapBuffers()
{
    __disable_irq();
    swapTxBuffers(&txPingPongBuffer);
    __enable_irq();
    rxBuffer = getAltRxBuffer(&rxPingPongBuffer);
    txBuffer = getAltTxBuffer(&txPingPongBuffer);
    //SCB_InvalidateDCache_by_Addr((uint32_t*)(((uint32_t)rxBuffer->rxBuffer) & ~(uint32_t)0x1F), BUFFER_ALIGNED_SIZE);
    //SCB_CleanDCache_by_Addr((uint32_t*)(((uint32_t)txBuffer->txBuffer) & ~(uint32_t)0x1F), BUFFER_ALIGNED_SIZE);
}

void RemoraComms::handleNssInterrupt()
{
	this->pin2->set(1);
    // Stop the previous DMA transfer
    //HAL_DMA_Abort(this->spiHandle.hdmarx);
    //HAL_DMA_Abort(this->spiHandle.hdmatx);

	//HAL_NVIC_EnableIRQ(this->irqDMAtx);
	//HAL_NVIC_EnableIRQ(this->irqDMArx);

	if (this->dmaRx != true && this->dmaTx != true)
	{
		// DMA should have been handled by now
		this->dmaRx = false;
		this->dmaTx = false;
		//printf("err\n");
		//return;
	}

	this->dmaRx = false;
	this->dmaTx = false;

    // Get alternative buffers
    //rxBuffer = getAltRxBuffer(&rxPingPongBuffer);
    //txBuffer = getAltTxBuffer(&txPingPongBuffer);

    // Define aligned buffer addresses for DCache operations
    uint32_t* alignedRxBuffer = (uint32_t*)(((uint32_t)rxBuffer->rxBuffer) & ~(uint32_t)0x1F);
    uint32_t* alignedTxBuffer = (uint32_t*)(((uint32_t)txBuffer->txBuffer) & ~(uint32_t)0x1F);

    // Invalidate the DCache for the received buffer
    //SCB_InvalidateDCache_by_Addr(alignedRxBuffer, BUFFER_ALIGNED_SIZE);

    // Handle different PRU header values
    switch (rxBuffer->header)
    {
        case PRU_READ:
            // Data is good, no buffer swap needed
        	//printf("r\n");
            this->SPIdata = true;
            break;

        case PRU_WRITE:

            // Disable and abort existing SPI and DMA
            //__HAL_SPI_DISABLE(&this->spiHandle);
            HAL_DMA_Abort(&this->hdma_spi_rx);
            HAL_DMA_Abort(&this->hdma_spi_tx);

            // Perform a reset
            //__HAL_RCC_SPI1_FORCE_RESET();
            //__HAL_RCC_SPI1_RELEASE_RESET();

            // Reinitialize SPI and DMA
            if (HAL_SPI_Init(&this->spiHandle) != HAL_OK) {
                printf("SPI reinitialization failed!\n");
                return;
            }
            HAL_DMA_Init(&this->hdma_spi_rx);
            HAL_DMA_Init(&this->hdma_spi_tx);

            // Swap and prepare buffers
            this->swapBuffers();

            rxBuffer = getAltRxBuffer(&rxPingPongBuffer);
            txBuffer = getAltTxBuffer(&txPingPongBuffer);

            // Restart SPI DMA communication
            if (HAL_SPI_TransmitReceive_DMA(&this->spiHandle, (uint8_t *)txBuffer->txBuffer, (uint8_t *)rxBuffer->rxBuffer, SPI_BUFF_SIZE) != HAL_OK) {
                printf("SPI DMA restart failed!\n");
            }

            break;


        default:
            // Invalid data, increment reject count
        	//printf("e\n");
            this->rejectCnt++;
            if (this->rejectCnt > 5)
            {
                this->SPIdataError = true;
            }
            break;
    }

    // If needed, clean the TX buffer's DCache and swap it
    if (rxBuffer->header != PRU_READ)
    {
        //SCB_CleanDCache_by_Addr(alignedTxBuffer, BUFFER_ALIGNED_SIZE);
        this->swapBuffers();
    }

    // SPI state does not get reset correctly, force it
    //this->spiHandle.State = HAL_SPI_STATE_READY;

    //txBuffer->header = 0;

    // Start SPI communication (DMA transfer)
    //this->dmaStatus = HAL_SPI_TransmitReceive_DMA(&this->spiHandle, (uint8_t *)txBuffer->txBuffer, (uint8_t *)rxBuffer->rxBuffer, SPI_BUFF_SIZE);

    //printf("%d\n", this->dmaStatus);
    this->pin2->set(0);
}


void RemoraComms::handleTxInterrupt()
{
	// Handle the DMA interrupt
	HAL_DMA_IRQHandler(&this->hdma_spi_tx);
	HAL_NVIC_EnableIRQ(this->irqDMAtx);

    this->dmaTx = true;
}


void RemoraComms::handleRxInterrupt()
{
	this->pin1->set(1);
	// Handle the DMA interrupt
	HAL_DMA_IRQHandler(&this->hdma_spi_rx);
    HAL_NVIC_EnableIRQ(this->irqDMArx);

    this->dmaRx = true;
    this->pin1->set(0);
}


bool RemoraComms::getStatus(void)
{
    return this->SPIdata;
}

void RemoraComms::setStatus(bool status)
{
    this->SPIdata = status;
}

bool RemoraComms::getError(void)
{
    return this->SPIdataError;
}

void RemoraComms::setError(bool error)
{
    this->SPIdataError = error;
}

void RemoraComms::update()
{
	// unused for RemoraComms
}
