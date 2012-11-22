#include "../inc/includes.h"

void rSetdo_States(uint16_t*, uint8_t );
void rSetOut_Val(uint16_t*, uint8_t );
void rSetService(uint8_t*, uint8_t );
void rSetdi_Polarity(uint16_t*, uint8_t );
void rSetdo_Polarity(uint16_t*, uint8_t );
void rSetOut_Clb(uint16_t*, uint8_t );
void rSetDo(uint8_t*, uint8_t );
void rSetSensTypeUN(uint8_t*, uint8_t );
void rSetInclinUN(float*, uint8_t );
void rSetOffsetUN(float*, uint8_t );
void rSetInclinXC(float*, uint8_t );
void rSetOffsetXC(float*, uint8_t );
void rSetInL(int16_t*, uint8_t );
void rSetInH(int16_t*, uint8_t );
void rSetLoiUN(int16_t*, uint8_t );
void rSetHigUN(int16_t*, uint8_t );
void rSetOutType(uint8_t*, uint8_t );
void rSetOutInclin(float*, uint8_t );
void rSetOutOffset(float*, uint8_t );
void rSetOutLoSc(float*, uint8_t );
void rSetOutHiSc(float*, uint8_t );
void rSetEvents(uint32_t*, uint8_t );
void rSetSmoothUN(uint8_t*, uint8_t );
void rSetdi_Smooth(uint16_t*, uint8_t );
void rSetlDiMap(uint8_t*, uint8_t );
void rSetlDoMap(uint8_t*, uint8_t );
void rSetlAiMap(uint8_t*, uint8_t );
void rSetlAoMap(uint8_t*, uint8_t );
///////////////////////////////////////////
void rSetDamperOpenCmd(uint8_t*, uint8_t );
void rSetDamperCloseCmd(uint8_t*, uint8_t );
void rSetHumPompCmd(uint8_t*, uint8_t );
void rSetChillerHeatCmd(uint8_t*, uint8_t );
void rSetChillerVentCmd(uint8_t*, uint8_t );
void rSetRhi(float*, uint8_t );
void rSetManualMode(uint8_t*, uint8_t );
void rSetDoLValue(uint8_t*, uint8_t );
void rSetAoLValue(float*, uint8_t );

/*
05.04.12 sky
Âíèìàíèå! Îáíàğóæåí áàã êîìïèëàòîğà. 
Ïğîÿâëÿåòñÿ, êîãäà ıëåìåíòîâ ñòğóêòóğû RAS ğàâíî 33
*/

