#include "main.h"

void SystemClock_Config(uint8_t clock_freq);
void Timer2_Init(void);
void HAL_GPIO_MspInit(void);
void MSO_Configuration(void);
void Error_Handler(void);

TIM_HandleTypeDef htimer2;

int main(void) {
	HAL_Init();

	SystemClock_Config(SYS_CLK_FREQ_50_MHZ);

	Timer2_Init();

	MSO_Configuration();

	HAL_GPIO_MspInit();

	while (1);

	return 0;
}

void SystemClock_Config(uint8_t clock_freq) {
	RCC_OscInitTypeDef osc_init = { 0 };
	RCC_ClkInitTypeDef clk_init = { 0 };

	uint8_t flash_latency = 0;

	// Enable Power Control clock to modify voltage regulators for 180MHz
	__HAL_RCC_PWR_CLK_ENABLE();

	osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSE;
	osc_init.HSIState = RCC_HSI_ON;
	osc_init.LSEState = RCC_LSE_ON;
	osc_init.HSICalibrationValue = 0;
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

void Timer2_Init(void) {
	TIM_IC_InitTypeDef timer2IC_config;

	htimer2.Instance = TIM2;
	htimer2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htimer2.Init.Period = 0xFFFFFFFF;
	htimer2.Init.Prescaler = 1;

	if (HAL_TIM_IC_Init(&htimer2) != HAL_OK) {
		Error_Handler();
	}

	timer2IC_config.ICFilter = 0;
	timer2IC_config.ICPolarity = TIM_ICPOLARITY_RISING;
	timer2IC_config.ICPrescaler = TIM_ICPSC_DIV1;
	timer2IC_config.ICSelection = TIM_ICSELECTION_DIRECTTI;

	if (HAL_TIM_IC_ConfigChannel(&htimer2, &timer2IC_config, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}

	if (HAL_TIM_IC_Start_IT(&htimer2, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}
}

void MSO_Configuration(void) {
	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1);
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

void Error_Handler(void) {
	__disable_irq();

	while (1);
}
