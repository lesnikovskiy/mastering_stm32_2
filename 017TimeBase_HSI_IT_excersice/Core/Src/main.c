#include "main.h"

void SystemClock_Config_HSE(void);
void GPIO_Init(void);
void TIM11_Init(void);
void Error_Handler(void);

TIM_HandleTypeDef htimer11;

int main(void) {
	HAL_Init();

	SystemClock_Config_HSE();

	GPIO_Init();
	TIM11_Init();

	// Let's start the timer
	HAL_TIM_Base_Start_IT(&htimer11);

	while (1);

	return 0;
}

void SystemClock_Config_HSE(void) {
	// Let's do it later and use HSI 16Mhz by default
}

void GPIO_Init(void) {
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef ledgpio;
	ledgpio.Pin = GPIO_PIN_13;
	ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
	ledgpio.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(GPIOC, &ledgpio);
}

void TIM11_Init(void) {
	htimer11.Instance = TIM11;

	htimer11.Init.CounterMode = TIM_COUNTERMODE_UP;
	htimer11.Init.Prescaler = 24;
	htimer11.Init.Period = 64000 - 1;

	if (HAL_TIM_Base_Init(&htimer11) != HAL_OK) {
		Error_Handler();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM11) {
		// Toggle pin 13
		GPIOC->ODR ^= GPIO_PIN_13;
	}
}

void Error_Handler(void) {
	__disable_irq();

	while (1);
}
