#include "main.h"

extern TIM_HandleTypeDef timer6;

void SysTick_Handler(void) {
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}
