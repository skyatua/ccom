#ifndef _uartmanager_h
#define _uartmanager_h

//#include "includes.h"
//#include "variables.h"

#define assign_uart_pin(name,port,pin)\
inline void name##_make_out(void){port##DIR|=(1<<pin);}\
inline void name##_set(void){port##OUT|=(1<<pin);}\
inline void name##_clr(void){port##OUT&=~(1<<pin);}

//assign_uart_pin(DE,P3,6);

typedef enum
{
UsartManagerFree,
UsartManagerBusy
}UsartManagerStatus_t;//?????? UART - ????????/?????


extern UsartManagerStatus_t UsartManagerSatus;//?????? UART (????????/?????)
extern uint8_t uart_speed_index;

extern void StartUartManager(void);
extern void SlaveSendPacket(uint8_t *p,uint8_t size);

#endif







