#ifndef EXTERN_H
#define EXTERN_H



#include "../Src/lib/ArduinoJson7/ArduinoJson.h"
#include "../Src/thread/pruThread.h"


extern uint32_t baseFreq;
extern uint32_t servoFreq;
extern uint8_t	baseCount;
extern uint8_t	servoCount;
extern uint8_t	commsCount;

extern JsonObject module;

//extern volatile bool PRUreset;

// pointers to objects with global scope
extern pruThread* baseThread;
extern pruThread* servoThread;
extern pruThread* commsThread;

// unions for RX and TX data
//extern volatile rxData_t rxData;
//extern volatile txData_t txData;
//extern volatile DMA_RxBuffer_t rxDMABuffer;

// pointers to data
//extern volatile rxData_t*  ptrRxData;
//extern volatile txData_t*  ptrTxData;
//extern volatile DMA_RxBuffer_t* ptrRxDMABuffer;

//extern volatile int32_t*   ptrTxHeader;
extern volatile bool*      ptrPRUreset;
//extern volatile int32_t*   ptrJointFreqCmd[JOINTS];
//extern volatile int32_t*   ptrJointFeedback[JOINTS];
//extern volatile uint8_t*   ptrJointEnable;
//extern volatile float*     ptrSetPoint[VARIABLES];
//extern volatile float*     ptrProcessVariable[VARIABLES];
//extern volatile uint16_t*  ptrInputs;
//extern volatile uint16_t*  ptrOutputs;

/*
// ping pong buffers
extern RxPingPongBuffer rxPingPongBuffer;
extern TxPingPongBuffer txPingPongBuffer;

extern void initRxPingPongBuffer(RxPingPongBuffer* buffer);
extern void initTxPingPongBuffer(TxPingPongBuffer* buffer);
extern void swapRxBuffers(RxPingPongBuffer* buffer);
extern void swapTxBuffers(TxPingPongBuffer* buffer);
extern void setCurrentRxBufferIndex(RxPingPongBuffer* buffer, int index);
extern void setCurrentTxBufferIndex(TxPingPongBuffer* buffer, int index);
extern rxData_t* getRxBuffer(RxPingPongBuffer* buffer, int index);
extern int getCurrentRxBufferIndex(RxPingPongBuffer* buffer);
extern int getCurrentTxBufferIndex(TxPingPongBuffer* buffer);
extern rxData_t* getCurrentRxBuffer(RxPingPongBuffer* buffer);
extern txData_t* getCurrentTxBuffer(TxPingPongBuffer* buffer);
extern rxData_t* getAltRxBuffer(RxPingPongBuffer* buffer);
extern txData_t* getAltTxBuffer(TxPingPongBuffer* buffer);
*/

#endif
