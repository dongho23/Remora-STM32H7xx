#include <cstdio>

#include "pruThread.h"
#include "../modules/module.h"


using namespace std;

// Thread constructor
pruThread::pruThread(TIM_TypeDef* timer, IRQn_Type irq, uint32_t frequency, uint8_t maxModules) :
	timer(timer),
	irq(irq),
	frequency(frequency),
	maxModules(maxModules)
{
	 printf("    Creating thread %lu with max modules: %d\n", this->frequency, this->maxModules);

	// Allocate memory for the arrays
	modules = new Module*[maxModules];
	modulesPost = new Module*[maxModules];

	// Initialize arrays to nullptr
	for (size_t i = 0; i < maxModules; ++i) {
		modules[i] = nullptr;
		modulesPost[i] = nullptr;
	}
}


// Destructor
pruThread::~pruThread() {
    delete[] modules;
    delete[] modulesPost;
}


void pruThread::startThread(void)
{
	TimerPtr = new pruTimer(this->timer, this->irq, this->frequency, this);
}


void pruThread::stopThread(void)
{
    this->TimerPtr->stopTimer();
}


void pruThread::registerModule(Module* module)
{
    if (moduleCount < maxModules) {
        modules[moduleCount++] = module;
    } else {
        printf("Error: Maximum modules reached for thread.\n");
    }
}


void pruThread::registerModulePost(Module* module)
{
    if (modulePostCount < maxModules) {
        modulesPost[modulePostCount++] = module;
        hasThreadPost = true;
    } else {
        printf("Error: Maximum post modules reached for thread.\n");
    }
}


void pruThread::run(void)
{
    // Run all main modules
    for (uint8_t i = 0; i < moduleCount; ++i) {
        if (modules[i]) {
            modules[i]->runModule();
        }
    }

    // Run all post modules if present
    if (hasThreadPost) {
        for (uint8_t i = 0; i < modulePostCount; ++i) {
            if (modulesPost[i]) {
                modulesPost[i]->runModulePost();
            }
        }
    }
}
