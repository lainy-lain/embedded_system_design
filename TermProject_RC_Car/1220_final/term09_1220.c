#include "stm32f10x.h"
#include "misc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"


// DC모터 속도 관련 상수 
#define DC_SPEED_FAST	 		0xFFFE
#define DC_SPEED_SLOW	 		0x0FFF
// 서보모터 회전각 관련 상수 
#define SERVO_ROTATE_INIT		        1350
#define SERVO_ROTATE_RIGHT		700
#define SERVO_ROTATE_LEFT		2300
// 압력센서 임계값 
#define PRESSURE_THRESHOLD		600
// 초음파센서 임계값
#define DISTANCE_THRESHOLD		5
// USART2를 통해 휴대폰으로 전송할 메시지
#define MSG_RUNMODE_STOP        "Runmode: STOP"
#define MSG_RUNMODE_FORWARD     "Runmode: Move Forward Only"
#define MSG_RUNMODE_BACKWARD    "Runmode: Move Backward Only"
// 운행 명령어 목록 (char여야 함)
#define OP_GO_FORWARD           'w'
#define OP_GO_FORWARD_LEFT      'q'
#define OP_GO_FORWARD_RIGHT     'e'
#define OP_GO_BACKWARD          's' 
#define OP_GO_BACKWARD_LEFT     'a'
#define OP_GO_BACKWARD_RIGHT    'd'
#define OP_STOP                 'p'
#define OP_SPEED_FAST           'm'
#define OP_SPEED_SLOW           'n'
// 초음파센서 핀 관련 상수
#define SONIC_FRONT_TRIG		10
#define SONIC_FRONT_ECHO		12
#define SONIC_BEHIND_TRIG		1
#define SONIC_BEHIND_ECHO		3
// 초음파센서 사용을 위한 레지스터 주소
#define PC_CRH		*(volatile uint32_t *)0x40011004
#define PC_ODR		*(volatile uint32_t *)0x4001100c
#define PC_IDR		*(volatile uint32_t *)0x40011008

// 차량 운행 모드를 나타내는 enum
typedef enum _RunningMode {
    RUNMODE_DEFAULT,    // 일반 모드
    RUNMODE_FORWARD,    // 전진 모드
    RUNMODE_BACKWARD,   // 후진 모드
    RUNMODE_STOP        // 정지 모드
} RunningMode;

// 현재 차량 운행 상태를 나타내는 enum
typedef enum _RunningState {
    RUNSTATE_FORWARD,   // 전진 상태
    RUNSTATE_BACKWARD,  // 후진 상태
    RUNSTATE_STOP       // 정지 상태
} RunningState;


/////////////////////////   Configure 함수에서 쓰이는 전역변수들   /////////////////////////////////////
// Configuration을 위한 구조체 변수
GPIO_InitTypeDef    GPIO_InitStructure;
USART_InitTypeDef   USART_InitStructure;         
NVIC_InitTypeDef    NVIC_InitStructure; 

// 타이머 설정을 위한 구조체 변수
TIM_TimeBaseInitTypeDef     TIM_TimeBaseStructure;
TIM_OCInitTypeDef           TIM_OCInitStructure;

// TIM prescale 설정을 위한 변수
uint16_t prescale = 0;
///////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////  사용할 함수들의 Declaration   /////////////////////////////////////
// Configuration 함수
void RCC_Configuration(void);     
void GPIO_Configuration(void);
void ADC_Configuration(void);
void USART2_Init(void); // for Bluetooth
void NVIC_Configuration(void); // for ADC, USART
void setTIMER2(void); // set Counter
void DCMotor_PWM_Configuration(void);
void ServoMotor_PWM_Configuration(void); 

// DC모터 회전/속도관련 함수
void setDCMotor_goFoward(void);
void setDCMotor_goBackward(void);
void setDCMotor_stop(void);
void setDCMotor_speedFast(void);
void setDCMotor_speedSlow(void);

// 서보모터 회전 관련 함수
void setServoMotor_rotateInit(void);
void setServoMotor_rotateRight(void);
void setServoMotor_rotateLeft(void);

