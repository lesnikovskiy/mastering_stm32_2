#include "main.h"

void SystemClock_Config_HSE(uint8_t clock_freq);
void HAL_GPIO_MspInit(void);
void TIM6_Init(void);
void Error_Handler(void);

TIM_HandleTypeDef timer6;

char *greeting_message = "The application is running on NUCLEO-F446RE\r\n";

int main(void) {
	HAL_Init();

	SystemClock_Config_HSE(SYS_CLK_FREQ_50_MHZ);

	TIM6_Init();

	HAL_GPIO_MspInit();

	while (1);

	return 0;
}

void SystemClock_Config_HSE(uint8_t clock_freq) {
	RCC_OscInitTypeDef osc_init = { 0 };
	RCC_ClkInitTypeDef clk_init = { 0 };

	uint8_t flash_latency = 0;

	// Enable Power Control clock to modify voltage regulators for 180MHz
	__HAL_RCC_PWR_CLK_ENABLE();

	osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	osc_init.HSIState = RCC_HSI_ON;
	osc_init.HSICalibrationValue = 16;
	osc_init.PLL.PLLState = RCC_PLL_ON;
	osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSI;

	clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1
			| RCC_CLOCKTYPE_PCLK2;
	clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

	switch (clock_freq) {
	case SYS_CLK_FREQ_50_MHZ: {
		osc_init.PLL.PLLM = 16;
		osc_init.PLL.PLLN = 100;
		osc_init.PLL.PLLP = RCC_PLLP_DIV2;
		osc_init.PLL.PLLR = 2;
		osc_init.PLL.PLLQ = 2;

		clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
		clk_init.APB1CLKDivider = RCC_HCLK_DIV2;
		clk_init.APB2CLKDivider = RCC_HCLK_DIV2;

		flash_latency = FLASH_LATENCY_1;

		break;
	}
	case SYS_CLK_FREQ_84_MHZ: {
		osc_init.PLL.PLLM = 16;
		osc_init.PLL.PLLN = 168;
		osc_init.PLL.PLLP = RCC_PLLP_DIV2;
		osc_init.PLL.PLLR = 2;
		osc_init.PLL.PLLQ = 2;

		clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
		clk_init.APB1CLKDivider = RCC_HCLK_DIV2;
		clk_init.APB2CLKDivider = RCC_HCLK_DIV2;

		flash_latency = FLASH_LATENCY_2;

		break;
	}
	case SYS_CLK_FREQ_120_MHZ: {
		osc_init.PLL.PLLM = 16;
		osc_init.PLL.PLLN = 240;
		osc_init.PLL.PLLP = RCC_PLLP_DIV2;
		osc_init.PLL.PLLR = 2;
		osc_init.PLL.PLLQ = 2;

		clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
		clk_init.APB1CLKDivider = RCC_HCLK_DIV4;
		clk_init.APB2CLKDivider = RCC_HCLK_DIV2;

		flash_latency = FLASH_LATENCY_3;

		break;
	}
	default: {
		break;
	}
	}

	if (HAL_RCC_OscConfig(&osc_init) != HAL_OK) {
		Error_Handler();
	}

	// CRITICAL: Activate Over-Drive mode to allow frequencies above 168 MHz
	if (clock_freq != SYS_CLK_FREQ_50_MHZ && clock_freq != SYS_CLK_FREQ_84_MHZ
			&& clock_freq != SYS_CLK_FREQ_120_MHZ) {
		if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
			Error_Handler();
		}
	}

	if (HAL_RCC_ClockConfig(&clk_init, flash_latency) != HAL_OK) {
		Error_Handler();
	}

	// SYS_Tick configuration
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

void HAL_GPIO_MspInit(void) {
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };
	gpio.Pin = GPIO_PIN_5;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(GPIOA, &gpio);
}

void TIM6_Init(void) {
	timer6.Instance = TIM6;

	timer6.Init.CounterMode = TIM_COUNTERMODE_UP;

	timer6.Init.Prescaler = 9;
	timer6.Init.Period = 50 - 1;

	if (HAL_TIM_Base_Init(&timer6) != HAL_OK) {
		Error_Handler();
	}

	if (HAL_TIM_Base_Start_IT(&timer6) != HAL_OK) {
		Error_Handler();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM6) {
		GPIOA->ODR ^= GPIO_PIN_5;
	}
}

void Error_Handler(void) {
	__disable_irq();

	while (1);
}
