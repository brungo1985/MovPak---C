#include "ble.h"

#include "ihm.h"

#include "main.h"

#include "slider.h"

#include "events.h"

#include <string.h>

#include <stdio.h>

#include "string.h"

unsigned char bufferRxBle[SIZE_BUFFER_RX ];

int indiceBufferRxBle=0;

unsigned char cmd=0;

unsigned char flagBleVerifyPaired=0;

unsigned char bleConnected=0;

unsigned char bleConfigured=0;

void clearBuffer( unsigned char *buffer , int len)
{
   for(int i=0 ; i < len ; i++ )
   {
     *buffer=0;
      buffer++;
   }
}

void USART1_IRQHandler(void)
{
  if (USART1->CR1 && USART_ISR_RXNE) //if a reception interrupt ocurred
  {
     bufferRxBle[indiceBufferRxBle++]=USART1->RDR;  
     
     USART1->RQR |= USART_RQR_RXFRQ; // cleans receive flag
     
     switch(cmd)
     {
        case CMD_SEND_FRAME_STATUS:
          
          if(indiceBufferRxBle==10)
             if (  bufferRxBle[0]=='O' && bufferRxBle[1]=='K' && bufferRxBle[8]=='O' && bufferRxBle[9]=='K'    )
                 eventAsynchronousOccurred |= ASYNCHRONOUS_EVENT_END_COMMUNICATION;
          break;  
          
         case CMD_WAIT_OK:
          
          if(indiceBufferRxBle==4 && bufferRxBle[0]=='O' && bufferRxBle[1]=='K')
                 eventAsynchronousOccurred |= ASYNCHRONOUS_EVENT_END_COMMUNICATION;
          break;  
          
          case CMD_WAIT_FIRST_CONFIGURATION:
          
            if( indiceBufferRxBle==1   )
            {
              setTimeoutCommunicationBle(50);
              
              flagWaitAnswerBle=1;
            }
          
          else if(indiceBufferRxBle==4 && bufferRxBle[0]=='O' && bufferRxBle[1]=='K')
                 eventAsynchronousOccurred |= ASYNCHRONOUS_EVENT_END_COMMUNICATION;
          break;  
          
          case CMD_SEND_SOLICITATION_VBAT_SKATE:
       
          
           
          break;
       

     }
  }
}


