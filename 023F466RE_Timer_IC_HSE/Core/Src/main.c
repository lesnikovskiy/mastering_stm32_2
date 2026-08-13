#include "main.h"

void SystemClock_Config(uint8_t clock_freq);
void Timer2_Init(void);
void TIM6_Init(void);
void UART2_Init(void);
void HAL_GPIO_MspInit(void);
void MSO_Configuration(void);
void Error_Handler(void);

UART_HandleTypeDef huart2;
TIM_HandleTypeDef htimer2;
TIM_HandleTypeDef htimer6;

uint32_t input_captures[2] = { 0 };
uint8_t count = 1;
uint8_t is_capture_done = FALSE;

int main(void) {
	uint32_t capture_difference = 0;
	double timer2_cnt_freq = 0;
	double timer2_cnt_res = 0;
	double user_signal_time_period = 0;
	double user_signal_freq = 0;

	char usr_msg[100];

	HAL_Init();

	SystemClock_Config(SYS_CLK_FREQ_50_MHZ);

	Timer2_Init();
	TIM6_Init();

	MSO_Configuration();

	UART2_Init();
	HAL_GPIO_MspInit();

	if (HAL_TIM_Base_Start_IT(&htimer6) != HAL_OK) {
		Error_Handler();
	}

	if (HAL_TIM_IC_Start_IT(&htimer2, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}

	while (1) {
		if (is_capture_done) {
			if (input_captures[1] > input_captures[0]) {
				capture_difference = input_captures[1] - input_captures[0];
			} else {
				capture_difference = (0xFFFFFFFF - input_captures[0]) + input_captures[1];
			}

			timer2_cnt_freq = (HAL_RCC_GetPCLK1Freq() * 2) / (htimer2.Init.Prescaler + 1);
			timer2_cnt_res = 1 / timer2_cnt_freq;
			user_signal_time_period = capture_difference * timer2_cnt_res;
			user_signal_freq = 1 / user_signal_time_period;

			snprintf(usr_msg, sizeof(usr_msg), "Frequency of the signal applied = %f\r\n",
					user_signal_freq);

			HAL_UART_Transmit(&huart2, (uint8_t*) usr_msg, strlen(usr_msg),
			HAL_MAX_DELAY);

			is_capture_done = FALSE;
		}
	}

	return 0;
}

void SystemClock_Config(uint8_t clock_freq) {
	RCC_OscInitTypeDef osc_init = { 0 };
	RCC_ClkInitTypeDef clk_init = { 0 };

	uint8_t flash_latency = 0;

	// Enable Power Control clock to modify voltage regulators for 180MHz
	__HAL_RCC_PWR_CLK_ENABLE();

	osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE
			| RCC_OSCILLATORTYPE_LSE;
	osc_init.HSIState = RCC_HSI_ON;
	osc_init.HSEState = RCC_HSE_ON;
	osc_init.LSEState = RCC_LSE_ON;
	osc_init.PLL.PLLState = RCC_PLL_ON;
	osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE;

	clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1
			| RCC_CLOCKTYPE_PCLK2;
	clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

	switch (clock_freq) {
		case SYS_CLK_FREQ_50_MHZ: {
			osc_init.PLL.PLLM = 8;
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
			osc_init.PLL.PLLM = 8;
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
			osc_init.PLL.PLLM = 8;
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
}

void TIM6_Init(void) {
	htimer6.Instance = TIM6;

	htimer6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htimer6.Init.Prescaler = 9;
	htimer6.Init.Period = 50 - 1;

	if (HAL_TIM_Base_Init(&htimer6) != HAL_OK) {
		Error_Handler();
	}
}

void MSO_Configuration(void) {
	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1);
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

void HAL_GPIO_MspInit(void) {
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };
	gpio.Pin = GPIO_PIN_5;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(GPIOA, &gpio);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (!is_capture_done) {
		if (count == 1) {
			input_captures[0] = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1);
			count++;
		} else if (count == 2) {
			input_captures[1] = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1);
			count = 1;
			is_capture_done = TRUE;
		}
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
