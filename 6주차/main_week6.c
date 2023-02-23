#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"

#include "misc.h"

/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);
void EXTI_Configure(void);
void USART1_Init(void);
void NVIC_Configure(void);

void EXTI15_10_IRQHandler(void);

void Delay(void);

void sendDataUART1(uint16_t data);

//---------------------------------------------------------------------------------------------------

void RCC_Configure(void) // stm32f10x_rcc.c, .h 참고, 아래에 주석이 묶여있는 기능끼리 합쳐서 함수에 매개변수 전달 하시오!!
{
/////////////////////// TODO: Enable the APB2 peripheral clock using the function 'RCC_APB2PeriphClockCmd'///////////////////////
	
	/* 코드 설명
	stm32f10x_rcc.c에 기록된 RCC_APB2PeriphClockCmd()의 declaration을 확인하면, 첫 번째 인자로 RCC_APB2Periph가 이용됨을 알 수 있다. 
	이를 검색하여 APB2Periph를 나타내는 매크로 상수들의 그룹을 찾아냈다.(line 493, @defgroup APB2_peripheral)
	찾아낸 그룹 내에서 필요로 하는 매크로 상수들을 함수의 인자로 넘겨주었다.
	*/
	/* JoyStick Up/Down port clock enable, LED port clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);    
	/* UART 1 TX/RX port clock enable, USART1 clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* Alternate Function IO clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configure(void) // stm32f10x_gpio.c, .h 참고
{
    GPIO_InitTypeDef GPIO_InitStructure; 
/////////////////////// TODO: Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' and the function 'GPIO_Init'///////
	/* 코드 설명
	GPIO_InitTypeDef 구조체 변수인 GPIO_InitStructure 이것 하나로 모든 pin들을 Initialize할 수 있는 이유는, struct GPIO_InitTypeDef의 구조를 살펴보면 알 수 있다.
	GPIO_InitTypeDef 구조체는 uint16_t 변수 하나(GPIO_Pin)와 Speed를 나타내는 enum, Mode를 나타내는 enum으로 구성되어 있다.
	그렇기 때문에, 값을 새로 할당(assignment)하면 이전의 값은 영향을 끼치지 못하게 되므로 하나의 변수로 여러 가지 핀들을 초기화하는데 사용할 수 있는 것이다.
	또한, stm32f10x_gpio.c 파일에 정의되어 있는 GPIO_Init() 함수의 body를 살펴보면, GPIO_Mode가 Input인 경우에는
	GPIO_Speed값을 사용하지 않기 때문에, 아래 코드와 같이 Input Mode인 경우엔 GPIO_Speed값을 따로 assign하지 않아도 되는 것이다.
	
	GPIO_Pin을 나타내는 매크로 상수들, 그리고 Speed와 Mode를 나타내는 enum은 stm32f10x_gpio.h 파일에서 찾았고,
	각각 포트에 해당되는 적절한 값들을 구조체 변수에 할당한 후 GPIO_Init함수를 호출, 그리고 재할당 후 다시 GPIO_Init함수를 호출하는 작업을 반복하였다.
	
	Reference Manual의 Table 24를 참조하면, USART가 full duplex가 되게 하려면 USARTx_Tx는 GPIO config을 Alternative function push-pull (AFPP)로 설정해주어야 하고,
	USARTx_Rx는 GPIO config을 Input floating 또는 Input pull up으로 설정해주어야 함을 알 수 있다.
	*/	
	
    /* JoyStick up, down pin setting, Input */
	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_2 | GPIO_Pin_5);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    /* USER S1 button pin setting, Input */
    GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_11);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    /* LED pin setting, Output */
    GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
	
    /* UART pin setting */ // 반드시 Reference Manual의 USART pinout상 Full duplex가 되는 GPIO configuration으로 설정!! 대충 설정해도 되는 경우가 있지만 Reference를 따른다에 의의를 둠
    //TX, Output
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //RX, Input
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void EXTI_Configure(void) // stm32f10x_gpio.c, .h 참고
{
    EXTI_InitTypeDef EXTI_InitStructure;

////// TODO: Select the GPIO pin (Joystick, button) used as EXTI Line using function 'GPIO_EXTILineConfig'///////////////////////
	// TODO: Initialize the EXTI using the structure 'EXTI_InitTypeDef' and the function 'EXTI_Init'

	/* 코드 설명
	EXTI_InitTypeDef도 GPIO_InitTypeDef와 마찬가지 이유로 하나의 변수만으로 여러가지 port를 초기화 할 수 있다.
	우선, GPIO_EXITLineConfig()를 이용해 GPIO pin을 EXTI Line으로 설정해주고,
	EXTI_InitTypeDef 구조체 변수의 값을 설정한 후 EXTI_Init()함수에 인자로 넘겨줌으로써 EXTI를 설정한다.
	
	EXTI_Line은 GPIO_PinSource와 일치시켜 주었고, 우리는 Event Mode가 아니라 Interrupt Mode를 이용할 것이므로 EXTI_Mode를 Interrupt로 설정하였다.
	또한, Joystick과 USER S1 버튼을 Pull-Up으로 설정하였기 때문에, Trigger를 Falling으로 설정하였다.
	*/
    /* Joystick Down */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);
    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Joystick Up */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5);
    EXTI_InitStructure.EXTI_Line = EXTI_Line5;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* USER S1 Button */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource11);
    EXTI_InitStructure.EXTI_Line = EXTI_Line11;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);  
	// NOTE: do not select the UART GPIO pin used as EXTI Line here, EXTI Line과 Pin Source는 일치해야한다.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void USART1_Init(void) // stm32f10x_usart.c, .h 참고
{
	USART_InitTypeDef USART1_InitStructure;

	// Enable the USART1 peripheral
	USART_Cmd(USART1, ENABLE);
	
////// TODO: Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'//////////////////////////
    /*  BaudRate : 115200
        WordLength or Data : 8 bits
        Parity : None
        StopBits : 1 bit
        Hardware Flow Control of Flow Control : None
    위의 설정대로 통신 설정
     */
	 
    /* 코드 설명
	stm32f10x_usart.h를 참조하면, USART_InitTypeDef의 정의와, 구조체 파라미터들의 reference가 나와있다.
	(e.g. USART_WordLength parameter can be a value of @ref USART_Word_Length)
	파라미터들의 reference를 동일 파일에서 찾은 후, 상기된 바와 동일하게 되도록 구조체에 값을 할당했고, 그 후 USART_Init() 함수를 이용하여 USART를 초기화하였다.
	*/
    USART1_InitStructure.USART_BaudRate = 115200;
	USART1_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART1_InitStructure.USART_Parity = USART_Parity_No;
	USART1_InitStructure.USART_StopBits = USART_StopBits_1;
	USART1_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART1_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART1_InitStructure);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
