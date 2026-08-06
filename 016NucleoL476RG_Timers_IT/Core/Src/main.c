#include "main.h"

void SystemClock_Config(void);
void GPIO_Init(void);
void TIM6_Init(void);
void Error_Handler(void);

TIM_HandleTypeDef htimer6;

int main(void) {
	HAL_Init();

	SystemClock_Config();

	GPIO_Init();
	TIM6_Init();

	HAL_TIM_Base_Start_IT(&htimer6);

	while (1);

	return 0;
}

void SystemClock_Config(void) {
	RCC_OscInitTypeDef osc_init = { 0 };
	RCC_ClkInitTypeDef clk_init = { 0 };

	// This is critical for L Series
	__HAL_RCC_PWR_CLK_ENABLE();
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
		Error_Handler();
	}

	osc_init.OscillatorType = RCC_OSCILLATORTYPE_MSI | RCC_OSCILLATORTYPE_LSE;
	osc_init.HSIState = RCC_HSI_OFF;
	osc_init.HSI48State = RCC_HSI48_OFF;
	osc_init.LSEState = RCC_LSE_ON;
	osc_init.MSIState = RCC_MSI_ON;
	osc_init.MSICalibrationValue = 0;
	osc_init.MSIClockRange = RCC_MSIRANGE_6; // 4MHz
	osc_init.PLL.PLLState = RCC_PLL_ON;
	osc_init.PLL.PLLSource = RCC_PLLSOURCE_MSI;
	osc_init.PLL.PLLM = 1;
	osc_init.PLL.PLLN = 40;
	osc_init.PLL.PLLR = RCC_PLLR_DIV2;
	osc_init.PLL.PLLQ = RCC_PLLQ_DIV2;
	osc_init.PLL.PLLP = RCC_PLLP_DIV7;

	if (HAL_RCC_OscConfig(&osc_init) != HAL_OK) {
		Error_Handler();
	}

	HAL_RCCEx_EnableMSIPLLMode();

	clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1
			| RCC_CLOCKTYPE_PCLK2;
	clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
	clk_init.APB1CLKDivider = RCC_HCLK_DIV1;
	clk_init.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&clk_init, FLASH_LATENCY_4) != HAL_OK) {
		Error_Handler();
	}
}

void GPIO_Init(void) {
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef ledgpio;
	ledgpio.Pin = GPIO_PIN_5;
	ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
	ledgpio.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(GPIOA, &ledgpio);
}

void TIM6_Init(void) {
	htimer6.Instance = TIM6;

	htimer6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htimer6.Init.Prescaler = 8000 - 1;
	htimer6.Init.Period = 250 - 1;

	if (HAL_TIM_Base_Init(&htimer6) != HAL_OK) {
		Error_Handler();
	}
}

void TIM6_DAC_IRQHandler(void) {
	// HAL cleans the UIF flag automatically
	HAL_TIM_IRQHandler(&htimer6);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM6) {
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	}
}

void Error_Handler(void) {
	__disable_irq();

	while (1);
}
