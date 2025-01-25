#include "remora.h"
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

	configHandler = std::make_unique<JsonConfigHandler>(this);

    ptrTxData = &txData;
    ptrRxData = &rxData;

	auto spiComms = std::make_unique<SPIComms>(ptrRxData, ptrTxData, SPI1);

	comms = std::make_unique<CommsHandler>();
	comms->setInterface(std::move(spiComms));
	comms->init();
	comms->start();




	/*
	commsThread = new pruThread(TIM4, TIM4_IRQn, PRU_COMMSFREQ, commsCount);
	NVIC_SetPriority(TIM4_IRQn, COMMS_THREAD_IRQ_PRIORITY);
	*/

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

    loadModules();
    while (1) {
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

