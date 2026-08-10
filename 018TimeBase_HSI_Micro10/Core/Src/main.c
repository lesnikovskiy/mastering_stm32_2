#include "main.h"

#define SYS_CLOCK_FREQ_50MHZ    50
#define SYS_CLOCK_FREQ_84MHZ    84
#define SYS_CLOCK_FREQ_100MHZ   100

void SystemClock_Config(uint8_t clock_freq);
void GPIO_Init(void);
void TIM11_Init(void);
void Error_Handler(void);

TIM_HandleTypeDef htimer11;

int main(void) {
	HAL_Init();

	SystemClock_Config(SYS_CLOCK_FREQ_50MHZ);

	GPIO_Init();
	TIM11_Init();

	// Let's start the timer
	HAL_TIM_Base_Start_IT(&htimer11);

	while (1);

	return 0;
}

void SystemClock_Config(uint8_t clock_freq) {
	RCC_OscInitTypeDef osc_init = { 0 };
	RCC_ClkInitTypeDef clk_init = { 0 };

	uint32_t flash_latency = 0;

	osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	osc_init.HSIState = RCC_HSI_ON;
	osc_init.HSICalibrationValue = 16;
	osc_init.PLL.PLLState = RCC_PLL_ON;
	osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSI;

	switch (clock_freq) {
	case SYS_CLOCK_FREQ_50MHZ: {
		osc_init.PLL.PLLM = 16;
		osc_init.PLL.PLLN = 100;
		osc_init.PLL.PLLP = RCC_PLLP_DIV2;
		osc_init.PLL.PLLQ = 2;

		clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1; // HCLK = 50MHz
		clk_init.APB1CLKDivider = RCC_HCLK_DIV1;  // PCLK1 = 50MHz
		clk_init.APB2CLKDivider = RCC_HCLK_DIV1;  // PLCK2 = 50MHz

		flash_latency = FLASH_ACR_LATENCY_1WS;

		break;
	}

	case SYS_CLOCK_FREQ_84MHZ: {
		osc_init.PLL.PLLM = 16;
		osc_init.PLL.PLLN = 168;
		osc_init.PLL.PLLP = RCC_PLLP_DIV2;
		osc_init.PLL.PLLQ = 4;

		clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1; // HCLK = 84MHz
		clk_init.APB1CLKDivider = RCC_HCLK_DIV2;  // PCLK1 = 42MHz
		clk_init.APB2CLKDivider = RCC_HCLK_DIV1;  // PLCK2 = 84MHz

		flash_latency = FLASH_ACR_LATENCY_2WS;

		break;
	}

	case SYS_CLOCK_FREQ_100MHZ: {
		osc_init.PLL.PLLM = 16;
		osc_init.PLL.PLLN = 200;
		osc_init.PLL.PLLP = RCC_PLLP_DIV2;
		osc_init.PLL.PLLQ = 4;

		clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1; // HCLK = 100MHz
		clk_init.APB1CLKDivider = RCC_HCLK_DIV2;  // PCLK1 = 50MHz
		clk_init.APB2CLKDivider = RCC_HCLK_DIV1;  // PLCK2 = 100MHz

		flash_latency = FLASH_ACR_LATENCY_3WS;

		break;
	}

	default: {
		return;
	}

	}

	if (HAL_RCC_OscConfig(&osc_init) != HAL_OK) {
		Error_Handler();
	}

	clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1
			| RCC_CLOCKTYPE_PCLK2;
	clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

	if (HAL_RCC_ClockConfig(&clk_init, flash_latency) != HAL_OK) {
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

	HAL_GPIO_Init(GPIOC, &ledgpio);
}

void TIM11_Init(void) {
	htimer11.Instance = TIM11;

	htimer11.Init.CounterMode = TIM_COUNTERMODE_UP;
	htimer11.Init.Prescaler = 9;
	htimer11.Init.Period = 50 - 1;

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
