#include <cstdio>

#include "pruThread.h"
#include "../modules/module.h"


pruThread::pruThread(const string& _name, TIM_TypeDef *_timer, IRQn_Type _irq, uint32_t _freq, uint8_t _prio)
    : threadName(_name),
      timer(_timer),
      irq(_irq),
      frequency(_freq),
	  hasModulesPost(false)
{
	printf("Creating %s thread at %lu hz\n", threadName.c_str(), (unsigned long)frequency);
    NVIC_SetPriority(irq, _prio);
}


bool pruThread::executeModules() {
    for (const auto& module : modules) {
        if (module) {
            module->runModule();
        }
    }

    if (hasModulesPost) {
		for (const auto& module : modulesPost) {
			if (module) {
				module->runModule();
			}
		}
    }

    return true;
}

bool pruThread::registerModule(shared_ptr<Module> module) {
    if (!module) {
        return false;
    }
    modules.push_back(module);
    return true;
}

bool pruThread::registerModulePost(shared_ptr<Module> module) {
    if (!module) {
        return false;
    }
    hasModulesPost = true;
    modulesPost.push_back(module);
    return true;
}

// For baremetal, this is just initialization
bool pruThread::startThread() {
    if (isRunning()) {
        return true;
    }

    setThreadRunning(true);
    setThreadPaused(false);

    timerPtr = new pruTimer(timer, irq, frequency, this);
    return true;
}

void pruThread::stopThread() {
    setThreadRunning(false);
    setThreadPaused(false);
}

// This is the main function that should be called periodically
bool pruThread::update() {
    if (!isRunning() || isPaused()) {
        return true;
    }

    // Execute all modules
    if (!executeModules()) {
        return false;
    }

    return true;
}

void pruThread::pauseThread() {
    setThreadPaused(true);
}

void pruThread::resumeThread() {
    setThreadPaused(false);
}

size_t pruThread::getModuleCount() const {
    return modules.size();
}

const string& pruThread::getName() const {
    return threadName;
}

uint32_t pruThread::getFrequency() const {
    return frequency;
}