// 초음파센서로 거리측정하는 함수 (Interface)
uint32_t getFrontDistance(void); 
uint32_t getBehindDistance(void);
// 위 Interface 내부에서 거리를 구하기 위해 사용되는 함수 2개 (Implementation)
void trig_pulse(uint16_t trigPin);
uint32_t echo_time(uint16_t trigPin, uint16_t echoPin);

// 부저 관련 함수
void turnOnBuzzer(void);
void turnOffBuzzer(void);

// 블루투스 관련 함수 
void sendStringUSART2(char *str); // USART2로 문자열 전송하는 함수
void sendRunModeToPhone(RunningMode mode); // 폰으로 현재 운전모드값 전송하는 함수

// Interrupt Handler
void ADC1_2_IRQHandler(void);
void USART2_IRQHandler(void); 

// 딜레이 관련 함수
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

// main while문 내부에서 사용할 함수
void setPolicy(void);
void checkRunmodeAndChange(void);
void alertIfObstruct(void);
void executeOperation(void);
///////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////  센서값 관련 변수  ///////////////////////////////////////////////////////
uint32_t pressureValue = 0; // 압력센서값을 ADC로 받아오기 위한 변수
char inputOperation = '\0'; // Bluetooth input값을 받기 위한 변수
///////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////  운전모드 관련 변수 //////////////////////////////////////////
RunningMode policy = RUNMODE_DEFAULT; // 차량 운행 모드에 관한 'Policy'를 나타내는 변수.
RunningMode previousPolicy = RUNMODE_DEFAULT; // 이전 policy를 나타내는 변수
// 현재 운행 상태를 나타내는 변수. 만약 Policy와 이것이 충돌되면 현재 운행 상태를 바꿔줘야 한다.
RunningState currentState = RUNSTATE_STOP; // 이 변수는 setDCMotor_goForward()와 같은 DC모터 조작 함수에서 값이 변경된다.
///////////////////////////////////////////////////////////////////////////////////////////////////


int main(void) {
    // 초기 설정 및 Configuration
    SystemInit();        
    RCC_Configuration();        
    GPIO_Configuration();
    //printf("adc bf\n");
    
    ADC_Configuration();
    //printf("after\n");
    USART2_Init();
    NVIC_Configuration();        
    setTIMER2();
    DCMotor_PWM_Configuration();
    ServoMotor_PWM_Configuration();
    while (1) {
    //printf("main\n");
        /* 운전 모드 Policy 설정 */      
        setPolicy();
        /* Policy와 현재 운행상태를 비교해서, Policy에 위반된다면 운행상태를 바꿔야 함 */
        checkRunmodeAndChange();
		/* 운행 모드가 기본 모드가 아닌 경우(장애물이 있는 경우) 알림을 보낸다(부저 및 블루투스 알림) */
		alertIfObstruct();
        /* 폰으로 입력받은 명령어(char)에 따라 명령 수행하는 부분 */
        executeOperation();
    } // end of while()
	
} // end of main()


///////////////////////      main-while문 내부에서 사용할 함수       //////////////////////////////////////////////
void setPolicy(void) {
	if (pressureValue > PRESSURE_THRESHOLD) { // 압력이 감지되면
		// 초음파센서를 이용해 앞/뒤 거리를 읽어온다
		uint32_t frontDistance = getFrontDistance();
		uint32_t behindDistance = getBehindDistance();
		
		/* 장애물 여부에 따라 Policy를 다르게 설정하는 부분 */
		// 앞 / 뒤 양쪽에 장애물이 존재하는 경우
		if ( frontDistance < DISTANCE_THRESHOLD && behindDistance < DISTANCE_THRESHOLD ) {
			policy = RUNMODE_STOP; // 정지 모드로 설정
		}
		// 앞부분에만 장애물이 존재하는 경우
		else if ( frontDistance < DISTANCE_THRESHOLD ) {
			policy = RUNMODE_BACKWARD; // 후진 모드로 설정
		}
		// 뒷부분에만 장애물이 존재하는 경우
		else if ( behindDistance < DISTANCE_THRESHOLD ) {
			policy = RUNMODE_FORWARD; // 전진 모드로 설정
		}
		// 장애물이 없는 경우
		else {
			policy = RUNMODE_DEFAULT; // 기본 모드로 설정
		}
	}
	else { // 압력이 감지되지 않는 경우
		policy = RUNMODE_DEFAULT; // 기본 모드로 설정
	}
}

