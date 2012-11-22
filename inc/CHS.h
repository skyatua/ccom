#ifndef _chs_h
#define _chs_h

//#include "../inc/includes.h"

typedef enum
{
calcCHS,
resetCHS
}CHS_cmd_t;

void CHS_16(uint16_t *crc,uint8_t byte,CHS_cmd_t cmd);
uint16_t CALC_CHS_16(uint8_t * pData, uint16_t len);

#endif
