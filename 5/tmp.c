#include "stm32f3xx.h"

#define MAN_PLLMUL_VALUE 0x000C0000 // 5
#define MAN_PLL_ON 0x01000000
#define MAN_PLLRDY_ON 0x02000000
#define MAN_RCC_GLIOE_ON 0x00200000
#define MAN_GPIOE_MODER_RESET 0xFFFFFCFF
#define MAN_GPIOE_MODER_OUT 0x55555555
#define MAN_GPIOE_MODER_AF 0xAAAA0000

#define MAN_TIM_ARR 100   // 50 ms
#define MAN_PW_MIN (MAN_TIM_ARR / 5)
#define MAN_PW_MIDDLE (MAN_TIM_ARR / 2)
#define MAX_PW_MAX MAN_TIM_ARR

unsigned int PW[3] = {MAN_PW_MIN, MAN_PW_MIDDLE, MAN_TIM_ARR};
unsigned int current_PW = 0;
	
int pin = 0x0100;
float runb;


void Delay(int del){
	while(del--);
}
void ledOff(){
	GPIOE->ODR = 0;
}


void ledBlue1(){
	GPIOE->ODR = 0x0100;
}
void ledBlue2(){
	GPIOE->ODR = 0x1100;
}

void ledRed1(){
	GPIOE->ODR = 0x0200;
}
void ledRed2(){
	GPIOE->ODR = 0x2200;
}


//0
int min = 0x5000;//1 blue

int low = 0x6000;//2 blue
int mid = 0x7000;//1 red

int max = 0x8500;//2 red


int value = 0;


void runTimer();

char irqCounter = 0;
			
__irq void TIM2_IRQHandler (void){ 
	TIM1->CCR1 = PW[++current_PW % 3];
	TIM1->CCR3 = PW[current_PW % 3];
	irqCounter = 0;
	TIM2->SR &= 0xFFFE;
	// TIM2->SR &=~ TIM_SR_UIF;
	
	ADC1->CR |= 0x00000004;
	
	//while(!(ADC1->ISR == 0x000C));
	
	value = (0.9 * value) + (0.1 * ADC1->DR);
	

	if(value <= min)
		ledOff();

	if(value > min && value <= low)
		ledBlue1();
	
	if(value > low && value <= mid)
		ledBlue2();
	
	if(value > mid && value < max)
		ledRed1();
	if(value > max)
		ledRed2();
}

int maskE	= 0x10600000;
void init5(){
	/*RCC->APB1ENR |=0x0000001;
	RCC->CFGR2 |= RCC_CFGR2_ADCPRE12_DIV8;	
	RCC->AHBENR |= maskE;				
	GPIOE->MODER= 0x55550000;
	
	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_SetPriority(TIM2_IRQn,4);
	
	TIM2->PSC = 32999;
	TIM2->ARR = 100;
	TIM2->CR1 |= 0x0001;
	TIM2->DIER |= TIM_DIER_UIE;
	TIM2->EGR |= 0x0001;
	*/
		
	ADC1->SMPR2 |= 0x03000000;
	ADC1->CFGR |= 0x40811020;
	ADC1->SQR1 = 0x00000400;
	ADC1->SQR2 = 0;
	ADC1->SQR3 = 0;
	ADC1->SQR4 = 0;
	ADC1_2_COMMON->CCR |= 0x00800000;
	ADC1->IER = 0;
	ADC1->CR |= 0x00000001;
}

void EXTI0_IRQHandler(void){
	++irqCounter;
	if (irqCounter == 1){
		runTimer();
		EXTI->PR |= 0x00000001;
	}
}

void runTimer(){
	TIM2->CR1 |= 0x00000001;
}
			
