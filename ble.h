
#ifndef __BLE_H
#define __BLE_H

#define SIZE_BUFFER_RX 500

#define CONNECT 0X01
#define DISCONNECT 0X00

#define ON 0X01
#define OFF 0X00

#define CONFIGURED 0X01

#define NO_CONFIGURED 0X00

/*definições de comandos*/
#define CMD_SEND_FRAME_STATUS 0x01
#define CMD_WAIT_OK 0x02
#define CMD_WAIT_FIRST_CONFIGURATION 0x03
#define CMD_SEND_SOLICITATION_VBAT_SKATE 0x04

// Comando de configuração do ble
#define BLE_CMD_MODE_MASTER_ON "SET CENT=ON\r"

#define BLE_CMD_ENABLE_SCAN "SET SCNA=ON\r"

#define BLE_CMD_DISABLE_SCAN "SET SCNA=OFF\r"

#define BLE_CMD_ENABLE_CCON "SET CCON=ON\r"

#define BLE_CMD_DISABLE_CCON "SET CCON=OFF\r"

//#define BLE_CMD_CONNECT "CON 20FABB0276F0 0\r"
#define BLE_CMD_CONNECT "CON" 

#define BLE_CMD_DISCONNECT "DCN\r"



#define BLE_CMD_SET_ADDR "SET ADDR=20FABB0276F0\r"

#define BLE_CMD_START_SCAN "SET ACON=ON\r"
#define BLE_CMD_STOP_SCAN "SET ACON=OFF\r"
#define BLE_CMD_TO_STORE_SETTING "WRT\r"
#define BLE_CMD_RESTART "RST\r"
#define BLE_CMD_CLEAR_CONFIG "RTR\r"
#define BLE_CMD_GET_CONFIG "GET\r"
#define BLE_CMD_STATUS "STS\r"


extern unsigned char bufferRxBle[];

extern int indiceBufferRxBle;

extern unsigned char cmd;

extern unsigned char flagBleVerifyPaired;

extern unsigned char bleConnected;

extern unsigned char bleConfigured;

void initUsartBluetooth(void);

void sendBufferBluetooth(unsigned char *bufferTx, unsigned char len );

void USART1_IRQHandler(void);

void mountFrameStatusBle( unsigned char *buffer );

void clearBuffer( unsigned char *buffer , int len);

void setTimeoutCommunicationBle( int timeOut );

unsigned char setBle(unsigned char *addr);

unsigned char bleMasterOn(void);

unsigned char bleEnableScan(unsigned char scan);

unsigned char bleSetAddr(char *addr);

unsigned char bleStartStopScan(unsigned char startStop);

unsigned char bleSaveConfig(void);

void bleRestart(void);

unsigned char bleClearConfig(void);

unsigned char bleEnableCcon(void);

unsigned char bleStatusConnection(void);

void stopParing(void);

void startParing(void);

unsigned char disableCCON(void);

void bleGetConfig(void);

unsigned char bleConnection(unsigned char connection);

unsigned char bleVerifyConfiguration(void);

unsigned char bleGetAddr(unsigned char* addrSlave);

unsigned char* bleReadAddr(void);

void bleWaitOk(void);

unsigned char *bleVerifyAnswerFrame(unsigned char *buffer, unsigned char *bufferReturn );

unsigned char crcCalc(unsigned char* buffer , unsigned char len);

void mountFrameSolicitationVbatSkate( unsigned char *buffer  );



#endif 
