#include <stm32f3xx.h>

int pin = 0x0100;
int runb;

void Synhronize(){

	RCC->CR |= 0x00000001;					
	while(!(RCC->CR&0x00000002));		
	
	RCC->CFGR = 0x0518000A;					
	
	RCC->CR |= 0x01000000;				//PLL on
	while(!(RCC->CR&0x02000000));
}	

void Delay(int del)
{
	while(del--);
}
		


int main(){
	int maskE;
	maskE	= 0x00600000;
	Synhronize();
	
	RCC->APB2ENR |=0x00000800;	
	RCC->AHBENR |= maskE;				// I/O ports F,E clock enable
	GPIOE->MODER= 0xAAAA0000;
	GPIOF->PUPDR= 0x55555555; 
	
	GPIOE ->AFR[1] =  0x00002222;
	
	TIM1->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
	TIM1->PSC = 31999;
	TIM1->ARR = 2000;
	TIM1->CCR1 = 1000;
	TIM1->CCER |= 0x00000005;
	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->CNT = 0; 
	TIM1->CR1 &= ~TIM_CR1_CMS;
	TIM1->CR1 |= 0x0081;
	

	while(1){
	
		Delay(50);
		if (!(GPIOF->IDR & 0x0010 ))
		{
			if(TIM1->CCR1 <= 1800)
				TIM1->CCR1 += 1;			
		}

		if (!(GPIOF->IDR & 0x0040 ))
		{
			if(TIM1->CCR1 >= 300)
				TIM1->CCR1 -= 1;
		}		
	}

	return 0;
}
