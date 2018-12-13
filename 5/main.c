#include <stm32f3xx.h>

  
void ClockInit (void)
{

 RCC->CR = 0x000000083;         
 RCC->CFGR = 0x00000000;      
 FLASH->ACR = 0x00000011;

                            
 RCC->CR |= 0x00000001;             
 while(!(RCC->CR &0x00000002));     

                           
 RCC->CFGR &= ~0x00018000;
                            
 RCC->CFGR |= 0x000C0000;      

 RCC->CR |= 0x01000000;       
 while(!(RCC->CR &0x02000000));

 RCC->CFGR |= 0x00000002;      
 RCC->CFGR |= 0x00400000;       

 RCC->CFGR &= ~0x000000F0;     

 RCC->CFGR &= ~0x00000700;      

 RCC->CFGR &= ~0x00003800;      
 RCC->AHBENR = 0x00000014;      
 RCC->APB2ENR = 0x00000001;     
 RCC->APB1ENR = 0x00000001;     

 RCC->APB2RSTR = 0x00000000;    
 RCC->APB1RSTR = 0x00000000;

}


void GpioInit (void)
{
              
    RCC->AHBENR |= 0x00240000;      
    GPIOB->MODER |= 0xF0000000;    
    GPIOB->MODER |= 0x08000000;     
    GPIOB->AFR[1] |= 0x00600000;    

    GPIOE->MODER |= 0x55550000;     
	
	
	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_SetPriority(TIM2_IRQn,4);
	
	
    GPIOE->OTYPER |= 0x00000000;   
    GPIOE->PUPDR |= 0x00000000;    

}

 

void Tim2Init (void)
{
    RCC->APB1ENR = 0x00000001;  

                            
    TIM2->PSC = 19;            
    TIM2->CNT = 0xFFFFFFFF;     

    TIM2->CR1 = 0x00000010;     
    TIM2->EGR = 0x00000001;     

    TIM2->CR1 |= 0x00000001;    
}

              
void    delay_mks (int32_t value)
{
    int32_t time;
    time = TIM2->CNT;
    while ( (time - TIM2->CNT) < value );
}

               

void    AdcInit (void)
{
    RCC->AHBENR |= 0x20000000;      
    RCC->CFGR2 = 0x00003000;       

    ADC4->CR = 0x00000001;                  
    while ( !(ADC4->ISR & 0x00000001) );    

    ADC4->SQR1 = 0x00000100;               
    ADC4->SMPR1 = 0x00003018;   

    ADC3_4_COMMON->CCR = 0x00000000;    
    ADC4->CFGR = 0x00000000;                

}


void Tim1PWMInit (void)
{
    RCC->APB2ENR |= 0x00000800;    
    TIM1->CR1 &= ~TIM_CR1_CMS;
		TIM1->CR1 |= 0x0081;   
	
    TIM1->CCMR1 = 0x00000060;   
    TIM1->EGR = 0x00000001;     

    TIM1->CCER = 0x00000005;    
                     
    TIM1->PSC = 4;              
    TIM1->ARR = 1000;          
    TIM1->CCR1 = 900;          

    TIM1->BDTR = 0x00008000;    
    TIM1->CR1 |= 0x00000001;

}
//0
int mask = 0;

int value = 0;


void start (void){
	  int ADC_DR;
		int i;

	
		ADC4->CR |= 0x00000004;                 
    while ( !(ADC4->ISR & 0x0000000C) );   
		ADC_DR = ADC4->DR;
	
		// simple static
		value = (0.9 * value) + (0.1 * ADC_DR);
		mask = 0;
		switch (value/480) {
			case 9:  mask += 0x0100<<9;
			case 8:  mask += 0x0100<<8;
			case 7:  mask += 0x0100<<7;
			case 6:  mask += 0x0100<<6;
			case 5:  mask += 0x0100<<5;
			case 4:  mask += 0x0100<<4;
			case 3:  mask += 0x0100<<3;
			case 2:  mask += 0x0100<<2;
			case 1:  mask += 0x0100<<1;
			case 0:  mask += 0x0100;
			break;
			default: mask = 0;
		}
		//  X^3
		TIM1->CCR1 = (value/400) * (value/400)* (value/400);
	
			if (TIM1->CNT < 50){
				GPIOE->ODR = mask;		
			} 
			
			if (TIM1->CNT >= TIM1->CCR1){
				GPIOE->ODR = 0;
				
			}

	// magic wtf code
    TIM1->PSC = (ADC_DR >> 4) + 4;       
    ADC4->ISR &= 0x0000000E;   
}

void ADC1Init(void){
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

int main()
{
    ClockInit ();
    GpioInit ();
    Tim2Init ();
    AdcInit ();
    Tim1PWMInit ();

		while(1){
				start();
		}
}







