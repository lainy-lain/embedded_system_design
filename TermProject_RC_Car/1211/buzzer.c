#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "stm32f10x_exti.h"

GPIO_InitTypeDef GPIO_InitStructure;                    // GPIO 초기화 변수 선언

 
void RCC_Configuration(void){    
	/* 코드 설명
	GPIOD, C는 TFT_LCD 및 LED을 이용하려고 Enable하였고,
	GPIOB는 TIM3(PB0)을 이용하기 위해, 그리고 AFIO는 Interrupt를 이용하기 위해 Enable 해주었다.
	TIM2와 TIM3는 자명하게 TIM2와 TIM3를 이용하기 위해 Enable해주었다.
	*/
	// APB2 RCC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	// APB1 RCC
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
}           
 
void GPIO_Configuration(void){
	
    
  // buzzer
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);	
}



void delay_us(uint32_t us){
	if ( us > 1 ) {
		uint32_t count = us * 8 - 6;
		while(count--);
	}
	else{
		uint32_t count = 2;
		while(count--);
	}
}

void delay_ms(uint32_t ms){
	uint32_t us = 1000 * ms;
	delay_us(us);
}

int main(void){
    SystemInit();        
 
    RCC_Configuration();        
    GPIO_Configuration();        

    while (1){
      GPIO_ResetBits(GPIOB, GPIO_Pin_8);
        delay_ms(1500);
       GPIO_SetBits(GPIOB, GPIO_Pin_8);
       delay_ms(100);
    }
    

}
