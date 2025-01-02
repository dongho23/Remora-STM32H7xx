/**
 * @file RemoraComms.cpp
 * @brief Implementation of the RemoraComms class for SPI communication with DMA.
 */

#include "../remoraComms/RemoraComms.h"
#include <cstdio>


/**
 * @class RemoraComms
 * @brief Handles SPI communication with DMA using the STM32 HAL library.
 *
 * This class configures and manages SPI communication in slave mode,
 * including GPIO initialization, DMA setup, and interrupt management.
 */

/**
 * @brief Constructs a new RemoraComms object.
 * @param spiType Pointer to the SPI instance.
 */
RemoraComms::RemoraComms(volatile rxData_t* ptrRxData, volatile txData_t* ptrTxData, volatile DMA_RxBuffer_t* ptrRxDMABuffer, SPI_TypeDef* spiType) :
	ptrRxData(ptrRxData),
	ptrTxData(ptrTxData),
	ptrRxDMABuffer(ptrRxDMABuffer),
	spiType(spiType)
{
    this->spiHandle.Instance = this->spiType;

    this->irqNss = 		EXTI4_IRQn;
    this->irqDMAtx = 	DMA1_Stream0_IRQn;
    this->irqDMArx = 	DMA1_Stream1_IRQn;

    // Note: Avoid performing complex initialization here as this constructor is called before DMA and cache setup.
}


/**
 * @brief Initializes the SPI, GPIO, and DMA configurations.
 *
 * Configures SPI as a slave device with associated GPIO and DMA settings.
 */
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

        printf("	Initialising SPI1 slave\n");

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

        printf("	Initialising DMA for Memory to Memory transfer\n");

        this->hdma_memtomem.Instance 				= DMA1_Stream2;
        this->hdma_memtomem.Init.Request 			= DMA_REQUEST_MEM2MEM;
        this->hdma_memtomem.Init.Direction 			= DMA_MEMORY_TO_MEMORY;
        this->hdma_memtomem.Init.PeriphInc 			= DMA_PINC_ENABLE;
        this->hdma_memtomem.Init.MemInc 			= DMA_MINC_ENABLE;
        this->hdma_memtomem.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        this->hdma_memtomem.Init.MemDataAlignment 	= DMA_MDATAALIGN_BYTE;
        this->hdma_memtomem.Init.Mode 				= DMA_NORMAL;
        this->hdma_memtomem.Init.Priority 			= DMA_PRIORITY_LOW;
        this->hdma_memtomem.Init.FIFOMode 			= DMA_FIFOMODE_ENABLE;
        this->hdma_memtomem.Init.FIFOThreshold 		= DMA_FIFO_THRESHOLD_FULL;
        this->hdma_memtomem.Init.MemBurst 			= DMA_MBURST_SINGLE;
        this->hdma_memtomem.Init.PeriphBurst 		= DMA_PBURST_SINGLE;

        HAL_DMA_Init(&this->hdma_memtomem);
    }
}


/**
 * @brief Starts the RemoraComms module, initializing and enabling necessary interrupts and buffers.
 *
 * This function sets up the SPI and DMA communication by registering interrupt handlers for
 * NSS (slave select), DMA Rx, and DMA Tx. It also initializes the data buffers and starts
 * the multi-buffer DMA SPI communication.
 */
