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
RemoraComms::RemoraComms(SPI_TypeDef* spiType) :
    spiType(spiType)
{
    this->spiHandle.Instance = this->spiType;

    this->irqNss = EXTI4_IRQn;
    this->irqDMAtx = DMA1_Stream0_IRQn;
    this->irqDMArx = DMA1_Stream1_IRQn;

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
}


/**
 * @brief Starts the SPI communication with configured interrupts and DMA buffers.
 */
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

	initTxPingPongBuffer(&txPingPongBuffer);
	initRxPingPongBuffer(&rxPingPongBuffer);

	// Use single buffer for TX
	txBuffer = getCurrentTxBuffer(&txPingPongBuffer);

	// Use alternate buffer in multi-buffer mode for RX
	rxBuffer = getAltRxBuffer(&rxPingPongBuffer);


    this->spiHandle.Lock = HAL_UNLOCKED;

    // Start DMA in multi-buffer circular mode
	this->dmaStatus = this->startMultiBufferDMASPI((uint8_t *)txBuffer->txBuffer, (uint8_t *)txBuffer->txBuffer, (uint8_t *)rxBuffer->rxBuffer, (uint8_t *)rxBuffer->rxBuffer, SPI_BUFF_SIZE);
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
  *             - 0: Otherwise.
  */
int RemoraComms::DMA_IRQHandler(DMA_HandleTypeDef *hdma)
{
  uint32_t tmpisr_dma;
  __IO uint32_t count = 0U;
  uint32_t timeout = SystemCoreClock / 9600U;
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
        interrupt = 1;
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
          interrupt = 0;
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

    /* manage error case */
    if(hdma->ErrorCode != HAL_DMA_ERROR_NONE)
    {
      if((hdma->ErrorCode & HAL_DMA_ERROR_TE) != 0U)
      {
        hdma->State = HAL_DMA_STATE_ABORT;

        /* Disable the stream */
        __HAL_DMA_DISABLE(hdma);

        do
        {
          if (++count > timeout)
          {
            break;
          }
        }
        while((((DMA_Stream_TypeDef   *)hdma->Instance)->CR & DMA_SxCR_EN) != 0U);

        if((((DMA_Stream_TypeDef   *)hdma->Instance)->CR & DMA_SxCR_EN) != 0U)
        {
          /* Change the DMA state to error if DMA disable fails */
          hdma->State = HAL_DMA_STATE_ERROR;
        }
        else
        {
          /* Change the DMA state to Ready if DMA disable success */
          hdma->State = HAL_DMA_STATE_READY;
        }

        /* Process Unlocked */
        __HAL_UNLOCK(hdma);
      }

      if(hdma->XferErrorCallback != NULL)
      {
        /* Transfer error callback */
        hdma->XferErrorCallback(hdma);
      }
    }
  }
  return interrupt;
}


/**
 * @brief  Retrieves the active DMA buffer index.
 *
 * This function checks the current target buffer of the DMA stream
 * in double-buffer mode and returns the index of the active buffer.
 *
 * @param  hdma Pointer to a DMA_HandleTypeDef structure that contains
 *              the configuration information for the specified DMA stream.
 * @retval int Active buffer index:
 *             - 0: First buffer (Memory 0) is active.
 *             - 1: Second buffer (Memory 1) is active.
 */
int RemoraComms::getActiveDMAbuffer(DMA_HandleTypeDef *hdma)
{
    DMA_Stream_TypeDef *dmaStream = (DMA_Stream_TypeDef *)hdma->Instance;

    return (dmaStream->CR & DMA_SxCR_CT) ? 1 : 0;
}



/**
 * @brief  Updates the memory address for a specified DMA buffer.
 *
 * This function sets the new memory address for either Memory 0 or Memory 1
 * of a DMA stream.
 *
 * @param  hdma Pointer to a DMA_HandleTypeDef structure that contains
 *              the configuration information for the specified DMA stream.
 * @param  Address New memory address to be set for the specified buffer.
 * @param  memory Specifies the target memory buffer:
 *                - MEMORY0: Updates the address for Memory 0.
 *                - MEMORY1: Updates the address for Memory 1.
 * @retval HAL_StatusTypeDef:
 *         - HAL_OK: Address successfully updated.
 *         - HAL_ERROR: Invalid parameters or memory type.
 */