float T[4] = {10.1, 11.2, 12.3, 13.4 };
const tRAS_t RAS[] = 
{
// var                 type  cnt        funct             name             info
/* 0*/ {&T,             6,   4,            0                         , "Temperature"},
};
uint8_t RAS_Count = (sizeof(RAS)/sizeof(RAS[0]));
/*
void rSetdi_Smooth(uint16_t *data, uint8_t idx)
{
 if ((*data >= 1) &&
      (*data <= 2000))
 {
  di_Smooth[idx] = *data;
  e_Write(&di_Smooth[idx],2,di_Smooth_Addr+(idx<<0x01)); 
 } 
 
 asm("nop");
}

void rSetSmoothUN(uint8_t *data, uint8_t idx)
{
 if ((*data >= 1) &&
      (*data <= 25))
 {
  SmoothUN = *data;
  e_Write(&SmoothUN,1,SmoothUN_Addr ); 
 } 
 
 asm("nop");
}

void rSetEvents(uint32_t *data, uint8_t idx)
{
 Events = *data; 
 
 asm("nop");
}

void rSetService(uint8_t *data, uint8_t idx)
{
  if (*data <= 1)
  {
    Service = *data;
  }
  asm("nop");
}

void rSetOutType(uint8_t * data, uint8_t idx)
{
  if ((*data  == 0) || 
         ((*data >= 142)&&(*data <= 143)) ||  // ÀÌ
          ((*data >= 150)&&(*data <= 152)))   // ÀÂ
    {
      if (*data != OutType[idx])
      {
         OutType[idx] = *data;
         e_Write(&OutType[idx], 1, OutType_Addr + idx );
       }
    }
  asm("nop");
}

void rSetSensTypeUN(uint8_t * data, uint8_t idx)
{
    if ((*data == 0) ||
         ((*data > 0)&&(*data <=13)) ||            // ÒÑ
          ((*data >= 20)&&(*data <= 27)) ||        // ÒÏ
           ((*data >= 41)&&(*data <= 43)) ||       // ÀÍ
            ((*data >= 50)&&(*data <= 52)))        // ÀÒ
    {
      if (*data != SensTypeUN[idx])
      {  
         SensTypeUN[idx] = *data;
         e_Write(&SensTypeUN[idx], 1, SensTypeUN_Addr + idx );
         
         CreateInquiries();
       }
    }
  asm("nop");
}
void rSetInL(int16_t * data, uint8_t idx)
{
  if ((*data >= -999) &&
        (*data <= 9999))
  {
   InL[idx] = *data;
   e_Write(&InL[idx],2,InL_Addr+(idx<<0x01));
  }
  asm("nop");
}

void rSetLoiUN(int16_t * data, uint8_t idx)
{
  if ((*data >= -999) &&
        (*data <= 9999))
  {
     LoiUN[idx] = *data;
     e_Write(&LoiUN[idx],2,LoiUN_Addr+(idx<<0x01));
  }
  asm("nop");
}

void rSetHigUN(int16_t * data, uint8_t idx)
{
  if ((*data >= -999) &&
        (*data <= 9999))
  {
     HigUN[idx] = *data;
     e_Write(&HigUN[idx],2,HigUN_Addr+(idx<<0x01));
  }
  asm("nop");
}

void rSetInH(int16_t * data, uint8_t idx)
{
  if ((*data >= -999) &&
        (*data <= 9999))
  {
     InH[idx] = *data;
     e_Write(&InH[idx],2,InH_Addr+(idx<<0x01));
  }
  asm("nop");
}
void rSetOutInclin(float * data, uint8_t idx)
{
  if ((*data >= 0.8) &&
        (*data <= 1.2))
  {
     OutInclin[idx] = *data;
     e_Write(&OutInclin[idx],4,OutInclin_Addr+(idx<<0x02));
  }
  asm("nop");
}

void rSetOutLoSc(float * data, uint8_t idx)
{
  if ((*data >= -999) &&
        (*data <= 9999))
  {
     OutLoSc[idx] = *data;
     e_Write(&OutLoSc[idx],4,OutLoSc_Addr+(idx<<0x02));
  }
  asm("nop");
}
void rSetOutHiSc(float * data, uint8_t idx)
{
  if ((*data >= -999) &&
        (*data <= 9999))
  {
     OutHiSc[idx] = *data;
     e_Write(&OutHiSc[idx],4,OutHiSc_Addr+(idx<<0x02));
  }
  asm("nop");
}

void rSetOutOffset(float * data, uint8_t idx)
{
  if ((*data >= -9.9) &&
        (*data <= 9.9))
  {
     OutOffset[idx] = *data;
     e_Write(&OutOffset[idx],4,OutOffset_Addr+(idx<<0x02));
  }
  asm("nop");
}

void rSetInclinXC(float * data, uint8_t idx)
{
  if ((*data >= 0.8) &&
        (*data <= 1.2))
  {
     InclinXC[idx] = *data;
     e_Write(&InclinXC[idx],4,InclinXC_Addr+(idx<<0x02));
  }
  asm("nop");
}

void rSetOffsetXC(float * data, uint8_t idx)
{
  if ((*data >= -9.9) &&
        (*data <= 9.9))
  {
     OffsetXC[idx] = *data;
     e_Write(&OffsetXC[idx],4,OffsetXC_Addr+(idx<<0x02));
  }
  asm("nop");
}

void rSetInclinUN(float * data, uint8_t idx)
{
  if ((*data >= 0.8) &&
        (*data <= 1.2))
  {
     InclinUN[idx] = *data;
     e_Write(&InclinUN[idx],4,InclinUN_Addr+(idx<<0x02));
  }
  asm("nop");
}

void rSetOffsetUN(float * data, uint8_t idx)
{
  if ((*data >= -9.9f) && 
        ( *data <= 9.9f))
  {
     OffsetUN[idx] = *data;
     e_Write(&OffsetUN[idx],4,OffsetUN_Addr+(idx<<0x02));
  }
  asm("nop");
}
void rSetDo(uint8_t * data, uint8_t idx)
{
  if (*data <= 1)
  {
     Do[idx] = *data;
  }
  asm("nop");
}

void rSetdo_States(uint16_t * data, uint8_t idx)
{
  if (*data <= 0x00ff)
  {
     do_States = *data;
  }
  asm("nop");
}

void rSetOut_Val(uint16_t * data, uint8_t idx)
{
  if (*data <= 4096)
  {
     Out_Val[idx] = *data;
  }
  asm("nop");
}

void rSetdi_Polarity(uint16_t * data, uint8_t idx)
{
  if (*data != di_Polarity )
  {
   di_Polarity = (*data & 0x00ff);
   e_Write(&di_Polarity,2,di_Polarity_Addr);
  }
  asm("nop");
}

void rSetdo_Polarity(uint16_t * data, uint8_t idx)
{
  if (*data != do_Polarity)
  {
     do_Polarity = (*data & 0x00ff);
     e_Write(&do_Polarity,2,do_Polarity_Addr);
  }
  asm("nop");
}

void rSetOut_Clb(uint16_t * data, uint8_t idx)
{
  if (*data <= 4096)
  {  
     uint8_t ch = (idx>>0x01),
             range = (idx%2);
     if (ch > 3) ch = 3;
     if (range > 1) range = 1;
     
     Out_clb[ch][range] = *data;  
  }
  asm("nop");
}
void rSetlDiMap(uint8_t * data, uint8_t idx)
{
  if ((*data&0x0f) < DI_MaxCount)
  {
   DiMap[idx].num = *data;  
   e_Write(&DiMap[idx],1, DiMap_Addr + idx);
  }
  asm("nop");
}

void rSetlDoMap(uint8_t * data, uint8_t idx)
{
  if ((*data&0x0f) < DO_MaxCount)
  {
   DoMap[idx].num = *data;  
   e_Write(&DoMap[idx],1, DoMap_Addr + idx);
  }
  asm("nop");
}

void rSetlAiMap(uint8_t * data, uint8_t idx)
{
  if ((*data&0x0f) < 12)
  {
   AiMap[idx].num = *data;  
   e_Write(&AiMap[idx],1, AiMap_Addr + idx);
  }
  asm("nop");
}

void rSetlAoMap(uint8_t * data, uint8_t idx)
{
  if ((*data&0x0f) < 0x04)
  {
   AoMap[idx].num = *data;  
   e_Write(&AoMap[idx],1, AoMap_Addr + idx);
  }
  asm("nop");
}

void rSetDamperOpenCmd(uint8_t * data, uint8_t idx)
{
  if (*data <= 1)
  {
   DamperOpenCmd = *data;
  }
  asm("nop");
}

void rSetDamperCloseCmd(uint8_t * data, uint8_t idx)
{
  if (*data <= 1)
  {
   DamperCloseCmd = *data;
  }
  asm("nop");
}

void rSetHumPompCmd(uint8_t * data, uint8_t idx)
{
  if (*data <= 1)
  {
   HumPompCmd = *data;
  }
  asm("nop");
}

void rSetChillerHeatCmd(uint8_t * data, uint8_t idx)
{
  if (*data <= 1)
  {
   ChillerHeatCmd = *data;
  }
  asm("nop");
}

void rSetChillerVentCmd(uint8_t * data, uint8_t idx)
{
  if (*data <= 1)
  {
   ChillerVentCmd = *data;
  }
  asm("nop");
}

void rSetRhi(float * data, uint8_t idx)
{
  if ((*data >= 10.0f) && (*data <= 95.0f))
  {
   Rhi = *data;
  }
  asm("nop");
}

void rSetManualMode(uint8_t * data, uint8_t idx)
{
  if (*data <= 1)
  {
   ManualMode = *data;
  }
  asm("nop");
}

void rSetDoLValue(uint8_t * data, uint8_t idx)
{
  if (*data <= 1)
  {
   DoLValue[idx] = *data;
  }
  asm("nop");
}

void rSetAoLValue(float * data, uint8_t idx)
{
  if ((*data >= 0.0f)&&(*data <= 100.0f))
  {
   AoLValue[idx] = *data;
  }
  asm("nop");
}
*/