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
		/* -- 코드 설명 --
		- HCLK를 SYSCLK과 동일하게 설정하겠다는 것은 AHB prescaler에서 SYSCLK을 divide하지 않겠다는 의미이다.
		- Reference의 RCC_CFGR를 확인하면 AHB prescaler 값을 설정하는 부분은 HPRE라고 표기되어 있다.
		이러한 사실들을 종합하여, 헤더 파일에서 HPRE를 검색하여 <SYSCLK not divided>를 나타내는 매크로 상수인
		RCC_CFGR_HPRE_DIV1을 선택하여 CFGR에 적용(or연산)해 주었다.
		*/
        RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;

		/* PCLK2 = (HCLK / ?(원하는 대로 계산)) */
		/* -- 코드 설명 --
		- PCLK2는 SYSCLK = HCLK = 32MHz의 절반인 16MHz가 되어야 한다.
		- 즉, APB2 prescaler에서 SYSCLK = HCLK를 2로 divide해줘야 한다.
		- Reference의 RCC_CFGR를 확인하면, APB2 prescaler 값을 설정하는 부분은 PPRE2라고 표기되어 있다.
		이러한 사실들을 종합하여, 헤더 파일에서 PPRE2를 검색하여, <HCLK divided by 2>를 나타내는 매크로 상수인
		RCC_CFGR_PPRE_DIV2를 선택하여 CFGR에 적용(or연산)해 주었다.
		*/
        RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV2;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /* PCLK1 = HCLK */
        RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV1;

//@TODO - Set the clock SYSCLK = 32 MHz 단, 계산식 제한 : 25 / 5 * 8 / 5 * 4 ////////////////////////////////////////////////////////////////////
        /* Configure PLLs ------------------------------------------------------*/
		/* --- 코드 설명 --
		아래에 나와있는 상수들은, PLLXTPRE를 제외하고는 모두
		PLLCLK을 HSE OSC / PREDIV2 * PLL2MUL / PREDIV1 * PLLMUL 으로 만들기 위해
		HSE CLOCK이 Clock Tree에서 거치게 되는 Register와 MUX의 명칭을 나타낸다.
		PLLXTPRE는 Clock Tree에는 표시되어 있지 않지만, Reference에는 PLL을 위해 HSE를 2로 나누는 역할을 하는
		부분이라고 나와 있다. 우리는 이 부분을 사용하지 않을 것이므로 초기화한 뒤 해당 부분의 bit을 0으로 만드는 매크로 상수를 적용(or연산)하였다.
		Clock Tree에 표시된 나머지 부분들은, Register의 경우 PREDIV2 = 5, PLL2MUL = 8, PREDIV1 = 5, PLLMUL = 4가 되도록 하는
		매크로 상수를 적용하였고, MUX의 경우 PREDIV1SRC는 HSE가 아니라 PLL2가 선택되도록, PLLSRC는 HSI_Div2 (HSI / 2)가 아니라 PREDIV1가
		선택되도록 하는 매크로 상수를 적용하였다.
		*/
        RCC->CFGR &= (uint32_t)~(RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL);
        RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLMULL4);

        RCC->CFGR2 &= (uint32_t)~(RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL | RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC);
        RCC->CFGR2 |= (uint32_t)(RCC_CFGR2_PREDIV2_DIV5 | RCC_CFGR2_PLL2MUL8 | RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV5);
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
		/* -- 코드 설명 --
		MCO(Microcontroller Clock Output)를 System Clock으로 설정하기 위해,
		CFGR에서 해당 부분(MCO bit)을 초기화 후 System Clock이 출력으로 선택되게 하는 매크로 상수를 선택하여 적용하였다.
		*/
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
    /* UART Tx, Rx, MCO port */ // pA 8 9 10
	/* -- 코드 설명 --
	Datasheet의 Table 5. Pin definitions에 의하면,
	MCO는 PA8, USART Tx는 PA9, USART Rx는 PA10 GPIO 포트를 사용한다.
	또한, 실험에서 사용하고자 하는 USER S1 버튼은 PD11 GPIO 포트를 사용한다.
	이러한 Port들을 사용하기 위해, IO Port A와 IO Port D를 Enable해주었다.
	*/
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPDEN;  //RCC설정(A,D En)

    /* USART RCC Enable */
	/* -- 코드 설명 --
	USART1의 clock을 enable하기 위해, RCC_APB2ENR 레지스터의 USART1EN 부분을 1로 만들어주는
	매크로 상수를 선택하여 적용(or연산)하였다.
	*/
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
	/* -- 코드 설명 --
	MCO(PA8)는 Alternate function output Push-pull이 되어야 하므로
	Mode : 11, CNF : 10이 되어야 한다.
	*/
    GPIOA->CRH |= (GPIO_CRH_MODE8 | GPIO_CRH_CNF8_1);
    /* USART Pin Configuration */
	/* -- 코드 설명 --
	USART1 Tx(PA9)는 MCO(PA8)과 마찬가지로 Mode : 11, CNF : 10이 되어야 한다.
	USART1 Rx(PA10)은 Input with pull-up / pull-down이 되어야 하므로
	Mode : 00, CNF : 10이 되어야 한다.
	*/
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
    /* -- 코드 설명 --
	USART Tx와 Rx를 Enable하기 위해, USART Control Register (USART_CR1)의
	TE(Transmitter enable) bit와 RE(Receiver enable) bit를 1로 만들어주는
	매크로 상수를 적용(or연산)해 주었다.
	*/
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
    /* -- 코드 설명 --
	설정하고자 하는 Baud Rate는 2400이고, f_ck(input clock to the peripheral, 'PCLK2' for USART1)는
	16M = 16000000 이므로, Reference의 Fractional baud rate generation 부분에 나와있는 식을 이용하면
	구하고자 하는 USARTDIV = f_ck / (16 * Baud_rate) = 16M / (16 * 2400) = 416.666 임을 알 수 있다.
	(즉, 현재 clock에서는 USARTDIV가 416.666이어야 Baud Rate가 2400이 된다는 의미이다.)
	그리고, Reference에서 How to derive USARTDIV from USART_BRR register values 부분을 참조하여
	USARTDIV = 416.666 값을 만들기 위한 USART_BRR register 값을 구해보면
	DIV_Mantissa = 0d416 = 0x1A0
	DIV_Fraction = 16 * 0d0.666 = 0d10.656 --> 0xB
	==> USART_BRR = 0x1A0B 임을 알 수 있다.
	즉, USART_BRR register의 값을 0x1A0B로 만들어 주어야 한다.
	그러기 위해선, USART->BRR = 0x1A0B와 같이 바로 값을 Assignment해주어도 되지만,
	USART_BRR은 0x0000으로 초기화되므로 0x1A0B를 or연산해주어도 조건을 만족한다.
	*/
	USART1->BRR |=  0x1A0B;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /*---------------------------- USART Enable ----------------------------------*/
    /* USART Enable Configuration */
