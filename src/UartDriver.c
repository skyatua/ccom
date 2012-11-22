#include "../inc/includes.h"

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
TX_FREE       =0,  
TX_COMPLETE   =1
}TxState_t;

struct UART_SPEED
{
uint8_t BR1;
uint8_t BR0;
uint8_t MCTL;
};

const  struct UART_SPEED uart_speed[] = 
{
//   Speed,      BR1,  BR0, MCTL
//  x149 @ 4.9152MHz 
///*   9600*/  {  0x01, 0x00, 0x00},
///*  19200*/  {  0x00, 0x80, 0x00},
///*  38400*/  {  0x00, 0x40, 0x00},
///*  57600*/  {  0x00, 0x2A, 0x6D},
///* 115200*/  {  0x00, 0x15, 0x24},
// x2418 @ 8MHz 
/*   9600*/  {  0x03, 0x41, 0x00},
/*  19200*/  {  0x01, 0xA0, 0x00},
/*  38400*/  {  0x00, 0xD0, 0x00},
/*  57600*/  {  0x00, 0x8A, 0x00},
/* 115200*/  {  0x00, 0x45, 0x00},
};

typedef enum
{
Free,
Busy
}status_t;

typedef struct
{
uint8_t  *pTxBuff;
uint8_t  *pRxBuff;
uint16_t  TxLen;
uint8_t  RxLen;
status_t status;
mode_tt   mode;
TxState_t TxState;
uint8_t   TimeOut;
int32_t   hSerial;
void(*RxCallback)(uint8_t *);
void(*TxCallback)(void);
}usart_t;

static usart_t usart0;
static usart_t usart1;

