#include "ble.h"
#include "main.h"
#include "slider.h"
#include "events.h"
#include "stdio.h"
#include "string.h"

//---------------------------------------------------------
// variáveis de controle dos eventos sincronos
uint16_t contTimeEventReadKeyPad = 0 ;
unsigned char flagEventReadKeyPad = 0 ;
 
uint16_t contTimeEventVerifySlider = 0 ; 
unsigned char flagEventVerifySlider = 0 ;

uint16_t contTimeEventUpdateValueVbat=0;
unsigned char flagEventUpdateValueVbat=0;

uint16_t contTimeEventPairing = 0 ;
unsigned char flagEventPairing = 0 ;

uint16_t contTimeEventVerifyBelConnected = 0;
unsigned char flagEventVerifyBelConnected = 0;

uint16_t timeToogle=1000;

uint16_t contTimeEventClearConfigBle = 0 ; 
unsigned char flagEventClearConfigBle = 0 ;
unsigned char flagEnableClearConfigBle = 0 ;

uint16_t contTimeEventSolicitationVbatSkate = 0;
unsigned char flagEventSolicitationVbatSkate = 0;


/* evento a ser avaliado, se realmente é necessario existir*/
uint16_t contTimeEventSendFrameBle = 0;

unsigned char flagEventSendFrameBle = 0;

uint16_t contTimeEventTimeOutBle = 0;

uint16_t setTimeEventTimeOutBle = 0;

unsigned char flagEventTimeOutBle = 0;


uint16_t timeEventToogleLed = 0;
//---------------------------------------------------------
// variáveis de controle dos eventos assincronos
unsigned char flagAsynchronousEvent=0;

unsigned char eventAsynchronousOccurred=0;
//---------------------------------------------------------

unsigned char flagWaitAnswerBle = 0;

unsigned char vBat = 0;

unsigned char lenStatusOn = 0;


void TIM1Conf(void)
{
  // Rotina de configuração do timer 1 para gerar interrupções a cada 1ms.
  
   NVIC_InitTypeDef             NVIC_InitStructure;
   TIM_TimeBaseInitTypeDef      TIM_TimeBaseStructure;
    
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 , ENABLE); // habilita clk do periferico
   
 
   TIM_TimeBaseStructure.TIM_Prescaler = 48 - 1; // set prescaler do clk do timer para operar em 1Mhz 
   TIM_TimeBaseStructure.TIM_Period = 1000; // seta a qnt de clks necessários para gerar a interrupção - periodo de 1ms
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // sem divisao
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // modo de contagem do timer
   TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
   TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); // seta a estrutura do timer
    
   // habilita config e liga o timer
   TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE); 
   TIM_Cmd(TIM1, ENABLE);  
 
    /* configuração da interrupção do Timer1 */
   NVIC_ClearPendingIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
   NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_UP_TRG_COM_IRQn; // rotina de tratamento da interrupção
   NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // habilita interrupção
   NVIC_Init(&NVIC_InitStructure);
}
 
