#define PD_BASE 0x40011400
#define PC_BASE 0x40011000
#define CRH_OFFSET 0x04
#define CRL_OFFSET 0x00

#define PD_CRH_INIT 0xF000
#define PC_CRL_INIT 0xFFFF00
#define PD_CRH_INPUT_SET 0x8000
#define PC_CRL_INPUT_SET 0x888800

#define PD_CRL_INIT 0xF00FFF00
#define PD_CRL_OUTPUT_SET 0x30033300

#define IDR_OFFSET 0x08
#define IDR11_CHECK 0x800
#define IDR2_CHECK 0x4
#define IDR3_CHECK 0x8
#define IDR4_CHECK 0x10
#define IDR5_CHECK 0x20

#define BSRR_OFFSET 0x10
#define BSRR_BR2 0x40000
#define BSRR_BR3 0x80000
#define BSRR_BR4 0x100000
#define BSRR_BR7 0x800000

int main(){
	// Reset and Clock Control. RCC
	*((volatile unsigned *)0x40021018) |= 0x20; 
	
	
	/* **** PORT CONFIGURATION PART **** */
	// USER S1(PD11) 초기화
	*(volatile unsigned *)(PD_BASE + CRH_OFFSET) &= ~PD_CRH_INIT;
	// USER S1(PD11) input mode(pull-up)으로 설정
	*(volatile unsigned *)(PD_BASE + CRH_OFFSET) |= PD_CRH_INPUT_SET;
	
	// 조이스틱 버튼(PC2,3,4,5) 초기화
	*(volatile unsigned *)(PC_BASE + CRL_OFFSET) &= ~PC_CRL_INIT;
	// 조이스틱 버튼(PC2,3,4,5) input mode(pull-up)으로 설정
	*(volatile unsigned *)(PC_BASE + CRL_OFFSET) |= PC_CRL_INPUT_SET;
	
	// LED(PD2,3,4,7) 초기화
	*(volatile unsigned *)(PD_BASE + CRL_OFFSET) &= ~PD_CRL_INIT;
	// LED(PD2,3,4,7) General purpose output push-pull 50MHZ로 설정
	*(volatile unsigned *)(PD_BASE + CRL_OFFSET) |= PD_CRL_OUTPUT_SET;
	
	
	/* **** PORT CONTROL PART **** */
	while (1){
		// if pushing USER S1 (PD11)
		if(! (*((volatile unsigned *)(PD_BASE + IDR_OFFSET)) & IDR11_CHECK)){ 
			// if pushing UP (PC5)
			if(! (*((volatile unsigned *)(PC_BASE + IDR_OFFSET)) & IDR5_CHECK)){
				*((volatile unsigned *)(PD_BASE + BSRR_OFFSET)) |= BSRR_BR7;
			}
			// if pushing LEFT (PC3)
			if(! (*((volatile unsigned *)(PC_BASE + IDR_OFFSET)) & IDR3_CHECK)){
				*((volatile unsigned *)(PD_BASE + BSRR_OFFSET)) |= BSRR_BR4;
			}
			// if pushing DOWN (PC2)
			if(! (*((volatile unsigned *)(PC_BASE + IDR_OFFSET)) & IDR2_CHECK)){
				*((volatile unsigned *)(PD_BASE + BSRR_OFFSET)) |= BSRR_BR3;
			}
			// if pushing RIGHT (PC4)
			if(! (*((volatile unsigned *)(PC_BASE + IDR_OFFSET)) & IDR4_CHECK)){
				*((volatile unsigned *)(PD_BASE + BSRR_OFFSET)) |= BSRR_BR2;
			}
		}
		else{ // not pushing USER S1 (PD11)
			// reset value
			*((volatile unsigned *)(PD_BASE + BSRR_OFFSET)) &= 0x0;
		}
	}
	
}