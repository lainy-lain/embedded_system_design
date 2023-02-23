#include "stm32f10x.h"

void SysInit(void) {
    /* Set HSION bit */
    /* Internal Clock Enable */
    RCC->CR |= (uint32_t)0x00000001; //HSION

    /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
    RCC->CFGR &= (uint32_t)0xF0FF0000;

    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= (uint32_t)0xFEF6FFFF;

    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

    /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
    RCC->CFGR &= (uint32_t)0xFF80FFFF;

    /* Reset PLL2ON and PLL3ON bits */
    RCC->CR &= (uint32_t)0xEBFFFFFF;

    /* Disable all interrupts and clear pending bits  */
    RCC->CIR = 0x00FF0000;

    /* Reset CFGR2 register */
    RCC->CFGR2 = 0x00000000;
}

void SetSysClock(void) {
    volatile uint32_t StartUpCounter = 0, HSEStatus = 0;
    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/
    /* Enable HSE */
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);
    /* Wait till HSE is ready and if Time out is reached exit */
    do {
        HSEStatus = RCC->CR & RCC_CR_HSERDY;
        StartUpCounter++;
    } while ((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

    if ((RCC->CR & RCC_CR_HSERDY) != RESET) {
        HSEStatus = (uint32_t)0x01;
    }
    else {
        HSEStatus = (uint32_t)0x00;
    }

    if (HSEStatus == (uint32_t)0x01) {
        /* Enable Prefetch Buffer */
        FLASH->ACR |= FLASH_ACR_PRFTBE;
        /* Flash 0 wait state */
        FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
        FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_0;

//@TODO - Set the clock PCLK2 = 16 MHz/////////////////////////////////////////////////////////////////////////////////////////////////////////
        /* HCLK = SYSCLK */
        RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;

		/* PCLK2 = (HCLK / ?(원하는 대로 계산)) */
        RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV2;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /* PCLK1 = HCLK */
        RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV1;

//@TODO - Set the clock SYSCLK = 32 MHz 단, 계산식 제한 : 25 / 5 * 8 / 5 * 4 ////////////////////////////////////////////////////////////////////
        /* Configure PLLs ------------------------------------------------------*/
        RCC->CFGR &= (uint32_t)~(RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL);
        RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_PLLSRC_PREDIV1 | 
                                RCC_CFGR_PLLMULL4);

        RCC->CFGR2 &= (uint32_t)~(RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL | 
                                RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC);                                
        RCC->CFGR2 |= (uint32_t)(RCC_CFGR2_PREDIV2_DIV5 | RCC_CFGR2_PLL2MUL8 |
                                RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV5);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /* Enable PLL2 */
        RCC->CR |= RCC_CR_PLL2ON;
        /* Wait till PLL2 is ready */
        while ((RCC->CR & RCC_CR_PLL2RDY) == 0)
        {
        }
        /* Enable PLL */
        RCC->CR |= RCC_CR_PLLON;
        /* Wait till PLL is ready */
        while ((RCC->CR & RCC_CR_PLLRDY) == 0)
        {
        }
        /* Select PLL as system clock source */
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
        RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;
        /* Wait till PLL is used as system clock source */
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08)
        {
        }
        /* Select System Clock as output of MCO */
//@TODO - Set the MCO port for system clock output///////////////////////////////////////////////////////////////////////////////////////////
        RCC->CFGR &= ~(uint32_t)RCC_CFGR_MCO;
        RCC->CFGR |= RCC_CFGR_MCO_SYSCLK;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    }
    else {
        /* If HSE fails to start-up, the application will have wrong clock
        configuration. User can add here some code to deal with this error */
    }
}

