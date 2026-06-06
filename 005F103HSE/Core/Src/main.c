#include "main.h"

void Clock_Config72MHz(void);
void GPIO_Init(void);
void Timer2_Init(void);
void delay_ms(uint32_t ms);

int main(void) {
	Clock_Config72MHz();
	GPIO_Init();
	Timer2_Init();

	while (1) {
		// Toggle PC13 LED
		if (GPIOC->ODR & GPIO_ODR_ODR13) {
			GPIOC->BSRR = GPIO_BSRR_BR13;
		} else {
			GPIOC->BSRR = GPIO_BSRR_BS13;
		}

		delay_ms(500);
	}

	return 0;
}

void Clock_Config72MHz(void) {
	// 1. Enable HSE
	RCC->CR |= RCC_CR_HSEON;
	while (!(RCC->CR & RCC_CR_HSERDY));

	// 2. Configure Flash Latency
	FLASH->ACR |= FLASH_ACR_LATENCY_2;

	// 3. Configure PLL: Source = HSE, Multiplier = 9 (8MHz * 9 = 72MHz)
	RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL); // Clear bits
	RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE;
	RCC->CFGR |= RCC_CFGR_PLLMULL9;

	// 4. Config Bus Prescalers
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;  // AHB Prescaler 1 = 72MHz
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // APB2 Prescaler 1 = 72MHz
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 Prescaler 2 = 36MHz

	// 5. Enable PLL and wait for it to lock
	RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY));

	// 6. Switch System Clock Source to PLL
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}

void GPIO_Init(void) {
	// Enable Port C on APB2 Peripheral
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

	// PC13 is controlled by CRH
	// Clear the 4 configuration bits
	GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);

	// Set Mode: Output mode, max speed 2MHz (01)
	// Set CNF: General purpose output push-pull (00)
	GPIOC->CRH |= (GPIO_CRH_MODE13_0);
}

void Timer2_Init(void) {
	// 1. Enable Timer 2 Clock on APB1 Bus
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	// 2. Calculate Prescaler (PSC)
	// TIM2 sits on APB1. Since APB1 prescaler is DIV2, the timer clock is doubled
	// internally by hardware back up to 72MHz.
	// To get a 10kHz internal timer tick clock 72,000,000 / 7,200 = 10,000Hz
	// Register values are 0 indexed
	TIM2->PSC = 7200 - 1;

	// 3. Set Auto-Reload Register (ARR)
	// To make the timer overflow exactly every 1ms at 10kHz
	// 10,000Hz / 1,000ms = 10 ticks.
	TIM2->ARR = 10 - 1;

	// 4. Force Update Generation to apply prescaler settings immediately
	TIM2->EGR |= TIM_EGR_UG;
}

void delay_ms(uint32_t ms) {
	// Clear update interrupt flag just in case
	TIM2->SR &= ~TIM_SR_UIF;

	// Start timer counter
	TIM2->CR1 |= TIM_CR1_CEN;

	for (uint32_t i = 0; i < ms; i++) {
		// Wait until Update Interrupt Flag (UIF) is set (indicates 1ms passed)
		while (!(TIM2->SR & TIM_SR_UIF));

		// Clear flag manually in bare-metal
		TIM2->SR &= ~TIM_SR_UIF;
	}

	// Stop Timer 2 Counter to save power when not delaying
	TIM2->CR1 &= ~TIM_CR1_CEN;
}
