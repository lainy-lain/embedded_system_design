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
    USART1_InitStructure.USART_BaudRate = 115200;
	USART1_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART1_InitStructure.USART_Parity = USART_Parity_No;
	USART1_InitStructure.USART_StopBits = USART_StopBits_1;
	USART1_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART1_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART1_InitStructure);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
////// TODO: Enable the USART1 RX interrupts using the function 'USART_ITConfig' and the argument value 'Receive Data register not empty interrupt'
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////// TODO: Initialize the NVIC using the structure 'NVIC_InitTypeDef' and the function 'NVIC_Init'/////////////////////////////
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