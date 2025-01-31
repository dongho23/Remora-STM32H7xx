#include <stdio.h>

#include "../interrupt/interrupt.h"
#include "timerInterrupt.h"
#include "timer.h"
#include "pruThread.h"



// Timer constructor
pruTimer::pruTimer(TIM_TypeDef* _timer, IRQn_Type _irq, uint32_t _frequency, pruThread* _ownerPtr):
	timer(_timer),
	irq(_irq),
	frequency(_frequency),
	timerOwnerPtr(_ownerPtr)
{
	interruptPtr = new TimerInterrupt(irq, this);	// Instantiate a new Timer Interrupt object and pass "this" pointer

	startTimer();
}


void pruTimer::timerTick(void)
{
	//Do something here
	timerOwnerPtr->update();
}



void pruTimer::startTimer(void)
{
    uint32_t TIM_CLK;

    if (timer == TIM2)
    {
        printf("	power on Timer 2\n\r");
        __HAL_RCC_TIM2_CLK_ENABLE();
        TIM_CLK = APB1CLK;
    }
    else if (timer == TIM3)
    {
        printf("	power on Timer 3\n\r");
        __HAL_RCC_TIM3_CLK_ENABLE();
        TIM_CLK = APB1CLK;
    }
    else if (timer == TIM4)
    {
        printf("	power on Timer 4\n\r");
        __HAL_RCC_TIM4_CLK_ENABLE();
        TIM_CLK = APB1CLK;
    }

    //timer update frequency = TIM_CLK/(TIM_PSC+1)/(TIM_ARR + 1)

    timer->CR2 &= 0;                                            // UG used as trigg output
    timer->PSC = TIM_PSC-1;                                     // prescaler
    timer->ARR = ((TIM_CLK / TIM_PSC / this->frequency) - 1);   // period
    timer->EGR = TIM_EGR_UG;                                    // reinit the counter
    timer->DIER = TIM_DIER_UIE;                                 // enable update interrupts

    timer->CR1 |= TIM_CR1_CEN;                                  // enable timer

    NVIC_EnableIRQ(irq);

    printf("	timer started\n");
}

void pruTimer::stopTimer()
{
    NVIC_DisableIRQ(irq);

    printf("	timer stop\n\r");
    timer->CR1 &= (~(TIM_CR1_CEN));     // disable timer
}
