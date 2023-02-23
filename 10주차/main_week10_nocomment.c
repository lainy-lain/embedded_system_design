#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "stm32f10x_exti.h"
#include "lcd.h"
#include "touch.h"

int color[12] ={WHITE,CYAN,BLUE,RED,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};
uint16_t value = 0; // 조도 센서 값을 저장하기 위한 전역변수

GPIO_InitTypeDef GPIO_init;                    // GPIO 초기화 변수 선언
USART_InitTypeDef USART_InitStructure;         // USART 초기화 변수 선언
NVIC_InitTypeDef NVIC_init;                    // 중첩 인터럽트 처리위한 NVIC 초기화 변수 선언    
 
void RCC_Configuration(void){    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);     
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}           
 
void GPIO_Configuration(void){
	// LCD_RD (PD15) Configure
    GPIO_init.GPIO_Pin = GPIO_Pin_15;              
	GPIO_init.GPIO_Speed = GPIO_Speed_10MHz;	
    GPIO_init.GPIO_Mode = GPIO_Mode_Out_PP;    
    GPIO_Init(GPIOD, &GPIO_init);        
	// LCD_RS (PD13) Configure
    GPIO_init.GPIO_Pin = GPIO_Pin_13;                           
	GPIO_init.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_init.GPIO_Mode = GPIO_Mode_Out_PP;    
    GPIO_Init(GPIOD, &GPIO_init);        
    // LCD_WR (PD14) Configure
    GPIO_init.GPIO_Pin = GPIO_Pin_14;       
	GPIO_init.GPIO_Speed = GPIO_Speed_10MHz;	
    GPIO_init.GPIO_Mode = GPIO_Mode_Out_PP;    
    GPIO_Init(GPIOD, &GPIO_init);        
    // LCD_CS (PC6) Configure
    GPIO_init.GPIO_Pin = GPIO_Pin_6;                           
	GPIO_init.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_init.GPIO_Mode = GPIO_Mode_Out_PP;    
    GPIO_Init(GPIOC, &GPIO_init);        
    // ADC_IN1 (PB0) Configure
    GPIO_init.GPIO_Pin = GPIO_Pin_0;  
    GPIO_init.GPIO_Mode = GPIO_Mode_AIN;    
    GPIO_Init(GPIOB, &GPIO_init);        
}
 
 
void NVIC_Configuration(void){      
    NVIC_init.NVIC_IRQChannel = ADC1_2_IRQn;            //USART2 IRQ 설정
    NVIC_init.NVIC_IRQChannelPreemptionPriority = 0x00; //IRQ 우선 패리티비트 0으로 설정 
    NVIC_init.NVIC_IRQChannelSubPriority = 0x00;        //Sub 패리티 0으로 설정
    NVIC_init.NVIC_IRQChannelCmd = ENABLE;           	// IRQ Channel command enable     
    NVIC_Init(&NVIC_init);								// IRQ 초기화 v
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
 

void ADC1_2_IRQHandler(void) {
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET) {
		value = ADC_GetConversionValue(ADC1);
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);	
	}    
}

static void LCD_Delay(uint32_t nCount)
{
	__IO uint16_t i;
	for (i = 0; i < (nCount * 5); i++)
	{
		;
	}
}

int main(void){
    SystemInit();        
 
    RCC_Configuration();        
    GPIO_Configuration();        
    ADC_Configuration();
    NVIC_Configuration();        
    
    LCD_Init();
    Touch_Configuration();
    Touch_Adjust();
    LCD_Clear(WHITE);
    uint16_t x, y;
    uint16_t px, py;
    
	while(1){
		LCD_ShowString(0, 0, "LCD-Team09", BRRED, WHITE);
		Touch_GetXY(&x, &y, 0);
		Convert_Pos(x, y, &px, &py);
		if(T_INT == 0){
			LCD_ShowNum(0, 100, value, 4 , BROWN, WHITE);
			LCD_ShowNum(0, 50, px, 4, BROWN, WHITE);
			LCD_ShowNum(0, 70, py, 4, BROWN, WHITE);
			LCD_DrawCircle(px, py, 10);
			LCD_Delay(10000);
		}
	}
}