void TIM1_BRK_UP_TRG_COM_IRQHandler (void)
{
  if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
  {
    /*
    O objetivo da rotina do timer é implementar base de tempos para verificação periódicas dos eventos do sistema:
    - Toogle Led.
    - Leitura do teclado.
    - Verificação da variação do acelerador.
    - Verificação da tensão da bateria.
    - Timeout de comunicação serial.
    - Solicitação da conexão entre controle e skate.
    - Limpar as configurações do ble.
    - Solicitação Vbat do skate.
    */

      // incremento dos contadores de controle dos eventos periódicos
      timeEventToogleLed++;
      
      if(bleConnected)
      {
          contTimeEventVerifySlider++;
          
          contTimeEventUpdateValueVbat++;
          
          contTimeEventSolicitationVbatSkate++;
      }

      if(bleConfigured)
        contTimeEventReadKeyPad++;

      contTimeEventTimeOutBle++;
      
      
      // Evento para modificar o status do ledStatus
      if(timeEventToogleLed>=timeToogle)
      {
          GPIOA->ODR ^= LED_STATUS;
          
          if( lenStatusOn ==1 )
               GPIOA->ODR |= LED_STATUS;
  
          else if( lenStatusOn==0 )
               GPIOA->ODR &= ~LED_STATUS;
          
          timeEventToogleLed=0;
      }
        // Evento que controla o tempo de leitura do teclado 
      if(contTimeEventReadKeyPad >= TIME_READ_KEYPAD)
      {
          contTimeEventReadKeyPad=0;
          
          flagEventReadKeyPad=1;
      }
      
      // Evento que controla o tempo de leitura do acelerador
      if(contTimeEventVerifySlider >= TIME_VERIFY_SLIDER)
      {
          contTimeEventVerifySlider=0;
          
          flagEventVerifySlider=1;
      }
      
      // Evento que verifica o tempo de leitura do valor da tensao da bateria
      if(contTimeEventUpdateValueVbat >= TIME_UPDATE_VALUE_VBAT)
      {
          contTimeEventUpdateValueVbat=0;
          
          flagEventUpdateValueVbat=1;
      }

      // Controle do timeout da comunicação serial.
      // onde o tempo deste evento é variável, e é controlado pela variável setTimeEventTimeOutBle.
      
     /* O evento de envio de frames pela serial, so é liberado quando o flagWaitAnswerBle está zerado,
       Indicando q a serial está liberada para comunicação.
      */
      if(contTimeEventTimeOutBle >= setTimeEventTimeOutBle && flagWaitAnswerBle)
      {
          flagEventTimeOutBle=1;
          
          eventAsynchronousOccurred |= ASYNCHRONOUS_EVENT_END_COMMUNICATION;
      }
       
      if(flagDelay)  
        contTimeDelay++;
      
      if(flagEventPairing)
      {
          contTimeEventPairing++;
          
          if( contTimeEventPairing >= TIME_TOOGLE_PAIRING )
          {
              eventAsynchronousOccurred |= ASYNCHRONOUS_EVENT_TOOGLE_PAIRING;
              
              flagEventPairing=0; 
          }
      }
      
      if(flagEnableClearConfigBle)
      {
          contTimeEventClearConfigBle++;
          
          if( contTimeEventClearConfigBle >= TIME_CLEAR_CONFIG_BLE )
          {
              eventAsynchronousOccurred |= ASYNCHRONOUS_EVENT_CLEAR_CONFIG_BLE;
              
              flagEnableClearConfigBle=0; 
          }
      }
          
      if(flagBleVerifyPaired && !flagWaitAnswerBle)
      {
        contTimeEventVerifyBelConnected++;
        
        if(   contTimeEventVerifyBelConnected   >= TIME_VERIFY_BLE_CONNECTED   )
        {
          contTimeEventVerifyBelConnected=0;
          
          flagEventVerifyBelConnected=1;
        }
      }
        
        if(   contTimeEventSolicitationVbatSkate   >= TIME_SOLICITATION_VBAT_SKATE    && !flagWaitAnswerBle  && bleConnected )
        {
           contTimeEventSolicitationVbatSkate=0;
          
           flagEventSolicitationVbatSkate=1;
           
           eventAsynchronousOccurred |= ASYNCHRONOUS_EVENT_SOLICITATION_VBAT_SKATE;
        }

      TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
   }
}

void firstConfiguration(void)
{
  unsigned char *addr;
  
  unsigned char answer=0;
  
  unsigned char addrVet[13];
  
 
   
  
  cmd = CMD_WAIT_FIRST_CONFIGURATION ;
  
  if( eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION)
  {
      addr = bleReadAddr();
      
      if(addr != NULL)
      {
        
         for(int i=0 ; i <12 ;i++)
         {
            addrVet[i] = *addr++ ;
  
         }
         
         addrVet[12] =0;
        
         answer = 0;
         
         sendBufferBluetooth(   (unsigned char*) "SND ADDR SET OK\r" , 16 );
         
         lenStatusOn = 0;
         
         bleWaitOk();
         
         setBle(addrVet); 
             //bleSetAddr((char*) addr);
      }
      else
      {
         answer = 0;
         
         sendBufferBluetooth(   (unsigned char*) "SND ERRO\r" , 9 );
            
         bleWaitOk();
      
      }
    
      contTimeEventTimeOutBle=0;
      
      flagEventTimeOutBle=0;

      eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
      
      flagWaitAnswerBle=0;
      
      clearBuffer(bufferRxBle, SIZE_BUFFER_RX);
      
      indiceBufferRxBle = 0;
  }
}

void checkEvents(void)
{
  timeEvents();  
  
  asynchronousEvents();
}

