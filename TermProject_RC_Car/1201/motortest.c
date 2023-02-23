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
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
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
    
    
    /* JoyStick up, down pin setting, Input */
    // 조이스틱 PC 2, 3, 4, 5
    GPIO_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_init.GPIO_Pin = (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5);
    GPIO_Init(GPIOC, &GPIO_init);
    
    GPIO_init.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_init.GPIO_Pin = (GPIO_Pin_5 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4);
    GPIO_Init(GPIOE, &GPIO_init);
    
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

int main(void)
{

    SystemInit();

    RCC_Configure();

    GPIO_Configure();
    
    while(1){
      if(~GPIOC->IDR & GPIO_IDR_IDR2){
         GPIO_SetBits(GPIOE, GPIO_Pin_2);
         GPIO_ResetBits(GPIOE, GPIO_Pin_3);
         GPIO_ResetBits(GPIOE, GPIO_Pin_4);
         GPIO_ResetBits(GPIOE, GPIO_Pin_5);
         printf("2\n");
      }
      if(~GPIOC->IDR & GPIO_IDR_IDR3){
         GPIO_SetBits(GPIOE, GPIO_Pin_3);
         GPIO_ResetBits(GPIOE, GPIO_Pin_2);
         GPIO_ResetBits(GPIOE, GPIO_Pin_4);
         GPIO_ResetBits(GPIOE, GPIO_Pin_5);
         printf("3\n");
      }
      if(~GPIOC->IDR & GPIO_IDR_IDR4){
          GPIO_SetBits(GPIOE, GPIO_Pin_4);
          GPIO_ResetBits(GPIOE, GPIO_Pin_3);
         GPIO_ResetBits(GPIOE, GPIO_Pin_2);
         GPIO_ResetBits(GPIOE, GPIO_Pin_5);
         printf("4\n");
      }
      if(~GPIOC->IDR & GPIO_IDR_IDR5){
         GPIO_SetBits(GPIOE, GPIO_Pin_5);
         GPIO_ResetBits(GPIOE, GPIO_Pin_3);
         GPIO_ResetBits(GPIOE, GPIO_Pin_4);
         GPIO_ResetBits(GPIOE, GPIO_Pin_2);
         printf("5\n");
      }
    }
    
    return 0;
}