//@TODO - : Enable UART (UE)///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* -- 코드 설명 --
	USART를 Enable하기 위해, USART_CR1 register의 USART enable(UE) bit를 1로 만들어주는
	매크로 상수를 적용(or연산)해 주었다.
	*/
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
        /* -- 코드 설명 --
		S1 버튼이 눌릴 때마다 SendData 함수를 이용하여 msg를 전송한다. 
		delay()를 사용하여 데이터가 송신되는 속도를 조절하였다.
        */
		if (~GPIOD->IDR & GPIO_IDR_IDR11) {
			for(i=0; msg[i]; i++) {
				SendData(msg[i] & (uint16_t)0x01FF);
				//delay();
			}
			delay();
        }

        // Function 2 (additional function)
		/*********** 추가 기능을 위한 부분 **********/
        /* -- 코드 설명 --
		S2 버튼이 눌리면 다시 S2 버튼이 눌리기 전까지 계속해서 SendData 함수를 이용하여 msg를 전송한다.
		다시 버튼을 누를 때는 exit이라는 메세지를 SendData함수를 사용하여 출력하고 msg 출력을 멈춘다.

		버튼을 한 번 누를 때, 한 번만 신호가 가게끔 짧게 눌러야하는데, 실제로는 그만큼 짧게 누르기가 어려워 여러번 신호가 전달된다.
		또한, 짧게 눌렀다 하더라도 bouncing 현상에 의해 신호가 여러 번 전달될 가능성이 존재한다.
		따라서, delay()를 사용하여 신호가 한 번만 전달될 수 있도록 하였다.
		*/
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
