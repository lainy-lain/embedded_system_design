#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "stm32f10x_exti.h"
#include "lcd.h"
#include "touch.h"

/* 코드 설명
직사각형을 그리기 위한 4개의 좌표를 매크로 상수로 정의해두었다. (LCD_DrawRectangle()에서 사용)
*/
/* 직사각형을 그리기 위한 4개의 좌표 */
#define XSTART 		60
#define YSTART 		100
#define XEND 		100
#define YEND 		140


int color[12] ={WHITE,CYAN,BLUE,RED,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};

GPIO_InitTypeDef GPIO_InitStructure;                    // GPIO 초기화 변수 선언
NVIC_InitTypeDef NVIC_InitStructure;                    // 중첩 인터럽트 처리위한 NVIC 초기화 변수 선언    
 
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
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
}           
 
void GPIO_Configuration(void){
	/* 코드 설명
	LCD 부분은 10주차 코드와 동일하다.
	LED 부분은 제공된 PWM Test Code의 LedGpioConfig()과 동일하게 GPIO 설정해주었다. (PD2,3,4,7 GP output push-pull 50MHz로 설정)
	PWM을 위한 TIM3 부분은 제공된 PWM Test Code의 setMotor()와 동일하게 작성하였다.
	Speed와 Mode가 동일한 경우엔 코드의 redundancy를 줄이기 위해 구조체의 Pin값만 바꿔주었다.
	*/
	
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

static void LCD_Delay(uint32_t nCount)
{
	__IO uint16_t i;
	for (i = 0; i < (nCount * 5); i++)
	{
		;
	}
}

/* 코드 설명 : LedOn() and LedOff()
제공된 PWM test code의 LedOn()과 LedOff()에서, BSRR 레지스터에 직접 접근하여 값을 조작하는 문장을 함수를 이용해 조작하도록 변경해주었다.
Reference 9.2.5를 살펴보면, GPIOx_BSRR의 하위 16bit은 Bit Set Register이고 상위 16bit은 Bit Reset Register이므로
LedOn()의 경우 GPIO_SetBits() 함수를 이용해주었고 LedOff()의 경우 GPIO_ResetBits() 함수를 이용해주었다.
*/
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

/* 코드 설명
제공된 PWM test code의 setMotor()의 내용을 그대로 쓰되, 분주를 위해 prescale과 period값을 변경해주고, 서보모터의 최초 위치를 설정하기 위해 TIM_Pulse값을 변경해주었다.
TIM3의 주파수를 50Hz로 만들기 위해, prescale을 72, period를 20000으로 설정해주었다.
( (f_clk = 72MHz) / 72 / 20000) = 50Hz )
또한, 모터의 초기위치를 TIM_Pulse = 700으로 설정하였다.
*/
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

/* 코드 설명
제공된 PWM test code의 openDoor() 함수를 그대로 쓰되, TIM_Pulse값만 변경해주었다.
rotate0는 모터의 각도를 0도로 설정(초기 위치로 설정)한다는 의미이며, 그렇기에 setMotor()에서와 같이
모터의 초기위치를 TIM_Pulse = 700으로 설정해 주었다.
*/
void rotate0(){
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 700;
    
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
}

/* 코드 설명
제공된 PWM test code의 openDoor() 함수를 그대로 쓰되, TIM_Pulse값만 변경해주었다.
rotate180은 모터의 각도를 180도로 설정(초기 위치에서 180도 회전)한다는 의미이다.
초기 위치 TIM_Pulse = 700에 대해 180도 회전시키려면, TIM_Pulse = 2300으로 설정하면 된다는 것을 실험적으로 파악해서
TIM_Pulse = 2300으로 설정해 주었다.
*/
void rotate180(){
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 2300;

    TIM_OC3Init(TIM3, &TIM_OCInitStructure);    
}


/* 코드 설명
led1Flag와 led3Flag는 led1과 led3를 켤지 말지를 나타내어주는 전역변수이다.
타이머 인터럽트 핸들러에서, 인터럽트에 의해 일정 주기마다 ledFlag값들이 변하도록  설정해 주었다.
led1은 2Hz 주기로 토글되게, led3는 0.2Hz 주기로 토글되게 설정하기 위해서, 2Hz 주기로 발생하는 TIM2 인터럽트가 10번 발생하면 led3를 토글시키는 방법을 택했다. 
그리고 이 '10번'을 카운트하기 위해 tCounter라는 전역변수를 추가로 선언해주었다.
*/
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
		
		/* 코드 설명
		LED의 경우, 특정 기간동안(Flag가 1인 동안 또는 0인 동안) LED on/off 함수를 반복해서 불러도 달라지는 것이 없기 때문에 플래그 값을 검사해서 바로 on/off 함수를 호출해도 상관없다.
		하지만, 모터의 경우 특정 기간동안 회전하는 작업이 반복되어선 안된다. 즉, 플래그가 바뀐 순간에 딱 한번만 회전이 수행되어야 한다.
		이를 위해서, 이전 플래그 상태와 현재 플래그 상태를 비교해서 값이 다를때만(플래그가 바뀐 순간) 모터가 회전하도록 구현하였다.
		*/
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
    }
}