void RemoraComms::start()
{
    // Register the NSS (slave select) interrupt
    NssInterrupt = new ModuleInterrupt(
        this->irqNss,
        this,
        static_cast<void (Module::*)()>(&RemoraComms::handleNssInterrupt)
    );
    HAL_NVIC_SetPriority(this->irqNss, SPI_NSS_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(this->irqNss);

    // Register the DMA Rx interrupt
    dmaRxInterrupt = new ModuleInterrupt(
        this->irqDMArx,
        this,
        static_cast<void (Module::*)()>(&RemoraComms::handleRxInterrupt)
    );
    HAL_NVIC_SetPriority(this->irqDMArx, SPI_DMA_RX_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(this->irqDMArx);

    // Register the DMA Tx interrupt
    dmaTxInterrupt = new ModuleInterrupt(
        this->irqDMAtx,
        this,
        static_cast<void (Module::*)()>(&RemoraComms::handleTxInterrupt)
    );
    HAL_NVIC_SetPriority(this->irqDMAtx, SPI_DMA_TX_IRQ_PRIORITY, 0); // TX needs higher priority than RX
    HAL_NVIC_EnableIRQ(this->irqDMAtx);

    // Initialize the data buffers
    memset((void*)this->ptrTxData->txBuffer, 0, sizeof(this->ptrTxData->txBuffer));
    memset((void*)this->ptrRxData->rxBuffer, 0, sizeof(this->ptrRxData->rxBuffer));
    memset((void*)this->ptrRxDMABuffer->buffer[0].rxBuffer, 0, sizeof(this->ptrRxDMABuffer->buffer[0].rxBuffer));
    memset((void*)this->ptrRxDMABuffer->buffer[1].rxBuffer, 0, sizeof(this->ptrRxDMABuffer->buffer[1].rxBuffer));
    this->ptrTxData->header = PRU_DATA;

    // Start the multi-buffer DMA SPI communication
    this->dmaStatus = this->startMultiBufferDMASPI(
        (uint8_t*)this->ptrTxData->txBuffer,
        (uint8_t*)this->ptrTxData->txBuffer,
        (uint8_t*)this->ptrRxDMABuffer->buffer[0].rxBuffer,
        (uint8_t*)this->ptrRxDMABuffer->buffer[1].rxBuffer,
        SPI_BUFF_SIZE
    );

    // Check for DMA initialization errors
    if (this->dmaStatus != HAL_OK)
    {
        printf("DMA SPI error\n");
    }
}


/**
* @brief  Starts multi-buffer DMA SPI communication in full-duplex mode.
*
* This function sets up the SPI peripheral and DMA to perform data transfer
* using two transmit and two receive buffers, enabling efficient data handling
* for continuous or circular data streams.
*
* @param[in] pTxBuffer0 Pointer to the primary transmit buffer.
* @param[in] pTxBuffer1 Pointer to the secondary transmit buffer. If NULL, the primary buffer is reused.
* @param[in] pRxBuffer0 Pointer to the primary receive buffer.
* @param[in] pRxBuffer1 Pointer to the secondary receive buffer. If NULL, the primary buffer is reused.
* @param[in] Size       Number of data elements to transfer.
*
* @return HAL_StatusTypeDef
*         - HAL_OK: Operation completed successfully.
*         - HAL_ERROR: Invalid parameters or DMA initialization error.
*         - HAL_BUSY: SPI is not in a ready state.
*
* @note  The SPI peripheral must be configured in 2-line full-duplex mode.
* @note  The function locks the SPI handle during execution to prevent concurrent access.
* @note  Circular mode is supported if the DMA is configured appropriately.
*/
HAL_StatusTypeDef RemoraComms::startMultiBufferDMASPI(uint8_t *pTxBuffer0, uint8_t *pTxBuffer1,
                                                      uint8_t *pRxBuffer0, uint8_t *pRxBuffer1,
                                                      uint16_t Size)
{
    /* Check Direction parameter */
    assert_param(IS_SPI_DIRECTION_2LINES(this->spiHandle.Init.Direction));

    if (this->spiHandle.State != HAL_SPI_STATE_READY)
    {
        return HAL_BUSY;
    }

    if ((pTxBuffer0 == NULL) || (pRxBuffer0 == NULL) || (Size == 0UL))
    {
        return HAL_ERROR;
    }

    /* If secondary Tx or Rx buffer is not provided, use the primary buffer */
    if (pTxBuffer1 == NULL)
    {
        pTxBuffer1 = pTxBuffer0;
    }

    if (pRxBuffer1 == NULL)
    {
        pRxBuffer1 = pRxBuffer0;
    }

    /* Lock the process */
    __HAL_LOCK(&this->spiHandle);

    /* Set the transaction information */
    this->spiHandle.State       = HAL_SPI_STATE_BUSY_TX_RX;
    this->spiHandle.ErrorCode   = HAL_SPI_ERROR_NONE;
    this->spiHandle.TxXferSize  = Size;
    this->spiHandle.TxXferCount = Size;
    this->spiHandle.RxXferSize  = Size;
    this->spiHandle.RxXferCount = Size;

    /* Init unused fields in handle to zero */
    this->spiHandle.RxISR       = NULL;
    this->spiHandle.TxISR       = NULL;

    /* Set Full-Duplex mode */
    SPI_2LINES(&this->spiHandle);

    /* Reset the Tx/Rx DMA bits */
    CLEAR_BIT(this->spiHandle.Instance->CFG1, SPI_CFG1_TXDMAEN | SPI_CFG1_RXDMAEN);

    /* Adjust XferCount according to DMA alignment / Data size */
    if (this->spiHandle.Init.DataSize <= SPI_DATASIZE_8BIT)
    {
        if (this->hdma_spi_tx.Init.MemDataAlignment == DMA_MDATAALIGN_HALFWORD)
        {
            this->spiHandle.TxXferCount = (this->spiHandle.TxXferCount + 1UL) >> 1UL;
        }
        if (this->hdma_spi_rx.Init.MemDataAlignment == DMA_MDATAALIGN_HALFWORD)
        {
            this->spiHandle.RxXferCount = (this->spiHandle.RxXferCount + 1UL) >> 1UL;
        }
    }
    else if (this->spiHandle.Init.DataSize <= SPI_DATASIZE_16BIT)
    {
        if (this->hdma_spi_tx.Init.MemDataAlignment == DMA_MDATAALIGN_WORD)
        {
            this->spiHandle.TxXferCount = (this->spiHandle.TxXferCount + 1UL) >> 1UL;
        }
        if (this->hdma_spi_rx.Init.MemDataAlignment == DMA_MDATAALIGN_WORD)
        {
            this->spiHandle.RxXferCount = (this->spiHandle.RxXferCount + 1UL) >> 1UL;
        }
    }

    /* Configure Tx DMA with Multi-Buffer */
    this->hdma_spi_tx.XferHalfCpltCallback = NULL;
    this->hdma_spi_tx.XferCpltCallback     = NULL;
    this->hdma_spi_tx.XferErrorCallback    = NULL;

    if (HAL_OK != HAL_DMAEx_MultiBufferStart_IT(&this->hdma_spi_tx,
                                                (uint32_t)pTxBuffer0,
                                                (uint32_t)&this->spiHandle.Instance->TXDR,
                                                (uint32_t)pTxBuffer1,
                                                this->spiHandle.TxXferCount))
    {
        __HAL_UNLOCK(&this->spiHandle);
        return HAL_ERROR;
    }

    /* Configure Rx DMA with Multi-Buffer */
    this->hdma_spi_rx.XferHalfCpltCallback = NULL;
    this->hdma_spi_rx.XferCpltCallback     = NULL;
    this->hdma_spi_rx.XferErrorCallback    = NULL;

    if (HAL_OK != HAL_DMAEx_MultiBufferStart_IT(&this->hdma_spi_rx,
                                                (uint32_t)&this->spiHandle.Instance->RXDR,
                                                (uint32_t)pRxBuffer0,
                                                (uint32_t)pRxBuffer1,
                                                this->spiHandle.RxXferCount))
    {
        (void)HAL_DMA_Abort(&this->hdma_spi_tx);
        __HAL_UNLOCK(&this->spiHandle);
        return HAL_ERROR;
    }

    /* Configure SPI TSIZE for full transfer or circular mode */
    if (this->hdma_spi_rx.Init.Mode == DMA_CIRCULAR || this->hdma_spi_tx.Init.Mode == DMA_CIRCULAR)
    {
        MODIFY_REG(this->spiHandle.Instance->CR2, SPI_CR2_TSIZE, 0UL);
    }
    else
    {
        MODIFY_REG(this->spiHandle.Instance->CR2, SPI_CR2_TSIZE, Size);
    }

    /* Enable Tx and Rx DMA Requests */
    SET_BIT(this->spiHandle.Instance->CFG1, SPI_CFG1_TXDMAEN | SPI_CFG1_RXDMAEN);

    /* Enable SPI error interrupt */
    __HAL_SPI_ENABLE_IT(&this->spiHandle, (SPI_IT_OVR | SPI_IT_UDR | SPI_IT_FRE | SPI_IT_MODF));

    /* Enable SPI peripheral */
    __HAL_SPI_ENABLE(&this->spiHandle);

    if (this->spiHandle.Init.Mode == SPI_MODE_MASTER)
    {
        SET_BIT(this->spiHandle.Instance->CR1, SPI_CR1_CSTART);
    }

    __HAL_UNLOCK(&this->spiHandle);

    return HAL_OK;
}



/**
  * @brief  Handles DMA interrupt request.
  *
  * This function processes DMA interrupt requests, managing various types
  * of DMA interrupts such as transfer errors, FIFO errors, direct mode errors,
  * half-transfer completions, and full transfer completions. It also invokes
  * the appropriate user-defined callback functions based on the interrupt type.
  *
  * @param  hdma Pointer to a DMA_HandleTypeDef structure that contains
  *              the configuration information for the specified DMA stream.
  * @retval int Indicates the type of interrupt:
  *             - 1: Half transfer complete.
  *             - 2: Full transfer complete.
  *             - 3: Otherwise.
  */
int RemoraComms::DMA_IRQHandler(DMA_HandleTypeDef *hdma)
{
  uint32_t tmpisr_dma;
  int interrupt;

  /* calculate DMA base and stream number */
  DMA_Base_Registers  *regs_dma  = (DMA_Base_Registers *)hdma->StreamBaseAddress;

  tmpisr_dma  = regs_dma->ISR;

  if(IS_DMA_STREAM_INSTANCE(hdma->Instance) != 0U)  /* DMA1 or DMA2 instance */
  {
    /* Transfer Error Interrupt management ***************************************/
    if ((tmpisr_dma & (DMA_FLAG_TEIF0_4 << (hdma->StreamIndex & 0x1FU))) != 0U)
    {
      if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_TE) != 0U)
      {
        /* Disable the transfer error interrupt */
        ((DMA_Stream_TypeDef   *)hdma->Instance)->CR  &= ~(DMA_IT_TE);

        /* Clear the transfer error flag */
        regs_dma->IFCR = DMA_FLAG_TEIF0_4 << (hdma->StreamIndex & 0x1FU);

        /* Update error code */
        hdma->ErrorCode |= HAL_DMA_ERROR_TE;
        interrupt =  DMA_OTHER;
      }
    }
    /* FIFO Error Interrupt management ******************************************/
    if ((tmpisr_dma & (DMA_FLAG_FEIF0_4 << (hdma->StreamIndex & 0x1FU))) != 0U)
    {
      if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_FE) != 0U)
      {
        /* Clear the FIFO error flag */
        regs_dma->IFCR = DMA_FLAG_FEIF0_4 << (hdma->StreamIndex & 0x1FU);

        /* Update error code */
        hdma->ErrorCode |= HAL_DMA_ERROR_FE;
        interrupt =  DMA_OTHER;
      }
    }
    /* Direct Mode Error Interrupt management ***********************************/
    if ((tmpisr_dma & (DMA_FLAG_DMEIF0_4 << (hdma->StreamIndex & 0x1FU))) != 0U)
    {
      if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_DME) != 0U)
      {
        /* Clear the direct mode error flag */
        regs_dma->IFCR = DMA_FLAG_DMEIF0_4 << (hdma->StreamIndex & 0x1FU);

        /* Update error code */
        hdma->ErrorCode |= HAL_DMA_ERROR_DME;
        interrupt =  DMA_OTHER;
      }
    }
    /* Half Transfer Complete Interrupt management ******************************/
    if ((tmpisr_dma & (DMA_FLAG_HTIF0_4 << (hdma->StreamIndex & 0x1FU))) != 0U)
    {
      if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_HT) != 0U)
      {
        /* Clear the half transfer complete flag */
        regs_dma->IFCR = DMA_FLAG_HTIF0_4 << (hdma->StreamIndex & 0x1FU);

        /* Multi_Buffering mode enabled */
        if(((((DMA_Stream_TypeDef   *)hdma->Instance)->CR) & (uint32_t)(DMA_SxCR_DBM)) != 0U)
        {
          /* Current memory buffer used is Memory 0 */
          if((((DMA_Stream_TypeDef   *)hdma->Instance)->CR & DMA_SxCR_CT) == 0U)
          {
            if(hdma->XferHalfCpltCallback != NULL)
            {
              /* Half transfer callback */
              //hdma->XferHalfCpltCallback(hdma);
            }
          }
          /* Current memory buffer used is Memory 1 */
          else
          {
            if(hdma->XferM1HalfCpltCallback != NULL)
            {
              /* Half transfer callback */
              //hdma->XferM1HalfCpltCallback(hdma);
            }
          }
        }
        else
        {
          /* Disable the half transfer interrupt if the DMA mode is not CIRCULAR */
          if((((DMA_Stream_TypeDef   *)hdma->Instance)->CR & DMA_SxCR_CIRC) == 0U)
          {
            /* Disable the half transfer interrupt */
            ((DMA_Stream_TypeDef   *)hdma->Instance)->CR  &= ~(DMA_IT_HT);
          }

          if(hdma->XferHalfCpltCallback != NULL)
          {
            /* Half transfer callback */
            hdma->XferHalfCpltCallback(hdma);
          }
        }
        interrupt = DMA_HALF_TRANSFER;
      }
    }
    /* Transfer Complete Interrupt management ***********************************/
    if ((tmpisr_dma & (DMA_FLAG_TCIF0_4 << (hdma->StreamIndex & 0x1FU))) != 0U)
    {
      if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_TC) != 0U)
      {
        /* Clear the transfer complete flag */
        regs_dma->IFCR = DMA_FLAG_TCIF0_4 << (hdma->StreamIndex & 0x1FU);

        if(HAL_DMA_STATE_ABORT == hdma->State)
        {
          /* Disable all the transfer interrupts */
          ((DMA_Stream_TypeDef   *)hdma->Instance)->CR  &= ~(DMA_IT_TC | DMA_IT_TE | DMA_IT_DME);
          ((DMA_Stream_TypeDef   *)hdma->Instance)->FCR &= ~(DMA_IT_FE);

          if((hdma->XferHalfCpltCallback != NULL) || (hdma->XferM1HalfCpltCallback != NULL))
          {
            ((DMA_Stream_TypeDef   *)hdma->Instance)->CR  &= ~(DMA_IT_HT);
          }

          /* Clear all interrupt flags at correct offset within the register */
          regs_dma->IFCR = 0x3FUL << (hdma->StreamIndex & 0x1FU);

          /* Change the DMA state */
          hdma->State = HAL_DMA_STATE_READY;

          /* Process Unlocked */
          __HAL_UNLOCK(hdma);

          if(hdma->XferAbortCallback != NULL)
          {
            hdma->XferAbortCallback(hdma);
          }
          interrupt = DMA_TRANSFER_COMPLETE;
        }


        if(((((DMA_Stream_TypeDef   *)hdma->Instance)->CR) & (uint32_t)(DMA_SxCR_DBM)) != 0U)
        {
          /* Current memory buffer used is Memory 0 */
          if((((DMA_Stream_TypeDef   *)hdma->Instance)->CR & DMA_SxCR_CT) == 0U)
          {
            if(hdma->XferM1CpltCallback != NULL)
            {
              /* Transfer complete Callback for memory1 */
              //hdma->XferM1CpltCallback(hdma);
            }
          }
          /* Current memory buffer used is Memory 1 */
          else
          {
            if(hdma->XferCpltCallback != NULL)
            {
              /* Transfer complete Callback for memory0 */
              //hdma->XferCpltCallback(hdma);
            }
          }
        }
        /* Disable the transfer complete interrupt if the DMA mode is not CIRCULAR */
        else
        {
          if((((DMA_Stream_TypeDef   *)hdma->Instance)->CR & DMA_SxCR_CIRC) == 0U)
          {
            /* Disable the transfer complete interrupt */
            ((DMA_Stream_TypeDef   *)hdma->Instance)->CR  &= ~(DMA_IT_TC);

            /* Change the DMA state */
            hdma->State = HAL_DMA_STATE_READY;

            /* Process Unlocked */
            __HAL_UNLOCK(hdma);
          }

          if(hdma->XferCpltCallback != NULL)
          {
            /* Transfer complete callback */
            hdma->XferCpltCallback(hdma);
          }
        }
        interrupt =  2;
      }
    }

  }

  return interrupt;
}