int8_t OpenUsart(UsartDescriptor_t *descriptor)
{
#ifdef __MSPX__
uint8_t uart_speed_index = (uint8_t)descriptor->baudrate;

  
if(USART_CHANNEL_0==descriptor->channel)
  {
  usart0.mode=descriptor->mode;
  usart0.pRxBuff=descriptor->rxBuff; 
  usart0.RxLen=descriptor->rxBuffSize;
  usart0.TxCallback=descriptor->txCallback;  
  usart0.RxCallback=descriptor->rxCallback;   
    
  UCA0CTL1 |= UCSWRST;
  UCA0CTL1 &= ~UCSWRST;
  //UCA0CTL1 = CHAR;
  UCA0BR1 = uart_speed[uart_speed_index].BR1;
  UCA0BR0 = uart_speed[uart_speed_index].BR0;
  //UCA0MCTL = uart_speed[uart_speed_index].MCTL;
  UCA0CTL1 |= UCSSEL1;
  
  UCA0MCTL = UCBRS2 + UCBRS0;
  
  IE2 |= UCA0TXIE+UCA0RXIE;
  P3SEL |= BIT4+BIT5;
  P3DIR |= BIT4;
  } 

if(USART_CHANNEL_1==descriptor->channel)
  {
  usart1.mode=descriptor->mode;
  usart1.pRxBuff=descriptor->rxBuff; 
  usart1.RxLen=descriptor->rxBuffSize;
  usart1.TxCallback=descriptor->txCallback;  
  usart1.RxCallback=descriptor->rxCallback;   

  UCA1CTL1 |= UCSWRST;
  UCA1CTL1 &= ~UCSWRST;
  //UCA1CTL1 = CHAR;
  UCA1BR1 = uart_speed[uart_speed_index].BR1;
  UCA1BR0 = uart_speed[uart_speed_index].BR0;
  //UCA1MCTL = uart_speed[uart_speed_index].MCTL;
  UCA1MCTL = UCBRS2 + UCBRS0;
  
  UCA1CTL1 |= UCSSEL1;
  UC1IE |= UCA1TXIE+UCA1RXIE; 
  P3SEL |= BIT6+BIT7;
  P3DIR |= BIT6; 
  }
#endif

#ifdef __WINX__
if(USART_CHANNEL_0 == descriptor->channel)
  {
  usart0.mode=descriptor->mode;
  usart0.pRxBuff=descriptor->rxBuff; 
  usart0.RxLen=descriptor->rxBuffSize;
  usart0.TxCallback=descriptor->txCallback;  
  usart0.RxCallback=descriptor->rxCallback;   
  
  usart0.hSerial = -1;
  
  int hSerial = (int) CreateFile
	(	"COM8",
	    GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		0,
		0);
	if (hSerial == -1)
	{  // can't open port
		return -1;
		}
		
	DCB dcbSP = {0};
	dcbSP.DCBlength = sizeof(dcbSP);
	if (!GetCommState((HANDLE)hSerial,&dcbSP))
	{  // could not get the state of the port
		return -1;
		}
		
	dcbSP.BaudRate = (uint32_t)descriptor->baudrate;
	dcbSP.ByteSize = descriptor->dataLength;
	dcbSP.StopBits = descriptor->stopbits;
	dcbSP.Parity   = descriptor->parity;
	
	if (!SetCommState((HANDLE)hSerial,&dcbSP))
	{  // analyse error
		return -1;
		}
  
	COMMTIMEOUTS timeouts={0};
	timeouts.ReadIntervalTimeout=50;
	timeouts.ReadTotalTimeoutConstant=50;
	timeouts.ReadTotalTimeoutMultiplier=10;
	timeouts.WriteTotalTimeoutConstant=50;
	timeouts.WriteTotalTimeoutMultiplier=10;
	
	if(!SetCommTimeouts((HANDLE)hSerial, &timeouts))
	{
       return -1;
     }	
	 
    usart0.hSerial = hSerial;
  }
#endif

#ifdef __LINX__
if (USART_CHANNEL_0 == descriptor->channel)
{
  usart0.mode=descriptor->mode;
  usart0.pRxBuff=descriptor->rxBuff; 
  usart0.RxLen=descriptor->rxBuffSize;
  usart0.TxCallback=descriptor->txCallback;  
  usart0.RxCallback=descriptor->rxCallback;   
  
  usart6.hSerial = -1;
  
  int hSerial = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
  if (hSerial == -1)
  {
	  return -1;
  	  } 
	//CONFIGURE THE UART
	//The flags (defined in termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
		    	  
  struct termios options;
  tcgetattr(hSerial, &options);
  cfsetispeed(&options, (uint32_t)descriptor->baudrate );
  cfsetospeed(&options, (uint32_t)descriptor->baudrate );     
  options.c_cflag = (uint32_t)descriptor->baudrate | 
                     descriptor->dataLength | 
					 CLOCAL | 
					 CREAD; 
  options.c_iflag = IGNPAR |
                     ICRNL;  
  options.c_oflag = 0;
  tcflush(hSerial, TCIFLUSH);
  tcsetattr(hSerial, TCSANOW, &options);
  
  usart0.hSerial = hSerial;
  }
#endif
 return 0;
 }
 
#ifdef __MSPX__
#pragma vector = USCIAB1RX_VECTOR
__interrupt void IntRXUART1(void)//Прием из UART1
{  
static uint8_t i=0;
static uint8_t *p;
uint8_t b;  

UC1IFG &= ~UCA1RXIFG;

b=UCA1RXBUF;

if((Free==usart1.status)||(RS232_Mode==usart1.mode))//Если UART в данный момент ничего не передает или UART в режиме RS232
  {  
  if(!i++)
    p=usart1.pRxBuff;
    *p++=b;
  (*usart1.RxCallback)(&i);
  if(i==usart1.RxLen)
    i=0;
  }  
}
#endif

#ifdef __MSPX__
#pragma vector = USCIAB1TX_VECTOR
__interrupt void IntTXUART1(void)//Передача по UART1
{
  UC1IFG &= ~UCA1TXIFG;
  
if(!usart1.TxLen)
  {
  for(uint16_t i=0;i!=1500;i++)asm("nop");
  usart1.status=Free;
  (*usart1.TxCallback)();
  } 
if(usart1.TxLen)
  {
  UCA1TXBUF = *usart1.pTxBuff++;
  usart1.TxLen--;    
  }    
}
#endif

