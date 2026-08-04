#include "main.h"

void HAL_MspInit(void) {
	// Setup priority grouping of the ARM Cortex MX processor
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	// Enable the required system exceptions of the ARM Cortex MX processor
	SCB->SHCSR |= (0x7 << 16); // Enable usage fault, memory fault and bus fault system exceptions

	// Configure priority for the system exceptions
	HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
	HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
	HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htimer) {
	if (htimer->Instance == TIM11) {
		// Enable the clock for the TIM11 peripheral
		__HAL_RCC_TIM11_CLK_ENABLE();

		// Enable the IRQ of TIM11
		HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);

		// Setup the priority for TIM1_TRG_COM_TIM11_IRQn
		HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 15, 0);
	}

}