void checkRunmodeAndChange(void) {
	switch (policy) {
	case RUNMODE_STOP : // Policy : 정지 모드인 경우
		setDCMotor_stop(); // 현재 운행 상태와 상관없이 무조건 DC모터 회전 중지
		break;
	case RUNMODE_FORWARD : // Policy : 전진 모드인 경우
		if (currentState == RUNSTATE_BACKWARD) { // 현재 운행 상태가 후진인 경우
			setDCMotor_stop(); // DC모터 회전 중지
		}
		break;
	case RUNMODE_BACKWARD : // Policy : 후진 모드인 경우
		if (currentState == RUNSTATE_FORWARD) { // 현재 운행 상태가 전진인 경우
			setDCMotor_stop(); // DC모터 회전 중지
		}
		break;
	default : // Policy가 기본 모드인 경우에는 별다른 조치를 취할 필요가 없다
		break;
	}
}

void alertIfObstruct(void) {
	/* Policy가 Default가 아닌 경우(장애물 존재) 부저 울림 */
	if (policy != RUNMODE_DEFAULT) {
		turnOnBuzzer();
	}
	else {
		turnOffBuzzer();
	}

	/* Policy가 변경되었을 때만 폰으로 알림이 전송되게 한다 */
	if (previousPolicy != policy) { // Policy가 바뀌었을 때만
		sendRunModeToPhone(policy); // 현재 Policy를 휴대폰으로 전송
	}
	previousPolicy = policy; // 이전 Policy값을 현재 값으로 갱신
}

void executeOperation(void) {
        //printf("policy : %d\n", policy);
        //printf("inputOp : %c\n", inputOperation);
        
	switch (inputOperation) {
	case OP_GO_FORWARD : // 전진
		if ( policy == RUNMODE_STOP || policy == RUNMODE_BACKWARD ) { 
			// Policy가 정지 또는 후진일때 전진 불가
			// 알림 보내는 기능 추가해도 될듯. 추후 결정.
                  //printf("GO_FORWARD_IF\n");
		}
		else {
                      //printf("GO_FORWARD_ELSE\n");
			setServoMotor_rotateInit(); // 정방향으로 조향
			setDCMotor_goFoward(); // 전진
		}
		break;
	case OP_GO_FORWARD_LEFT : // 전진 + 좌회전
		if ( policy == RUNMODE_STOP || policy == RUNMODE_BACKWARD ) { 
			// Policy가 정지 또는 후진일때 전진 불가
		}
		else {
			setServoMotor_rotateLeft(); // 왼쪽으로 조향
			setDCMotor_goFoward(); // 전진
		}
		break;
	case OP_GO_FORWARD_RIGHT : // 전진 + 우회전
		if ( policy == RUNMODE_STOP || policy == RUNMODE_BACKWARD ) { 
			// Policy가 정지 또는 후진일때 전진 불가
		}
		else {
			setServoMotor_rotateRight(); // 오른쪽으로 조향
			setDCMotor_goFoward(); // 전진
		}
		break;
	case OP_GO_BACKWARD : // 후진
		if ( policy == RUNMODE_STOP || policy == RUNMODE_FORWARD ) {
			// Policy가 정지 또는 전진일때 후진 불가 
		}
		else{
			setServoMotor_rotateInit(); // 정방향으로 조향
			setDCMotor_goBackward(); // 후진
		}
		break;
	case OP_GO_BACKWARD_LEFT : // 후진 + 좌회전
		if ( policy == RUNMODE_STOP || policy == RUNMODE_FORWARD ) {
			// Policy가 정지 또는 전진일때 후진 불가 
		}
		else{
			setServoMotor_rotateLeft(); // 왼쪽으로 조향
			setDCMotor_goBackward(); // 후진
		}
		break;
	case OP_GO_BACKWARD_RIGHT : // 후진 + 우회전
		if ( policy == RUNMODE_STOP || policy == RUNMODE_FORWARD ) {
			// Policy가 정지 또는 전진일때 후진 불가 
		}
		else{
			setServoMotor_rotateRight(); // 오른쪽으로 조향
			setDCMotor_goBackward(); // 후진
		}
		break;
	case OP_SPEED_FAST : // 빠른 속도
		setDCMotor_speedFast(); // DC모터 속도 빠르게 설정
		break;
	case OP_SPEED_SLOW : // 느린 속도
		setDCMotor_speedSlow(); // DC모터 속도 느리게 설정
		break;
	case OP_STOP : // 정지 
		setDCMotor_stop(); // DC모터 정지
		break;
	default :
		break;
	} // end of switch (inputOperation)
        inputOperation = '\0';
}
////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////      Configuration       //////////////////////////////////////////////
void RCC_Configuration(void) {    
    // PWM_SERVO_OUTPUT, USART
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    // PWM_DC_OUTPUT, ADC, Buzzer
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    // 초음파 센서, 모터 드라이버
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);     
    // 모터 드라이버
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);     
    // for DC Motor
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE); 

    // ADC    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);     
    // AFIO, for Interrupt
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
    // USART Bluetooth
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);
    
    // TIM2 for Counter - 초음파센서 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    // TIM3 for PWM - Servo Motor 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    // TIM4 for PWM - DC Motor 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); 
}           
 
