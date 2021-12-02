#include "main.h"
#include "ihm.h"
#include "ble.h"
#include "slider.h"
#include "events.h"
#include "string.h"
#include "stdio.h"

unsigned char flagDelay;

uint16_t setTimeDelay;

uint16_t contTimeDelay;

void delay(uint16_t x1ms)
{
  flagDelay=1;

  setTimeDelay=x1ms;
  
  contTimeDelay=0;
  
  while( contTimeDelay <setTimeDelay ) 
  {
  
  }
 
  flagDelay = 0;
  
  contTimeDelay = 0;
}

void OutputMCO(void) 
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  /* Output clock on MCO pin ---------------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;

  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // pick one of the clocks to spew
  RCC_MCOConfig(RCC_MCOSource_SYSCLK); // Put on MCO pin the: System clock selected
  //RCC_MCOConfig(RCC_MCOSource_HSE); // Put on MCO pin the: freq. of external crystal
  //RCC_MCOConfig(RCC_MCOSource_PLLCLK_Div2); // Put on MCO pin the: System clock selected
}

void initSystemClock(void)
{
/* Set the sys clock for 48 Mhz*/
  RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_12);
  
  RCC_PLLCmd(ENABLE);
  
  while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}
  
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
}

int main(void)
{ 
  initSystemClock();
   //OutputMCO();
  
  initLeds();
  
  initButton();
  
  TIM1Conf();

  initUsartBluetooth();
  
  initAdcSlider();
 
  StartReadSlider();
  
  clearBuffer(statusKeypad,5);
 
  bleRestart();
  
  delay(1000);
  
  if( bleVerifyConfiguration() )
  {
    bleConfigured = CONFIGURED;
    lenStatusOn=255;
  }
  else
  {
    bleConfigured = NO_CONFIGURED;
    lenStatusOn=1;
  } 
  
  clearBuffer(bufferRxBle, SIZE_BUFFER_RX);
  
  //setBle( );
 
  // disableCCON();
  // bleEnableCcon();
  // bleClearConfig();
  // bleSaveConfig();
  // bleRestart();
  // bleGetConfig();
  // bleStatusConnection();
  // bleStatusConnection();
  // bleConnection(OFF);
  // bleStatusConnection();
   //bleConnection(ON);
  
  while (1)
  {   
    switch ( bleConfigured ) 
    {  
      case CONFIGURED :
        checkEvents();
      break;
      
      case NO_CONFIGURED :
        firstConfiguration();
      break;
      
    }
  }
  
  
}