/**
 * @brief  Retrieves the active DMA buffer index.
 *
 * This function checks the current target memory of the DMA stream
 * in double-buffer mode and returns the index of the active buffer.
 *
 * @param  hdma Pointer to a DMA_HandleTypeDef structure that contains
 *              the configuration information for the specified DMA stream.
 * @retval int Active buffer index:
 *             - 0: First buffer (Memory 0) is active.
 *             - 1: Second buffer (Memory 1) is active.
 */
int RemoraComms::getActiveDMAmemory(DMA_HandleTypeDef *hdma)
{
    DMA_Stream_TypeDef *dmaStream = (DMA_Stream_TypeDef *)hdma->Instance;

    return (dmaStream->CR & DMA_SxCR_CT) ? 1 : 0;
}



/**
 * @brief  Handles the NSS (Slave Select) interrupt for the communication interface.
 *
 * This function checks if the `copyRXbuffer` flag is set, indicating that the receive
 * DMA buffer requires copying to the main Rx buffer. If the flag is set, the function:
 * 1. Starts a DMA memory-to-memory transfer to copy data from the DMA buffer to the main buffer.
 * 2. Waits for the transfer to complete.
 * 3. Aborts the DMA for safety once the transfer is complete.
 *
 * After completing these steps, the `copyRXbuffer` flag is cleared.
 */
