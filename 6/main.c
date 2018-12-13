#include <stm32f3xx.h>

int pin = 0x0100;
float runb;

void Synhronize(){

	RCC->CR |= 0x00000001;					//HSI on
	while(!(RCC->CR&0x00000002));		
	
	RCC->CFGR = 0x0518000A;					
	
	RCC->CR |= 0x01000000;				//PLL on
	while(!(RCC->CR&0x02000000));	
}	

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

__irq void TIM2_IRQHandler(void){
	TIM2->SR &=~ TIM_SR_UIF;
	
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
	
	
	//GPIOE->ODR = 0x4400 | 0x8800;
	


int maskE	= 0x10600000;
void init5(){
		RCC->APB1ENR |=0x0000001;
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


void initN5(){
	RCC->APB2ENR |=0x00000800;	
	RCC->AHBENR |= maskE;				// I/O ports F,E clock enable
	GPIOE->MODER= 0xAAAA0000;
	GPIOF->PUPDR= 0x55555555; 
	
	GPIOE ->AFR[1] =  0x00002222;
	
	
	TIM1->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
	TIM1->PSC = 479;
	TIM1->ARR = 150;
	TIM1->CCR1 = 145;
	TIM1->CCER |= 0x00000005;
	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->CNT = 0; 
	TIM1->CR1 &= ~TIM_CR1_CMS;
	TIM1->CR1 |= 0x0081;
}



int main(){
	runb = 0;
	Synhronize();
	
	init5();
	


	while(1){}
	return 0;
}