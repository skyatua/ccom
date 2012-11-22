#ifndef _connect_h
#define _connect_h

//#include "connect.h"
//#include "variables.h"

//#define RAS_Count 5
typedef	void (*func_t)(void * param, uint8_t index);

//#pragma pack (push,1)
typedef struct tRAS 
{
  void * Addr;
  uint8_t vType;
  uint8_t vCount;
  func_t func;
  //uint8_t vName[16];
  uint8_t vInfo[18];
}tRAS_t;
//#pragma pack (pop)

extern uint8_t RAS_Count;
extern const tRAS_t RAS[];

#endif