void GPIO_Configuration(void) {
    /* 압력센서 입력포트 */
    // ADC_IN1 (PB0) Configure
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;    
    GPIO_Init(GPIOB, &GPIO_InitStructure);        
    
    /* 초음파 포트 설정 */
	// 초음파센서(뒤) trigger
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    // 초음파센서(앞) trigger
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    // 초음파센서(뒤) echo
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    // 초음파센서(앞) echo
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 모터 드라이버 부분*/
    // 앞바퀴 모터드라이버 입력으로 들어갈 출력포트
    GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_13 | GPIO_Pin_15);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    // 뒷바퀴 모터드라이버 입력으로 들어갈 출력포트
    GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_12 | GPIO_Pin_14);
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    
	
    /* 모터 PWM 출력포트 */
	// TIM3 CH1 (PA6) Configure -- 서보모터
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	// TIM4 CH1 (PB6) Configure -- DC모터
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 부저 Configure*/ 
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);	
    
    /* USART2 Bluetooth Configure */
    // UART2 TX, Output
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // UART2 RX, Input
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}
 
void ADC_Configuration(void) {
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

void USART2_Init(void) {
	USART_InitTypeDef USART_InitStructure;
	// Enable the USART2 peripheral
	USART_Cmd(USART2, ENABLE);

    USART_InitStructure.USART_BaudRate = 9600;  
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;  
	USART_InitStructure.USART_Parity = USART_Parity_No;  
	USART_InitStructure.USART_StopBits = USART_StopBits_1;  
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
    USART_Init(USART2, &USART_InitStructure);  

    // Enable the USART2 RX interrupts
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

void NVIC_Configuration(void) {      
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  // ADC
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;            
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;        
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           	
    NVIC_Init(&NVIC_InitStructure);								

    // USART2 
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;        
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           	
    NVIC_Init(&NVIC_InitStructure);
    NVIC_EnableIRQ(USART2_IRQn);
}

void setTIMER2(void) { // 시간 측정을 위한 카운터로 사용하기 위해 TIM2 이용
    prescale = (uint16_t) 7200 - 1;
    
    TIM_TimeBaseStructure.TIM_Period = 10000-1;
    TIM_TimeBaseStructure.TIM_Prescaler = prescale;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    // TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void DCMotor_PWM_Configuration(void) {
	// (SysCoreClock = 72MHz) / 1000000 = 72
    // 50Hz로 분주
    prescale = (uint16_t) (SystemCoreClock / 1000000) - 1;
    
    TIM_TimeBaseStructure.TIM_Period = 20000-1;         
    TIM_TimeBaseStructure.TIM_Prescaler = prescale;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = DC_SPEED_FAST; // 모터의 초기속도 설정
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
    
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
}

void ServoMotor_PWM_Configuration(void) {
	// (SysCoreClock = 72MHz) / 1000000 = 72
    // 50Hz로 분주
    prescale = (uint16_t) (SystemCoreClock / 1000000) - 1;
    
    TIM_TimeBaseStructure.TIM_Period = 20000-1;         
    TIM_TimeBaseStructure.TIM_Prescaler = prescale;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = SERVO_ROTATE_INIT; // 모터의 초기위치 설정
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}
////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////     DC모터 속도/회전관련 함수    //////////////////////////////////////////////
void setDCMotor_goFoward(void) {
    // 1010 1010
    // 앞부분
    GPIO_SetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_13);
    GPIO_ResetBits(GPIOC, GPIO_Pin_2 | GPIO_Pin_15);
    // 뒷부분
    GPIO_SetBits(GPIOE, GPIO_Pin_8 | GPIO_Pin_12);
    GPIO_ResetBits(GPIOE, GPIO_Pin_10 | GPIO_Pin_14);
	

    // 현재 운행 상태를 '전진'으로 설정
    currentState = RUNSTATE_FORWARD;
    
    //printf("in dc_go_forward\n");
}

void setDCMotor_goBackward(void) {
    // 0101 0101
    // 앞부분
    GPIO_ResetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_13);
    GPIO_SetBits(GPIOC, GPIO_Pin_2 | GPIO_Pin_15);
    // 뒷부분	
    GPIO_ResetBits(GPIOE, GPIO_Pin_8 | GPIO_Pin_12);
    GPIO_SetBits(GPIOE, GPIO_Pin_10 | GPIO_Pin_14);


    // 현재 운행 상태를 '후진'으로 설정
    currentState = RUNSTATE_BACKWARD;
    
    //printf("in dc_go_backward\n");
}

