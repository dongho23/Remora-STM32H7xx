#ifndef REMORA_H
#define REMORA_H
#pragma pack(push, 1)


typedef union
{
  // this allow structured access to the incoming SPI data without having to move it
  struct
  {
    uint8_t rxBuffer[SPI_BUFF_SIZE];
  };
  struct
  {
    int32_t header;
    volatile int32_t jointFreqCmd[JOINTS]; 	// Base thread commands ?? - basically motion
    float setPoint[VARIABLES];		  // Servo thread commands ?? - temperature SP, PWM etc
    uint8_t jointEnable;
    uint16_t outputs;
    uint8_t spare0;
  };
}  __attribute__((aligned(32))) rxData_t;

//extern volatile rxData_t rxData;


typedef union
{
  // this allow structured access to the out going SPI data without having to move it
  struct
  {
    uint8_t txBuffer[SPI_BUFF_SIZE];
  };
  struct
  {
    int32_t header;
    int32_t jointFeedback[JOINTS];	  // Base thread feedback ??
    float processVariable[VARIABLES];		     // Servo thread feedback ??
	uint16_t inputs;
  };
} __attribute__((aligned(32))) txData_t;

//extern volatile txData_t txData;


typedef struct {
    volatile rxData_t buffer[2]; // DMA RX buffers
} DMA_RxBuffer_t;





typedef struct {
    rxData_t rxBuffers[2]; // Two buffers for rxData_t
    int currentRxBuffer;   // Index of the current rxData_t buffer
} __attribute__((aligned(32))) RxPingPongBuffer;


typedef struct {
    txData_t txBuffers[2]; // Two buffers for txData_t
    int currentTxBuffer;   // Index of the current txData_t buffer
} __attribute__((aligned(32))) TxPingPongBuffer;



#pragma pack(pop)
#endif
