#include <stm32f3xx.h>

#define MAN_PLLMUL_VALUE 0x000C0000 // 5
#define MAN_PLL_ON 0x01000000
#define MAN_PLLRDY_ON 0x02000000

uint16_t diod;
uint16_t n = 5000;
uint16_t lamp = 0;
uint16_t counter;
uint16_t t;

void setPLLMUL(const uint32_t value){
	if (RCC->CR & MAN_PLL_ON){  // if PLL is on turn off PLL
		RCC->CR &= !MAN_PLL_ON; 
	}
	if (!(RCC->CR & MAN_PLL_ON)){ // if PLL is off
		while (RCC->CR & MAN_PLLRDY_ON);
		if (!(RCC->CR & MAN_PLLRDY_ON)){ // if PLLRDY == 0
			RCC->CFGR &= !0x003C0000; // reset PLLMUL    
			RCC->CFGR |= value;  // set PLLMUL
			RCC->CR |= MAN_PLL_ON;  // turn on PLL
		}
	}
}
void init_timing() {

	RCC -> AHBENR |= 0x00200000;
	RCC -> APB2ENR |= 0x800;
	TIM1 -> CR1 |= 0x11;
	TIM1 -> PSC = 2000;
	TIM1 -> ARR |= 0;
	diod = 0x100;
	RCC -> CFGR |= 0x000C0002;
	GPIOE -> MODER |= 0x55555555;
};


void led_on() {
	if(lamp++ > 7) {
		diod = 0x100;
		lamp = 0;
	}
	GPIOE -> ODR = diod;
	diod = diod<<1;
}

void wait() {
	t = TIM1 -> CNT;
	while((t - TIM1 -> CNT) < n) {
	}
}

void led_off() {GPIOE -> ODR = 0x00000000;}

int main(void) {
	setPLLMUL(MAN_PLLMUL_VALUE);
	init_timing();
	while(1) {
		wait();
		led_on();
	}
}