void asynchronousEvents(void)
{
  unsigned char bufferTxBle[15];
  
  unsigned char answer=0;
  
  unsigned char *pAnswer;
  
  unsigned char pReturn[2];
   
  // Função que trata os eventos assincronos
   
   if( (eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_SEND_FRAME_STATUS_BLE)  &&  !flagWaitAnswerBle && bleConnected )
   {
        indiceBufferRxBle=0;
        
        clearBuffer(bufferRxBle,SIZE_BUFFER_RX);
        
        mountFrameStatusBle(bufferTxBle);
        
        sendBufferBluetooth(bufferTxBle,15);
        
        setTimeoutCommunicationBle(300);
        
        eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_SEND_FRAME_STATUS_BLE;
   }   
   
   // essa rotina verifica o frame enviado do ble e libera o envio de outro frame para o ble
   if( (eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_END_COMMUNICATION)  )
   {
        flagWaitAnswerBle = 0;
      
        flagEventTimeOutBle=0;
        
        flagWaitAnswerBle=0;
      
        eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_END_COMMUNICATION;
        
        pAnswer  = strstr ( (char*)bufferRxBle , "RCV") ;
        
        if (pAnswer != NULL )
        {
            bleVerifyAnswerFrame(pAnswer , pReturn );
        }
       
        cmd=0;
        
        clearBuffer(bufferRxBle,SIZE_BUFFER_RX);
        
        indiceBufferRxBle=0;
   }
   
   if(  eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_TOOGLE_PAIRING)
   {
       answer = bleStatusConnection();
       
       if(answer)
       {
          bleConnection(OFF);
          
          flagBleVerifyPaired=0;
          
          timeToogle=1000;
          
          clearBuffer(statusKeypad,5);
          
          bleConnected=0;
       }
       
       else
       {
          clearBuffer(statusKeypad,5);
          
          bleConnection(ON);
          
          // precisa implementar aqui o teste para saber se realmete está conectado
          
          flagBleVerifyPaired=1;  
       }
       
       flagEventPairing=0;
       
       eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_TOOGLE_PAIRING;
   }

   if( (eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_CLEAR_CONFIG_BLE)  )
   {
        bleConnected = 0;
        
        bleClearConfig();
  
        bleSaveConfig();
  
        bleRestart();
 
        eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_CLEAR_CONFIG_BLE;  
        
        lenStatusOn = 1;
        
        bleConfigured = NO_CONFIGURED;
   }
  
   if( (eventAsynchronousOccurred & ASYNCHRONOUS_EVENT_SOLICITATION_VBAT_SKATE)  )
   {
        indiceBufferRxBle=0;
        
        clearBuffer(bufferRxBle,SIZE_BUFFER_RX);
        
        mountFrameSolicitationVbatSkate(bufferTxBle);
        
        sendBufferBluetooth(bufferTxBle,8);
        
        setTimeoutCommunicationBle(300);
      
        eventAsynchronousOccurred &= ~ASYNCHRONOUS_EVENT_SOLICITATION_VBAT_SKATE;  
   }
   
  
   if( (eventAsynchronousOccurred & ASYNCHRONOUS_UPDATE_LED_LEVEL_VBAT ))
   {
      eventAsynchronousOccurred &= ~ASYNCHRONOUS_UPDATE_LED_LEVEL_VBAT;  
  
      if(levelVbatSkate > 0 && levelVbatSkate <25 )
        GPIOA->ODR = LED_0;
      
      
      if(levelVbatSkate >=26 && levelVbatSkate <=50 )
      {
        GPIOA->ODR = LED_0;
        GPIOA->ODR |= LED_1;
      }

      if(levelVbatSkate >=51 && levelVbatSkate <=75 )
      {
        GPIOA->ODR = LED_0;
        GPIOA->ODR |= LED_1;
        GPIOA->ODR |= LED_2;
      }
      
      if(levelVbatSkate >=76 && levelVbatSkate <=100 )
      {
        GPIOA->ODR = LED_0;
        GPIOA->ODR |= LED_1;
        GPIOA->ODR |= LED_2;
        GPIOA->ODR |= LED_3;
      }
   
   }
   
   
}

void timeEvents(void)
{
    unsigned char answer;
     /*evento de leitura do teclado */
     if(flagEventReadKeyPad)
     {
        readKeypad();
        
        flagEventReadKeyPad=0;
        
        contTimeEventReadKeyPad=0; 
     }
     
     /*Evento de verificação do acelerador */
     if( flagEventVerifySlider  )
     {
        /*porcentagem do valor lido*/
        valueSlider = (100* sliderAdc[0])/4095;
        
        if(valueSlider>=3)
        {
            sliderVariation =1;
            eventAsynchronousOccurred |=ASYNCHRONOUS_EVENT_SEND_FRAME_STATUS_BLE;
        }
        else
        {
          valueSlider=0;
          
          if(sliderVariation)
          {    
            eventAsynchronousOccurred |=ASYNCHRONOUS_EVENT_SEND_FRAME_STATUS_BLE;

            sliderVariation=0;
          }
        }
         
        /*Limpla flags*/
        contTimeEventVerifySlider=0; 
        
        flagEventVerifySlider=0;
     }
     
      if(flagEventUpdateValueVbat)
     {
        vBat = (100* sliderAdc[1])/4095;
        
        flagEventUpdateValueVbat=0;
     }
     
     if(flagEventVerifyBelConnected)
     {
        answer = bleStatusConnection();
        
        if(answer)
        {
          bleConnected=1;
          
          timeToogle=50;
        }
        else
        {
          timeToogle=1000;
          
          bleConnected=0;
          
          bleConnection(ON);
          
        }
        
        flagEventVerifyBelConnected=0;
     }
}
