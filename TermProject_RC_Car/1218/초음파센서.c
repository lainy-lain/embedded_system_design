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
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}           
 
void GPIO_Configuration(void){
	// sonic trigger
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    // sonic echo
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
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
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}


void trig_pulse(void) // output trigger pulse to HC-SR04 trig pin at least 10us
{
    PC_ODR |= (1<<1); // |= 0000 0100 0000 0000
    delay_us(11); // delay little over than 10us
    PC_ODR &= ~(1<<1); // &= 1111 1011 1111 1111
    delay_us(11); // delay little over than 10us
}

unsigned long echo_time(void)
{
    unsigned long echo;
    trig_pulse(); // give trig pulse to u_sonic sensor
    
    while((PC_IDR & 0x0000008) != 0x0000008); // wait echo pin status turns HIGH
// != 0000 1000 0000 0000 (wait until bit11=1 )
    echo = TIM2->CNT;      
    
    while((PC_IDR & 0x00000008) == 0x0000008);
// == 0000 1000 0000 0000 (wait until bit11=0 )
    echo = TIM2->CNT - echo;  
    
    return echo;
}

int main(void){
    SystemInit();        
 
    RCC_Configuration();        
    GPIO_Configuration();        
    NVIC_Configuration();        
    
    setTIMER2();
    
    unsigned long echo = 0;

// Except PB10(input), PB08-PB15 All Set Output(Push Pull/High Spped-50Mhz)
    //PB_CRH = 0x33338333; // PB_H 8bit : 0011 0011 0011 0011 1000 0011 0011 0011
    printf("\n\n HC-SR04 Test... \n\n");
    while(1)
    {
       echo = echo_time();
       //printf("%d\n", TIM2->CNT);
       printf("distance = %5d.%d mm\n",17*echo/100,(17*echo)%100);
// echo = time(us) (It is time for ultra sonic wave go and back)
// Velocity = Distance/Time, 340m/s = 2×Distance/echo(us), 340m/1,000,000us = 2×Distance/echo
// 340,000mm/1000,000us = 2×Distance/echo , 34mm/100 = 2×Distance/echo, 17mm/100us = Distance/echo
// Distance = 17 × echo / 100 mm
       delay_ms(100);
    }
}