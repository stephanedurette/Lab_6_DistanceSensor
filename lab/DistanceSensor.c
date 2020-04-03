#include "DistanceSensor.h"
#include "utils.h"
#include "stm32l476xx.h"
#include "lcd.h"
#include "LED.h"

/* DistanceSensor.c - Implementation for Ultrasonic Distance Sensor
*	 Includes funtions for initializing and reading distance from 
*	 HCSR404 ultra sonic distance sensor using slave mode interrupts.
*  By: Nolan Thomas and Stephane Durette
*	 April 3, 2020
*/

#define TRIG_PIN 6
#define MAX_ECHO_PULSE_WIDTH 15000

static volatile uint32_t pulse_width = 0;

/*
* Call to setup 
*/
void DS_Init(){
	DS_Init_Trigger();
	DS_Init_Echo();
}

/* DS_Init_Trigger() sets up PB6 on TIM4 CH2
*  to send pulses to trigger the ultrasonic sensor.
*/
static void DS_Init_Trigger(){
		
	uint32_t pulseWidth_uS = 200; //should at least 10uS
	uint32_t pulseDelay_uS = 500000;
	
	float dutyCycle = (float)pulseWidth_uS / (float)pulseDelay_uS;
	
	//enable clock on port B
	SET_BITS(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN);
	
	//set mode to alternate function
	FORCE_BITS(GPIOB->MODER, 3UL << (2*TRIG_PIN), 2UL << (2*TRIG_PIN));

	//set alternate function to AF 2
	FORCE_BITS(GPIOB->AFR[0], 0xF << (4 * TRIG_PIN), 2UL << (4 * TRIG_PIN));
	
	//no pull up, no pull down
	FORCE_BITS(GPIOB->PUPDR, 3UL << (2*TRIG_PIN), 0);
	
	//enable timer 4 clock
	SET_BITS(RCC->APB1ENR1, RCC_APB1ENR1_TIM4EN);
	
	//up counting
	CLR_BITS(TIM4-> CR1, TIM_CR1_DIR);
	
	//prescaler
	//TIM4->PSC = prescaleValue; --> 1kHz clock is 1ms
	TIM4->PSC = 80000-1;
	
	//auto reload
	TIM4->ARR = 5000 - 1; //auto reload every 500 ms 
	
	//clear ch 1 output cmp registers
	CLR_BITS(TIM4->CCMR1, TIM_CCMR1_OC1M);
	
	//Set the duty cycle
	TIM4 -> CCR1 = dutyCycle * (TIM4->ARR + 1);
	//TIM4 -> CCR1 = 500;
	
	//main output enable
	SET_BITS(TIM4->BDTR, TIM_BDTR_MOE);

	//pwm mode
	SET_BITS(TIM4->CCMR1, TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);
	
	//output polarity active high
	CLR_BITS(TIM4 -> CCER, TIM_CCER_CC1P);
	
	//enable CH1
	SET_BITS(TIM4 -> CCER, TIM_CCER_CC1E);
	
	//enable counter
	SET_BITS(TIM4->CR1, TIM_CR1_CEN);
}

