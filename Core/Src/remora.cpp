#include "remora.h"
#include "json/jsonConfigHandler.h"


Remora::Remora()
	: baseFreq(Config::PRU_BASEFREQ),
	  servoFreq(Config::PRU_SERVOFREQ),
	  commsFreq(Config::PRU_COMMSFREQ)
{

	this->configHandler = new JsonConfigHandler(this);

	/*
	baseThread = new pruThread(TIM3, TIM3_IRQn, baseFreq, baseCount);
	NVIC_SetPriority(TIM3_IRQn, BASE_THREAD_IRQ_PRIORITY);

	servoThread = new pruThread(TIM2, TIM2_IRQn, servoFreq, servoCount);
	NVIC_SetPriority(TIM2_IRQn, SERVO_THREAD_IRQ_PRIORITY);

	commsThread = new pruThread(TIM4, TIM4_IRQn, PRU_COMMSFREQ, commsCount);
	NVIC_SetPriority(TIM4_IRQn, COMMS_THREAD_IRQ_PRIORITY);
	*/

    baseThread = make_unique<pruThread>(
    									"Base",
										TIM3,
										TIM3_IRQn,
										baseFreq,
										Config::BASE_THREAD_IRQ_PRIORITY
										);
    servoThread = make_unique<pruThread>(
    									"Servo",
										TIM2,
										TIM2_IRQn,
										servoFreq,
										Config::SERVO_THREAD_IRQ_PRIORITY
										);

    loadModules();
    while (1);
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
            std::unique_ptr<Module> _mod = factory->createModule(threadName, moduleType, modules[i]);
            if (strcmp(threadName, "Servo") == 0)
                servoThread->registerModule(move(_mod));
            else if (strcmp(threadName, "Base") == 0)
                baseThread->registerModule(move(_mod));
            else {
                //onLoad.push_back(move(_mod));
            }
        }
    }

}

