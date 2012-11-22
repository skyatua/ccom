
#include "../inc/includes.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    StartUartManager();
 	printf("Init module - OK\n");
	
	while(1)
	{
       
	   TBusTimeOutCounter();
       UartDriverTimeOutCnt();
	   
	  IntRXUART0();	
	  
      TBusRun(); 
		}
	
	return 0;
}
