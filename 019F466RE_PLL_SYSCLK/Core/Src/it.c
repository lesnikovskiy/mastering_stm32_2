#include "main.h"

extern TIM_HandleTypeDef timer6;

void SysTick_Handler(void) {
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void TIM6_DAC_IRQHandler(void) {
	HAL_TIM_IRQHandler(&timer6);
}
