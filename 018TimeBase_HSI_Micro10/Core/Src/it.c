#include "main.h"

extern TIM_HandleTypeDef htimer11;

/* See startup file startup_stm32f411ceux.s for handlers */

void SysTick_Handler(void) {
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void TIM1_TRG_COM_TIM11_IRQHandler(void) {
	HAL_TIM_IRQHandler(&htimer11);
}