void RCC_Enable(void) {
//@TODO - RCC Setting////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /*---------------------------- RCC Configuration -----------------------------*/
    /* GPIO RCC Enable  */
    /* UART Tx, Rx, MCO port */ 
    // PA 8 9 10
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPDEN;  //RCC설정(A,D En)

    /* USART RCC Enable */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void PortConfiguration(void) {
    /* Reset(Clear) Port A CRH - MCO, USART1 TX,RX*/
    GPIOA->CRH &= ~(
	    (GPIO_CRH_CNF8 | GPIO_CRH_MODE8) |
	    (GPIO_CRH_CNF9 | GPIO_CRH_MODE9) |
	    (GPIO_CRH_CNF10 | GPIO_CRH_MODE10)
	);
//@TODO - GPIO Configuration/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* MCO Pin Configuration */
    GPIOA->CRH |= (GPIO_CRH_MODE8 | GPIO_CRH_CNF8_1);
    /* USART Pin Configuration */
    GPIOA->CRH |= (GPIO_CRH_MODE9 | GPIO_CRH_CNF9_1 | GPIO_CRH_CNF10_1);

	/*********** 추가 기능을 위한 부분 **********/
	// 추가 기능을 위해, USER S1(PD11) 뿐만 아니라, USER S2(PD12) 또한 Reset, Configuration 해주었다.
    /* Reset(Clear) Port D CRH - User S1 Button, User S2 Button */
    GPIOD->CRH &= ~(GPIO_CRH_MODE11 | GPIO_CRH_CNF11);
    GPIOD->CRH &= ~(GPIO_CRH_MODE12 | GPIO_CRH_CNF12);
    /* User S1, S2 Button Configuration */
    GPIOD->CRH |= (GPIO_CRH_CNF11_1);
    GPIOD->CRH |= (GPIO_CRH_CNF12_1);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void UartInit(void) {
    /*---------------------------- USART CR1 Configuration -----------------------*/
    /* Clear M, PCE, PS, TE and RE bits */
    USART1->CR1 &= ~(uint32_t)(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE);
    /* Configure the USART Word Length, Parity and mode ----------------------- */
    /* Set the M bits according to USART_WordLength value */
    /* Set PCE and PS bits according to USART_Parity value */
    /* Set TE and RE bits according to USART_Mode value */
//@TODO - : Enable Tx and Rx///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	USART1->CR1 |= (USART_CR1_TE | USART_CR1_RE);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /*---------------------------- USART CR2 Configuration -----------------------*/
    /* Clear STOP[13:12] bits */
    USART1->CR2 &= ~(uint32_t)(USART_CR2_STOP);
    /* Configure the USART Stop Bits, Clock, CPOL, CPHA and LastBit ------------*/
    USART1->CR2 &= ~(uint32_t)(USART_CR2_CPHA | USART_CR2_CPOL | USART_CR2_CLKEN);
    /* Set STOP[13:12] bits according to USART_StopBits value */
    /*---------------------------- USART CR3 Configuration -----------------------*/
    /* Clear CTSE and RTSE bits */
    USART1->CR3 &= ~(uint32_t)(USART_CR3_CTSE | USART_CR3_RTSE);
    /* Configure the USART HFC -------------------------------------------------*/
    /* Set CTSE and RTSE bits according to USART_HardwareFlowControl value */
    /*---------------------------- USART BRR Configuration -----------------------*/
    /* Configure the USART Baud Rate -------------------------------------------*/
    /* Determine the integer part */
    /* Determine the fractional part */
//@TODO - : Calculate & configure BRR//////////////////////////////////////////////////////////////////////////////////////////////////////////
	USART1->BRR |=  0x1A0B;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /*---------------------------- USART Enable ----------------------------------*/
    /* USART Enable Configuration */
//@TODO - : Enable UART (UE)///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	USART1->CR1 |= USART_CR1_UE;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void delay(void){
    int i = 0;
    for(i=0;i<1000000;i++);
}

void SendData(uint16_t data) {
    /* Transmit Data */
	USART1->DR = data;

	/* Wait till TC is set */
	while ((USART1->SR & USART_SR_TC) == 0);
}

int main() {
    int i;
    char msg[] = "Team09 USART\r\n";

    SysInit();
    SetSysClock();
    RCC_Enable();
    PortConfiguration();
    UartInit();

    while (1) {
//@TODO - : Send the message when button is pressed, 1 byte씩 전송//////////////////////////////////////////////////////////////////////////////		
		// Function 1 (default function)
        if (~GPIOD->IDR & GPIO_IDR_IDR11) {
			for(i=0; msg[i]; i++) {
				SendData(msg[i] & (uint16_t)0x01FF);
				//delay();
			}
			delay();
        }

        // Function 2 (additional function)
		/*********** 추가 기능을 위한 부분 **********/
        if (~GPIOD->IDR & GPIO_IDR_IDR12) { // S2 버튼을 한 번 눌렀을 때
            while(1) {
                for(int j=0; msg[j]; j++) {
                    SendData(msg[j] & (uint16_t)0x01FF);
                }
                delay();
                if(~GPIOD->IDR & GPIO_IDR_IDR12){ // S2 버튼을 다시 눌렀을 때
                  char end_msg[] = "exit\r\n";
                    for(int k=0; end_msg[k]; k++) {
                        SendData(end_msg[k] & (uint16_t)0x01FF);
                        delay();
                    }
                    break;
                }
            }
        }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    }
}// end main
