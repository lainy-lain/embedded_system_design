/*
 * hjtest.c
 *
 *  Created on: 2019. 9. 18.
 *      Author: HJ
 */
#include <misc.h>
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
/* RCC */
#define RCC_APB2ENR			((volatile unsigned *)0x40021018)
#define RCC_APB2ENR_IOPBEN 	0x08
#define RCC_APB2ENR_IOPCEN 	0x10
#define RCC_APB2ENR_IOPDEN 	0x20
// RCC B,C,D en => 0x38

/* GPIO port config */
#define GPIO_B_CRH			((volatile unsigned *)0x40010C04)
#define GPIO_C_CRL			((volatile unsigned *)0x40011000)
#define GPIO_D_CRL			((volatile unsigned *)0x40011400)

/* GPIO BSRR */
#define GPIO_D_BSRR			((volatile unsigned *)0x40011410)

/* GPIO ODR */
#define GPIO_D_ODR			((volatile unsigned *)0x4001140C)
/* GPIO IDR */
#define GPIO_B_IDR			((volatile unsigned *)0x40010C08)
#define GPIO_C_IDR			((volatile unsigned *)0x40011008)

void APB2ClockEnable() {
	// RCC
	*(RCC_APB2ENR) |=
		RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN;
}

void joystickGpioConfig() {
	// joystick selection - PB8 input
	*GPIO_B_CRH &= ~0xf;		// reset 0
	*GPIO_B_CRH |= 0x8;		// input mode
	// joystick down - PC2
	*GPIO_C_CRL &= ~0xf00;		// reset
	*GPIO_C_CRL |= 0x800;		// input mode
	// joystick left - PC3
	*GPIO_C_CRL &= ~0xf000;	// reset
	*GPIO_C_CRL |= 0x8000;		// input mode
	// joystick RIGHT - PC4
	*GPIO_C_CRL &= ~0xf0000;	// reset
	*GPIO_C_CRL |= 0x80000;	// input mode
	// joystick UP - PC5
	*GPIO_C_CRL &= ~0xf00000;	// reset
	*GPIO_C_CRL |= 0x800000;	// input mode
}

void LedGpioConfig() {
	// LED1 - PD2 output
	*GPIO_D_CRL &= ~0xf00;		// reset
	*GPIO_D_CRL |= 0x300;		// output mode
	// LED2 - PD3
	*GPIO_D_CRL &= ~0xf000;	// reset
	*GPIO_D_CRL |= 0x3000;		// output mode
	// LED3 - PD4
	*GPIO_D_CRL &= ~0xf0000;	// reset
	*GPIO_D_CRL |= 0x30000;	// output mode
	// LED4 - PD7
	*GPIO_D_CRL &= ~0xf0000000;// reset
	*GPIO_D_CRL |= 0x30000000;	// output mode
}

void init() {
	// RCC clock
	APB2ClockEnable();

	// GPIO port config
	joystickGpioConfig();
	LedGpioConfig();
}

void LedOn(unsigned char i) {
	switch (i) {
	case 1:
		*GPIO_D_BSRR |= (0x1 << 2);
		break;
	case 2:
		*GPIO_D_BSRR |= (0x1 << 3);
		break;
	case 3:
		*GPIO_D_BSRR |= (0x1 << 4);
		break;
	case 4:
		*GPIO_D_BSRR |= (0x1 << 7);
		break;
	default:
		// nothing to do
		break;
	}
}

void LedOff(unsigned char i) {
	switch (i) {
	case 1:
		*GPIO_D_BSRR |= ((0x1 << 16) << 2);
		break;
	case 2:
		*GPIO_D_BSRR |= ((0x1 << 16) << 3);
		break;
	case 3:
		*GPIO_D_BSRR |= ((0x1 << 16) << 4);
		break;
	case 4:
		*GPIO_D_BSRR |= ((0x1 << 16) << 7);
		break;
	default:
		// nothing to do
		break;
	}
}

void delay() {
	unsigned int i;
	for (i = 0; i < 10000000; i++) {}
}
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
uint16_t prescale = 0;
void setMotor(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    prescale = (uint16_t) (SystemCoreClock / 1000000) - 1;
    
    TIM_TimeBaseStructure.TIM_Period = 20000-1;         
    TIM_TimeBaseStructure.TIM_Prescaler = prescale;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 1500;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}
void openDoor(){
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 2300;

    TIM_OC3Init(TIM3, &TIM_OCInitStructure);    
}

void closeDoor(){
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 700;
    
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
}

void resetDoor(){
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 1500;
    
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
}
int main() {

	init();
        setMotor();

	while (1) {
                resetDoor();
                delay();
                openDoor();
                delay();
                LedOn(2);
                closeDoor();
                delay();
                LedOff(2);
	}
}
