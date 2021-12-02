
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
 #include "stm32f0xx.h"
 #include "ble.h"
 #include "ihm.h"

 
 extern uint16_t timeEventToogleLed;

 extern unsigned char flagDelay;

 extern uint16_t setTimeDelay;

 extern uint16_t contTimeDelay;
 


 void configura_Timer3(void);

 void delay(uint16_t x1ms);

 void OutputMCO(void);

 void initSystemClock(void);

#endif
