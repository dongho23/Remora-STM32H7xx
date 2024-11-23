#include "interrupt.h"

#include <cstdio>

// Define the vector table, it is only declared in the class declaration
Interrupt* Interrupt::ISRVectorTable[] = {0};

// Constructor
Interrupt::Interrupt(void){}


// Methods

void Interrupt::Register(int interruptNumber, Interrupt* intThisPtr)
{
	printf("Registering interrupt for interrupt number = %d\n", interruptNumber);
	ISRVectorTable[interruptNumber] = intThisPtr;
}

void Interrupt::EXTI4_Wrapper(void)
{
	ISRVectorTable[EXTI4_IRQn]->ISR_Handler();
}

void Interrupt::TIM2_Wrapper(void)
{
	ISRVectorTable[TIM2_IRQn]->ISR_Handler();
}

void Interrupt::TIM3_Wrapper(void)
{
	ISRVectorTable[TIM3_IRQn]->ISR_Handler();
}

void Interrupt::TIM4_Wrapper(void)
{
	ISRVectorTable[TIM4_IRQn]->ISR_Handler();
}
