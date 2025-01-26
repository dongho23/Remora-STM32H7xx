#include "remora.h"
#include "interrupt/irqHandlers.h"
#include "interrupt/interrupt.h"
#include "json/jsonConfigHandler.h"
#include "comms/SPIComms.h"


// unions for TX and RX data
__attribute__((section(".DmaSection"))) volatile txData_t txData;
__attribute__((section(".DmaSection"))) volatile rxData_t rxData;

volatile bool*    	ptrPRUreset;
volatile int32_t* 	ptrJointFreqCmd[Config::joints];
volatile int32_t* 	ptrJointFeedback[Config::joints];
volatile uint8_t* 	ptrJointEnable;
volatile float*   	ptrSetPoint[Config::variables];
volatile float*   	ptrProcessVariable[Config::variables];
volatile uint16_t* 	ptrInputs;
volatile uint16_t* 	ptrOutputs;


Remora::Remora() :
	baseFreq(Config::pruBaseFreq),
	servoFreq(Config::pruServoFreq),
	commsFreq(Config::pruCommsFreq)
{
	threadsRunning = false;
	currentState = ST_SETUP;
	prevState = ST_RESET;

	configHandler = std::make_unique<JsonConfigHandler>(this);

    ptrTxData = &txData;
    ptrRxData = &rxData;

	auto spiComms = std::make_unique<SPIComms>(ptrRxData, ptrTxData, SPI1);

	comms = std::make_shared<CommsHandler>();
	comms->setInterface(std::move(spiComms));
	comms->init();
	comms->start();

    baseThread = make_unique<pruThread>(
    									"Base",
										TIM3,
										TIM3_IRQn,
										baseFreq,
										Config::baseThreadIrqPriority
										);
    servoThread = make_unique<pruThread>(
    									"Servo",
										TIM2,
										TIM2_IRQn,
										servoFreq,
										Config::servoThreadIrqPriority
										);

	/*
	commsThread = new pruThread(TIM4, TIM4_IRQn, PRU_COMMSFREQ, commsCount);
	NVIC_SetPriority(TIM4_IRQn, COMMS_THREAD_IRQ_PRIORITY);
	*/


    servoThread->registerModule(comms);
}

void Remora::run() {
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

				              loadModules();

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
				                  printf("\nStarting the SERVO thread\n");
				                  servoThread->startThread();

				                  printf("\nStarting the BASE thread\n");
				                  baseThread->startThread();

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

				              if (comms->getStatus() == false)
				              {
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
				              printf("   Resetting rxBuffer\n");
				              {
								  int n = Config::dataBuffSize;
								  while(n-- > 0)
								  {
									  ptrRxData->rxBuffer[n] = 0;
								  }
				              }

				              currentState = ST_IDLE;
				              break;

				          case ST_WDRESET:
				        	  // force a reset
				        	  HAL_NVIC_SystemReset();
				              break;
				  }
			comms->tasks();
		}
}

void Remora::loadModules() {
    ModuleFactory* factory = ModuleFactory::getInstance();
    JsonArray modules = configHandler->getModules();
    if (modules.isNull()) {
      //printf something here
    }

    for (size_t i = 0; i < modules.size(); i++) {
        //if (modules[i].containsKey("Thread") && modules[i].containsKey("Type")) {
    	if (modules[i]["Thread"].is<const char*>() && modules[i]["Type"].is<const char*>()) {
            const char* threadName = modules[i]["Thread"];
            const char* moduleType = modules[i]["Type"];
            uint32_t threadFreq = 0;

            // Determine the thread frequency based on the thread name
            if (strcmp(threadName, "Servo") == 0) {
                threadFreq = servoFreq;
            } else if (strcmp(threadName, "Base") == 0) {
                threadFreq = baseFreq;
            }

            // Add the "ThreadFreq" key and its value to the module's JSON object
            modules[i]["ThreadFreq"] = threadFreq;

            // Create module using factory
            std::shared_ptr<Module> _mod = factory->createModule(threadName, moduleType, modules[i]);

            // Check if the module creation was successful
            if (!_mod) {
            	printf("Error: Failed to create module of type '%s' for thread '%s'. Skipping registration.\n",
						moduleType, threadName);
				continue; // Skip to the next iteration
			}

            bool _modPost = _mod->getUsesModulePost();

            if (strcmp(threadName, "Servo") == 0) {
                servoThread->registerModule(_mod);
            	if (_modPost) {
            		servoThread->registerModulePost(_mod);
            	}
            }
            else if (strcmp(threadName, "Base") == 0) {
                baseThread->registerModule(_mod);
            	if (_modPost) {
            		servoThread->registerModulePost(_mod);
            	}
            }
            else {
                //onLoad.push_back(move(_mod));
            }
        }
    }

}

