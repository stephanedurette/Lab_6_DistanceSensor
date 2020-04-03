#include "stm32l476xx.h"

void DS_Init_Trigger( void );
void DS_Init_Echo(void);
void ConfigPA0(void);
void TIM2_IRQHandler(void);
uint32_t DS_GetDistance_cm( void );

void DS_Init( void );


