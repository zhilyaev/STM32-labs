#include <stm32f3xx.h>

int pin = 0x4400;
int runb;

void Synhronize(){

	RCC->CR |= 0x00000001;					//HSI on
	while(!(RCC->CR&0x00000002));		//HSI ready
	
	RCC->CFGR = 0x0518000A;					// 0000 0101 0001 1000 0000 0000 0000 1010
	
	RCC->CR |= 0x01000000;				//PLL on
	while(!(RCC->CR&0x02000000));	// PLL ready
}	
#define MAN_PLLMUL_VALUE 0x000C0000 // 5
#define MAN_PLL_ON 0x01000000
#define MAN_PLLRDY_ON 0x02000000
#define MAN_RCC_GLIOE_ON 0x00200000
#define MAN_GPIOE_MODER_RESET 0xFFFFFCFF
#define MAN_GPIOE_MODER_OUT 0x55555555

#define MAN_LED_BLUE1 0x00000100
#define MAN_LED_BLUE2 0x00001000
#define MAN_LED_RED1 0x00000200
#define MAN_LED_RED2 0x00002000
#define MAN_LED_GREEN1 0x00000800
#define MAN_LED_GREEN2 0x00008000
#define MAN_LED_ORANGE1 0x00000400
#define MAN_LED_ORANGE2 0x00004000

const uint32_t RESET_VALUE = 200000;
uint32_t LED_ON_VALUE;
uint32_t counter;
char colorNumber;
uint32_t ledOrder[15] = { 
	MAN_LED_RED1, 
	MAN_LED_RED1 | MAN_LED_ORANGE1,
	MAN_LED_RED2 | MAN_LED_ORANGE2
};
void Delay(int del)
{
	while(del--);
}
int t = 0x1100;
__irq void TIM2_IRQHandler(void){
	TIM2->SR &=~ TIM_SR_UIF;
	if(GPIOE->ODR == t)
		GPIOE->ODR = pin;
	else
		GPIOE->ODR = t;
}
void ledOn(const uint32_t ledSet) {
	GPIOE->ODR &= 0;
	GPIOE->ODR |= ledSet;
}
void ledOff() {
	GPIOE->ODR &= 0;
}
int indexLed;
int main(){
	int maskE;
	maskE	= 0x00600000;
	
	Synhronize();
	
	RCC->APB1ENR |=0x00000001;	// TIM2 timer clock enable 
	RCC->AHBENR |= maskE;				// I/O ports F,E clock enable
  
	GPIOE->MODER= 0x55550000;
	GPIOF->PUPDR= 0x55555555; 
	
	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_SetPriority(TIM2_IRQn,4);
	
	TIM2->PSC = 31999;
	TIM2->ARR = 800;
	TIM2->CR1 |= 0x0001;// tim up, tim on
	TIM2->DIER |= TIM_DIER_UIE;// allow interrupt
	TIM2->EGR |= 0x0001;
	
	runb = 0;
	indexLed = 0;
	
	SYSCFG->EXTICR[3] = 0x0500 ;	
	
	EXTI->IMR |= 0x00000400;
	//EXTI->FTSR|= 0x00000400;
	EXTI->RTSR|= 0x00000400;
	NVIC->ISER[1] = 0x00000100;
	//NVIC_EnableIRQ(EXTI15_10_IRQn);
	NVIC_SetPriority(EXTI15_10_IRQn,5);


	while(1){
		int push = 0x00000400;//push
		int up = 0x0010;
		int down = 0x0040;
		
		NVIC->ICPR[0] &= ~0x00000040;
		indexLed = (indexLed < sizeof(ledOrder)/sizeof(uint32_t)) ? indexLed : 0;
		if(runb){
			if (GPIOF->IDR & down){
        Delay(50);
        if (GPIOF->IDR & down){
					runb = 0;
						indexLed++;
						pin = ledOrder[1];
					} 
		}
			
		if (GPIOF->IDR & up){
			Delay(50);
			if (GPIOF->IDR & up){
					runb = 0;
					indexLed--; 
					pin = ledOrder[2];
				}
		}
	}
	else{
				 if (!(GPIOF->IDR & up)){
            Delay (50);
            if (!(GPIOF->IDR & up)){
                runb = 1;
            }
        }
				 
				if (!(GPIOF->IDR & down)){
            Delay (50);
            if (!(GPIOF->IDR & down)){
                runb = 1;
            }
        }
			}
		}
	return 0;
}
