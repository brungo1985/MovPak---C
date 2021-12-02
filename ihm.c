#include "ihm.h"
#include "ble.h"
#include "main.h"
#include "events.h"

unsigned char statusKeypad[SIZE_STATUS_KEYPAD]={0,0,0,0,0};

GPIO_InitTypeDef ledStruct;

GPIO_InitTypeDef botoesStruct;

unsigned char flagButtuonLeft=0;

unsigned char flagButtuonMode=0;

unsigned char flagButtuonFR=0;

unsigned char flagButtuonOnOff=0;

unsigned char flagButtuonRight=0;

unsigned char levelVbatSkate;
  
void initLeds(void)
{
    /* Enable the clock to bus of I/O's */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  /*Define the Pin */
  ledStruct.GPIO_Pin = LED_0 | LED_1 | LED_2 | LED_3 | LED_STATUS;
  /*set pin output*/
  ledStruct.GPIO_Mode = GPIO_Mode_OUT;
    /*kind of de output push pull*/
  ledStruct.GPIO_OType = GPIO_OType_PP;
  /*Clk gpio*/ 
  ledStruct.GPIO_Speed = GPIO_Speed_50MHz;
   /*none resistor of pull up/down*/
  ledStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  /*init the structure*/
  GPIO_Init(GPIOA, &ledStruct);
}

void initButton(void)
{
  /* Enable the clock to bus of I/O's */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  /*Define the Pin */
  botoesStruct.GPIO_Pin = BOTAO_LEFT |  BOTAO_MODE | BOTAO_F_R | BOTAO_ON_OFF | BOTAO_RIGHT ;
  /*set pin input*/
  botoesStruct.GPIO_Mode = GPIO_Mode_IN;
  /*kind of de output push pull*/
  botoesStruct.GPIO_OType = GPIO_OType_OD;
  /*Clk do gpio*/ 
  botoesStruct.GPIO_Speed = GPIO_Speed_50MHz;
  /*none resistor of pull up/down*/
  botoesStruct.GPIO_PuPd = GPIO_PuPd_UP;
  /*init the structure*/
  GPIO_Init(GPIOB, &botoesStruct);
}

void readKeypad(void)
{
  uint16_t StatePortButton = 255;
  
  unsigned char stateButton = 255;
  
  //retorna o estado da porta dos botões
  StatePortButton =    ( GPIO_ReadInputData(GPIOB) ); 
    //stateButton = GPIO_ReadInputDataBit(GPIOB, BOTAO_LEFT);
    
    /*  o botão FR - 0x03 está com problema */
  stateButton =  (StatePortButton&0x01) | (StatePortButton&0x02) |  (StatePortButton&0x04) |  ((StatePortButton&0x0400)>>7)  | ((StatePortButton&0x0800)>>7) ;
    
    /*analayse button left*/
  if( (stateButton&0x01) == flagButtuonLeft    ) 
  {
       delay(5);
       
       if( (stateButton&0x01) == flagButtuonLeft )
       {
            /*condition of button held down*/
             if(!flagButtuonLeft)
             {
        //        GPIOA->ODR ^= LED_0;
                statusKeypad[0]^=1;
                eventAsynchronousOccurred |= ASYNCHRONOUS_EVENT_SEND_FRAME_STATUS_BLE;
             }
               
             flagButtuonLeft^=1;
       }
  }
  
      /*analayse button mode*/
  if( (stateButton&0x02) == flagButtuonMode   )
  {
       delay(5);
       
       if( (stateButton&0x02) == flagButtuonMode )
       {
            /*condition of button held down*/
         
         
             if(flagButtuonMode)
             {
               
               if(  flagEnableClearConfigBle)
               {
        //          GPIOA->ODR ^= LED_1;
                  statusKeypad[1]^=1;
                  eventAsynchronousOccurred |= ASYNCHRONOUS_EVENT_SEND_FRAME_STATUS_BLE;
                  flagEnableClearConfigBle=0;
               }
               
             }
       
            else if (!flagButtuonMode) 
            {
             flagEnableClearConfigBle = 1 ;
             contTimeEventClearConfigBle =0;     
            }
                    
             flagButtuonMode^=2;   
        
       }
       
  }
        /*analayse button FR*/
  if( (stateButton&0x04) == flagButtuonFR  )
  {
       delay(5);
       
       if( (stateButton&0x04) == flagButtuonFR )
       {
            /*condition of button held down*/
             if(!flagButtuonFR)
             {
          //      GPIOA->ODR ^= LED_2;
                statusKeypad[2]^=1;
                eventAsynchronousOccurred |= ASYNCHRONOUS_EVENT_SEND_FRAME_STATUS_BLE;
             }
             flagButtuonFR^=0x04;
       }
  }
  
   /*analayse button onOff*/
  if( (stateButton&0x08) == flagButtuonOnOff   )
  {
       delay(5);
       
       if( (stateButton&0x08) == flagButtuonOnOff )
       {
            /*condition of button held down*/
             if(flagButtuonOnOff)
             {
               
                if(flagEventPairing) // flag zerado pela identificação do evento 
                {
           //       GPIOA->ODR ^= LED_3;
                  statusKeypad[3]^=1;
                  eventAsynchronousOccurred |= ASYNCHRONOUS_EVENT_SEND_FRAME_STATUS_BLE;
                  contTimeEventPairing=0;
                  flagEventPairing=0;
                }
                
             }
             else if ( !flagButtuonOnOff   )
             {
                 flagEventPairing=1;
                 contTimeEventPairing=0;
             }   
             
             flagButtuonOnOff^=0x08;
       }
  }
  
  
  if( (stateButton&0x10) == flagButtuonRight )
  {
       delay(5);
       
       if( (stateButton&0x10) == flagButtuonRight )
       {
             /*condition of button held down*/
             if(!flagButtuonRight)
             {
                GPIOA->ODR ^= LED_3;
                
                statusKeypad[4]^=1;
                
                eventAsynchronousOccurred |= ASYNCHRONOUS_EVENT_SEND_FRAME_STATUS_BLE;
             }
             flagButtuonRight^=0x10;
       }
  }

}
