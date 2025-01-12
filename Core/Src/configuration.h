#ifndef CONFIGURATION_H
#define CONFIGURATION_H

namespace Config {
    constexpr uint32_t PRU_BASEFREQ = 40000;        // PRU Base thread ISR update frequency (hz)
    constexpr uint32_t PRU_SERVOFREQ = 1000;        // PRU Servo thread ISR update frequency (hz)
    constexpr uint32_t OVERSAMPLE = 3;
    constexpr uint32_t SWBAUDRATE = 19200;          // Software serial baud rate
    constexpr uint32_t PRU_COMMSFREQ = SWBAUDRATE * OVERSAMPLE;

    constexpr uint32_t STEPBIT = 22;                // Bit location in DDS accum
    constexpr uint32_t STEP_MASK = (1L << STEPBIT);

    constexpr uint32_t JOINTS = 8;                  // Number of joints
    constexpr uint32_t VARIABLES = 6;              // Number of command values

    constexpr uint32_t PRU_DATA = 0x64617461;       // "data" SPI payload
    constexpr uint32_t PRU_READ = 0x72656164;       // "read" SPI payload
    constexpr uint32_t PRU_WRITE = 0x77726974;      // "writ" SPI payload
    constexpr uint32_t PRU_ESTOP = 0x65737470;      // "estp" SPI payload
    constexpr uint32_t PRU_ACKNOWLEDGE = 0x61636b6e;// "ackn" SPI payload
    constexpr uint32_t PRU_ERR = 0x6572726f;        // "erro" payload

    // IRQ priorities
    constexpr uint32_t BASE_THREAD_IRQ_PRIORITY = 1;
    constexpr uint32_t SERVO_THREAD_IRQ_PRIORITY = 2;
    constexpr uint32_t COMMS_THREAD_IRQ_PRIORITY = 3;
    constexpr uint32_t SPI_DMA_TX_IRQ_PRIORITY = 4;
    constexpr uint32_t SPI_DMA_RX_IRQ_PRIORITY = 5;
    constexpr uint32_t SPI_NSS_IRQ_PRIORITY = 6;

    // Serial configuration
    constexpr uint32_t PC_BAUD = 115200;            // UART baudrate

    constexpr uint32_t DATA_ERR_MAX = 100;

    // SPI configuration
    constexpr uint32_t DATA_BUFF_SIZE = 64;         // Size of SPI receive buffer
}

#endif
