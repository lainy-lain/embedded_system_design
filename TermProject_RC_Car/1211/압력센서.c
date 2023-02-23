#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "stm32f10x_exti.h"

uint16_t value = 0;

GPIO_InitTypeDef GPIO_init;                    // GPIO 초기화 변수 선언
USART_InitTypeDef USART_InitStructure;         // USART 초기화 변수 선언
NVIC_InitTypeDef NVIC_InitStructure;                    // 중첩 인터럽트 처리위한 NVIC 초기화 변수 선언    
 
void RCC_Configuration(void){    
    
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
	// ADC_IN1 (PB0) Configure
    GPIO_init.GPIO_Pin = GPIO_Pin_0;  
    GPIO_init.GPIO_Mode = GPIO_Mode_AIN;    
    GPIO_Init(GPIOB, &GPIO_init);        
}
 
 
void NVIC_Configuration(void){      
	/* 코드 설명
	ADC1 Interrupt의 NVIC를 아래와 같이 설정해줌.
	*/
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;            //USART2 IRQ 설정
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; //IRQ 우선 패리티비트 0으로 설정 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;        //Sub 패리티 0으로 설정
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           	// IRQ Channel command enable     
    NVIC_Init(&NVIC_InitStructure);								// IRQ 초기화 v
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
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET) {
		value = ADC_GetConversionValue(ADC1);
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);	
    }    
}

int main(void){
    SystemInit();        
 
    RCC_Configuration();        //RCC 레지스터 설정
    GPIO_Configuration();        //GPIO 래지스터 설정
    ADC_Configuration();
    NVIC_Configuration();        // NIVC 설정  
    
 
    while(1){
        printf("%d\n", value);
    }
}