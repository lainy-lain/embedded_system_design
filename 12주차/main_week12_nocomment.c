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

GPIO_InitTypeDef GPIO_init;                    // GPIO 초기화 변수 선언
NVIC_InitTypeDef NVIC_init;                    // 중첩 인터럽트 처리위한 NVIC 초기화 변수 선언    
 
void RCC_Configuration(void){    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
}           
 
void GPIO_Configuration(void){
	// LCD_RD (PD15) Configure
    GPIO_init.GPIO_Pin = GPIO_Pin_15;              
	GPIO_init.GPIO_Speed = GPIO_Speed_10MHz;	
    GPIO_init.GPIO_Mode = GPIO_Mode_Out_PP;    
    GPIO_Init(GPIOD, &GPIO_init);        
	// LCD_RS (PD13) Configured
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
    ADC_DMACmd(ADC1, ENABLE); // 인터럽트가 아닌 DMA를 사용해야 하므로 ADC_DMACmd() 사용
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

// ADC값을 저장하기 위한 전역변수(배열) 선언
volatile uint32_t ADC_Value[1];

void DMA_Configuration(){
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &ADC1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) ADC_Value;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);
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
    DMA_Configuration();
    
    LCD_Init();
	// beforeFlag, 이전에 ADC 값이 800 미만이었는지를 나타내는 변수이다.
    uint32_t bflag = 0; 
    
	while(1){
		if (ADC_Value[0] < 800) { // 플래시로 조도센서를 비추고 있을 때
			if(!bflag) { // 이전에 플래시를 비추고 있지 않던 경우
				LCD_Clear(GRAY);
				bflag = 1;
			}
			LCD_ShowString(0, 0, "LCD-Team09", WHITE, GRAY);
			LCD_ShowNum(0, 100, ADC_Value[0], 4 , WHITE, GRAY);
		}
		else { // 평상시
			if(bflag) { // 이전에 플래시를 비추고 있던 경우
				LCD_Clear(WHITE);
				bflag = 0;
			}
			LCD_ShowString(0, 0, "LCD-Team09", BLACK, WHITE);
			LCD_ShowNum(0, 100, ADC_Value[0], 4 , BLACK, WHITE);            
		}      
	}
}