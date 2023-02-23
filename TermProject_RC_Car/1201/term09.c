#include "misc.h"
#include "stm32f10x.h"

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_exti.h"

#define DC_SPEED_HIGH 0xFFFE
#define DC_SPEED_LOW 0x00FF
#define SERVO_ROTATE_INIT 700
#define SERVO_ROTATE_RIGHT 2300
#define SERVO_ROTATE_LEFT 0

GPIO_InitTypeDef GPIO_InitStructure;           
USART_InitTypeDef USART_InitStructure;         
NVIC_InitTypeDef NVIC_InitStructure;           
 
void RCC_Configuration(void){    
    // pin for PWM, USART
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    // pin for PWM, ADC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);     
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);     

    // ADC    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);     
    // AFIO, for Interrupt
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
    // USART Bluetooth
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);
    
    // TIM3 for PWM - Servo Motor 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    // TIM4 for PWM - DC Motor 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); 
    // TIM5 for PWM - Buzzer
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    
}           
 
void GPIO_Configuration(void){
    /* ADC Configure */
    // ADC_IN1 (PB0) Configure
    GPIO_init.GPIO_Pin = GPIO_Pin_0;  
    GPIO_init.GPIO_Mode = GPIO_Mode_AIN;    
    GPIO_Init(GPIOB, &GPIO_init);        
    
    /* USART2 Bluetooth Configure */
    // UART2 TX, Output
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // UART2 RX, Input
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    
    /* TIM for PWM Configure */
	// TIM3 CH1 (PA6) Configure 
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	// TIM4 CH1 (PB6) Configure 
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	// TIM5 CH1 (PA0) Configure
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Genral IO pin configure 할 것*/
    /* 초음파 부분 */

    /* 모터 드라이버 부분*/
}
 

void ADC_Configuration(){
    ADC_DeInit(ADC1);
	ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
	
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);    
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

// ADC, USART
void NVIC_Configuration(void){      
    // ADC
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;            
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;        
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           	
    NVIC_Init(&NVIC_InitStructure);								

    // USART2 
    NVIC_EnableIRQ(USART2_IRQn);
} 


TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
uint16_t prescale = 0;

void setDCMotor_PWM() {
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
    TIM_OCInitStructure.TIM_Pulse = DC_SPEED_HIGH; // 모터의 초기속도 설정
    TIM_OC3Init(TIM4, &TIM_OCInitStructure);
    
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
}

void setServoMotor_PWM() {
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
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void setDCMotor_speedHigh();
void setDCMotor_speedLow();
void setDCMotor_goFoward();
void setDCMotor_goBackward();
void setDCMotor_stop();

void setServoMotor_rotateInit();
void setServoMotor_rotateRight();
void setServoMotor_rotateLeft();


uint32_t pressureValue = 0;

void ADC1_2_IRQHandler(void) {
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET) {
		pressureValue = ADC_GetConversionValue(ADC1);
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);	
	}    
}


int main(void){
    SystemInit();        
 
    RCC_Configuration();        
    GPIO_Configuration();        
    ADC_Configuration();
    NVIC_Configuration();        
    

}

void setDCMotor_speedHigh() {
    TIM_OCInitStructure.TIM_Pulse = DC_SPEED_HIGH;
    TIM_OC3Init(TIM4, &TIM_OCInitStructure);
}

void setDCMotor_speedLow() {
    TIM_OCInitStructure.TIM_Pulse = DC_SPEED_LOW;
    TIM_OC3Init(TIM4, &TIM_OCInitStructure);
}

void setDCMotor_goFoward() {
    // 10 10
    GPIO_SetBits(GPIOx, GPIO_Pin_x);
    GPIO_ResetBits(GPIOx, GPIO_Pin_x);
    GPIO_SetBits(GPIOx, GPIO_Pin_x);
    GPIO_ResetBits(GPIOx, GPIO_Pin_x);
}

void setDCMotor_goBackward() {
    // 01 01
    GPIO_ResetBits(GPIOx, GPIO_Pin_x);
    GPIO_SetBits(GPIOx, GPIO_Pin_x);
    GPIO_ResetBits(GPIOx, GPIO_Pin_x);
    GPIO_SetBits(GPIOx, GPIO_Pin_x);
}

void setDCMotor_stop() {
    // 00 00
    GPIO_ResetBits(GPIOx, GPIO_Pin_x);
    GPIO_ResetBits(GPIOx, GPIO_Pin_x);
    GPIO_ResetBits(GPIOx, GPIO_Pin_x);
    GPIO_ResetBits(GPIOx, GPIO_Pin_x);
}

void setServoMotor_rotateInit() {
    TIM_OCInitStructure.TIM_Pulse = SERVO_ROTATE_INIT;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
}

void setServoMotor_rotateRight() {
    TIM_OCInitStructure.TIM_Pulse = SERVO_ROTATE_RIGHT;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
}

void setServoMotor_rotateLeft() {
    TIM_OCInitStructure.TIM_Pulse = SERVO_ROTATE_LEFT;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
}