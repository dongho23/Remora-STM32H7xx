#include "interrupt.h"

extern "C" {

	void EXTI4_IRQHandler()
	{
		// Chip select pin is PC_4 (EXTI4 PIN 4)
		if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET)
		{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
		Interrupt::EXTI4_Wrapper();
		}
	}


/*
	void TIM1_BRK_TIM9_IRQHandler()
	{
	  if(TIM9->SR & TIM_SR_UIF) // if UIF flag is set
	  {
		TIM9->SR &= ~TIM_SR_UIF; // clear UIF flag

		Interrupt::TIM9_Wrapper();
	  }
	}

	void TIM1_UP_TIM10_IRQHandler()
	{
	  if(TIM10->SR & TIM_SR_UIF) // if UIF flag is set
	  {
		TIM10->SR &= ~TIM_SR_UIF; // clear UIF flag

		Interrupt::TIM10_Wrapper();
	  }
	}

	void DMA2_Stream2_IRQHandler(void)
	{
		Interrupt::DMA2_Stream2_Wrapper();
	}
*/
}

