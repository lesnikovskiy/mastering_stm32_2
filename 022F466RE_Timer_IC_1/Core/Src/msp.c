#include "main.h"

void HAL_MspInit(void) {
	// Setup priority grouping of the ARM Cortex MX processor
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	// Enable the required system exceptions of the ARM Cortex MX processor
	SCB->SHCSR |= (0x7 << 16); // Enable usage fault, memory fault and bus fault system exceptions

	// Configure priority for the system exceptions
	HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
	HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
	HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
}

void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *htim) {
	GPIO_InitTypeDef gpio = { 0 };

	// Enable the clock for timer 2
	__HAL_RCC_TIM2_CLK_ENABLE();
	// Enable RCC Clock for port A
	__HAL_RCC_GPIOA_CLK_ENABLE();

	// Configure a GPIO to behave as timer2 channel 1
	gpio.Pin = GPIO_PIN_0;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Alternate = GPIO_AF1_TIM2;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(GPIOA, &gpio);

	// NVIC settings
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	HAL_NVIC_SetPriority(TIM2_IRQn, 15, 0);
}
