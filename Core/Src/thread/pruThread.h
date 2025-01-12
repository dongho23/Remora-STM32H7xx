#ifndef PRUTHREAD_H
#define PRUTHREAD_H

#include "stm32h7xx_hal.h"
#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include "timer.h"

using namespace std;

class Module;

class pruThread
{
private:

	string threadName;
	uint32_t frequency;
	
	atomic<bool> threadRunning{false};
    atomic<bool> threadPaused{false};

    vector<unique_ptr<Module>> modules;
    vector<unique_ptr<Module>> modulesPost;

	pruTimer* timerPtr;
	TIM_TypeDef* timer;
	IRQn_Type irq;

	bool hasModulesPost;		// run updatePost()

    [[ nodiscard ]] void setThreadRunning(bool val) { threadRunning.store(val, std::memory_order_release); }
    [[ nodiscard ]] void setThreadPaused(bool val) { threadPaused.store(val, std::memory_order_release); }

    bool executeModules();

public:

	pruThread(const string& _name, TIM_TypeDef *_timer, IRQn_Type _irq, uint32_t _freq, uint8_t _prio);

	bool registerModule(unique_ptr<Module> module);
	bool registerModulePost(unique_ptr<Module> module);

    [[nodiscard]] bool isRunning() const { return threadRunning.load(std::memory_order_acquire);  }
    [[nodiscard]] bool isPaused() const { return threadPaused.load(std::memory_order_acquire); }

	bool startThread(void);
	void stopThread(void);
    bool update();
    void pauseThread();
    void resumeThread();
    const string&getName() const;
    uint32_t getFrequency() const;
    size_t getModuleCount() const;;
};

#endif