void RemoraComms::handleNssInterrupt()
{
	if (this->copyRXbuffer == true)
    {
	    uint8_t* srcBuffer = (uint8_t*)this->ptrRxDMABuffer->buffer[this->RXbufferIdx].rxBuffer;
	    uint8_t* destBuffer = (uint8_t*)this->ptrRxData->rxBuffer;

	    this->status = HAL_DMA_Start(
	    							&this->hdma_memtomem,
									(uint32_t)srcBuffer,
									(uint32_t)destBuffer,
									SPI_BUFF_SIZE
	    							);

	    // Wait for transfer to complete
	    if (this->status == HAL_OK) {
	        this->status = HAL_DMA_PollForTransfer(&hdma_memtomem, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
	    }

	    // Stop the DMA if needed (optional for safety)
	    HAL_DMA_Abort(&this->hdma_memtomem);

		this->copyRXbuffer = false;
    }
}



/**
 * @brief  Handles the SPI DMA transmit (Tx) interrupt.
 *
 * This function processes the DMA interrupt request for the SPI transmit stream
 * and re-enables the corresponding NVIC interrupt.
 */
void RemoraComms::handleTxInterrupt()
{
	DMA_IRQHandler(&this->hdma_spi_tx);
	HAL_NVIC_EnableIRQ(this->irqDMAtx);
}


/**
 * @brief Handles the SPI DMA receive (Rx) interrupt.
 *
 * This function processes the DMA interrupt for SPI receive operations and manages
 * DMA buffers, data validation, and error handling based on the type of interrupt:
 * - Half-transfer complete: Verifies the buffer header and flags valid PRU_READ or PRU_WRITE headers.
 * - Transfer complete: Currently a placeholder for further handling.
 * - Other interrupt sources: Prints an error message.
 *
 * @details
 * - Validates the data in the buffer headers.
 * - Manages buffer indices and sets the `copyRXbuffer` flag to trigger data transfer in
 *   the NSS interrupt handler for PRU_WRITE headers.
 * - Tracks consecutive invalid headers and sets the `SPIdataError` flag if errors exceed the threshold.
 */
void RemoraComms::handleRxInterrupt()
{
    // Handle the interrupt and determine the type of interrupt
    this->interruptType = DMA_IRQHandler(&this->hdma_spi_rx);

    this->RxDMAmemoryIdx = getActiveDMAmemory(&this->hdma_spi_rx);

    if (this->interruptType == DMA_HALF_TRANSFER) // Use the HTC interrupt to check the packet being received
    {
        switch (this->ptrRxDMABuffer->buffer[RxDMAmemoryIdx].header)
        {
            case PRU_READ:
                this->SPIdata = true;
                this->rejectCnt = 0;
                // No action needed for PRU_READ.
                break;

            case PRU_WRITE:
                this->SPIdata = true;
                this->rejectCnt = 0;
                // Valid PRU_WRITE header, set up for data transfer.
                RXbufferIdx = RxDMAmemoryIdx;
                copyRXbuffer = true; // Trigger memory-to-memory copy in the NSS interrupt handler.
                break;

            default:
                this->rejectCnt++;
                if (this->rejectCnt > 5)
                {
                    this->SPIdataError = true; // Flag an error if too many invalid headers are received.
                }
                break;
        }
    }
    else if (this->interruptType == DMA_TRANSFER_COMPLETE) // Transfer complete interrupt
    {
        // Placeholder for transfer complete handling if needed in the future.
    }
    else // Other interrupt sources
    {
        printf("DMA SPI Rx error\n");
    }

    HAL_NVIC_EnableIRQ(this->irqDMArx);
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
