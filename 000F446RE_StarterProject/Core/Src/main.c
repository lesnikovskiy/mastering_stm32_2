#include "main.h"

void SystemClock_Config(void);
void HAL_GPIO_MspInit(void);
void UART2_Init(void);
void TIM6_Init(void);
void Error_Handler(void);

UART_HandleTypeDef huart2;
TIM_HandleTypeDef timer6;

char *greeting_message = "The application is running on NUCLEO-F446RE\r\n";

int main(void) {
	HAL_Init();

	SystemClock_Config();

	HAL_GPIO_MspInit();

	UART2_Init();

	TIM6_Init();

	HAL_UART_Transmit(&huart2, (uint8_t*) greeting_message, strlen(greeting_message),
	HAL_MAX_DELAY);

	char msg[100];

	snprintf(msg, sizeof(msg), "SYSCLK : %luHz\r\n", HAL_RCC_GetSysClockFreq());
	HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

	snprintf(msg, sizeof(msg), "HLCLK : %luHz\r\n", HAL_RCC_GetHCLKFreq());
	HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

	snprintf(msg, sizeof(msg), "PCLK1 : %luHz\r\n", HAL_RCC_GetPCLK1Freq());
	HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

	snprintf(msg, sizeof(msg), "PCLK2 : %luHz\r\n", HAL_RCC_GetPCLK2Freq());
	HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

	while (1);

	return 0;
}

void SystemClock_Config(void) {
	RCC_OscInitTypeDef osc_init = { 0 };
	RCC_ClkInitTypeDef clk_init = { 0 };

	osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	osc_init.HSEState = RCC_HSE_ON;
	osc_init.PLL.PLLState = RCC_PLL_ON;
	osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	osc_init.PLL.PLLM = 8;
	osc_init.PLL.PLLN = 100;
	osc_init.PLL.PLLP = RCC_PLLP_DIV2;
	osc_init.PLL.PLLQ = 2;
	if (HAL_RCC_OscConfig(&osc_init) != HAL_OK) {
		Error_Handler();
	}

	clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1
			| RCC_CLOCKTYPE_PCLK2;
	clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1; // HCLK = 50MHz
	clk_init.APB1CLKDivider = RCC_HCLK_DIV2;  // PCLK1 = 25MHz
	clk_init.APB2CLKDivider = RCC_HCLK_DIV1;  // PLCK2 = 50MHz
	if (HAL_RCC_ClockConfig(&clk_init, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}

	__HAL_RCC_HSI_DISABLE();

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

void UART2_Init(void) {
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
}

void TIM6_Init(void) {
	timer6.Instance = TIM6;

	timer6.Init.CounterMode = TIM_COUNTERMODE_UP;
	timer6.Init.Period = 50000 - 1;
	timer6.Init.Prescaler = 999 - 1;

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
