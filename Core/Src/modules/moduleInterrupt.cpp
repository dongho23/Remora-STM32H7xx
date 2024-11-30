#include "../interrupt/interrupt.h"
#include "moduleInterrupt.h"
#include "module.h"


ModuleInterrupt::ModuleInterrupt(IRQn_Type interruptNumber, Module* ownerptr, void (Module::*handler)())
    : InterruptOwnerPtr(ownerptr),
	  InterruptHandler(handler)
{
	// Allows interrupt to access owner's data
	//InterruptOwnerPtr = owner;

	// When a device interrupt object is instantiated, the Register function must be called to let the
	// Interrupt base class know that there is an appropriate ISR function for the given interrupt.
	Interrupt::Register(interruptNumber, this);
}


void ModuleInterrupt::ISR_Handler(void)
{
	//this->InterruptOwnerPtr->handleInterrupt();

    if (this->InterruptOwnerPtr && this->InterruptHandler) {
        (this->InterruptOwnerPtr->*InterruptHandler)(); // Call the member function of the owner
    }
}

