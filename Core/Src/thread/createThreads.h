#include "extern.h"


void createThreads(void)
{
	servoThread = new pruThread(TIM2, TIM2_IRQn, servo_freq);
    NVIC_SetPriority(TIM2_IRQn, 2);

    baseThread = new pruThread(TIM3, TIM3_IRQn , base_freq);
    NVIC_SetPriority(TIM3_IRQn , 3);

    commsThread = new pruThread(TIM4, TIM4_IRQn, PRU_COMMSFREQ);
    NVIC_SetPriority(TIM4_IRQn, 4);
}
