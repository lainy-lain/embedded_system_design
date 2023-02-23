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
	
 GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_13 | GPIO_Pin_15);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
}

/* 코드 설명
타이머 관련 전역변수들을 선언해두었다.
*/
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
uint16_t prescale = 0;

void setMotor(){
	// (SysCoreClock = 72MHz) / 1000000 = 72
    // 50Hz로 분주
    prescale = (uint16_t) (SystemCoreClock / 1000000) - 1;
    
    TIM_TimeBaseStructure.TIM_Period = 20000-1;         
    TIM_TimeBaseStructure.TIM_Prescaler = prescale;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0xFF00; // 모터의 초기위치 설정
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

/* 코드 설명
제공된 PWM test code의 openDoor() 함수를 그대로 쓰되, TIM_Pulse값만 변경해주었다.
rotate0는 모터의 각도를 0도로 설정(초기 위치로 설정)한다는 의미이며, 그렇기에 setMotor()에서와 같이
모터의 초기위치를 TIM_Pulse = 700으로 설정해 주었다.
*/
void rotate0(){
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0x0FFF;
    
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
}

/* 코드 설명
제공된 PWM test code의 openDoor() 함수를 그대로 쓰되, TIM_Pulse값만 변경해주었다.
rotate180은 모터의 각도를 180도로 설정(초기 위치에서 180도 회전)한다는 의미이다.
초기 위치 TIM_Pulse = 700에 대해 180도 회전시키려면, TIM_Pulse = 2300으로 설정하면 된다는 것을 실험적으로 파악해서
TIM_Pulse = 2300으로 설정해 주었다.
*/
void rotate180(){
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0xFFFE;

    TIM_OC3Init(TIM3, &TIM_OCInitStructure);    
}

void rotate90(){
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0xFFFF >> 1;
    
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
}

int main(void){
    SystemInit();        
 
    RCC_Configuration();        
    GPIO_Configuration();        

    setMotor();

    while(1){
      if(~GPIOC->IDR & GPIO_IDR_IDR2){  //down
         printf("2\n");
         GPIO_SetBits(GPIOE, GPIO_Pin_2);
         GPIO_ResetBits(GPIOE, GPIO_Pin_3);
         GPIO_SetBits(GPIOE, GPIO_Pin_4);
         GPIO_ResetBits(GPIOE, GPIO_Pin_5);
      }
      if(~GPIOC->IDR & GPIO_IDR_IDR3){  //left
         rotate0();
      }
      if(~GPIOC->IDR & GPIO_IDR_IDR4){  //right
         rotate180();
      }
      if(~GPIOC->IDR & GPIO_IDR_IDR5){  //up
          GPIO_ResetBits(GPIOE, GPIO_Pin_2);
          GPIO_ResetBits(GPIOE, GPIO_Pin_3);
          GPIO_ResetBits(GPIOE, GPIO_Pin_4);
          GPIO_ResetBits(GPIOE, GPIO_Pin_5);
      }
    }
}
