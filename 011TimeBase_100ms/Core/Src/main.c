#include "main.h"

void SystemClock_Config(void);
void GPIO_Init(void);
void TIM11_Init(void);
void Error_Handler(void);

TIM_HandleTypeDef htimer11;

int main(void) {
	HAL_Init();
	SystemClock_Config();
	GPIO_Init();
	TIM11_Init();

	// Let's start timer
	HAL_TIM_Base_Start(&htimer11);

	while (1) {
		// Loop until the update event flag is set
		while (!(TIM11->SR & TIM_SR_UIF));

		/* The required time delay has been elapsed */
		/* User code can be executed */
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		TIM11->SR |= 0 << 1;
	}

	return 0;
}

void SystemClock_Config(void) {
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
	GPIO_InitTypeDef ledgpio;

	ledgpio.Pin = GPIO_PIN_13;
	ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
	ledgpio.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(GPIOC, &ledgpio);
}

void TIM11_Init(void) {
	htimer11.Instance = TIM11;
// CounterMode is always default for Basic Timers and cannot be changed
	htimer11.Init.CounterMode = TIM_COUNTERMODE_UP;
// CNT_CLK = TIMxCLK from RCC
// CNT_CLK = TIMx_CLK / (prescaler + 1)
// HSE CNT_CLK = 25 / (1 + 1) = 12
// Range 0x00 to 0xFFFF (16bit)
	htimer11.Init.Prescaler = 999;
// if value 0 timer won't start
// we need -1 to get the exact count as it will take +1 more
	htimer11.Init.Period = 10000 - 1;

	if (HAL_TIM_Base_Init(&htimer11) != HAL_OK) {
		Error_Handler();
	}
}

void Error_Handler(void) {
	__disable_irq();

	while (1);
}
