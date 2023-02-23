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
void USART2_Init(void);
void NVIC_Configure(void);

void USART1_IRQHandler(void); 
void USART2_IRQHandler(void); 
void EXTI15_10_IRQHandler(void);

void Delay(void);

void sendDataUART1(uint16_t data);
void sendDataUART2(uint16_t data);

//---------------------------------------------------------------------------------------------------

void RCC_Configure(void) // stm32f10x_rcc.c, .h 참고, 아래에 주석이 묶여있는 기능끼리 합쳐서 함수에 매개변수 전달 하시오!!
{
/////////////////////// TODO: Enable the APB2 peripheral clock using the function 'RCC_APB2PeriphClockCmd'///////////////////////
	/* USER S1(Port D) Clock Enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);    
	/* UART 1,2 TX/RX(Port A) clock enable, USART1 clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
	/* USART2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* Alternate Function IO clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configure(void) // stm32f10x_gpio.c, .h 참고
{
    GPIO_InitTypeDef GPIO_InitStructure; 
/////////////////////// TODO: Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' and the function 'GPIO_Init'///////
    /* USER S1 button pin setting, Input */
    GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_11);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	/* UART pin setting */ // 반드시 Reference Manual의 USART pinout상 Full duplex가 되는 GPIO configuration으로 설정!! 대충 설정해도 되는 경우가 있지만 Reference를 따른다에 의의를 둠
	// UART1 TX, Output
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // UART1 RX, Input
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
      
	// UART2 TX, Output
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // UART2 RX, Input
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void EXTI_Configure(void) // stm32f10x_gpio.c, .h 참고
{
    EXTI_InitTypeDef EXTI_InitStructure;
////// TODO: Select the GPIO pin (button) used as EXTI Line using function 'GPIO_EXTILineConfig'///////////////////////
	// TODO: Initialize the EXTI using the structure 'EXTI_InitTypeDef' and the function 'EXTI_Init'
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
    USART1_InitStructure.USART_BaudRate = 9600;
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

void USART2_Init(void) // stm32f10x_usart.c, .h 참고
{
	USART_InitTypeDef USART2_InitStructure;
	// Enable the USART1 peripheral
	USART_Cmd(USART2, ENABLE);
////// TODO: Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'//////////////////////////
    USART2_InitStructure.USART_BaudRate = 9600;  
	USART2_InitStructure.USART_WordLength = USART_WordLength_8b;  
	USART2_InitStructure.USART_Parity = USART_Parity_No;  
	USART2_InitStructure.USART_StopBits = USART_StopBits_1;  
	USART2_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  
    USART2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
    USART_Init(USART2, &USART2_InitStructure);  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////// TODO: Enable the USART2 RX interrupts using the function 'USART_ITConfig' and the argument value 'Receive Data register not empty interrupt'
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}


void NVIC_Configure(void) { // stm32f10x.h, misc.c, .h 참고
    NVIC_InitTypeDef NVIC_InitStructure;
////// TODO: fill the arg Priority Group 0///////////////////////////////////////////////////////////////////////////////////////
    // misc.h에 나와있는 함수의 declaration과 매크로 상수를 확인하여, NVIC Priority Group을 0으로 설정하였다.
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////// TODO: Initialize the NVIC using the structure 'NVIC_InitTypeDef' and the function 'NVIC_Init'/////////////////////////////
    // PD11(USER S1 Button) NVIC Configure
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	// USART 1,2 NVIC Configure. 'NVIC_EnableIRQ' is only required for USART setting
    NVIC_EnableIRQ(USART1_IRQn);	
    NVIC_EnableIRQ(USART2_IRQn);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void USART1_IRQHandler(void) {
    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET){
        char word = USART_ReceiveData(USART1) & 0xFF;
        sendDataUART2(word);
        // clear 'Read data register not empty' flag
    	USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
}

void USART2_IRQHandler(void) {
    if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET){
        char word = USART_ReceiveData(USART2) & 0xFF;
        sendDataUART1(word);
        // clear 'Read data register not empty' flag
    	USART_ClearITPendingBit(USART2,USART_IT_RXNE);
    }
}

int sendflag = 0; 

void EXTI15_10_IRQHandler(void) { // when the button is pressed
	if (EXTI_GetITStatus(EXTI_Line11) != RESET) {
		if (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11) == Bit_RESET) {
////////////// TODO implement, USER S1 Button 동작 작성 UART 1 Send///////////////////////////////////////////////////////////////
		sendflag = 1;	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
        EXTI_ClearITPendingBit(EXTI_Line11);
	}
}

void Delay(void) {
	int i;

	for (i = 0; i < 2000000; i++) {}
}

void sendDataUART1(uint16_t data) {
	USART_SendData(USART1, data);
}

void sendDataUART2(uint16_t data) {
	USART_SendData(USART2, data);
}

int main(void)
{
    SystemInit();
    RCC_Configure();
    GPIO_Configure();
    EXTI_Configure();
    USART1_Init();
    USART2_Init();
    NVIC_Configure();
     
	char msg[] = "Team09\r\n";
	
	while (1) {
		if (sendflag) {
			for (int i=0; msg[i]; ++i) {
				sendDataUART1(msg[i] & (uint16_t)0xFF);
				sendDataUART2(msg[i] & (uint16_t)0xFF);
				Delay();
			}
			sendflag = 0; // reset sendflag
		}
	}
	
    return 0;
}