#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "stm32f10x_exti.h"

#define PB_CRH *(volatile unsigned long *)0x40010C04
#define PB_ODR *(volatile unsigned long *)0x40010c0c
#define PB_IDR *(volatile unsigned long *)0x40010c08

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
	/* 코드 설명
	GPIOD, C는 TFT_LCD 및 LED을 이용하려고 Enable하였고,
	GPIOB는 TIM3(PB0)을 이용하기 위해, 그리고 AFIO는 Interrupt를 이용하기 위해 Enable 해주었다.
	TIM2와 TIM3는 자명하게 TIM2와 TIM3를 이용하기 위해 Enable해주었다.
	*/
	// APB2 RCC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	// APB1 RCC
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}           
 
void GPIO_Configuration(void){
	/* 코드 설명
	LCD 부분은 10주차 코드와 동일하다.
	LED 부분은 제공된 PWM Test Code의 LedGpioConfig()과 동일하게 GPIO 설정해주었다. (PD2,3,4,7 GP output push-pull 50MHz로 설정)
	PWM을 위한 TIM3 부분은 제공된 PWM Test Code의 setMotor()와 동일하게 작성하였다.
	Speed와 Mode가 동일한 경우엔 코드의 redundancy를 줄이기 위해 구조체의 Pin값만 바꿔주었다.
	*/
	
	
	// TIM3 CH3 (PB0) Configure
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
}
 
void NVIC_Configuration(void){      
	/* 코드 설명
	TIM2 Interrupt의 NVIC를 아래와 같이 설정해줌.
	*/
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;              //TIM2 IRQ 설정
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; //IRQ 우선 패리티비트 0으로 설정 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;        //Sub 패리티 0으로 설정
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           	// IRQ Channel command enable     
    NVIC_Init(&NVIC_InitStructure);								// IRQ 초기화 
}

/* 코드 설명
타이머 관련 전역변수들을 선언해두었다.
*/
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
uint16_t prescale = 0;

/* 코드 설명
제공된 PWM test code의 setMotor()의 내용을 그대로 쓰되, 분주를 위해 prescale과 period값을 변경해주었다.
TIM2의 주파수를 2Hz로 만들기 위해, prescale을 600, period를 60000으로 설정해주었다.
( (f_clk = 72MHz) / 600 / 60000) = 2Hz )

추가로, prescale = 600;과 같이 바로 값을 할당하는게 아니라 prescale = (SysClk / 120000);의 형태로 값을 할당하면 검산을 쉽게 할 수 있다.
SysClk * (1 / prescale) * (1 / period) = SysClk * (120000 / SysClk) * (1 / 60000)
= 120000 / 60000 = 2Hz. 즉, SysClk이 소거되므로 상수 두개를 나눠봄으로써 분주 결과를 바로 파악하기 용이하다.
*/
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

uint32_t tCounter = 0;
uint8_t triggerFlag = 0;

void TIM2_IRQHandler(void) {
  if (triggerFlag) {
    tCounter++;
  }
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

void trig_pulse(void) // output trigger pulse to HC-SR04 trig pin at least 10us
{
    PB_ODR |= (1<<10); // |= 0000 0100 0000 0000
    delay_us(11); // delay little over than 10us
    PB_ODR &= ~(1<<10); // &= 1111 1011 1111 1111
    delay_us(11); // delay little over than 10us
}

unsigned long echo_time(void)
{
    unsigned long echo;
    trig_pulse(); // give trig pulse to u_sonic sensor
    
    while((PB_IDR & 0x00000800) != 0x0000800); // wait echo pin status turns HIGH
// != 0000 1000 0000 0000 (wait until bit11=1 )
    echo = TIM2->CNT;      
    
    while((PB_IDR & 0x00000800) == 0x0000800);
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