#include <string.h>
#include "stm32l476xx.h"
#include "main.h"

#define MAX_BUFFER_SIZE (100UL)
#define TRUE 			(1U)
#define FALSE 			(0)

void SystemClockConfig(void);
void UART2_Init(void);
void Error_Handler(void);
void convert_buffer_to_capital(uint8_t *pData, uint32_t size);

UART_HandleTypeDef huart2;

char *user_data = "The application is running\r\n";

uint8_t received_data;
uint8_t data_buffer[MAX_BUFFER_SIZE] = { 0 };
uint32_t count = 0;
uint8_t reception_complete = FALSE;

int main(void) {
	HAL_Init();
	SystemClockConfig();

	UART2_Init();

	HAL_UART_Transmit(&huart2, (uint8_t*) user_data, strlen(user_data), HAL_MAX_DELAY);

	while (reception_complete != TRUE) {
		HAL_UART_Receive_IT(&huart2, &received_data, 1);
	}

	while (1);

	return 0;
}

void SystemClockConfig(void) {
	// TODO: Use it later
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (received_data == '\r' || count >= MAX_BUFFER_SIZE) {
		data_buffer[count++] = '\n';

		reception_complete = TRUE;

		convert_buffer_to_capital(data_buffer, count);
		HAL_UART_Transmit(&huart2, (uint8_t*) data_buffer, count, HAL_MAX_DELAY);
	}

	data_buffer[count++] = received_data;
}

void Error_Handler(void) {
	// blink red LED
}

void convert_buffer_to_capital(uint8_t *pData, uint32_t size) {
	if (pData == NULL)
		return;

	for (uint32_t i = 0; i < size; i++) {
		if (pData[i] >= 'a' && pData[i] <= 'z') {
			pData[i] -= ('a' - 'A');
		}
	}
}
