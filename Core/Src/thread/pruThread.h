#ifndef PRUTHREAD_H
#define PRUTHREAD_H

#include "stm32h7xx_hal.h"
#include "timer.h"

class Module;

class pruThread
{
	private:

		pruTimer* 		    TimerPtr;
	
		TIM_TypeDef* 	    timer;
		IRQn_Type 			irq;
		uint32_t 			frequency;

	    Module** modules;       // Dynamic array for main modules
	    Module** modulesPost;   // Dynamic array for post modules
	    size_t maxModules;      // Maximum number of modules
	    size_t moduleCount = 0; // Current count of main modules
	    size_t modulePostCount = 0; // Current count of post modules

		bool hasThreadPost;		// run updatePost()

	public:

		pruThread(TIM_TypeDef* timer, IRQn_Type irq, uint32_t frequency, uint8_t maxModules);
		~pruThread();

		void registerModule(Module *module);
		void registerModulePost(Module *module);
		void startThread(void);
        void stopThread(void);
		void run(void);
};

#endif

