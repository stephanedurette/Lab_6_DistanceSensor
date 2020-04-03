#include "lcd.h"
#include <stdio.h>
#include <stdarg.h>

extern void Delay_ms(uint16_t delayTime ){
	//Bill said there was a typo in this
	
	SysTick->CTRL = 0;																			//Disable systick
	SysTick->LOAD = (800000000 / 1000UL) * delayTime;				//set reload register
	SysTick->VAL = 0;																				//Reset the systick counter value
	
	//select processor clock: 1 = processor clock; 0 = external clock
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
	
	//disable systick interrupt, 1 = Enable, 0 = disable
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
	
	//enable systick
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	
	//wait for timeout
	while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
	
	//disable systick until next time
	SysTick->CTRL = 0;
	

}

extern void LCD_GPIO_Init( void ){
	//Enable GPIO Clock for port E
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
	
	//Enable PE10 to PE15 for GPIO output use in stepper motor driver L293D
	for(int i = 10; i < 16; i++){
		// GPIO Mode: Input(00, reset), Output(01), AlterFunc(10), Analog(11, reset)
		GPIOE->MODER &= ~(3UL<<(2*i));  
		GPIOE->MODER |= (1UL<<(2*i));      //  Output(01)
		
		// GPIO Speed: Low speed (00), Medium speed (01), Fast speed (10), High speed (11)
		GPIOE->OSPEEDR &= (~(3UL<<(2*i)));
		GPIOE->OSPEEDR |=   (3UL<<(2*i));  // High speed
		
			// GPIO Output Type: Output push-pull (0, reset), Output open drain (1) 
		//GPIOE->OTYPER &= ~(1UL<<i);       // Push-pull
		
		// GPIO Push-Pull: No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
		GPIOE->PUPDR   &= ~(3UL<<(2*i));  // No pull-up, no pull-down
		
	}
		
}

extern void LCDinit( void ){
		LCD_GPIO_Init();
		LCD_E_HI;
		LCD_BUS(0x03);	//wake up display & sync
		LCD_E_LO;
	
		Delay_ms(5);
	
		LCD_E_HI;
		LCD_BUS(0x03);	//wake up display & sync
		LCD_E_LO;
	
		Delay_ms(1);
	
		LCD_E_HI;
		LCD_BUS(0x03);	//wake up display & sync
		LCD_E_LO;
	
		LCD_E_HI;
		LCD_BUS(0x02);	//wake up display & sync - go to 4 bit mode
		LCD_E_LO;
	
		Delay_ms(2);
		
		//we are sync'd and in 4-bit mode, issue commands to configure the display
		LCDcmd( LCD_CMD_FUNCTION | LCD_FUNCTION_4BIT | LCD_FUNCTION_5X8FONT);
		LCDcmd( LCD_CMD_DISPLAY | LCD_DISPLAY_OFF);
		LCDclear();
		LCDcmd(LCD_CMD_ENTRY| LCD_ENTRY_MOVE_CURSOR | LCD_ENTRY_INC);
		LCDcmd(LCD_CMD_DISPLAY | LCD_DISPLAY_ON | LCD_DISPLAY_NOCURSOR | LCD_DISPLAY_NOBLINK);

}

extern void LCDclear( void ){
	LCDcmd(LCD_CMD_CLEAR);
}

extern void LCDhomeCursor( void ){
	LCDcmd(LCD_CMD_HOME);
}

extern void LCDprintf( char * formatString, ... ){
	va_list args;
	char buffer[MAX_LCD_BUFSIZ];
	
	va_start(args, formatString);
	vsnprintf(buffer, MAX_LCD_BUFSIZ, formatString, args);
	va_end(args);
	
	LCDputs(buffer);
	
}

extern void LCDputs( char * thisString ){
	while(*thisString){
		LCDputc(*thisString++);
	}
}

extern void LCDputc( unsigned char thisChar ){	
	if ('\n' == thisChar){
		LCDcmd(LCD_CMD_SET_DDADDR | LCD_ADDR_LINE2);
	} else {
		LCDdata(thisChar);
	}
}

extern void LCDcmd( unsigned char thisByte ){
	LCD_E_LO; 		//Pull the E line low
	LCD_RS_IR; 		//Set the RS line to select the instruction (low) or data register high
	LCD_E_HI; 		//pull the E line high
	LCD_BUS(HI_NYBBLE(thisByte));		//Output the upprt4 bits of data to LCD Data Bus
	LCD_E_LO;		//Pull the E line low
	LCD_E_HI;		//Pull the E line high
	LCD_BUS(LO_NYBBLE(thisByte)); 	//Output the lower 4 bits of data to LCD Data Bus [7:4]
	LCD_E_LO; 		//Pull the E line low
	Delay_ms(1);
}

extern void LCDdata( unsigned char thisByte ){
	LCD_E_LO; 		//Pull the E line low
	LCD_RS_DR; 		//Set the RS line to select the instruction (low) or data register high
	LCD_E_HI; 		//pull the E line high
	LCD_BUS(HI_NYBBLE(thisByte));		//Output the upprt4 bits of data to LCD Data Bus
	LCD_E_LO;		//Pull the E line low
	LCD_E_HI;		//Pull the E line high
	LCD_BUS(LO_NYBBLE(thisByte)); 	//Output the lower 4 bits of data to LCD Data Bus [7:4]
	LCD_E_LO; 		//Pull the E line low
	Delay_ms(1);
}



