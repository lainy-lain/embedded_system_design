#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "stm32f10x_exti.h"
#include "lcd.h"
#include "touch.h"


/* 직사각형을 그리기 위한 4개의 좌표 */
#define XSTART 		60
#define YSTART 		100
#define XEND 		100
#define YEND 		140


int color[12] ={WHITE,CYAN,BLUE,RED,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};

GPIO_InitTypeDef GPIO_InitStructure;                    // GPIO 초기화 변수 선언
NVIC_InitTypeDef NVIC_InitStructure;                    // 중첩 인터럽트 처리위한 NVIC 초기화 변수 선언    
 
void RCC_Configuration(void){    
	// APB2 RCC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	// APB1 RCC
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
}           
 
void GPIO_Configuration(void){
	/* LCD Configure */
	// LCD_RD (PD15) Configure
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;              
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    
    GPIO_Init(GPIOD, &GPIO_InitStructure);        
	// LCD_RS (PD13) Configure
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;                               
    GPIO_Init(GPIOD, &GPIO_InitStructure);        
    // LCD_WR (PD14) Configure
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;       
    GPIO_Init(GPIOD, &GPIO_InitStructure);        
    // LCD_CS (PC6) Configure
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;                              
    GPIO_Init(GPIOC, &GPIO_InitStructure);        
    
	/* LED Configure */
	// LED1 (PD2) Configure
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;                           
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    
    GPIO_Init(GPIOD, &GPIO_InitStructure);        
	// LED2 (PD3) Configure
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;                           
    GPIO_Init(GPIOD, &GPIO_InitStructure);        
	// LED3 (PD4) Configure
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;                           
    GPIO_Init(GPIOD, &GPIO_InitStructure);        
	// LED4 (PD7) Configure
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;                           
    GPIO_Init(GPIOD, &GPIO_InitStructure);        
	
	/* PWM Configure */
	// TIM3 CH3 (PB0) Configure
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
} // end of GPIO_Configuration()
 
void NVIC_Configuration(void){      
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;              //TIM2 IRQ 설정
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; //IRQ 우선 패리티비트 0으로 설정 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;        //Sub 패리티 0으로 설정
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           	// IRQ Channel command enable     
    NVIC_Init(&NVIC_InitStructure);								// IRQ 초기화 
}

static void LCD_Delay(uint32_t nCount)
{
	__IO uint16_t i;
	for (i = 0; i < (nCount * 5); i++)
	{
		;
	}
}


void LedOn(unsigned char i) {
	switch (i) {
	case 1:
		// *GPIO_D_BSRR |= (0x1 << 2);
		GPIO_SetBits(GPIOD, GPIO_Pin_2);
		break;
	case 2:
		// *GPIO_D_BSRR |= (0x1 << 3);
		GPIO_SetBits(GPIOD, GPIO_Pin_3);
		break;
	case 3:
		// *GPIO_D_BSRR |= (0x1 << 4);
		GPIO_SetBits(GPIOD, GPIO_Pin_4);
		break;
	case 4:
		// *GPIO_D_BSRR |= (0x1 << 7);
		GPIO_SetBits(GPIOD, GPIO_Pin_7);
		break;
	default:
		// nothing to do
		break;
	}
}

void LedOff(unsigned char i) {
	switch (i) {
	case 1:
		// *GPIO_D_BSRR |= ((0x1 << 16) << 2);
		GPIO_ResetBits(GPIOD, GPIO_Pin_2);
		break;
	case 2:
		// *GPIO_D_BSRR |= ((0x1 << 16) << 3);
		GPIO_ResetBits(GPIOD, GPIO_Pin_3);
		break;
	case 3:
		// *GPIO_D_BSRR |= ((0x1 << 16) << 4);
		GPIO_ResetBits(GPIOD, GPIO_Pin_4);
		break;
	case 4:
		// *GPIO_D_BSRR |= ((0x1 << 16) << 7);
		GPIO_ResetBits(GPIOD, GPIO_Pin_7);
		break;
	default:
		// nothing to do
		break;
	}
}


TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
uint16_t prescale = 0;


