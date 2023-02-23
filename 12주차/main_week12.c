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
	/* 코드 설명
	요약 : 10주차 코드에서 DMA1 부분만 추가하였다.
	
	GPIOD, C는 TFT_LCD Pin을 이용하려고 Enable하였고,
	GPIOB는 ADC_IN1(PB0) Pin을 이용하기 위해(조도센서를 PB0에 연결하여 사용) Enable 해주었다.
	ADC1은 ADC를 이용하기 위해 Enable 해주었고, DMA1은 DMA1 Channel을 사용하기 위해 Enable해주었다.
	*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
}           
 
void GPIO_Configuration(void){
	/* 코드 설명
	요약 : 10주차 코드와 동일함.
	
	Board의 Output이 LCD의 Input으로 전달됨으로써 LCD가 제어되는 것이기 때문에
	TFT-LCD Pin은 모두 Speed 10MHz의 Push-Pull Output Mode로 설정해 주었다.
	PB0 Pin의 경우 조도센서 값을 아날로그 값으로 받아와야 하기 때문에 Analog Input Mode로 설정해 주었다.
	*/
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
	/* 코드 설명
	코드의 대부분은 10주차와 동일하지만 딱 한줄이 바뀌었는데, 
	ADC_ITConfig()을 이용하는 대신 ADC_DMACmd()를 이용한 것이 유일한 변경점이다.
	ADC_DMACmd() 함수를 이용하여, ADC1의 DMA request 기능을 Enable해주었다.
	*/
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
	/* 코드 설명
	DMA_InitTypeDef 구조체 및 두가지의 함수를 이용하여 DMA를 Configuration하는 함수이다.
	우선, DMA_InitTypeDef 구조체의 값을 왜 아래와 같이 할당했는지에 대해 알아보자.
	
	DMA_PeripheralBaseAddr 필드는, DMA channel x peripheral address register(DMA_CPARx)와 대응되는 필드이다.
	Reference에서 DMA_CPARx의 설명을 읽어보면, peripheral 'data register'의 base address를 저장하는 register라고 되어 있다.
	우리는 ADC1에 연결된 peripheral을 이용하므로, ADC1의 data register의 base address로 설정해주면 된다.
	ADC1의 data register 주소를 알아내기 위해, 10주차에서 전역변수에 ADC값을 받아오는 과정을 살펴보았다.
	10주차에서 우리는 ADC_GetConversionValue(ADC_TypeDef* ADCx)라는 함수를 이용해 전역변수에 ADC값을 받았는데,
	이 함수의 정의를 stm32f10x_adc.c 파일에서 들여다보면, 단순히 ADCx의 Data Register값을 반환함을 알 수 있다. (return ADCx->DR)
	즉, ADC_TypeDef라는 구조체를 이용하면 ADCx의 Data Register에 쉽게 접근할수 있었던 것이다. 
	그래서 이 구조체를 이용하여, 아래 코드와 같이 해당 필드를 설정해주었다.
	
	DMA_MemoryBaseAddr는, DMA에 사용할 stm32 보드의 메모리 주소를 나타낸다.
	우리는 ADC_Value라는 전역변수(배열)에 ADC값을 DMA로 read할 것이므로, ADC_Value의 주소로 해당 필드를 설정해주었다.
	
	DMA_DIR 필드는, 주변기기가 source이냐 destination이냐를 나타내는 필드이다.
	우리는 주변기기에 데이터를 Write하는게 아니라, 주변기기로부터 데이터를 Read하는 것이므로, 주변기기는 source가 되어야 한다.
	그러므로 아래와 같이 해당 필드를 설정해주었다.
	
	DMA_BufferSize, DMA_PeripheralInc, DMA_MemoryInc 필드에 대해선 한번에 알아보자.
	우리는 ADC1의 Data Register에 저장되는 ADC값을, ADC_Value라는 메모리 공간에 반복해서 읽어오기만 할 것이므로,
	DMA_BufferSize는 1로 설정해 주었고, (한번에 여러개의 값을 읽어오는 것이 아니라, 단 하나의 값만 읽어오기 때문)
	peripheral addr register와 memory address register의 값이 증가하지 않도록 설정해 주었다.
	(연속된 메모리 공간에서 순차적으로 읽어오는 것도 아니고, 연속된 메모리 공간에 순차적으로 작성하는 것도 아니기 때문이다)
	
	PeripheralDataSize와 MemoryDataSize는 둘 다 Word 단위(32-bit)로 설정하여서 다양한 범위의 값을 나타낼 수 있도록 하였다.
	(Half Word 혹은 Byte로 설정하면 ADC값이 작은 범위로 나타나지만, Word로 설정하면 큰 범위로 나타난다)
	
	연속해서 데이터를 읽어올 것이므로 Circular Mode로 설정해 주었고, Circular Mode는 M2M Mode와 동시 사용 불가이므로 M2M은 Disable해주었다.
	DMA_Priority의 경우, 어차피 ADC1(PB0) 하나에 대해 DMA를 설정해주는 것이므로 크게 의미는 없지만, 우선 Very High로 설정해 주었다.
	
	이렇게 DMA_InitTypeDef 구조체의 값들을 설정해준 후, DMA_Init() 함수와 설정해준 구조체를 이용하여 DMA1 Channel1을 초기화해준다.
	(Datasheet Table 78에 의하면 ADC1은 DMA1의 Channel 1을 이용하기 때문이다)
	그리고 마지막으로, DMA_Cmd 함수를 이용해 DMA1의 Channel 1을 Enable해준다.
	*/
	
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
    
	/* 코드 설명
	조도센서 값 threshold는 800이다. 즉, ADC값이 800 미만이면 플래시가 비춰지고 있는 것이고
	ADC값이 800 이상이면 평상 상태로 인식하는 것이다.
	배경색을 바꾸는 LCD_Clear()함수를 호출하게 되면 화면이 깜빡이므로, 플래시를 비추고 있는 기간동안, 또는 평상시에 계속해서 LCD_Clear() 함수가 호출되면
	while문이 돌때마다 화면이 깜빡이게 된다. 그래서 bflag라는 변수를 사용하여, 상태가 변할때 딱 한번만 LCD_Clear()함수를 호출하도록 한다.
	글자를 출력하는 부분은 반복해서 호출되어도 깜빡이지 않으므로, bflag와 상관없이 ADC_Value값에 따라서 바로 글자색이 출력되도록 설정해 주었다.
	*/
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