////// TODO: Enable the USART1 RX interrupts using the function 'USART_ITConfig' and the argument value 'Receive Data register not empty interrupt'
    /* 코드 설명
	우리가 사용하고자 하는 USART1의 Interrupt를 'Receive Data register not empty interrupt'로 설정하기 위해, stm32f10x_usart.h의
	@defgroup USART_Interrupt_definition 부분에 나와있는 매크로 상수들 중 USART_IT_RXNE를 USART_ITConfig의 두 번째 인자로 주었다.
	*/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void NVIC_Configure(void) { // stm32f10x.h, misc.c, .h 참고

    NVIC_InitTypeDef NVIC_InitStructure;
    
    /*  Priority Group : 0
        Preemtion Priority : Joystick down = up = User S1 = UART 1
        Sub Priority 순서 : Joystick down -> up -> User S1 -> UART 1
    위의 설정대로 IRQ 우선순위 설정
     */
////// TODO: fill the arg Priority Group 0///////////////////////////////////////////////////////////////////////////////////////
    // misc.h에 나와있는 함수의 declaration과 매크로 상수를 확인하여, NVIC Priority Group을 0으로 설정하였다.
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////// TODO: Initialize the NVIC using the structure 'NVIC_InitTypeDef' and the function 'NVIC_Init'/////////////////////////////
	/* 코드 설명
	NVIC_InitTypeDef도 GPIO_InitTypeDef, EXTI_InitTypeDef와 마찬가지 이유로, 하나의 구조체 변수만으로 모든 채널에 대해서 우선순위를 설정할 수 있다.
	우선 우선순위를 설정할 IRQ Channel을 정하기 위해, stm32f10x.h에 나와있는 enum IRQn을 참조하여 설정하고자 하는 EXTI IRQ 채널에 해당되는
	enum값을 구조체의 IRQChannel 변수에 할당하고, Preemtion Priority와 Sub Priority를 설정하고, IRQ 채널을 ENABLE시킨 후
	NVIC_Init()함수를 호출하는 방식을 반복하여 Joystick과 USER S1 Button에 대한 NVIC를 설정하였다.
	
	UART의 경우에도 NVIC_InitStructure를 이용하여 NVIC_Init()을 호출하여야 하는 줄 알았으나, 그러지 않고 NVIC_EnableIRQ()만을 이용해도
	NVIC가 설정된다는 것을 확인하였기 떄문에 UART에 대해 NVIC_Init()을 사용하는 부분은 주석처리하였다.
	*/
    // Joystick Down
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // Joystick Up
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // User S1 Button
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // UART1
    /*
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	*/
    NVIC_EnableIRQ(USART1_IRQn);	// 'NVIC_EnableIRQ' is only required for USART setting

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/* 코드 설명
upflag : LED down, up 상태를 control하기 위한 전역변수. 1일 경우 up, -1일 경우 down.
sendflag : USART1을 이용한 문자 전송을 control하기 위한 전역변수. 1일 경우 문자를 전송한다.
*/
int upflag = 1;  
int sendflag = 0;

void USART1_IRQHandler() {
    uint16_t word;
    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET){
    	// the most recent received data by the USART1 peripheral
        word = USART_ReceiveData(USART1);

////////// TODO implement, UART로 키보드 입력 'd' or 'u' 에 맞게 동작 (d= down, u= up 방향으로 LED 방향 변경)//////////////////////////
		// UART로 'd' 입력이 들어올 때, upflag를 -1로 설정해서 led 방향을 down으로 변경
		if(word == 'd'){
			upflag = -1;
		}
		// UART로 'u' 입력이 들어올 때, upflag를 1로 설정해서 led 방향을 up으로 변경
		else if(word == 'u'){
			upflag = 1;
		}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // clear 'Read data register not empty' flag
    	USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
}

void EXTI15_10_IRQHandler(void) { // when the button is pressed
	if (EXTI_GetITStatus(EXTI_Line11) != RESET) {
		if (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11) == Bit_RESET) {
////////////// TODO implement, USER S1 Button 동작 작성 UART 1 Send///////////////////////////////////////////////////////////////
			// sendflag를 1로 설정해서 문자를 전송하도록 만듬.
			sendflag = 1;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
        EXTI_ClearITPendingBit(EXTI_Line11);
	}
}

// "EXTI2_IRQHandler", "EXTI9_5_IRQHandler" 중 맞는 것으로 아래 두개 IRQ 동작 완성////////////////////////////////////////////////////
// TODO: Create Joystick up interrupt handler functions (up 방향으로 LED 방향 변경)
void EXTI9_5_IRQHandler(void) { 
	if (EXTI_GetITStatus(EXTI_Line5) != RESET) {
		if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == Bit_RESET) {
////////////// TODO implement, USER S1 Button 동작 작성 UART 1 Send///////////////////////////////////////////////////////////////
			// 조이스틱 up일때, upflag를 1로 설정해서 led 방향이 up이 되도록 만듬.
			upflag = 1;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
        EXTI_ClearITPendingBit(EXTI_Line5);
	}
}
// TODO: Create Joystick down interrupt handler functions (down 방향으로 LED 방향 변경)
void EXTI2_IRQHandler(void) { 
	if (EXTI_GetITStatus(EXTI_Line2) != RESET) {                
		if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == Bit_RESET) {
////////////// TODO implement, USER S1 Button 동작 작성 UART 1 Send///////////////////////////////////////////////////////////////
			// 조이스틱 down일때, upflag를 -1로 설정해서 led 방향이 down이 되도록 만듬.
			upflag = -1;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
        EXTI_ClearITPendingBit(EXTI_Line2);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void Delay(void) {
	int i;

	for (i = 0; i < 2000000; i++) {}
}

void sendDataUART1(uint16_t data) {
	/* Wait till TC is set */
	while ((USART1->SR & USART_SR_TC) == 0);
	USART_SendData(USART1, data);
}

int main(void)
{

    SystemInit();

    RCC_Configure();

    GPIO_Configure();

    EXTI_Configure();

    USART1_Init();

    NVIC_Configure();
     
    int cnt=0;  //LED 물결 방향 변경시의 LED 번호를 기억하기 위한 변수
    char msg[] = "Team09 USART\r\n";  //USER S1 버튼 입력시 출력될 문자
    while (1) {
////////// TODO: implement //////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    1. 4개 LED가 순차로 아래방향으로 흐르듯이 켜지고 꺼지는 동작
    2. Joystick up 과 UART1 키보드 입력 'u'에 맞춰서 up(윗) 방향으로 LED가 흐르듯이 켜지고 꺼지는 동작으로 전환
    3. Joystick down 과 UART1 키보드 입력 'd'에 맞춰서 down(아랫) 방향으로 LED가 흐르듯이 켜지고 꺼지는 동작으로 전환
    4. S1 버튼 입력에 맞춰서 UART1으로 "TEAM00" SEND (00은 각 조번호로)
*/      

/* 코드 설명
4개의 LED Pin에 대한 배열을 정의하였고, 이를 이용해 순차적인 LED 점등 및 자연스러운 방향 전환이 가능하도록 구현하였다.
우선, cnt % 4 값에 해당되는 LED Pin을 점등한 후(Set Bit), Delay를 가진 후 다시 소등한다(Reset Bit).
이후 cnt에 upflag값을 더해주게 되는데, upflag는 LED 방향이 UP이면 1의 값을 가지고, DOWN이면 -1의 값을 가지게 된다.
즉, LED UP인 경우엔 0123 0123의 형태로 LED가 켜질 것이고, DOWN인 경우엔 3210 3210의 형태로 LED가 켜질 것이다.
또한 이렇게 upflag값을 직접 더해주었기 떄문에 자연스러운 방향 전환이 이루어지게 된다.

error control이라고 되어있는 부분을 작성한 이유는, 이론적으로 C에선 음수에 modulo(%) 연산을 취해도 양수가 나와야 하지만, 
인터럽트를 bursty하게 입력하면 cnt % 4의 값이 음수가 나오는 현상이 발생하였기 때문이다.
즉, 배열의 index가 음수가 되는 것을 방지하기 위해 추가한 부분이다.

USER S1 버튼이 눌려 인터럽트가 발생하면 sendflag가 1로 변경되게 되어 if(sendflag)문이 실행되고 메시지가 전송된다.
이후, sendflag를 0으로 초기화하여 문자가 계속 전송되는 것을 막는다.
*/
        uint16_t pin_arr[4] = {GPIO_Pin_2, GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_7};
              
        GPIO_SetBits(GPIOD, pin_arr[cnt%4]);
        Delay();
        GPIO_ResetBits(GPIOD, pin_arr[cnt%4]);
        cnt += upflag; //down시 upflag = - 1, up시 upflag = 1
        if (cnt < 0) { // error control
			cnt += 4;
        }
            
        if(sendflag) {
			for(int i=0; msg[i]; i++) {
				sendDataUART1(msg[i] & (uint16_t)0x01FF);
			}
			Delay();
			sendflag = 0; // sendflag를 0으로 초기화.
        }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    	// Delay
		Delay();
    }
    return 0;
}