void setDCMotor_stop(void) {
    // 0000 0000
    // 앞부분
    GPIO_ResetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_13 | GPIO_Pin_15);
    // 뒷부분
    GPIO_ResetBits(GPIOE, GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_12 | GPIO_Pin_14);

    // 현재 운행 상태를 '정지'로 설정
    currentState = RUNSTATE_STOP;
}

void setDCMotor_speedFast(void) {
    // TIM_OCInitStructure의 다른 필드들(OCMode, Polarity 등)은 값이 고정돼있어서 코드의 중복을 줄이기위해 뺐다.
    // PWM 설정하는 함수 이외의 부분에서 Pulse를 제외한 필드값을 변경할경우 큰일나니 주의
    TIM_OCInitStructure.TIM_Pulse = DC_SPEED_FAST;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
}

void setDCMotor_speedSlow(void) {
    TIM_OCInitStructure.TIM_Pulse = DC_SPEED_SLOW;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
}
////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////     서보모터 회전 관련 함수     //////////////////////////////////////////////
void setServoMotor_rotateInit(void) {
    TIM_OCInitStructure.TIM_Pulse = SERVO_ROTATE_INIT;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    
    //printf("in servo_init\n");
}

void setServoMotor_rotateRight(void) {
    TIM_OCInitStructure.TIM_Pulse = SERVO_ROTATE_RIGHT;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    
    //printf("in servo_right \n");
}

