#ifndef REMORASPI_H
#define REMORASPI_H

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_dma.h"

#include "configuration.h"
#include "remora.h"
#include "extern.h"
#include "../../modules/module.h"
#include "../../modules/moduleinterrupt.h"

#include "../../drivers/pin/pin.h"


typedef struct
{
  __IO uint32_t ISR;   /*!< DMA interrupt status register */
  __IO uint32_t Reserved0;
  __IO uint32_t IFCR;  /*!< DMA interrupt flag clear register */
} DMA_Base_Registers;


class RemoraComms : public Module
{
	friend class ModuleInterrupt;

    private:

		Pin					*pin1, *pin2, *pin3, *pin4;		// debugging pins

        SPI_TypeDef*        spiType;

        txData_t* 			txBuffer;
        rxData_t* 			rxBuffer[2];
        uint32_t			rxBufferAddress[2];			// array of RX buffer addresses
        uint8_t				RXbufferIdx;
        uint8_t				nextRXbufferIdx;

		ModuleInterrupt*	NssInterrupt;
        ModuleInterrupt*	dmaTxInterrupt;
		ModuleInterrupt*	dmaRxInterrupt;
		IRQn_Type			irqNss;
		IRQn_Type			irqDMArx;
		IRQn_Type			irqDMAtx;

        SPI_HandleTypeDef   spiHandle;
        DMA_HandleTypeDef   hdma_spi_tx;
        DMA_HandleTypeDef   hdma_spi_rx;
        HAL_StatusTypeDef   status;

        uint8_t				interruptType;
        uint8_t				dmaStatus;
        uint8_t				RxDMAmemoryIdx;
        uint8_t				RXnextDMAmemoryIdx;

        //uint8_t				rxDMAbuffer;

        uint32_t			RxDMAaddress[2];

        uint32_t			RxCount;
        bool				swapRX;

        rxData_t            spiRxBuffer;
        uint8_t             rejectCnt;
        bool				dmaRx;
        bool				dmaTx;
        bool                SPIdata;
        bool                SPIdataError;

        //PinName             interruptPin;
        //InterruptIn         slaveSelect;
        //bool                sharedSPI;

		HAL_StatusTypeDef startMultiBufferDMASPI(uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint16_t);
		int getActiveDMAmemory(DMA_HandleTypeDef*);
		HAL_StatusTypeDef changeDMAAddress(DMA_HandleTypeDef*, uint32_t, int);

		int DMA_IRQHandler(DMA_HandleTypeDef *);
		void handleRxInterrupt(void);
		void handleTxInterrupt(void);
		void handleNssInterrupt(void);


    public:

        RemoraComms(SPI_TypeDef*);
		virtual void update(void);

        void init(void);
        void start(void);
        bool getStatus(void);
        void setStatus(bool);
        bool getError(void);
        void setError(bool);

};

#endif
