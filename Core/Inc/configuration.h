#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define PRU_BASEFREQ    	40000 //24000   // PRU Base thread ISR update frequency (hz)
#define PRU_SERVOFREQ       1000            // PRU Servo thread ISR update freqency (hz)
#define OVERSAMPLE          3
#define SWBAUDRATE          19200           // Software serial baud rate
#define PRU_COMMSFREQ       (SWBAUDRATE * OVERSAMPLE)

#define STEPBIT     		22            	// bit location in DDS accum
#define STEP_MASK   		(1L<<STEPBIT)

#define JOINTS			    8				// Number of joints - set this the same as LinuxCNC HAL compenent. Max 8 joints
#define VARIABLES           6             	// Number of command values - set this the same as the LinuxCNC HAL compenent

#define PRU_DATA		    0x64617461 	    // "data" SPI payload
#define PRU_READ            0x72656164      // "read" SPI payload
#define PRU_WRITE           0x77726974      // "writ" SPI payload
#define PRU_ESTOP           0x65737470      // "estp" SPI payload
#define PRU_ACKNOWLEDGE		0x61636b6e	    // "ackn" payload
#define PRU_ERR		        0x6572726f	    // "erro" payload

#define BASE_THREAD_IRQ_PRIORITY   	1
#define SERVO_THREAD_IRQ_PRIORITY  	2
#define COMMS_THREAD_IRQ_PRIORITY  	3
#define SPI_DMA_TX_IRQ_PRIORITY 	4
#define SPI_DMA_RX_IRQ_PRIORITY 	5
#define SPI_NSS_IRQ_PRIORITY 		6


// Serial configuration
#define PC_BAUD             115200          // UART baudrate

#define DATA_ERR_MAX        10

// SPI configuration
#define SPI_BUFF_SIZE 		64            	// Size of SPI recieve buffer - same as HAL component, 64

#endif
