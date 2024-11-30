#ifndef MODULEINTERRUPT_H
#define MODULEINTERRUPT_H

#include "../interrupt/interrupt.h"
#include "module.h"

// Derived class for module interrupts


class ModuleInterrupt : public Interrupt
{
	private:
	    
		Module* InterruptOwnerPtr;                                // Pointer to the owning module
		void (Module::*InterruptHandler)();                       // Member function pointer for the ISR

	
	public:

		//ModuleInterrupt(int interruptNumber, Module* ownerptr);
		ModuleInterrupt(IRQn_Type interruptNumber, Module* ownerptr, void (Module::*handler)());
    
		void ISR_Handler(void);
};

#endif
