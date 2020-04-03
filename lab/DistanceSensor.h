#ifndef __DISTANCE_SENSOR__H
#define __DISTANCE_SENSOR__H

/* DistanceSensor.h - Interface for Ultrasonic Distance Sensor
*	 Includes funtions for initializing and reading distance from 
*	 HCSR404 ultra sonic distance sensor using slave mode interrupts.
*  By: Nolan Thomas and Stephane Durette
*	 April 3, 2020
*/

#include "stm32l476xx.h"

static void DS_Init_Trigger(void);  			//Initilaize trigger input
static void DS_Init_Echo(void);     			//Initilize echo input capture and interrupt
static void ConfigPA0(void); 			//Config PA0 to do input capture as TIM2 CH1 
void TIM2_IRQHandler(void);	 			//ISR for reading distance
uint32_t DS_GetDistance_cm(void); //Gets time value in useconds and return converted cm distance
void DS_Init(void);								//Calls functions to init trigger and echo for sensor

#endif
