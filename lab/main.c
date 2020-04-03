//-------------------------------------------------------------------
// Lab 6 Embedded Software ESE Winter 2020
// Ultrasonic Range Finder Sensor
// Stephane Durette and Nolan Thomas 
//-------------------------------------------------------------------
#include "stm32l476xx.h"
#include "SysClock.h"
#include "LED.h"
#include "lcd.h"
#include "DistanceSensor.h"

// delay count between changing LED states
#define DELAY_COUNT 10000000UL
#define DELAY for( uint32_t i = 0; i < DELAY_COUNT; i++)
#define FOREVER for(;;)

int main(void){
	System_Clock_Init(); 	// Switch System Clock = 80 MHz
	LED_Init();						// Init GPIO bits to drive LEDs
	LCDinit();						// Init LCD Display
	DS_Init();						// Init Untrasonic Distance Sensor
	
	FOREVER {
		LCDclear();
		uint32_t dist = DS_GetDistance_cm(); //read the current distance value held
		if (dist != -1){
			LCDprintf("Distance: %dcm", dist);	
		} else {
			LCDprintf("Out of range");
		}	
		DELAY;	
	}
	
} 

