#include "../inc/includes.h"

//Нижняя 
//DE P3.6

UsartManagerStatus_t UsartManagerSatus;//?????? UART (????????/?????)
UsartDescriptor_t UART_Descriptor;

uint8_t _RxBuff[8];
uint8_t uart_speed_index;

void UART_TxConfirm(void);
void UART_RxEvent(uint8_t *len);

void StartUartManager(void)
{
#ifdef __MSPX__	
DE_make_out();  
DE_clr();  
  
UART_Descriptor.channel=USART_CHANNEL_0;
UART_Descriptor.baudrate = (baudrate_t)uart_speed_index; //USART_BAUDRATE_115200;

UART_Descriptor.txCallback=UART_TxConfirm;
UART_Descriptor.rxCallback=UART_RxEvent;
UART_Descriptor.rxBuff=_RxBuff;
UART_Descriptor.mode = RS485_Mode;//RS232_Mode;
UART_Descriptor.rxBuffSize=sizeof(_RxBuff);
#endif

#ifdef __WINX__
UART_Descriptor.channel      = USART_CHANNEL_0;
UART_Descriptor.baudrate     = USART_BAUDRATE_115200;
UART_Descriptor.dataLength   = USART_DATA8;
UART_Descriptor.stopbits     = USART_STOPBIT_1;
UART_Descriptor.parity       = USART_PARITY_NONE;
UART_Descriptor.flowControl  = USART_FLOW_CONTROL_NONE;

UART_Descriptor.txCallback   = UART_TxConfirm;
UART_Descriptor.rxCallback   = UART_RxEvent;
UART_Descriptor.rxBuff       = _RxBuff;
UART_Descriptor.mode         = RS485_Mode;
UART_Descriptor.rxBuffSize   = sizeof(_RxBuff);
#endif

#ifdef __LINX__
UART_Descriptor.channel      = USART_CHANNEL_0;
UART_Descriptor.baudrate     = USART_BAUDRATE_9600;
UART_Descriptor.dataLength   = USART_DATA8;
UART_Descriptor.stopbits     = USART_STOPBIT_1;
UART_Descriptor.parity       = USART_PARITY_NONE;
UART_Descriptor.flowControl  = USART_FLOW_CONTROL_NONE;

UART_Descriptor.txCallback   = UART_TxConfirm;
UART_Descriptor.rxCallback   = UART_RxEvent;
UART_Descriptor.rxBuff       = _RxBuff;
UART_Descriptor.mode         = RS485_Mode;
UART_Descriptor.rxBuffSize   = sizeof(_RxBuff);

#endif
OpenUsart(&UART_Descriptor);
}



void SlaveSendPacket(uint8_t *p,uint8_t size)
{
UsartManagerSatus = UsartManagerBusy;

#ifdef __MSPX__
DE_set();    
#endif

  WriteUsart(&UART_Descriptor,p,size);
}


void UART_RxEvent(uint8_t *len)
{
  TBusDataReceiver(_RxBuff[(*len-1)]);
}


void UART_TxConfirm(void)
{
UsartManagerSatus = UsartManagerFree;

#ifdef __MSPX__
DE_clr();    
#endif

asm("nop");  
}











