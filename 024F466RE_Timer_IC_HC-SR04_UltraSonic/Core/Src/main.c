#include "main.h"

void SystemClock_Config(uint8_t clock_freq);
void DWT_Init(void);
void delay_us(uint32_t us);
void Timer2_Init(void);
void UART2_Init(void);
void HAL_GPIO_MspInit(void);
void MSO_Configuration(void);
void Error_Handler(void);

UART_HandleTypeDef huart2;
TIM_HandleTypeDef htimer2;

volatile uint8_t echo_received = 0;
volatile float distance_cm = 0;
volatile uint32_t capture_rise = 0;
volatile uint32_t capture_fall = 0;

int main(void) {
	char usr_msg[100];

	HAL_Init();

	SystemClock_Config(SYS_CLK_FREQ_84_MHZ);

	Timer2_Init();

	MSO_Configuration();

	UART2_Init();
//	HAL_GPIO_MspInit();

	DWT_Init();

	if (HAL_TIM_IC_Start_IT(&htimer2, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}

	while (1) {
		echo_received = 0;

		// Send 10us trigger pulse
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
		delay_us(10);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

		// Wait for echo with 30ms timeout
		uint32_t timeout = HAL_GetTick() + 30;
		while (echo_received == 0 && (HAL_GetTick() < timeout));

		if (echo_received) {
			snprintf(usr_msg, sizeof(usr_msg), "Distance = %.2f\r\n", distance_cm);
			HAL_UART_Transmit(&huart2, (uint8_t*) usr_msg, strlen(usr_msg), HAL_MAX_DELAY);
		} else {
			snprintf(usr_msg, sizeof(usr_msg), "Timeout - out of range (> 400cm)!\r\n");
			HAL_UART_Transmit(&huart2, (uint8_t*) usr_msg, strlen(usr_msg), HAL_MAX_DELAY);
		}

		HAL_Delay(100); // Read every 100ms
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

void DWT_Init(void) {
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(uint32_t us) {
	uint32_t ticks = us * (SystemCoreClock / 1000000); // Convert us to CPU cycles
	uint32_t start = DWT->CYCCNT;
	while ((DWT->CYCCNT - start) < ticks);
}

void Timer2_Init(void) {
	TIM_IC_InitTypeDef timer2IC_config = { 0 };

	htimer2.Instance = TIM2;
	htimer2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htimer2.Init.Period = 0xFFFF; // 65.5 ms max
	htimer2.Init.Prescaler = (SystemCoreClock / 1000000) - 1; // Auto: 83 for 84MHz, 49 for 50MHz
	htimer2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; // Optional here
	htimer2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; // Optional here

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
	if (htim->Instance == TIM2) {
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			// Check polarity 0 = rising 1 = falling (CC1P bit in CCER)
			if ((TIM2->CCER & TIM_CCER_CC1P) == 0) {
				// Rising Edge
				capture_rise = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
				__HAL_TIM_SetCounter(htim, 0); // Reset timer for direct read

				// Switch to falling edge
				TIM2->CCER |= TIM_CCER_CC1P;
			} else {
				// Rising Edge
				capture_fall = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

				// Pulse width in microseconds (since counter started at 0)
				uint32_t pulse_width_us = capture_fall;

				// Distance = pulse_width_us / 58.0 (works for any clock as long as timer ticks 1us)
				distance_cm = (float) pulse_width_us / 58.0f;

				echo_received = 1;

				// Switch back to rising edge for next measurement
				TIM2->CCER &= ~TIM_CCER_CC1P;
			}
		}
	}
}

void Error_Handler(void) {
	__disable_irq();

	while (1);
}