/* Configure PA.0 as input capture for echo signal on
*  TIM2 CH1. Enable it to generate slave mode interrupts that will
*  measure the width of incoming pulses
*/
static void DS_Init_Echo(void){	 

	ConfigPA0();
	
	//Enable the clock to timer 2
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	
	//set up appropriate prescaler to slow down timer input clock
	//in this case we will use 80 for 1Mhz from 80MHz
	TIM2->PSC = 80UL;
	
	//set Auto-reload value to maximum value
	TIM2->ARR = 0xFFFF; //Max 16 bit value
	
	//set the direction of channel 1 as input, and select active input
	TIM2->CCMR1 &= ~TIM_CCMR1_CC1S; //Clear capture/compare 1 selection bits
	TIM2->CCMR1 |= TIM_CCMR1_CC1S_0; //CC1S[1:0] for channel 1:
																	 // 00 = output
																	 // 01 = input, CC1 is mapped on timer input 1
																	 // 10 = input, CC1 is mapped on timer input 2
																	 // 11 = input, CC1 is mapped on slave timer
												
	//Program the input filter duration: disable digital filtering by clearing
	//IC1F[3:0] bits because we want to capture every event
	TIM2->CCMR1 &= ~TIM_CCMR1_IC1F; // No filtering
	
	// CC1NP:CC1P bits: 00 = rising edge    01= falling edge
	//									10 = reserved				11 = both edges
	TIM2->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP); //clear polarity bits
	TIM2->CCER |= TIM_CCER_CC1P; //Only falling edges generate interrupts
	
	//select input trigger source
	TIM2->SMCR &= ~TIM_SMCR_TS; //clear the trigger selection bits
	TIM2->SMCR |= 4UL << 4; // 100: T1 Edge Detector
	
	//select slave mode with reset
	TIM2->SMCR &= ~TIM_SMCR_SMS; //clear slave mode bit
	TIM2->SMCR |= 4; 						// 1000 slave mode with reset
	
	//program the prescaler: Clear the prescaler to capture each transition
	TIM2->CCMR1 &= ~(TIM_CCMR1_IC1PSC);
	
	//enable capture for channel 1
	TIM2->CCER |= TIM_CCER_CC1E;
	
	//Allow channel 1 of timer 2 to generate interrupts
	TIM2->DIER |= TIM_DIER_CC1IE;
	
	//Allow channel 1 of timer 4 ro generate DMA requests
	TIM2->DIER |= TIM_DIER_CC1DE;
	
	//enable the timer counter
	TIM2->CR1 |= TIM_CR1_CEN;
	
	//set priority of the timer 2 intterupt to 0 (highest urgency)
	NVIC_SetPriority(TIM2_IRQn, 0);

	//enable the timer 4 interrupt in the interrupt controller
	NVIC_EnableIRQ(TIM2_IRQn);

}

// This function is setup for PA.0 to work as TIM2_CH1
static void ConfigPA0(void){

		//enable clock on port A
	SET_BITS(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN);
	
	//set mode of pin 0 to alternate function
	// 00 = input, 01 = output, 10 = alternate function, 11 = Analog
	GPIOA->MODER &= ~(3UL << 0); //clear bit 0 and 1
	GPIOA->MODER |= 2UL << 0;    //set bit 0 and 1

	//Select alternate function 
	GPIOA->AFR[0] &= ~(0xF); //AFR[0] for TIM2_CH1 on PA.0
	GPIOA->AFR[0] |= 1UL;
	
	//set I/O speed to Fast
	// 00 = slow, 01 = medium, 10 = fast, 11 = High
	GPIOA->OSPEEDR &= ~3UL<<0;
	GPIOA->OSPEEDR |=  2UL<<0;
	
	//Set pin PA.0 as no pull-up/pull-down
	//00 = none, 01 = pull up, 10 = pull down, 11 = Reserved
	GPIOA->PUPDR &= ~3UL<<0;

}

/* Interrupt Service Routine for responding to
*  input events on TIM2 CH1 our input capture for the sensor
*  reads pulse width of echo in microseconds and puts that value into pulse_width variable
*/
void TIM2_IRQHandler(void){
	Green_LED_Toggle(); //Toggle the green LED for debugging
	
	if((TIM2->SR & TIM_SR_UIF) != 0){ //Check if overflow has taken place
		TIM2->SR &= ~TIM_SR_UIF;				//clear UIF flag to prevent re-entering
	}
	
	if((TIM2->SR & TIM_SR_CC1IF) != 0){ //Check if interupt flag has been set
		pulse_width = TIM2->CCR1;					//reading CCR1 clears CC1IF	flag		
	}
}

//Takes pulse_width and returns a distance in CM or -1 if the pulse is too large
uint32_t DS_GetDistance_cm(){
	return (pulse_width <= MAX_ECHO_PULSE_WIDTH ? pulse_width / 58 : -1);
}

