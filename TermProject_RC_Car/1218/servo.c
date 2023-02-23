#include "stm32f10x.h"
#include "misc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"

// 서보모터 회전각 관련 상수 (일단 보류)
#define SERVO_ROTATE_INIT		1500
#define SERVO_ROTATE_RIGHT		2300
#define SERVO_ROTATE_LEFT		700

// Configuration을 위한 구조체 변수
GPIO_InitTypeDef    GPIO_InitStructure;
USART_InitTypeDef   USART_InitStructure;         
NVIC_InitTypeDef    NVIC_InitStructure; 

// 타이머 설정을 위한 구조체 변수
TIM_TimeBaseInitTypeDef     TIM_TimeBaseStructure;
TIM_OCInitTypeDef           TIM_OCInitStructure;

// TIM prescale 설정을 위한 변수
uint16_t prescale = 0;


void Configuration(void) {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  
  	// TIM3 CH1 (PA6) Configure -- 서보모터
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
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
    TIM_OCInitStructure.TIM_Pulse = SERVO_ROTATE_INIT; // 모터의 초기위치 설정
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void setServoMotor_rotateInit(void) {
    TIM_OCInitStructure.TIM_Pulse = SERVO_ROTATE_INIT;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
}

void setServoMotor_rotateRight(void) {
    TIM_OCInitStructure.TIM_Pulse = SERVO_ROTATE_RIGHT;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
}

void setServoMotor_rotateLeft(void) {
    TIM_OCInitStructure.TIM_Pulse = SERVO_ROTATE_LEFT;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
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

int main(void) {
SystemInit();
  Configuration();
  
  while (1) {
    setServoMotor_rotateInit();
    delay_ms(1000);
    setServoMotor_rotateRight();
    delay_ms(1000);
    setServoMotor_rotateLeft();
    delay_ms(1000);
    
  }
}