void initUsartBluetooth(void)
{ 
  NVIC_InitTypeDef   NVIC_InitStructure;
  
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN|RCC_AHBENR_GPIOBEN ;
  
  GPIOB->MODER |= GPIO_MODER_MODER6_1|GPIO_MODER_MODER7_1;
  GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR6|GPIO_OSPEEDER_OSPEEDR7;
  GPIOB->AFR[0] &= ~(GPIO_AFRL_AFR7|GPIO_AFRL_AFR6);
  
    /*--Enable Clock--*/
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
  /*-- USART disable --*/
  USART1->CR1 &= ~USART_CR1_UE;  
  /*--Word length - 1 start bit, 8 data bits --*/
  USART1->CR1 &= ~USART_CR1_M;
  /*-- Parity control disable--*/
  USART1->CR1 &= ~USART_CR1_PCE;
  /*-- Oversampling by 16--*/
  USART1->CR1 &= ~USART_CR1_OVER8;
  /*-- 1 Stop bit--*/
  USART1->CR2 &= ~USART_CR2_STOP;
  /*--Character detection - \r - stored on ADD[7:0] in CR2--*/
  USART1->CR2 |= 0x0D000000;
  /*-- Disables overrun --*/
  USART1->CR3 |= USART_CR3_OVRDIS;
  /*-- Enables character detection interrupt --*/
  //USART1->CR1 |= USART_CR1_CMIE;
  /*--Baud rate 9600 bps -> USARTDIV = 48MHz/9600 = 5000--*/
  USART1->BRR = 5000;
  /*--Receiver enable--*/
  USART1->CR1 |= USART_CR1_RE;
  /*--RX interrupt enable--*/
  USART1->CR1 |= USART_CR1_RXNEIE;
  /*-- Enables DMA reception--*/
  USART1->CR3 |= USART_CR3_DMAR;
  /*--USART enable--*/
  USART1->CR1 |= USART_CR1_UE;

  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
     

unsigned char crcCalc(unsigned char* buffer , unsigned char len)
{
  unsigned char crc = *buffer;
  
  buffer++;
  
  for(unsigned char i=1; i < len ; i++)
    crc ^= *buffer++;
  
  return crc;
}


void setTimeoutCommunicationBle( int timeOut )
{
 contTimeEventTimeOutBle=0;
 
 setTimeEventTimeOutBle=timeOut;
 
 flagWaitAnswerBle=1;
 
 flagEventTimeOutBle=0;
}

void sendBufferBluetooth(unsigned char *bufferTx, unsigned char len )
{
   int i;
   
   unsigned char b;
   
   for(i=0; i < len ; i++)
   {
      delay(2);
    
      b= *bufferTx;
       
      USART1->TDR = b;

      bufferTx++;
   
      while(USART1->ISR & USART_ISR_TC);
    
      USART1->CR1 |= USART_CR1_TE;
   }
}

void mountFrameSolicitationVbatSkate( unsigned char *buffer  )
{ 
   unsigned char frame[5];
  
   // Preambulo
   *buffer='S';
   buffer++;
  // Preambulo
   *buffer='N';
   buffer++;
   // Preambulo
   *buffer='D';
   buffer++;
   // Preambulo
   *buffer=' ';
   buffer++;
   
   /*CMD*/
   *buffer='V';
   frame[0] = *buffer ;
   buffer++;
   
   /*Tamanho do frame */
   *buffer=0x03;
   frame[1] = *buffer ;
   buffer++;
   /*CRC */
   *buffer = crcCalc(frame,2);
    buffer++;
   
   /*fim de frame*/
   *buffer=0x0d;
   
   cmd = CMD_SEND_SOLICITATION_VBAT_SKATE;
}

void mountFrameStatusBle( unsigned char *buffer  )
{
   unsigned char frame[10];
  
   // Preambulo
   *buffer='S';
   buffer++;
  // Preambulo
   *buffer='N';
   buffer++;
   // Preambulo
   *buffer='D';
   buffer++;
   // Preambulo
   *buffer=' ';
   buffer++;
   
   /*CMD*/
   *buffer='S';
   frame[0] = *buffer ;
   buffer++;
   
   /*Tamanho do frame */
   *buffer=0x0A;
   frame[1] = *buffer ;
   buffer++;
    
   /*Status botão left*/
   *buffer=statusKeypad[0] + 0x30 ;
   frame[2] = *buffer ;
   buffer++;

   /*Status botão mode*/
   *buffer=statusKeypad[1] + 0x30;
   frame[3] = *buffer ;
   buffer++;

   /*Status botão fr*/
   *buffer=statusKeypad[2] + 0x30;
   frame[4] = *buffer ;
   buffer++;
   
   /*Status botão on/off*/
   *buffer=statusKeypad[3] + 0x30;
   frame[5] = *buffer ;
   buffer++;
   
   /*Status botão right*/
   *buffer=statusKeypad[4] + 0x30;
   frame[6] = *buffer ;
   buffer++;
   
   /*Status Slider*/
   *buffer=valueSlider + 1 ;
    frame[7] = *buffer ;
    buffer++;
   
   *buffer= vBat ;
   frame[8] = *buffer ;
   buffer++;
   
   *buffer = crcCalc(frame,9);
    buffer++;
   
   /*fim de frame*/
   *buffer=0x0d;
   
   cmd = CMD_SEND_FRAME_STATUS;
}

unsigned char bleMasterOn(void)
{
  unsigned char bufferTxBle[50];
  
  unsigned char len;

  indiceBufferRxBle=0;
  
  clearBuffer(bufferRxBle,SIZE_BUFFER_RX);
  
  // configura o ble como mestre da comunicação
  strcpy((char*)bufferTxBle, BLE_CMD_MODE_MASTER_ON);
  //tamanho do frame
  len=strlen((char*) bufferTxBle);
  // envia buffer pro ble
  sendBufferBluetooth( bufferTxBle,  len );
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  // set flag que define a provável resposta esperada pela serial
  
  cmd = CMD_WAIT_OK;
  // seta timeout da comunicação
  setTimeoutCommunicationBle(3000);
  // espera pelo fim da comunicação
  while(!(eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION))
  {
  }  
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  
  if(indiceBufferRxBle==4 && bufferRxBle[0]=='O' && bufferRxBle[1]=='K')
    return 1;
  else
    return 0;
}

unsigned char bleEnableCcon(void)
{
  unsigned char bufferTxBle[50];
  
  unsigned char len;
  
  indiceBufferRxBle=0;
  
  clearBuffer(bufferRxBle,SIZE_BUFFER_RX);
    
  strcpy((char*)bufferTxBle, BLE_CMD_ENABLE_CCON);
  //tamanho do frame
  len=strlen((char*) bufferTxBle);
  // envia buffer pro ble
  sendBufferBluetooth( bufferTxBle,  len );
  // set flag que define a provável resposta esperada pela serial
  cmd = CMD_WAIT_OK;
  // seta timeout da comunicação
  setTimeoutCommunicationBle(200);
  // espera pelo fim da comunicação
  while(!(eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION))
  {
  }  
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  
  if(indiceBufferRxBle==4 && bufferRxBle[0]=='O' && bufferRxBle[1]=='K')
    return 1;
  else
    return 0;
}


unsigned char bleEnableScan(unsigned char scan)
{
  unsigned char bufferTxBle[50];
  
  unsigned char len;
  
  indiceBufferRxBle=0;
  
  clearBuffer(bufferRxBle,SIZE_BUFFER_RX);
  
  if(scan==ON)  
    strcpy((char*)bufferTxBle, BLE_CMD_ENABLE_SCAN);
  else
   strcpy((char*)bufferTxBle, BLE_CMD_DISABLE_SCAN); 
  
  //tamanho do frame
  len=strlen((char*) bufferTxBle);
  // envia buffer pro ble
  sendBufferBluetooth( bufferTxBle,  len );
  // set flag que define a provável resposta esperada pela serial
  cmd = CMD_WAIT_OK;
  // seta timeout da comunicação
  setTimeoutCommunicationBle(200);
  // espera pelo fim da comunicação
  while(!(eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION))
  {
  }  
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  
  if(indiceBufferRxBle==4 && bufferRxBle[0]=='O' && bufferRxBle[1]=='K')
    return 1;
  else
    return 0;
}


unsigned char bleSetAddr(char *addr)
{
  unsigned char bufferTxBle[50];
  
  unsigned char len;
 
  indiceBufferRxBle=0;
  
  clearBuffer(bufferRxBle,SIZE_BUFFER_RX);  
  
  //strcpy((char*)bufferTxBle, BLE_CMD_SET_ADDR);
   //"SET ADDR=20FABB0276F0\r"
  
  sprintf( (char*)bufferTxBle , "SET ADDR=%s\r",addr);
  
  //tamanho do frame
  len=strlen((char*) bufferTxBle);
  // envia buffer pro ble
  sendBufferBluetooth( bufferTxBle,  len );
  // set flag que define a provável resposta esperada pela serial
  cmd = CMD_WAIT_OK;
  // seta timeout da comunicação
  setTimeoutCommunicationBle(200);
  // espera pelo fim da comunicação
  while(!(eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION))
  {
  }  
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  
  if(indiceBufferRxBle==4 && bufferRxBle[0]=='O' && bufferRxBle[1]=='K')
    return 1;
  else
    return 0;
}

unsigned char bleStartStopScan(unsigned char startStop)
{
  unsigned char bufferTxBle[50];
  
  unsigned char len;
  
  indiceBufferRxBle=0;
  
  clearBuffer(bufferRxBle,SIZE_BUFFER_RX);
  
  if(startStop)
    strcpy((char*)bufferTxBle, BLE_CMD_START_SCAN);
  else
    strcpy((char*)bufferTxBle, BLE_CMD_STOP_SCAN);
  
  //tamanho do frame
  len=strlen((char*) bufferTxBle);
  // envia buffer pro ble
  sendBufferBluetooth( bufferTxBle,  len );
  // set flag que define a provável resposta esperada pela serial
  cmd = CMD_WAIT_OK;
  // seta timeout da comunicação
  setTimeoutCommunicationBle(200);
  // espera pelo fim da comunicação
  while(!(eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION))
  {
  }  
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  
  if(indiceBufferRxBle==4 && bufferRxBle[0]=='O' && bufferRxBle[1]=='K')
    return 1;
  else
    return 0;
}

unsigned char bleSaveConfig(void)
{
  unsigned char bufferTxBle[50];
  
  unsigned char len;
 
  indiceBufferRxBle=0;
  
  clearBuffer(bufferRxBle,SIZE_BUFFER_RX);  
  
  strcpy((char*)bufferTxBle, BLE_CMD_TO_STORE_SETTING);
  //tamanho do frame
  len=strlen((char*) bufferTxBle);
  // envia buffer pro ble
  sendBufferBluetooth( bufferTxBle,  len );
  // set flag que define a provável resposta esperada pela serial
  cmd = CMD_WAIT_OK;
  // seta timeout da comunicação
  setTimeoutCommunicationBle(200);
  // espera pelo fim da comunicação
  while(!(eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION))
  {
  }  
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  
  if(indiceBufferRxBle==4 && bufferRxBle[0]=='O' && bufferRxBle[1]=='K')
    return 1;
  else
    return 0;
}

unsigned char bleClearConfig(void)
{
  unsigned char bufferTxBle[50];
  
  unsigned char len;
 
  indiceBufferRxBle=0;
  
  clearBuffer(bufferRxBle,SIZE_BUFFER_RX);  
  
  strcpy((char*)bufferTxBle, BLE_CMD_CLEAR_CONFIG);
  //tamanho do frame
  len=strlen((char*) bufferTxBle);
  // envia buffer pro ble
  sendBufferBluetooth( bufferTxBle,  len );
  // set flag que define a provável resposta esperada pela serial
  cmd = CMD_WAIT_OK;
  // seta timeout da comunicação
  setTimeoutCommunicationBle(200);
  // espera pelo fim da comunicação
  while(!(eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION))
  {
  }  
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  
  if(indiceBufferRxBle==4 && bufferRxBle[0]=='O' && bufferRxBle[1]=='K')
    return 1;
  else
    return 0;
}

void bleRestart(void)
{
  unsigned char bufferTxBle[50];
  
  unsigned char len;
 
  indiceBufferRxBle=0;
  
  clearBuffer(bufferRxBle,SIZE_BUFFER_RX);  
  
  strcpy((char*)bufferTxBle, BLE_CMD_RESTART);
  //tamanho do frame
  len=strlen((char*) bufferTxBle);
  // envia buffer pro ble
  sendBufferBluetooth( bufferTxBle,  len );
  
  delay(6000);
}

void stopParing(void)
{
  // para o scan automatico
  bleStartStopScan(DISCONNECT);
  
  bleSaveConfig();
  
  bleRestart();
}

void startParing(void)
{
  // para o scan automatico
  bleStartStopScan(CONNECT);
  
  bleSaveConfig();
  
  bleRestart();
}

unsigned char bleStatusConnection(void)
{
  unsigned char bufferTxBle[50];
  
  unsigned char len;
 
  indiceBufferRxBle=0;
  
  clearBuffer(bufferRxBle,SIZE_BUFFER_RX);  
  
  strcpy((char*)bufferTxBle, BLE_CMD_STATUS);
  //tamanho do frame
  len=strlen((char*) bufferTxBle);
  // envia buffer pro ble
  sendBufferBluetooth( bufferTxBle,  len );
  // set flag que define a provável resposta esperada pela serial
  cmd = 0;
  // seta timeout da comunicação
  setTimeoutCommunicationBle(100);
  // espera pelo fim da comunicação
  
  flagWaitAnswerBle=1;
  
  while(!(eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION))
  {
  }  
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  
  flagWaitAnswerBle=0;
  
   if ( strstr ( (char*)bufferRxBle , "CON") != NULL )
   {
      clearBuffer(bufferRxBle,SIZE_BUFFER_RX);  
      indiceBufferRxBle=0;
      return 1;
   }
   else
   {
      clearBuffer(bufferRxBle,SIZE_BUFFER_RX);  
      indiceBufferRxBle=0;
      return 0;
   }
}

void bleGetConfig(void)
{
  unsigned char bufferTxBle[50];
  
  unsigned char len;
 
  indiceBufferRxBle=0;
  
  clearBuffer(bufferRxBle,SIZE_BUFFER_RX);  
  
  strcpy((char*)bufferTxBle, BLE_CMD_GET_CONFIG);
  //tamanho do frame
  len=strlen((char*) bufferTxBle);
  // envia buffer pro ble
  sendBufferBluetooth( bufferTxBle,  len );
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  
  setTimeoutCommunicationBle(600);
  // espera pelo fim da comunicação
  while(!(eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION))
  {
    
  }  
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
}

unsigned char disableCCON(void)
{
  unsigned char bufferTxBle[50];
  
  unsigned char len;
 
  indiceBufferRxBle=0;
  
  clearBuffer(bufferRxBle,SIZE_BUFFER_RX);  
  
  strcpy((char*)bufferTxBle, BLE_CMD_DISABLE_CCON);
  //tamanho do frame
  len=strlen((char*) bufferTxBle);
  // envia buffer pro ble
  sendBufferBluetooth( bufferTxBle,  len );
  // set flag que define a provável resposta esperada pela serial
  cmd = 0;
  // seta timeout da comunicação
  setTimeoutCommunicationBle(100);
  // espera pelo fim da comunicação
  while(!(eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION))
  {
  }  
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  
  if(indiceBufferRxBle==4 && bufferRxBle[0]=='O' && bufferRxBle[1]=='K')
    return 1;
  else
    return 0;
}

unsigned char bleConnection(unsigned char connection)
{
  unsigned char bufferTxBle[50];
  
  unsigned char addr[20];
  
  unsigned char len;
  
  indiceBufferRxBle=0;
  
  clearBuffer(bufferRxBle,SIZE_BUFFER_RX);
  
  if(connection==ON)
  {
    bleGetAddr(addr);
    
    sprintf( (char*)bufferTxBle , "CON %s 0\r",addr);
    
    indiceBufferRxBle=0;
  
    clearBuffer(bufferRxBle,SIZE_BUFFER_RX);
    
  }
  else
    strcpy((char*)bufferTxBle, BLE_CMD_DISCONNECT);
  
  //tamanho do frame
  len=strlen((char*) bufferTxBle);
  // envia buffer pro ble
  sendBufferBluetooth( bufferTxBle,  len );
  // set flag que define a provável resposta esperada pela serial
  cmd = 0;
  
  if(connection==ON)
      // seta timeout da comunicação
      setTimeoutCommunicationBle(2000);
  else
    setTimeoutCommunicationBle(200);
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  
  // espera pelo fim da comunicação
  while(!(eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION))
  {
  }  
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  
  return 0;
}

void bleWaitOk(void)
{
   contTimeEventTimeOutBle=0;
      
   flagEventTimeOutBle=0;

   eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
      
   flagWaitAnswerBle=0;
      
   clearBuffer(bufferRxBle, SIZE_BUFFER_RX);
      
   indiceBufferRxBle = 0;
            
   cmd = CMD_WAIT_OK;
   // seta timeout da comunicação
   setTimeoutCommunicationBle(100);
   // espera pelo fim da comunicação
   while(!(eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION))
   {     
   }  
  
   eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
}

unsigned char bleGetAddr(unsigned char* addrSlave)
{
   char *strFind;
   
   unsigned char j;
   
   bleGetConfig();
   
   strFind = strstr( (char*)bufferRxBle,"ADDR" ); 
  
   if(strFind != NULL)
   {
      strFind+=5;
    
      for(j=0; j <12 ; j++)
        *(addrSlave++) =  *(strFind++);
     
      *(addrSlave) =  0;
      
       return 1;
   }
   
   return 0;
}


unsigned char* bleReadAddr(void)
{
   char *strFind;
   
   unsigned char j=0;
   
   unsigned char frame[20];
  
   unsigned char i=0;
  
   unsigned char crcCalculate;
  
   unsigned char* addrSlave;
    
   unsigned char crcFrame;
   
   addrSlave = &frame[2];
   
   strFind = strstr( (char*)bufferRxBle , "RCV" ); 
  
   if(strFind != NULL)
   {
      strFind+=4;
      
      if(  *(strFind)  == 'C')
      {
      
        frame[0] = *strFind++; /*comando*/
  
        frame[1] = *strFind++; /*tamanho*/
        
        for(i=2 ; i < frame[1] ; i++)
          frame[i] = *strFind++;
        
        crcFrame = frame[ frame[1] - 1 ] ;
  
        crcCalculate = crcCalc( (unsigned char*) frame, frame[1] - 1);
        
        if(crcCalculate != crcFrame  )
        {
          addrSlave=NULL;
          
          return addrSlave;
        }
        else 
        {
          frame[14] = 0;
          
          return &frame[2];
        }
        
      }
   }
   
   addrSlave=NULL;
   
   return addrSlave;
}


unsigned char* bleVerifyAnswerFrame(unsigned char *buffer, unsigned char *bufferReturn )
{
  unsigned char frame[20];
  
  unsigned char i;
  
  unsigned char crcCalculate;
  
  unsigned char crcFrame;
  
  buffer+=4;
  
  frame[0] = *buffer++; /*comando*/
  
  frame[1] = *buffer++; /*tamanho*/
  
  for(i=2 ; i < frame[1] ; i++)
      frame[i] = *buffer++;
  
  crcFrame = frame[ frame[1]- 1];
  
  crcCalculate = crcCalc( frame, frame[1] - 1);
  
  if(crcCalculate == crcFrame  )
  {
    crcFrame = 0;
  }
  
  else
    crcFrame = 0;
 
   if( frame[0] == 'S' )
   { 
     switch (frame[2])
     {
        case 0x01: /* realizado com sucesso*/
            *bufferReturn = 'S';
             bufferReturn++;
            *bufferReturn= 1 ;
            return bufferReturn;
        break;
        
        case 0x02: /* erro de crc enviado pelo skate*/
            *bufferReturn = 'S';
             bufferReturn++;
            *bufferReturn= 2 ;
            return bufferReturn;
        break;
        
        case 0x03: /*  falha na identificação do frame */
            *bufferReturn = 'S';
             bufferReturn++;
            *bufferReturn= 3 ;
            return bufferReturn;
        break;
     }
   }
   
   if( frame[0] == 'V' )
   { 
     switch (frame[2])
     {
        case 0x01: 
            *bufferReturn = 'V';
             bufferReturn++;
            *bufferReturn= 1;
            levelVbatSkate = frame[3];
            eventAsynchronousOccurred |= ASYNCHRONOUS_UPDATE_LED_LEVEL_VBAT;
            
            return bufferReturn;
        break;
        
        case 0x02: 
            *bufferReturn = 'V';
             bufferReturn++;
            *bufferReturn= 2 ;
            return bufferReturn;
        break;
        
        case 0x03: 
            *bufferReturn = 'V';
             bufferReturn++;
            *bufferReturn= 3 ;
            return bufferReturn;
        break;
     }
   }

   return bufferReturn;
}
   
unsigned char bleVerifyConfiguration(void)
{
  unsigned char bufferTxBle[50];
  
  unsigned char len;
 
  indiceBufferRxBle=0;
  
  clearBuffer(bufferRxBle,SIZE_BUFFER_RX);  
  
  strcpy((char*)bufferTxBle, BLE_CMD_STATUS);
  //tamanho do frame
  len=strlen((char*) bufferTxBle);
  // envia buffer pro ble
  sendBufferBluetooth( bufferTxBle,  len );
  // set flag que define a provável resposta esperada pela serial
  cmd = 0;
  // seta timeout da comunicação
  setTimeoutCommunicationBle(50);
  // espera pelo fim da comunicação
  while(!(eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION))
  {
  }  
  
  eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
  
   if ( bufferRxBle[4] == 'C' )
      return 1;
   else
     return 0;
}

unsigned char setBle(unsigned char *addr)
{
  unsigned char answer,b[12];
  
  char* p=addr; 

  //----------------------------------------------------------------------------
  answer = bleMasterOn();
  
  if(answer)
  {
   //  answer = bleEnableScan();
    
     answer = bleStartStopScan(DISCONNECT);
    
     if(answer)
     {
        // set endereço bt
        answer = bleSetAddr(p);
        
        if(answer)
        {
          // set ccon= off
          answer = bleEnableCcon();
          
          if( answer )
          {
              answer = bleEnableScan(OFF);
            
              if(answer )
              {
                  answer = bleSaveConfig();
                  
                  if(answer )
                  {
                    bleRestart();
                    
                    disableCCON();
                    
                    bleSaveConfig();
  
                    bleRestart();
                    
                    lenStatusOn = 255;
                    
                    bleConfigured=CONFIGURED;
               
                    return 1;
                  }
              }
          }
        }
      }
  }
  
  return 0;
}
