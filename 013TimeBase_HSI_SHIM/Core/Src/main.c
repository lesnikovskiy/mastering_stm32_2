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
	HAL_TIM_Base_Start(&htimer11);

	/*
	 * VERY DIM        = 5 (2%)
	 * MEDIUM          = 125 (50%)
	 * FULL BRIGHTNESS = 245 (100%) */
	uint16_t brightness = 125;

	while (1) {
		if (brightness >= 245) {
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
			if (TIM11->SR & TIM_SR_UIF) {
				TIM11->SR = ~TIM_SR_UIF;
			}
		} else {
			// At the very start of the cycle turn the LED ON
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

			// Wait until the counter reaches the brightness threshold
			while (TIM11->CNT < brightness);

			// Threshold reached! Turn the LED OFF (Pull HIGH)
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

			// Wait for the complete 0.25ms period to finish
			while (!(TIM11->SR & TIM_SR_UIF));
			TIM11->SR = ~TIM_SR_UIF; // Clear the flag for the next cycle
		}
	}

	return 0;
}

void SystemClock_Config_HSE(void) {
	RCC_OscInitTypeDef osc_init = { 0 };
	RCC_ClkInitTypeDef clk_init = { 0 };

	osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	osc_init.HSIState = RCC_HSI_OFF;
	osc_init.HSEState = RCC_HSE_ON;
	osc_init.PLL.PLLState = RCC_PLL_ON;
	osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE;

	osc_init.PLL.PLLM = 25;
	osc_init.PLL.PLLN = 200;
	osc_init.PLL.PLLP = RCC_PLLP_DIV2;
	osc_init.PLL.PLLQ = 4;

	clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1; // HCLK = 100MHz
	clk_init.APB1CLKDivider = RCC_HCLK_DIV2;  // PCLK1 = 50MHz
	clk_init.APB2CLKDivider = RCC_HCLK_DIV1;  // PLCK2 = 100MHz

	if (HAL_RCC_OscConfig(&osc_init) != HAL_OK) {
		Error_Handler();
	}

	clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

	if (HAL_RCC_ClockConfig(&clk_init, FLASH_ACR_LATENCY_3WS) != HAL_OK) {
		Error_Handler();
	}

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

void GPIO_Init(void) {
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef ledgpio;
	ledgpio.Pin = GPIO_PIN_13;
	ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
	ledgpio.Pull = GPIO_NOPULL;
	ledgpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	HAL_GPIO_Init(GPIOC, &ledgpio);
}

void TIM11_Init(void) {
	htimer11.Instance = TIM11;

	htimer11.Init.CounterMode = TIM_COUNTERMODE_UP;
	htimer11.Init.Prescaler = 100 - 1; // 100 MHz / 100 = 1 MHz timer clock
	htimer11.Init.Period = 250 - 1;    // 250 ticks = 0.25 ms total period

	if (HAL_TIM_Base_Init(&htimer11) != HAL_OK) {
		Error_Handler();
	}
}

void Error_Handler(void) {
	__disable_irq();

	while (1);
}