#ifdef __MSPX__
#pragma vector = USCIAB0RX_VECTOR
__interrupt void IntRXUART0(void)//Прием из UART0
{  
static uint8_t i=0;
static uint8_t *p;
uint8_t b;  
IFG2 &= ~UCA0RXIFG;

b=UCA0RXBUF;

if((Free==usart0.status)||(RS232_Mode==usart0.mode))//Если UART в данный момент ничего не передает или UART в режиме RS232
  {  
  if(!i++)
    p=usart0.pRxBuff;
    *p++=b;
  (*usart0.RxCallback)(&i);
  if(i==usart0.RxLen)
    i=0;
  }  
}
#endif

#ifdef __WINX__
void IntRXUART0(void) //Прием из UART0
{  
static uint8_t i=0;
static uint8_t *p;
uint8_t b;
uint32_t cnt = 0;

if (usart0.hSerial != -1)
{
	// read data
	do{
	ReadFile((HANDLE)usart0.hSerial, &b, 1, (PDWORD)&cnt, NULL );
		
	if ( cnt )
	{
	    //////////////	
	    if (i == 0) printf("\n<- ");  
		else  
	      if ((i%16) == 0) printf("\n");
	    printf("%02X ",b);
	    //////////////	 
		
		if((Free==usart0.status)||(RS232_Mode==usart0.mode))//Если UART в данный момент ничего не передает или UART в режиме RS232
		{  
			if(!i++)
				p=usart0.pRxBuff;
			*p++=b;
			(*usart0.RxCallback)(&i);
			if(i==usart0.RxLen)
				i = 0;
		} 
	 } 
	}while(cnt);
 }
}
#endif

#ifdef __LINX__
void IntRXUART0(void) //Прием из UART0
{  
static uint8_t i=0;
static uint8_t *p;
uint8_t b;
uint32_t cnt = 0;

if (usart0.hSerial != -1)
{
	// read data
	do{
	cnt = read(usart0.hSerial, &b, 1);
		
	if ( cnt )
	{
	    //////////////	
	    if (i == 0) printf("\n<- ");  
		else  
	      if ((i%16) == 0) printf("\n");
	    printf("%02X ",b);
	    //////////////	 
		
		if((Free==usart0.status)||(RS232_Mode==usart0.mode))//Если UART в данный момент ничего не передает или UART в режиме RS232
		{  
			if(!i++)
				p=usart0.pRxBuff;
			*p++=b;
			(*usart0.RxCallback)(&i);
			if(i==usart0.RxLen)
				i = 0;
		} 
	 } 
	}while(cnt);
 }
}
#endif
#ifdef __MSPX__
#pragma optimize=none
#pragma vector = USCIAB0TX_VECTOR
__interrupt void IntTXUART0(void)//Передача по UART0
{
  IFG2 &= ~UCA0TXIFG;
  
if(!usart0.TxLen)
  {
    
  //usart0.TimeOut=1;  
  //usart0.TxState = TX_COMPLETE;  
  uint32_t inc=0;
  while(/*(!(UCA0CTL1&UCTXEPT))&&*/(inc<200))//Ждем отправки последнего байта
  {
  inc++;
  } 
    asm("nop");  
 
  (*usart0.TxCallback)();
  usart0.status=Free;
  } 
if(usart0.TxLen)
  {
  UCA0TXBUF = *usart0.pTxBuff++;
  usart0.TxLen--;    
  }    
}
#endif

#ifdef __WINX__
void IntTXUART0(void) //Передача по UART0
{
 uint32_t cnt;	
//  IFG2 &= ~UCA0TXIFG;

 if ( usart0.hSerial != -1 ) 
  {
	// write data  
	if ( usart0.TxLen )
	 {
  	  //////	
      printf("\n-> ");  
	  uint8_t aa = 0;
	  for (aa = 0; aa < usart0.TxLen; aa++)
        {  if ((aa%16)== 0 ) printf("\n");
			printf("%02X ", ((uint8_t*)usart0.pTxBuff)[aa] ); }
	  ///////
	  if( WriteFile((HANDLE)usart0.hSerial, usart0.pTxBuff, usart0.TxLen,(PDWORD)&cnt, 0))	
	  {
		 usart0.TxLen -= cnt;   
  	    
         if (!usart0.TxLen)
	      { 
		   (*usart0.TxCallback)(); 
		   usart0.status=Free; 
	       }
	    }
      }
  }
}
#endif

