#include "remora.h"
#include "interrupt/irqHandlers.h"
#include "interrupt/interrupt.h"
#include "json/jsonConfigHandler.h"
#include "comms/SPIComms.h"


// unions for TX and RX data
__attribute__((section(".DmaSection"))) volatile txData_t txData;
__attribute__((section(".DmaSection"))) volatile rxData_t rxData;

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

void Remora::transitionToState(State newState) {
    if (currentState != newState) {
        printf("\n## Transitioning from state %d to state %d\n", currentState, newState);
        prevState = currentState;
        currentState = newState;
    }
}

void Remora::handleSetupState() {
    if (currentState != prevState) {
        printf("\n## Entering SETUP state\n");
    }
    prevState = currentState;

    loadModules();
    transitionToState(ST_START);
}

void Remora::handleStartState() {
    if (currentState != prevState) {
        printf("\n## Entering START state\n");
    }
    prevState = currentState;

    if (!threadsRunning) {
        startThread(servoThread, "SERVO");
        startThread(baseThread, "BASE");
        threadsRunning = true;
    }

    transitionToState(ST_IDLE);
}

void Remora::handleIdleState() {
    if (currentState != prevState) {
        printf("\n## Entering IDLE state\n");
    }
    prevState = currentState;

    if (comms->getStatus()) {
        transitionToState(ST_RUNNING);
    }
}

void Remora::handleRunningState() {
    if (currentState != prevState) {
        printf("\n## Entering RUNNING state\n");
    }
    prevState = currentState;

    if (!comms->getStatus()) {
        transitionToState(ST_RESET);
    }

    if (reset) {
    	transitionToState(ST_SYSRESET);
    }
}

void Remora::handleResetState() {
    if (currentState != prevState) {
        printf("\n## Entering RESET state\n");
    }
    prevState = currentState;

    printf("   Resetting rxBuffer\n");
    resetBuffer(ptrRxData->rxBuffer, Config::dataBuffSize);
    transitionToState(ST_IDLE);
}

void Remora::handleSysResetState() {
	HAL_NVIC_SystemReset();
}

void Remora::startThread(const std::unique_ptr<pruThread>& thread, const char* name) {
    printf("Starting the %s thread\n", name);
    thread->startThread();
}

void Remora::resetBuffer(volatile uint8_t* buffer, size_t size) {
    memset((void*)buffer, 0, size);
}

void Remora::run() {
    while (true) {
        switch (currentState) {
            case ST_SETUP:
                handleSetupState();
                break;
            case ST_START:
                handleStartState();
                break;
            case ST_IDLE:
                handleIdleState();
                break;
            case ST_RUNNING:
                handleRunningState();
                break;
            case ST_RESET:
                handleResetState();
                break;
            case ST_SYSRESET:
                handleSysResetState();
                break;
            default:
                printf("Error: Invalid state\n");
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
            std::shared_ptr<Module> _mod = factory->createModule(threadName, moduleType, modules[i], this);

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