void setServoMotor_rotateLeft(void) {
    TIM_OCInitStructure.TIM_Pulse = SERVO_ROTATE_LEFT;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    
    //printf("in servo_left\n");
}
////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////      초음파센서 관련 함수      //////////////////////////////////////////////
uint32_t getFrontDistance(void) {
    uint32_t distance = 0;
    distance = echo_time(SONIC_FRONT_TRIG, SONIC_FRONT_ECHO);
    //printf("%d\n", distance);

    // distance에 값을 곱하거나 나누어서 편한 값으로 수정할수도 있음.
    // 일단은 echo_time()값을 바로 반환
    return distance;

}

uint32_t getBehindDistance(void) {
    uint32_t distance = 0;
    distance = echo_time(SONIC_BEHIND_TRIG, SONIC_BEHIND_ECHO);

    return distance;
}

// !!!!!!!!!!!!! 아래 두 함수는 나중에 라이브러리 함수를 이용한 코드로 변경해야함. 
// 함수 body 내용만 바꾸면 됨. 인터페이스는 동일
void trig_pulse(uint16_t trigPin) // output trigger pulse to HC-SR04 trig pin at least 10us
{
    PC_ODR |= (1<<trigPin); // |= 0000 0100 0000 0000
    delay_us(11); // delay little over than 10us
    PC_ODR &= ~(1<<trigPin); // &= 1111 1011 1111 1111
    delay_us(11); // delay little over than 10us
}

uint32_t echo_time(uint16_t trigPin, uint16_t echoPin)
{
    uint32_t echo;
    trig_pulse(trigPin); // give trig pulse to u_sonic sensor
    
    while((PC_IDR & (1<<echoPin)) != (1<<echoPin)); // wait echo pin status turns HIGH
// != 0000 1000 0000 0000 (wait until bit11=1 )
    echo = TIM2->CNT;      
    
    while((PC_IDR & (1<<echoPin)) == (1<<echoPin));
// == 0000 1000 0000 0000 (wait until bit11=0 )
    echo = TIM2->CNT - echo;  
    
    return echo;
}

////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////      부저 관련 함수      //////////////////////////////////////////////
void turnOnBuzzer(void) {
  GPIO_SetBits(GPIOB, GPIO_Pin_8);
  /*
      GPIO_ResetBits(GPIOB, GPIO_Pin_8);
        delay_ms(1500);
       GPIO_SetBits(GPIOB, GPIO_Pin_8);
       delay_ms(100);
  */
}

void turnOffBuzzer(void) {
  GPIO_ResetBits(GPIOB, GPIO_Pin_8);
}
////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////      블루투스 관련 함수      //////////////////////////////////////////////
void sendStringUSART2(char *str) {
    for (uint16_t i = 0; str[i]; ++i) {
        USART_SendData( USART2, str[i] & (uint16_t)0xFF );
    }
}

void sendRunModeToPhone(RunningMode mode) {
    switch (mode) {
    case RUNMODE_STOP :     // 정지 모드
        sendStringUSART2(MSG_RUNMODE_STOP);
        break;
    case RUNMODE_FORWARD :  // 전진 모드
        sendStringUSART2(MSG_RUNMODE_FORWARD);
        break;
    case RUNMODE_BACKWARD : // 후진 모드
        sendStringUSART2(MSG_RUNMODE_BACKWARD);
        break;
    default :   // 그 외의 모든 경우. 기본 모드인 경우에도 메시지 전송 X
        // do nothing
        break;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////      Interrupt Handler      //////////////////////////////////////////////
void ADC1_2_IRQHandler(void) {
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET) {
		pressureValue = ADC_GetConversionValue(ADC1);
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);	
	}    
}

void USART2_IRQHandler(void) {
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        inputOperation = USART_ReceiveData(USART2) & 0xFF;
    	USART_ClearITPendingBit(USART2,USART_IT_RXNE);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////     딜레이 관련 함수     //////////////////////////////////////////////
void delay_us(uint32_t us) {
	if ( us > 1 ) {
		uint32_t count = us * 8 - 6;
		while(count--);
	}
	else{
		uint32_t count = 2;
		while(count--);
	}
}

void delay_ms(uint32_t ms) {
	uint32_t us = 1000 * ms;
	delay_us(us);
}
////////////////////////////////////////////////////////////////////////////////////////////