#ifdef __LINX__
void IntTXUART0(void) //Передача по UART0
{
 uint32_t cnt;	
//  IFG2 &= ~UCA0TXIFG;

 if ( usart0.hSerial != -1 ) 
  {
	// write data  
	if ( usart0.TxLen )
	 {
  	  //////	
      printf("\n-> ");  
	  uint8_t aa = 0;
	  for (aa = 0; aa < usart0.TxLen; aa++)
        {  if ((aa%16)== 0 ) printf("\n");
			printf("%02X ", ((uint8_t*)usart0.pTxBuff)[aa] ); }
	  ///////
	  if((cnt = write( usart0.hSerial, usart0.pTxBuff, usart0.TxLen)) > 0)	
	  {
		 usart0.TxLen -= cnt;   
  	    
         if (!usart0.TxLen)
	      { 
		   (*usart0.TxCallback)(); 
		   usart0.status=Free; 
	       }
	    }
      }
  }
}
#endif
void WriteUsart(UsartDescriptor_t *descriptor,uint8_t *p,uint16_t len)
{
#ifdef __MSPX__	
if(USART_CHANNEL_0==descriptor->channel)
  {
  if(Free==usart0.status)//Если UART в данный момент ничего не передает  
    {
    usart0.pTxBuff=p;
    if(RS232_Mode == usart0.mode)
      usart0.TxLen=len-1;
    else
      usart0.TxLen=len;
    usart0.status=Busy;
    IFG2 |= UCA0TXIFG;
    }
  }

if(USART_CHANNEL_1==descriptor->channel)
  {
  if(Free==usart1.status)//Если UART в данный момент ничего не передает  
    {
    usart1.pTxBuff=p;
    if(RS232_Mode == usart1.mode)
      usart1.TxLen=len-1;
    else
      usart1.TxLen=len;
    usart1.status=Busy;
      UC1IFG |= UCA1TXIFG;
    }
  }
#endif  

#ifdef __WINX__
if(USART_CHANNEL_0==descriptor->channel)
  {
  if(Free==usart0.status)//Если UART в данный момент ничего не передает  
    {
    usart0.pTxBuff=p;
    if(RS232_Mode == usart0.mode)
      usart0.TxLen=len-1;
    else
      usart0.TxLen=len;
    usart0.status=Busy;
    IntTXUART0();
    }
  }

if(USART_CHANNEL_1==descriptor->channel)
  {
  if(Free==usart1.status)//Если UART в данный момент ничего не передает  
    {
    usart1.pTxBuff=p;
    if(RS232_Mode == usart1.mode)
      usart1.TxLen=len-1;
    else
      usart1.TxLen=len;
    usart1.status=Busy;
	
    //  UC1IFG |= UCA1TXIFG;
    }
  }
#endif

#ifdef __LINX__
if(USART_CHANNEL_0==descriptor->channel)
  {
  if(Free==usart0.status)//Если UART в данный момент ничего не передает  
    {
    usart0.pTxBuff=p;
    if(RS232_Mode == usart0.mode)
      usart0.TxLen=len-1;
    else
      usart0.TxLen=len;
    usart0.status=Busy;
    IntTXUART0();
    }
  }

if(USART_CHANNEL_1==descriptor->channel)
  {
  if(Free==usart1.status)//Если UART в данный момент ничего не передает  
    {
    usart1.pTxBuff=p;
    if(RS232_Mode == usart1.mode)
      usart1.TxLen=len-1;
    else
      usart1.TxLen=len;
    usart1.status=Busy;
	
    //  UC1IFG |= UCA1TXIFG;
    }
  }
#endif
}


void UartDriverTimeOutCnt(void)
{
if(TX_COMPLETE==usart0.TxState) 
  {
  if(0==usart0.TimeOut)  
    {
    usart0.TxState=TX_FREE;
    (*usart0.TxCallback)();
    usart0.status=Free;
    }
  else
    usart0.TimeOut--;
  }

if(TX_COMPLETE==usart1.TxState) 
  {
  if(0==usart1.TimeOut)  
    {
    usart1.TxState=TX_FREE;
    (*usart1.TxCallback)();
    usart1.status=Free;
    }
  else
    usart1.TimeOut--;
  }
}