void setPLLMUL(const uint32_t value){
	/* value: RCC_CFGR = RCC_CFGR | value */
	if (RCC->CR & MAN_PLL_ON){  // if PLL is on turn off PLL
		RCC->CR &= !MAN_PLL_ON;
	}
	if (!(RCC->CR & MAN_PLL_ON)){ // if PLL is off
		while (RCC->CR & MAN_PLLRDY_ON);
		if (!(RCC->CR & MAN_PLLRDY_ON)){ // if PLLRDY == 0
			RCC->CFGR &= !0x003C0000; // reset PLLMUL    RCC-> CFGR &= ~0x003C0000;
			RCC->CFGR |= value;  // set PLLMUL
			RCC->CR |= MAN_PLL_ON;  // turn on PLL
			while (!(RCC->CR & MAN_PLLRDY_ON));
			RCC->CFGR &= 0xFFFFFFFC;
			RCC->CFGR |= 0x00000002;
		}
	}
}
void Synhronize(){

	RCC->CR |= 0x00000001;					//HSI on
	while(!(RCC->CR&0x00000002));		
	
	RCC->CFGR = 0x0518000A;					
	
	RCC->CR |= 0x01000000;				//PLL on
	while(!(RCC->CR&0x02000000));	
}	
void ledInit(){
	RCC->AHBENR |= MAN_RCC_GLIOE_ON;  // connect port E to RCC	
	GPIOE->MODER &= MAN_GPIOE_MODER_RESET;
	GPIOE->MODER |= MAN_GPIOE_MODER_AF;
	GPIOE->AFR[1] = 0x22222222;
}

void buttonInit(){
	RCC->AHBENR |= 0x00020000; // podkl. k taktirovaniyu
	GPIOA->MODER &= 0xFFFFFFFC; // rezhim 'input'
	SYSCFG->EXTICR[0] &= 0xFFFFFFF8;
	EXTI->IMR |= 0x1F800001; // vkl. poluchenie zaprosov po liniyam 0
	EXTI->RTSR |= 0x00000001; // rising trigger
	NVIC->ISER[0] |= 0x00000040; // razreshit prerivaniya ot EXTI 0
}

// tim clock: 100 kHz
void timer1Init(){
  RCC->APB2ENR |= 0x00000800;  // vkluchit' taktirovanie TIM1
	TIM1->BDTR &= !0x000003FF;  // LOCK, DTG
	TIM1->BDTR |= 0x00008000; // MOE
	TIM1->ARR = (unsigned int)MAN_TIM_ARR;
	TIM1->PSC = (unsigned int)200; // delitel'  200
	TIM1->CCMR1 &= !0x0101FFFF;
	TIM1->CCMR1 |= 0x00006868; // OC2M, OC2PE, CC2S
	TIM1->CCMR2 &= !0x0101FFFF;
	TIM1->CCMR2 |= 0x00006868; // OC2M, OC2PE, CC2S
	current_PW = 2;
	TIM1->CCR1 = PW[current_PW];
	TIM1->CCR3 = PW[current_PW];
	
	TIM1->CCER &= 0xFFCC4000; // CCxP, CCxNP
	TIM1->CCER |= 0x00001111; // CCxE
 	TIM1->CR1 &= 0xFFE0; // DIR
	TIM1->CR1 |= 0x0081; // CEN, ARPE
	TIM1->EGR |= 0x0001; // generate update event
}

void timer2Init(){
  RCC->APB1ENR |= 0x00000001;  // vkluchit' taktirovanie TIM2
	TIM2->ARR = (unsigned int)5000;  // 50 ms
	TIM2->PSC = (unsigned int)200; // delitel'  200
	TIM2->DIER |= 0x00000001; // razreshit' generaciu prerivaniy
	NVIC->ISER[0] |= 0x10000000; // razreshit' prerivaniya ot TIM2
	TIM2->CR1 &= !0x0000007A;
	TIM2->CR1 |= 0x0000000C; // OPM and ...
}

int main(void) {
	///setPLLMUL(MAN_PLLMUL_VALUE);
	Synhronize();
	
	//init5();
	ledInit();
  buttonInit();
	timer1Init();
	timer2Init();
	
	runb = 0;

	while(1);
}