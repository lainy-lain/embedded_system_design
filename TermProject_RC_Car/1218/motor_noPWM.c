#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"

#include "misc.h"

/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);

//---------------------------------------------------------------------------------------------------

void RCC_Configure(void) // stm32f10x_rcc.c, .h 참고, 아래에 주석이 묶여있는 기능끼리 합쳐서 함수에 매개변수 전달 하시오!!
{
/////////////////////// TODO: Enable the APB2 peripheral clock using the function 'RCC_APB2PeriphClockCmd'///////////////////////
	
	/* JoyStick Up/Down port clock enable, LED port clock enable */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	/* UART 1 TX/RX port clock enable, USART1 clock enable */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* Alternate Function IO clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configure(void) // stm32f10x_gpio.c, .h 참고
{
    GPIO_InitTypeDef GPIO_InitStructure;


/////////////////////// TODO: Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' and the function 'GPIO_Init'///////
	
    GPIO_InitTypeDef GPIO_init; //for joystick
    
    
    
    GPIO_init.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_init.GPIO_Pin = (GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_8 | GPIO_Pin_10); 
    GPIO_Init(GPIOD, &GPIO_init);
    
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void delay_us(uint32_t us) {
	if ( us > 1 ) {
		uint32_t count = us * 8 - 6;
		while(count--);
	}
	else{
		uint32_t count = 2;
		while(count--);
	}
}

void delay_ms(uint32_t ms) {
	uint32_t us = 1000 * ms;
	delay_us(us);
}

int main(void)
{

    SystemInit();

    RCC_Configure();

    GPIO_Configure();
    
    while(1){
		GPIO_SetBits(GPIOD, GPIO_Pin_9 );
		GPIO_ResetBits(GPIOD, GPIO_Pin_11);
		delay_ms(1000);
		
		GPIO_ResetBits(GPIOD, GPIO_Pin_9 );
		GPIO_SetBits(GPIOD, GPIO_Pin_11);
		delay_ms(1000);
    }
    
    return 0;
}