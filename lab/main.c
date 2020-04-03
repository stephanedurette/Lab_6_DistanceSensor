//-------------------------------------------------------------------
// Hack demo to toggle red and green LEDS on Discovery board
// Jan 2019 - WPS
//
// Uses Zhu's SysClock code to init system clocks
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
	LCDinit();
	DS_Init();
	
	FOREVER {
		LCDclear();
		uint32_t dist = DS_GetDistance_cm();
		if (dist){
			LCDprintf("Distance: %dcm", DS_GetDistance_cm());	
		} else {
			LCDprintf("Out of range");
		}	
		DELAY;	
	}
	
} 

