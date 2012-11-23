#ifndef _UartDriver_h
#define _UartDriver_h

#ifdef __WINX__
#include <windows.h>
#endif

#ifdef __LINX__
#include <unistd.h>			
#include <fcntl.h>			
#include <termios.h>		
#endif

typedef enum
{
USART_CH_NOINIT	 = -1,
USART_CHANNEL_0  = 0x00,
USART_CHANNEL_1  = 0x01,
}channel_t;

typedef enum
{
#ifdef __MSPX__	
USART_FLOW_CONTROL_NONE    = 0x00,
USART_FLOW_CONTROL_TX      = 0x01,
USART_FLOW_CONTROL_RX      = 0x02
#endif
#ifdef __LINX__
USART_FLOW_CONTROL_NONE    = 0x00,
USART_FLOW_CONTROL_RTS     = 0x01,
USART_FLOW_CONTROL_CTS     = 0x02,
USART_FLOW_CONTROL_RTS_CTS = 0x03,
#endif
#ifdef __WINX__
USART_FLOW_CONTROL_NONE    = 0x00,
#endif
}flowControl_t;

typedef enum
{
#ifdef __MSPX__	
USART_DATA5       = 0x00,
USART_DATA6       = 0x01,
USART_DATA7       = 0x02,
USART_DATA8       = 0x03
#endif
#ifdef __LINX__  
USART_DATA5       = CS5,
USART_DATA6       = CS6, 
USART_DATA7       = CS7,
USART_DATA8       = CS8 
#endif
#ifdef __WINX__
USART_DATA5       = DATABITS_5,
USART_DATA6       = DATABITS_6, 
USART_DATA7       = DATABITS_7,
USART_DATA8       = DATABITS_8
#endif
}dataLength_t;

typedef enum
{
#ifdef	__MSPX__
USART_PARITY_NONE  = 0x00,
USART_PARITY_EVEN  = 0x01,
USART_PARITY_ODD   = 0x02,
#endif
#ifdef __LINX__
USART_PARITY_NONE  = 0x00,
USART_PARITY_EVEN  = 0x01,
USART_PARITY_ODD   = 0x02,
#endif
#ifdef __WINX__
USART_PARITY_NONE  = PARITY_NONE,
USART_PARITY_EVEN  = PARITY_EVEN,
USART_PARITY_ODD   = PARITY_ODD,
USART_PARITY_MARK  = PARITY_MARK,
USART_PARITY_SPACE = PARITY_SPACE,
#endif
}parity_t;

typedef enum
{
#ifdef __MSPX__	
USART_STOPBIT_1   = 0x00,
USART_STOPBIT_2   = 0x01,
#endif
#ifdef __LINX__
USART_STOPBIT_1   = 0x01,
USART_STOPBIT_0_5 = 0x02,
USART_STOPBIT_2   = 0x03,
USART_STOPBIT_1_5 = 0x04
#endif
#ifdef __WINX__
USART_STOPBIT_1   = ONESTOPBIT,
USART_STOPBIT_1_5 = ONE5STOPBITS,
USART_STOPBIT_2   = TWOSTOPBITS,
#endif
}stopbits_t;

typedef enum
{
#ifdef __MSPX__	
USART_BAUDRATE_9600   = 0x00,
USART_BAUDRATE_19200  = 0x01,
USART_BAUDRATE_38400  = 0x02,
USART_BAUDRATE_57600  = 0x03,
USART_BAUDRATE_115200 = 0x04,
#endif
#ifdef __WINX__
USART_BAUDRATE_110    = CBR_110,
USART_BAUDRATE_300    = CBR_300,
USART_BAUDRATE_600    = CBR_600,
USART_BAUDRATE_1200   = CBR_1200,
USART_BAUDRATE_2400   = CBR_2400,
USART_BAUDRATE_4800   = CBR_4800,
USART_BAUDRATE_9600   = CBR_9600,
USART_BAUDRATE_14400  = CBR_14400,
USART_BAUDRATE_19200  = CBR_19200,
USART_BAUDRATE_38400  = CBR_38400,
USART_BAUDRATE_56000  = CBR_56000,
USART_BAUDRATE_57600  = CBR_57600,
USART_BAUDRATE_115200 = CBR_115200,
USART_BAUDRATE_128000 = CBR_128000,
USART_BAUDRATE_256000 = CBR_256000,
#endif
#ifdef __LINX__
USART_BAUDRATE_9600   = B9600,  
USART_BAUDRATE_19200  = B19200,
USART_BAUDRATE_38400  = B38400,
USART_BAUDRATE_57600  = B57600,
USART_BAUDRATE_115200 = B115200,
#endif
}baudrate_t;

typedef enum
{
RS485_Mode = 0x00,
RS232_Mode = 0x01
}mode_tt;

typedef struct
{
channel_t      channel;
mode_tt        mode;
flowControl_t  flowControl;
baudrate_t     baudrate;
dataLength_t   dataLength;
parity_t       parity;
stopbits_t     stopbits;
uint8_t        *rxBuff;
uint8_t        rxBuffSize;
void (*rxCallback)(uint8_t *);
void (*txCallback)(void);
}UsartDescriptor_t;

extern void UartDriverTimeOutCnt(void);
extern int8_t OpenUsart(UsartDescriptor_t *descriptor);
extern void WriteUsart(UsartDescriptor_t *descriptor,uint8_t *p,uint16_t len);
extern void IntRXUART0(void);
#endif

