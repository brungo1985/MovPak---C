#ifndef __EVENTS_H
#define __EVENTS_H

/* tempo da ocorrencia dos eventos em ms*/
#define TIME_BLINK 1000
#define TIME_READ_KEYPAD 100
#define TIME_VERIFY_SLIDER 500
#define TIME_SEND_FRAME_BLE 1000
#define TIME_UPDATE_VALUE_VBAT 10000
#define TIME_TOOGLE_PAIRING 3000
#define TIME_CLEAR_CONFIG_BLE 3000
#define TIME_VERIFY_BLE_CONNECTED 4000
#define TIME_SOLICITATION_VBAT_SKATE 6000


/*definições dos eventos assincronos */
#define ASYNCHRONOUS_EVENT_SEND_FRAME_STATUS_BLE 0x01
#define ASYNCHRONOUS_EVENT_END_COMMUNICATION 0x02
#define ASYNCHRONOUS_EVENT_TOOGLE_PAIRING 0x04
#define ASYNCHRONOUS_EVENT_CLEAR_CONFIG_BLE 0x08
#define ASYNCHRONOUS_EVENT_SOLICITATION_VBAT_SKATE 0x10

#define ASYNCHRONOUS_UPDATE_LED_LEVEL_VBAT 0x20

extern unsigned char lenStatusOn;

extern uint16_t contTimeEventReadKeyPad;
extern unsigned char flagEventReadKeyPad;

extern uint16_t contTimeEventVerifySlider;
extern unsigned char flagEventVerifySlider;

extern uint16_t contTimeEventPairing;
extern unsigned char flagEventPairing;

extern uint16_t contTimeEventSendFrameBle;
extern unsigned char flagEventSendFrameBle;

extern uint16_t contTimeEventUpdateValueVbat;
extern unsigned char flagEventUpdateValueVbat;

extern uint16_t contTimeEventTimeOutBle;
extern uint16_t setTimeEventTimeOutBle;
extern unsigned char flagEventTimeOutBle;

extern uint16_t timeEventToogleLed;

extern unsigned char flagAsynchronousEvent;
extern unsigned char eventAsynchronousOccurred;

extern uint16_t contTimeEventVerifyBelConnected;
extern unsigned char flagEventVerifyBelConnected;

extern unsigned char flagEnableClearConfigBle;
extern uint16_t contTimeEventClearConfigBle  ; 
extern unsigned char flagEventClearConfigBle  ;


extern uint16_t contTimeEventSolicitationVbatSkate;
extern unsigned char flagEventSolicitationVbatSkate;



extern unsigned char flagWaitAnswerBle;

extern unsigned char vBat;
 
extern uint16_t timeToogle;

void timeEvents(void);

void checkEvents(void);

void TIM1_BRK_UP_TRG_COM_IRQHandler (void);

void TIM1Conf(void);

void asynchronousEvents(void);

void firstConfiguration(void);


#endif 
