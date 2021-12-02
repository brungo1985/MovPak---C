
#ifndef __IHM_H
#define __IHM_H

#include "stm32f0xx.h"

#define SIZE_STATUS_KEYPAD 5

#define LED_0 GPIO_Pin_0
#define LED_1 GPIO_Pin_1
#define LED_2 GPIO_Pin_2
#define LED_3 GPIO_Pin_3
#define LED_STATUS GPIO_Pin_4

#define BOTAO_LEFT GPIO_Pin_0
#define BOTAO_MODE GPIO_Pin_1
#define BOTAO_F_R GPIO_Pin_2
#define BOTAO_ON_OFF GPIO_Pin_10
#define BOTAO_RIGHT GPIO_Pin_11




extern GPIO_InitTypeDef ledStruct;

extern GPIO_InitTypeDef botoesStruct;
 
extern unsigned char statusKeypad[];

extern unsigned char flagButtuonLeft;

extern unsigned char flagButtuonMode;
extern unsigned char flagButtuonFR;
extern unsigned char flagButtuonOnOff;
extern unsigned char flagButtuonRight;

extern unsigned char levelVbatSkate;

void initLeds(void);

void initButton(void);

void readKeypad(void);

#endif 