void setTIMER2() {
    // (SysCoreClock = 72MHz) / 120000 = 600
	prescale = (uint16_t) (SystemCoreClock / 120000) - 1;
    
    TIM_TimeBaseStructure.TIM_Period = 60000-1;         
    TIM_TimeBaseStructure.TIM_Prescaler = prescale;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}


void setMotor(){
	// (SysCoreClock = 72MHz) / 1000000 = 72
    prescale = (uint16_t) (SystemCoreClock / 1000000) - 1;
    
    TIM_TimeBaseStructure.TIM_Period = 20000-1;         
    TIM_TimeBaseStructure.TIM_Prescaler = prescale;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 700; // 모터의 초기위치 설정
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}


void rotate0(){
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 700;
    
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
}


void rotate180(){
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 2300;

    TIM_OC3Init(TIM3, &TIM_OCInitStructure);    
}


uint8_t led1Flag = 0;
uint8_t led3Flag = 0;
uint8_t tCounter = 0;

void TIM2_IRQHandler(void) {
  tCounter++;
  led1Flag = 1 - led1Flag;
    
  if (tCounter >= 10){
    led3Flag = 1 - led3Flag;
    tCounter = 0;
  }
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}


int main(void){
    SystemInit();        
 
    RCC_Configuration();        
    GPIO_Configuration();        
    NVIC_Configuration();        
    
    LCD_Init();
    Touch_Configuration();
    Touch_Adjust();
    LCD_Clear(WHITE);
    
    setTIMER2();
	setMotor();
    
	uint16_t x, y;
    uint16_t px, py;
    char* toggle[2] = {"ON   ", "OFF"}; // LED 토글 ON / OFF를 나타내기 위한 string 배열
    uint8_t index = 1; // 위 string 배열의 index를 나타내기 위한 변수
	uint8_t beforeLed1Flag = 0; // led1Flag의 이전 값을 담기 위한 변수
    uint8_t beforeLed3Flag = 0; // led3Flag의 이전 값을 담기 위한 변수

    while(1){
		// "LCD_Team09"를 LCD에 출력
		LCD_ShowString(0, 0, "LCD-Team09", BLUE, WHITE);
		// index값에 따라 toggle이 ON상태인지 OFF상태인지를 출력. index값은 버튼 터치 시 변경(아래부분 참조)
		LCD_ShowString(0, 50, toggle[index], BRRED, WHITE);
		// 미리 정의된 매크로 상수를 이용해 직사각형 모양의 버튼 생성
		LCD_DrawRectangle(XSTART, YSTART, XEND, YEND);
		// 버튼 안에 BUT라는 문자열 출력
		LCD_ShowString(70, 120, "BUT", BRRED, WHITE);
		// 터치된 좌표를 받아와서 변환하는 과정. 만약 터치가 되지 않았다면 해당 변수들(x, y, px, py)에 제대로 된 값이 들어가지 않음.
		Touch_GetXY(&x, &y, 0);
		Convert_Pos(x, y, &px, &py);
          
		if(T_INT == 0){ // 터치 인터럽트(이벤트)가 발생한 경우
			if(px>XSTART && px<XEND && py>YSTART && py<YEND){
				// 만약 버튼 영역을 눌렀다면, index값을 변경하여 토글 상태가 변경되었음을 알림.
				index = 1-index;
			}            
			LCD_Delay(12000);
		}
		
		if(!index){ // index == 0인 경우, 즉 toggle 상태가 ON인 경우를 나타냄(char* toggle[] 참조)
			if(led3Flag){
				LedOn(3);
				if (beforeLed3Flag != led3Flag)
					rotate180();
			}
			else{
				LedOff(3);
				if (beforeLed3Flag != led3Flag)
					rotate180();
			}
			
			if(led1Flag){
				LedOn(1);
				if (beforeLed1Flag != led1Flag)
					rotate0();
			}
			else{
				LedOff(1);
				if (beforeLed1Flag != led1Flag)
					rotate0();
			}
		}
		else{ // toggle off인 경우. 모든 동작 해제.
			LedOff(1);
			LedOff(3);
			rotate0();
		}
        
		// 현재 플래그 값을 이전 플래그 값에 할당.
		beforeLed1Flag = led1Flag;
		beforeLed3Flag = led3Flag;
    } // end of while
} // end of main()