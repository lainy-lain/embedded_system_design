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
	/* 코드 설명
	GPIOD, C는 TFT_LCD Pin을 이용하려고 Enable하였고,
	GPIOB는 ADC_IN1(PB0) Pin을 이용하기 위해(조도센서를 PB0에 연결하여 사용) Enable 해주었다.
	ADC1은 ADC를 이용하기 위해 Enable 해주었고, AFIO는 Interrupt를 이용하기 위해 Enable 해주었다.
	*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);     
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}           
 
void GPIO_Configuration(void){
	/* 코드 설명
	Board의 Output이 LCD의 Input으로 전달됨으로써 LCD가 제어되는 것이기 때문에
	TFT-LCD Pin은 모두 Speed 10MHz의 Push-Pull Output Mode로 설정해 주었다.
	PB0 Pin의 경우 조도센서 값을 아날로그 값으로 받아와야 하기 때문에 Analog Input Mode로 설정해 주었다.
	*/
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
	/* 코드 설명
	ADC1 Interrupt의 NVIC를 아래와 같이 설정해줌.
	*/
    NVIC_init.NVIC_IRQChannel = ADC1_2_IRQn;            //USART2 IRQ 설정
    NVIC_init.NVIC_IRQChannelPreemptionPriority = 0x00; //IRQ 우선 패리티비트 0으로 설정 
    NVIC_init.NVIC_IRQChannelSubPriority = 0x00;        //Sub 패리티 0으로 설정
    NVIC_init.NVIC_IRQChannelCmd = ENABLE;           	// IRQ Channel command enable     
    NVIC_Init(&NVIC_init);								// IRQ 초기화 v
}

void ADC_Configuration(){
	/* 코드 설명
	ADC를 Configure하기 위해서 처음으로, ADC_DeInit() 함수를 이용해 ADC1 채널을 Deinitialize 해준다.
	그 후, stm32f10x_adc.h에 정의되어 있는 ADC_InitTypeDef 구조체와 ADC_Init() 함수를 이용해 ADC를 Initialize해준다. 
	다음으로, ADC_RegularChannelConfig() 함수를 이용해 ADC 채널을 설정해준다.
	우리는 ADC1을 이용할 것이므로 첫 번째 인자는 ADC1으로 주었고, 
	두 번째 인자의 경우는, schematic에 PBO가 ADC12의 8번 채널로 쓰일 수 있다고 적혀있으므로 (PB0/ADC12_IN8/...)  채널 8로 인자를 설정해 주었다.
	ADC_ITConfig() 함수는 ADC Interrupt를 enable/disable하는 함수이다. 
	이 함수를 이용해 ADC1의 interrupt를 Enable해주었으며, 두 번째 인자인 ADC_IT_EOC는 인터럽트가 End of Conversion, 즉 변환이 완료되었을 때 발생한다는 것을 의미한다.
	ADC_Cmd() 함수는 ADC peripheral을 enable/disable하는 함수이다. ADC1 peripheral을 Enable해주었다.
	
	그 후, 강의 자료에 나와있는 대로 Calibration 관련 함수를 이용하여 ADC1의 Calibration을 설정해주고,
	ADC_SoftwareStartConvCmd()를 이용해 ADC1의 ADC 변환을 시작함으로써 ADC의 Configuration을 완료한다.
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
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}
 

void ADC1_2_IRQHandler(void) {
	/* 코드 설명
	ADC1의 변환이 완료되었으면, 변환된 값을 전역변수인 value에 할당한 후 Pending Bit을 Clear해준다.
	*/
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
    
	/* 코드 설명
	while문 내부를 살펴보자.
	우선, (0, 0)에 LCD-Team09라는 String을 출력한다.
	다음으로, Touch_GetXY() 함수를 통해 변수 x, y에 '터치된' 화면의 좌표를 받아오고, Convert_Pos() 함수를 통해 교정(calibration)된 정확한 좌표값을 px, py에 할당한다.
	(touch.c의 Touch_GetXY() 함수의 정의를 살펴보면, 사용자가 터치를 하지 않았다면 x, y에 값이 새로 할당되지 않음을 알 수 있다.)
	그런데, 터치된 위치에 작은 원을 그리고, 좌표를 나타내고, 조도센서 값을 출력하는 행위(a)는 '화면 터치'라는 동작을 선행조건으로 가지기 때문에, 화면이 터치되었을 때만 수행되어야 한다.
	그래서 이용하는 것이 touch.h에 정의된 T_INT라는 매크로 상수이다. 직관적으로 생각해보면 Touch_Interrupt의 약자라고 추측할 수 있을 것이다.
	T_INT의 정의를 해석해 보면, PC5의 IDR값(정확히는 Port C의 IDR5값)과 동일한 값을 나타냄을 알 수 있다.
	PC5는 touch.c의 Touch_Configuration에서 사용된 Port이고, Input pull-up mode로 configure되었다.
	즉, Touch와 관련된 Input(pull-up) port이므로, 터치 인터럽트가 발생하면 PC의 IDR5가 0이 될 것이라는 점을 알 수 있다.
	정리하자면, T_INT == 1이면 터치 인터럽트가 발생하지 않았다는 것을 의미하며(PC5가 Input pull-up이므로)
	반대로 T_INT == 0이면 터치 인터럽트가 발생했다는 것을 의미한다는 사실을 도출해 낼 수 있다.
	이러한 도출 결과를 이용하여, '터치 인터럽트가 발생했을 때' (a)와 같은 행위를 수행하기 위해서 if (T_INT == 0) 안에 (a)를 삽입해 주었다.
	*/
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