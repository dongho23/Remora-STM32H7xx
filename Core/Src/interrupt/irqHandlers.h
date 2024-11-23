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


	void TIM2_IRQHandler()
	{
	  if(TIM2->SR & TIM_SR_UIF) // if UIF flag is set
	  {
		TIM2->SR &= ~TIM_SR_UIF; // clear UIF flag

		Interrupt::TIM2_Wrapper();
	  }
	}


	void TIM3_IRQHandler()
	{
	  if(TIM3->SR & TIM_SR_UIF) // if UIF flag is set
	  {
		TIM3->SR &= ~TIM_SR_UIF; // clear UIF flag

		Interrupt::TIM3_Wrapper();
	  }
	}

	void TIM4_IRQHandler()
	{
	  if(TIM4->SR & TIM_SR_UIF) // if UIF flag is set
	  {
		TIM4->SR &= ~TIM_SR_UIF; // clear UIF flag

		Interrupt::TIM4_Wrapper();
	  }
	}
}