HAL_StatusTypeDef RemoraComms::changeDMAAddress(DMA_HandleTypeDef *hdma, uint32_t Address, int memory)
{
    if (hdma == NULL)
    {
        return HAL_ERROR; // Null pointer check
    }

    if (memory == MEMORY0)
    {
        ((DMA_Stream_TypeDef *)hdma->Instance)->M0AR = Address;
    }
    else if (memory == MEMORY1)
    {
        ((DMA_Stream_TypeDef *)hdma->Instance)->M1AR = Address;
    }
    else
    {
        return HAL_ERROR; // Invalid memory parameter
    }

    return HAL_OK;
}



/**
 * @brief  Handles the NSS (Slave Select) interrupt for the communication interface.
 *
 * This function checks if the `swapRx` flag is set, indicating that the
 * receive buffer requires swapping. If so, it disables interrupts, performs
 * the buffer swap, and re-enables interrupts. The `swapRx` flag is then cleared.
 *
 * @note  Ensure that the `swapTxBuffers` function operates correctly in interrupt-disabled
 *        regions to avoid potential race conditions or deadlocks.
 */
void RemoraComms::handleNssInterrupt()
{
    if (this->swapRx == true)
    {
        __disable_irq(); 					// Disable interrupts to ensure atomic buffer swapping
        swapTxBuffers(&txPingPongBuffer); 	// Swap the transmit buffers
        __enable_irq(); 					// Re-enable interrupts
        this->swapRx = false; 				// Clear the swapRx flag
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
 * @brief Handles the receive DMA SPI (Rx) interrupt.
 *
 * This function processes the DMA interrupt for SPI receive operations and performs
 * actions based on the type of interrupt (half-transfer or transfer complete).
 * It ensures that the DMA buffers are managed appropriately and flags errors when necessary.
 *
 * @details
 * - Handles two interrupt types:
 *   - Half-transfer complete: Checks the header in the buffer and updates the DMA
 *     buffer address to the alternate buffer if needed.
 *   - Transfer complete: Verifies the header validity and manages buffer swaps and errors.
 *
 * - Errors:
 *   - If invalid data is received consecutively more than 5 times, `SPIdataError` is set.
 */
void RemoraComms::handleRxInterrupt()
{
    // Determine the type of interrupt (1 = half-transfer, 2 = transfer complete, 0 = none)
    this->interruptType = DMA_IRQHandler(&this->hdma_spi_rx);

    switch (interruptType)
    {
        case 1: // Half-transfer complete
            // Check the header in the buffer
            if (this->rxBuffer->header == PRU_WRITE)
            {
                this->memory = 1 - getActiveDMAbuffer(&this->hdma_spi_rx);
                this->address = (uint32_t)getAltRxBuffer(&rxPingPongBuffer);
                // Point the DMA transactions at the alternate buffer
                changeDMAAddress(&this->hdma_spi_rx, this->address, this->memory);
                // Flag the swap of Rx buffers in NSS interrupt handler
                this->swapRx = true;
            }
            break;

        case 2: // Transfer complete
            // Check if header is valid
            if (this->rxBuffer->header == PRU_READ || this->rxBuffer->header == PRU_WRITE)
            {
                if (this->swapRx == true)
                {
                    // Change the now inactive DMA buffer address as well
                    changeDMAAddress(&this->hdma_spi_rx, this->address, 1 - this->memory);
                }
                this->SPIdata = true;
                this->rejectCnt = 0; // Reset reject counter on valid data
            }
            else
            {
                this->rejectCnt++;
                if (this->rejectCnt > 5)
                {
                    this->SPIdataError = true; // Flag data error if rejects exceed threshold
                }
                // TODO: Implement SPI reset logic if required
            }
            break;

        default:
            break;
    }

    // Re-enable the NVIC interrupt for SPI Rx DMA
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
