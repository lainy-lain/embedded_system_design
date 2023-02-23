#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "stm32f10x_exti.h"

#define PC_CRH *(volatile unsigned long *)0x40011004
#define PC_ODR *(volatile unsigned long *)0x4001100c
#define PC_IDR *(volatile unsigned long *)0x40011008

#define PE_CRH *(volatile unsigned long *)0x40011804
#define PE_ODR *(volatile unsigned long *)0x4001180c
#define PE_IDR *(volatile unsigned long *)0x40011808

GPIO_InitTypeDef GPIO_InitStructure;                    // GPIO 초기화 변수 선언
NVIC_InitTypeDef NVIC_InitStructure;                    // 중첩 인터럽트 처리위한 NVIC 초기화 변수 선언    

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

void RCC_Configuration(void){    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);     
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
}           
 
void GPIO_Configuration(void){
	// sonic1 trigger
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    // sonic1 echo
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // sonic2 trigger
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    // sonic2 echo
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}
 
/* 코드 설명
타이머 관련 전역변수들을 선언해두었다.
*/
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
uint16_t prescale = 0;

void setTIMER2() {
    prescale = (uint16_t) 7200 - 1;
    
    TIM_TimeBaseStructure.TIM_Period = 10000-1;
    TIM_TimeBaseStructure.TIM_Prescaler = prescale;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    // TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void setTIMER5() {
    prescale = (uint16_t) 7200 - 1;
    
    TIM_TimeBaseStructure.TIM_Period = 10000-1;
    TIM_TimeBaseStructure.TIM_Prescaler = prescale;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

    TIM_ARRPreloadConfig(TIM5, ENABLE);
    TIM_Cmd(TIM5, ENABLE);
    // TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}


void trig_pulse(uint16_t trigPin) // output trigger pulse to HC-SR04 trig pin at least 10us
{
    PC_ODR |= (1<<trigPin); // |= 0000 0100 0000 0000
    delay_us(11); // delay little over than 10us
    PC_ODR &= ~(1<<trigPin); // &= 1111 1011 1111 1111
    delay_us(11); // delay little over than 10us
}

void trig_pulse2(uint16_t trigPin) // output trigger pulse to HC-SR04 trig pin at least 10us
{
    PE_ODR |= (1<<trigPin); // |= 0000 0100 0000 0000
    delay_us(11); // delay little over than 10us
    PE_ODR &= ~(1<<trigPin); // &= 1111 1011 1111 1111
    delay_us(11); // delay little over than 10us
}

unsigned long echo_time(uint16_t trigPin, uint16_t echoPin)
{
    unsigned long echo;
    trig_pulse(trigPin); // give trig pulse to u_sonic sensor
    
    while((PC_IDR & (1<<echoPin)) != (1<<echoPin)) {
	// test
	printf("in 1\n");
	} // wait echo pin status turns HIGH
// != 0000 1000 0000 0000 (wait until bit11=1 )
    echo = TIM2->CNT;      
    
    while((PC_IDR & (1<<echoPin)) == (1<<echoPin)) {
	//test
	printf("in 2\n");
	}
// == 0000 1000 0000 0000 (wait until bit11=0 )
    echo = TIM2->CNT - echo;  
    
    return echo;
}

unsigned long echo_time2(uint16_t trigPin, uint16_t echoPin)
{
    unsigned long echo;
    trig_pulse2(trigPin); // give trig pulse to u_sonic sensor
    
    while((PE_IDR & (1<<echoPin)) != (1<<echoPin)){
	// test
	printf("in 3\n");
	} // wait echo pin status turns HIGH
// != 0000 1000 0000 0000 (wait until bit11=1 )
    echo = TIM5->CNT;      
    
    while((PE_IDR & (1<<echoPin)) == (1<<echoPin)) {
	// test
	printf("in 4\n");
	}
// == 0000 1000 0000 0000 (wait until bit11=0 )
    echo = TIM5->CNT - echo;  
    
    return echo;
}

int main(void){
    SystemInit();        
 
    RCC_Configuration();        
    GPIO_Configuration();           
    
    setTIMER2();
	setTIMER5();
    
    unsigned long echo1 = 0;
    unsigned long echo2 = 0;

// Except PB10(input), PB08-PB15 All Set Output(Push Pull/High Spped-50Mhz)
    //PB_CRH = 0x33338333; // PB_H 8bit : 0011 0011 0011 0011 1000 0011 0011 0011
    printf("\n\n HC-SR04 Test... \n\n");
    while(1)
    {
       echo1 = echo_time(1, 3);
       printf("distance_1 = %5d.%d mm\n",17*echo1/100,(17*echo1)%100);
       delay_ms(50);
	   
       echo2 = echo_time2(0, 1);
       printf("distance_2 = %5d.%d mm\n",17*echo2/100,(17*echo2)%100);
       delay_ms(50);
// echo = time(us) (It is time for ultra sonic wave go and back)
// Velocity = Distance/Time, 340m/s = 2×Distance/echo(us), 340m/1,000,000us = 2×Distance/echo
// 340,000mm/1000,000us = 2×Distance/echo , 34mm/100 = 2×Distance/echo, 17mm/100us = Distance/echo
// Distance = 17 × echo / 100 mm
       
    }
}