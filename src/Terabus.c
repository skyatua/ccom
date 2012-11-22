/*
19.01.12 sky изменена комманда получения геометрии архива
  - вместо RecCount отдаем &arch_struct       
  - вместо CtrlTime отдаем &arch_points   
*/
#include "../inc/includes.h"

//#include "Terabus.h"
//#include "CHS.h" 
//#include "connect.h"
//#include "UartManager.h"

//#include "restore.h"
//#include "Archive.h"
//#include "extflash.h"
//#include "eeprom.h"
//#include "drivers/inteeprom.h"

#define TxBuffSize 128 //256 // Размер передающего буфера (должен быть не меньше 128 Бт)
#define RxBuffSize 128 //128 // Размер приемного буфера

const uint8_t DevName[20]=TeraDeviceName;                   //Имя прибора
const uint8_t ProgrammVersion[10] = DeviceProgrammVersion;  //Версия программы прибора
uint8_t ProtocolVersion     = 0x04;                         //Версия протокола 1 - для 4-х байтной адресации
uint8_t NetworkDeviceAddr   = DeviceNetworkAddr;            //Сетевой адрес прибора
static uint16_t RxFirst=0;
static uint16_t RxLast=0;

static uint16_t TxTimeDelay;  //!!!

#ifdef __MSPX__
#pragma data_alignment = 2
#endif
static uint8_t TxBuff[TxBuffSize];//Передающий буфер

#ifdef __MSPX__
#pragma data_alignment = 2
#endif
static uint8_t RxBuff[RxBuffSize];//Приемный буфер

#ifdef __MSPX__
#pragma data_alignment = 2
#endif

static Tree_t Tree;//Дерево синтаксического разбора (содержит в себе результат синтаксического анализа)

static volatile uint32_t Time;//Счетчик таймаута

static uint8_t SizeOfType(uint8_t type)//Подпрограмма вычисения размера переменной по ее типу
{
switch(type)
  {
  case 0:
  case 1:
    return 1;
  case 2:
  case 3:
    return 2;
  case 4:
  case 5:
  case 6:
    return 4;
  case 8:
    return 6;
  default:
    return 0;
  }
}


void TBusDataReceiver(uint8_t byte)//Приемник байт входного потока данных
{
RxBuff[RxLast]=byte;
RxLast++;
if(RxLast>=(sizeof(RxBuff)-1))
  RxLast=0;
}


static ParcerState_t TbState=SIGNATURE_ANALISE_STATE;//Состояни парсера

//Анализатор входного потока байт
void TBusFlowAnaliser(uint8_t byte)
{
static uint16_t crc16;//Контрольная сумма (вычисляется)
static uint8_t  inc;  //инкремент общего назначения

static uint8_t  *pReceiveCHS;//Указатель для приема контрольной суммы
static uint8_t  *pReceive;   //Указатель общего назначения для приема

Time=0;

if(TbState!=CHS_ANALISE_STATE)
  CHS_16(&crc16,byte,calcCHS);

switch(TbState)
  {
  case SIGNATURE_ANALISE_STATE:
  if(byte==0x50)
    {
    Tree.IdxRAS=0x0000;
    Tree.IdxMAS=0x0000;
    Tree.CntRAS=0x0000;
    pReceiveCHS=(uint8_t *)&Tree.chs;
    pReceiveCHS++;
    Tree.error=errNone;
    Tree.DataLen=0;
    CHS_16(&crc16,0,resetCHS);
    TbState=DEV_ADDR_ANALISE_STATE;
    }
  break;

  case DEV_ADDR_ANALISE_STATE:
     if((NetworkDeviceAddr==byte)||(0xFF==byte))
      {
      Tree.DevAddr=byte;
      TbState=COMMAND_ANALISE_STATE;
      }
    else
      TbState=SIGNATURE_ANALISE_STATE;
  break;

  case NEW_DEV_ADDR_RECEIVE_STATE:
  Tree.NewDevAddr=byte;
  TbState=CHS_ANALISE_STATE;
  inc=0;
  break;

  case COMMAND_ANALISE_STATE:
  Tree.Command=(TeraBusCmd_t )byte;//Записать принятую комманду в буфферное дерево разбора

  switch(Tree.Command)
      {
      case cmdReadRAM:
      case cmdWriteRAM:
      case cmdReadFlash:
      case cmdWriteFlash:
      case cmdReadEEPROM:
      case cmdWriteEEPROM:

      TbState=VAR_ADDR_RECEIVE_STATE;
      pReceive=(uint8_t *)&Tree.VarAddr;
      inc=0;
      break;

      case cmdId:
      case cmdArchParams:
      TbState=CHS_ANALISE_STATE;
      inc=0;
      break;

      case cmdSetAddr:
      TbState=NEW_DEV_ADDR_RECEIVE_STATE;
      break;

      case cmdArchData:
      TbState=ARCH_REC_NO_RECEIVE_STATE;
      pReceive=(uint8_t *)&Tree.ArchiveRecNo;
      inc=0;
      break;

      case cmdGetVarList:
      case cmdReadRAM_EXT:
      case cmdWriteRAM_EXT:
      TbState=IDX_RAS_RECEIVE_STATE;
      pReceive=(uint8_t *)&Tree.IdxRAS;
      inc=0;
      break;

      case cmdDataTransfer:
      TbState=RESERVED_RECEIVE_STATE;
      break;  
      
      default:
      TbState=SIGNATURE_ANALISE_STATE;
      Tree.error=errUnknownCmd;
      Tree.Signature=0x50;
      break;
      };

  break;

  case IDX_RAS_RECEIVE_STATE:
    *pReceive++=byte;
     if(inc++>=1)
      {
      if(cmdWriteRAM_EXT==Tree.Command)
        {
        TbState=IDX_MAS_RECEIVE_STATE;
        pReceive=(uint8_t *)&Tree.IdxMAS;
        }
      else
        {
        TbState=CNT_RAS_RECEIVE_STATE;
        pReceive=(uint8_t *)&Tree.CntRAS;
        }
      inc=0;
      }
  break;

  case CNT_RAS_RECEIVE_STATE:
     *pReceive++=byte;
     if(inc++>=1)
      {
      TbState=CHS_ANALISE_STATE;
      inc=0;
      }
   //Tree.CntRAS=byte;
   //TbState=CHS_ANALISE_STATE;
   //inc=0;
  break;

  case IDX_MAS_RECEIVE_STATE:
   *pReceive++=byte;
     if(inc++>=1)
      {
      TbState=VAR_CNT_RECEIVE_STATE;
      inc=0;
      }
  break;


  case VAR_ADDR_RECEIVE_STATE:
  *pReceive++=byte;

  if(inc++>=3)
    {
    TbState=VAR_CNT_RECEIVE_STATE;
    }
  break;

  case VAR_CNT_RECEIVE_STATE:
  Tree.VarCnt=byte;
  TbState=VAR_TYPE_ANALISE_STATE;
  break;

  case VAR_TYPE_ANALISE_STATE:
  Tree.VarType=(VarType_t)byte;


  switch(Tree.Command)
    {
    case cmdReadRAM:
    case cmdReadEEPROM:
    case cmdReadFlash:
      TbState=CHS_ANALISE_STATE;
    break;

    case cmdWriteRAM:
    case cmdWriteRAM_EXT:
      TbState=DATA_RECEIVE_STATE;
    break;

    default: break;
    }
  inc=0;

  Tree.VarSize=SizeOfType(Tree.VarType);
  Tree.DataLen=Tree.VarCnt*Tree.VarSize;
  if(0==Tree.DataLen)
    {
    TbState=SIGNATURE_ANALISE_STATE;
    Tree.error=errInvalidType;
    Tree.Signature=0x50;
    }
  break;

  case DATA_RECEIVE_STATE:
  Tree.data[inc]=byte;
  if(inc++>=Tree.DataLen-1)
    {
    inc=0;
    TbState=CHS_ANALISE_STATE;
    }
  break;

  case ARCH_REC_NO_RECEIVE_STATE:
  *pReceive++=byte;

  if(inc++>=3)
    {
    TbState=ARCH_REC_CNT_RECEIVE_STATE;
    pReceive=(uint8_t *)&Tree.ArchiveRecCnt;
    inc=0;
    }
  break;

  case ARCH_REC_CNT_RECEIVE_STATE:
  *pReceive++=byte;

  if(inc++>=3)
    {
    TbState=CHS_ANALISE_STATE;
    inc=0;
    }
  break;

 
  case RESERVED_RECEIVE_STATE:
  Tree.Reserved = byte;  
  TbState       = STATE_RECEIVE_STATE;
  break;  
  
  case STATE_RECEIVE_STATE:
  Tree.State  = (DTState_t)byte;  
  TbState     = FRAG_CNT_IDX_RECEIVE_STATE;  
  break;  
 
  case FRAG_CNT_IDX_RECEIVE_STATE:
  Tree.FragCnt  = byte>>4;
  Tree.FragIdx  = 0x0F&byte;
  TbState       = PROT_TYPE_RECEIVE_STATE;
  break;  
 
    
  case PROT_TYPE_RECEIVE_STATE:
  Tree.ProtType=(ProtType_t)byte;  
  TbState = DATA_CNT_RECEIVE_STATE;
  break;  
  
  
  case DATA_CNT_RECEIVE_STATE:
  Tree.DataLen=byte;
  TbState=DATA_RECEIVE_STATE;
  inc=0;    
  break;  
  
  case CHS_ANALISE_STATE:

  *pReceiveCHS--=byte;
  if(inc++>=1)
    {
      
    //////////////////////////////////////
   // TxTimeDelay = 1; // x20 ~ 40 ms
   // for (volatile uint32_t i =0; i< 0x00007fff; i++ )
   //   {
   //   if ( TxTimeDelay == 0 ) break;
   //   }
    //////////////////////////////////////  
      
    TbState=SIGNATURE_ANALISE_STATE;
    if(crc16==Tree.chs)
      {
      Tree.Signature=0x50;
      }
    else
      {
      Tree.error=errCHS;
      Tree.Signature=0x50;
      }
    }
  break;

  default:break;
  };
}


void RxEvent(DTConf_t *Conf)
{
 
asm("nop");  
  
Conf->Reserved=0x00; 
Conf->DTState=Answer;
Conf->FragCnt =0x01;
Conf->FragIdx =0x01;

//Conf->ProtType=BOOT_PROTOCOL;
//Conf->pData="ccabddc";
//Conf->DataCnt=7;
}


//Сборщик пакетов для отправки
void TBusRun(void)
{
uint8_t  *pBuff;
uint8_t  *pSource;
uint16_t TxLen;      //Длина передающейся посылки
uint16_t CHS;        //Контрольная сумма
uint8_t  dataIdx;
DTConf_t DTConf;

uint16_t i,j,inc;
#ifdef _archive_h
uint16_t ArchRecCnt; //Колличество архивных записей, которые будут отправлены за 1 раз
#endif// _archive_h


while((RxFirst!=RxLast)&&(0==Tree.Signature))//Анализ входного потока байт
  {
  TBusFlowAnaliser(RxBuff[RxFirst++]);
  if(RxFirst>=(sizeof(RxBuff)-1))
    RxFirst=0;
  }



if(0x50==Tree.Signature)//Дерево разбора было сформировано подпрограммой TBusFlowAnaliser
{
  if(errNone==Tree.error)//Если запрос идентифицирован без ошибок
    {
    switch(Tree.Command)
    {
    case cmdId://Кооманда идентификации
    for( i=0;i!=TxBuffSize;i++)//Очистка буфера
      TxBuff[i]=0;

    TxLen=37; //Для этой комманды длина ответа фиксированная
     pBuff=&TxBuff[0];
    *pBuff++=Tree.Signature;
    *pBuff++=NetworkDeviceAddr;
    *pBuff++=Tree.Command;
    pSource=(uint8_t *)&DevName[0];
    Tree.Signature=0x00;//Сброс признака законченного разбора
    for( i=0;i!=20;i++)
      *pBuff++=*pSource++;
     pBuff++;//Резервный байт в новой версии протокола
    *pBuff++=ProtocolVersion;
    pSource=(uint8_t *)&ProgrammVersion[0];
    for( i=0;i!=10;i++)
      *pBuff++=*pSource++;
    CHS=CALC_CHS_16(&TxBuff[1],TxLen-3);
    *pBuff++=(uint8_t)(CHS>>0x08);
    *pBuff++=(uint8_t)(CHS);
    SlaveSendPacket(&TxBuff[0],TxLen);//Послать в UART
    break;

    case cmdSetAddr:
    TxLen=6; //Для этой комманды длина ответа фиксированная
     pBuff=&TxBuff[0];
    *pBuff++=Tree.Signature;
    *pBuff++=Tree.NewDevAddr;
    *pBuff++=Tree.Command;
    *pBuff++=Tree.DevAddr;
    Tree.Signature=0x00;
    CHS=CALC_CHS_16(&TxBuff[1],TxLen-3);
    *pBuff++=(uint8_t)(CHS>>0x08);
    *pBuff++=(uint8_t)(CHS);
    //Вставить подпрограмму записи нового адреса
    NetworkDeviceAddr=Tree.NewDevAddr;
    SlaveSendPacket(&TxBuff[0],TxLen);//Послать в UART

//#ifdef _eeprom_h
 //   int_Write(&NetworkDeviceAddr,1, NetworkDeviceAddr_Addr);
//#endif

    break;


#ifdef _connect_h
    case cmdGetVarList://Комманда вычитывания карты памяти
    case cmdReadRAM_EXT://Комманда расширенного чтения ОЗУ

    if((0xFFFF==Tree.IdxRAS)&&(0xFFFF==Tree.CntRAS))
      {                   //Если запросили колличество строк структуры RAS
      TxLen=9;

      for( i=0;i!=TxBuffSize;i++)//Очистка буфера
        TxBuff[i]=0;

       pBuff=&TxBuff[0];
      *pBuff++=Tree.Signature;
      *pBuff++=NetworkDeviceAddr;
      *pBuff++=Tree.Command;

      *pBuff++=(uint8_t)Tree.IdxRAS;
      *pBuff++=(uint8_t)(Tree.IdxRAS>>0x08);
      *pBuff++=(uint8_t)RAS_Count;
      *pBuff++=(uint8_t)(RAS_Count>>0x08);

      CHS=CALC_CHS_16(&TxBuff[1],TxLen-3);
      *pBuff++=(uint8_t)(CHS>>0x08);
      *pBuff++=(uint8_t)(CHS);
      Tree.Signature=0x00;
      SlaveSendPacket(&TxBuff[0],TxLen);//Послать в UART
      }
     else if((0!=Tree.CntRAS)&&(UsartManagerSatus==UsartManagerFree))
      {//Вычитывание и передача структуры RAS

      for( i=0;i!=TxBuffSize;i++)//Очистка буфера
        TxBuff[i]=0;

       pBuff=&TxBuff[0];
      *pBuff++=Tree.Signature;
      *pBuff++=NetworkDeviceAddr;
      *pBuff++=Tree.Command;

       UsartManagerSatus=UsartManagerBusy;

      if(RAS_Count<=Tree.IdxRAS)
        {
        TxLen=9;
        *pBuff++=0xFF;
        *pBuff++=0xFF;
        *pBuff++=0xFF;
        *pBuff++=0xFF;
        Tree.Signature=0x00;
        }
      else
        {

        *pBuff++=(uint8_t)Tree.IdxRAS;
        *pBuff++=(uint8_t)(Tree.IdxRAS>>0x08);
        *pBuff++=0x01;
        *pBuff++=0x00;
        *pBuff++=RAS[Tree.IdxRAS].vCount;
        *pBuff++=RAS[Tree.IdxRAS].vType;

        if(cmdGetVarList==Tree.Command)
          {
          TxLen=30;

          if(RAS[Tree.IdxRAS].func)
            *pBuff++=0x01;
          else
            *pBuff++=0x00;

          pSource=(uint8_t *)RAS[Tree.IdxRAS].vInfo;
          for( i=0;i!=18;i++)
            *pBuff++=*pSource++;
          }
        else if(cmdReadRAM_EXT==Tree.Command)
          {
          Tree.DataLen=RAS[(uint8_t)Tree.IdxRAS].vCount*SizeOfType(RAS[(uint8_t)Tree.IdxRAS].vType);
          TxLen=11+Tree.DataLen;
          pSource=(uint8_t *)((CPU_BUS_LEN *)RAS[(uint8_t)Tree.IdxRAS].Addr);
          for( i=0;i!=Tree.DataLen;i++)
            *pBuff++=*pSource++;
          }
        }

      CHS=CALC_CHS_16(&TxBuff[1],TxLen-3);
      *pBuff++=(uint8_t)(CHS>>0x08);
      *pBuff++=(uint8_t)(CHS);

      Tree.CntRAS--;
      Tree.IdxRAS++;

      if(0==Tree.CntRAS)
        Tree.Signature=0x00;
      SlaveSendPacket(&TxBuff[0],TxLen);//Послать в UART
       }
    break;
#endif //_connect_h


    case cmdWriteRAM_EXT://Комманда расширенной записи в RAM
    TxLen=11;//Длина ответа в байтах (для этой комманды фиксированная)
    pBuff=&TxBuff[0];
    *pBuff++=Tree.Signature;
    *pBuff++=Tree.DevAddr;
    *pBuff++=Tree.Command;
    Tree.Signature=0x00;

    if(Tree.IdxRAS>=RAS_Count)//Если обатились к несуществующей записи
      {
      for( i=0x00;i!=0x06;i++)
        *pBuff++=0xFF;
      TxLen=0;  //Использование TxLen=0, как признака проблемм
      }
    else
      {
      *pBuff++=(uint8_t)Tree.IdxRAS;
      *pBuff++=(uint8_t)(Tree.IdxRAS>>0x08);
       if((Tree.IdxMAS>=RAS[Tree.IdxRAS].vCount))//Если обратились к несуществующему индексу массива
        {
        *pBuff++=0xFF;
        *pBuff++=0xFF;
        TxLen=0;
        }
      else
        {
        *pBuff++=(uint8_t)Tree.IdxMAS;
        *pBuff++=(uint8_t)(Tree.IdxMAS>>0x08);
        }
      if((Tree.IdxMAS*Tree.VarCnt)>RAS[Tree.IdxRAS].vCount)//Если возможен выход за пределы массива в процессе записи
        {
        TxLen=0;
        *pBuff++=0xFF;
        }
      else
        {
        *pBuff++=(uint8_t)Tree.VarCnt;
        }
      if(Tree.VarType!=RAS[Tree.IdxRAS].vType)//Если тип не совпадает
        {
        *pBuff++=0xFF;
        TxLen=0;
        }
      else
        {
        *pBuff++=(uint8_t)Tree.VarType;
        }
      }

    if(0==TxLen)//Если были проблемы
      {
      TxLen=11;
      }
    else
      {//Если проблем не было

      if(0!=RAS[Tree.IdxRAS].func)
        {
        dataIdx=0;
        for(j=0;j!=Tree.VarCnt;j++)
          {
          (*RAS[Tree.IdxRAS].func)(&Tree.data[dataIdx],j+Tree.IdxMAS);
          dataIdx+=SizeOfType(Tree.VarType);
          }
        }
      else
        {
        pSource=(uint8_t *)((CPU_BUS_LEN)RAS[Tree.IdxRAS].Addr);
        pSource+=Tree.IdxMAS*SizeOfType(Tree.VarType);
        for( i=0;i!=Tree.VarCnt*SizeOfType(Tree.VarType);i++)
          *pSource++=Tree.data[i];
        }
      }


    CHS=CALC_CHS_16(&TxBuff[1],TxLen-3);
    *pBuff++=(uint8_t)(CHS>>0x08);
    *pBuff++=(uint8_t)(CHS);

    SlaveSendPacket(&TxBuff[0],TxLen);
    break;


    case cmdReadRAM:
    case cmdReadEEPROM:
    case cmdReadFlash:
    TxLen=11+Tree.DataLen;//Длина ответа в байтах
    pBuff=&TxBuff[0];
    *pBuff++=Tree.Signature;
    *pBuff++=NetworkDeviceAddr;
    *pBuff++=Tree.Command;
    Tree.Signature=0x00;

    pSource=(uint8_t *)&Tree.VarAddr;

    for( inc=0;inc!=4;inc++)
      *pBuff++=*pSource++;

    *pBuff++=Tree.VarCnt;
    *pBuff++=Tree.VarType;

    if(cmdReadRAM==Tree.Command)
      {
        pSource=(uint8_t *)((CPU_BUS_LEN)Tree.VarAddr);//Вычитывать по адресу

      for( i=0;i!=Tree.DataLen;i++)
        *pBuff++=*pSource++;
      }

#ifdef _eeprom_h
   else if(cmdReadEEPROM==Tree.Command)
      {
    //  e_Read(pBuff,Tree.dataLen,Tree.VarAddr);
     // int_Read(pBuff,Tree.dataLen,Tree.VarAddr);
     // pBuff+=Tree.dataLen;
      }
#endif//_eeprom_h

#ifdef _extflash_h
   else if(cmdReadFlash==Tree.Command)
      {
      f_Read(pBuff,Tree.dataLen,Tree.VarAddr);
      pBuff+=Tree.dataLen;
      }
#endif //_extflash_h


    CHS=CALC_CHS_16(&TxBuff[1],TxLen-3);
    *pBuff++=(uint8_t)(CHS>>0x08);
    *pBuff++=(uint8_t)(CHS);
    SlaveSendPacket(&TxBuff[0],TxLen);
    break;

    case cmdWriteRAM:
    case cmdWriteFlash:
    case cmdWriteEEPROM:
    TxLen=11;//Длина ответа в байтах (для этой комманды фиксированная)
    pBuff=&TxBuff[0];
    *pBuff++=Tree.Signature;
    *pBuff++=Tree.DevAddr;
    *pBuff++=Tree.Command;
    Tree.Signature=0x00;

    *pBuff++=(uint8_t)((Tree.VarAddr));
    *pBuff++=(uint8_t)((Tree.VarAddr)>>0x08);
    *pBuff++=(uint8_t)((Tree.VarAddr)>>0x10);
    *pBuff++=(uint8_t)((Tree.VarAddr)>>0x18);

    *pBuff++=Tree.VarCnt;
    *pBuff++=Tree.VarType;
    CHS=CALC_CHS_16(&TxBuff[1],TxLen-3);
    *pBuff++=(uint8_t)(CHS>>0x08);
    *pBuff++=(uint8_t)(CHS);

    for( i=0;i!=RAS_Count;i++)
      {
      if((Tree.VarAddr >= (uint32_t)RAS[i].Addr)&&//Если адрес лежит в допустимых пределах
         (Tree.VarAddr < ((uint32_t)RAS[i].Addr+(SizeOfType(RAS[i].vType))*RAS[i].vCount)))
        {//То i-номер строки в структуре RAS
        if(0!=RAS[i].func)
          {
          dataIdx=0;
          for( j=0;j!=Tree.VarCnt;j++)
            {
            uint8_t index=(Tree.VarAddr-(uint32_t)RAS[i].Addr)/Tree.VarSize;
            (*RAS[i].func)(&Tree.data[dataIdx],index);
            dataIdx+=Tree.VarSize;
            }
          }
        else
        {
         if(cmdWriteRAM==Tree.Command)
          {
          pBuff=(uint8_t *)((CPU_BUS_LEN)Tree.VarAddr);
          pSource=&Tree.data[0];
          for( i=0;i!=Tree.DataLen;i++)//Запись принятых данных в ОЗУ
            *pBuff++=*pSource++;
          }
        #ifdef _eeprom_h
          else if(cmdWriteEEPROM==Tree.Command)//Запись принятых данный в EEPROM
            {
            //e_Write(&Tree.data[0],Tree.dataLen,Tree.VarAddr);
            //  int_Write(&Tree.data[0],Tree.dataLen,Tree.VarAddr);
            }
        #endif //_eeprom_h
        #ifdef _extflash_h
          else if(cmdWriteFlash==Tree.Command)//Запись принятых данный во Flash
            {
            f_Write(&Tree.data[0],Tree.dataLen,Tree.VarAddr);
            }
        #endif //_extflash_h
        }
        }
      }
    SlaveSendPacket(&TxBuff[0],TxLen);
    break;

#ifdef _archive_h
    case cmdArchParams:
    TxLen=5+20;//Длина ответа в байтах (для этой комманды фиксированная)
    pBuff=&TxBuff[0];
    *pBuff++=Tree.Signature;
    *pBuff++=NetworkDeviceAddr;
    *pBuff++=Tree.Command;
    Tree.Signature=0x00;

    *pBuff++=(uint8_t)((uint32_t)(ArchStart));
    *pBuff++=(uint8_t)((uint32_t)(ArchStart)>>0x08);
    *pBuff++=(uint8_t)((uint32_t)(ArchStart)>>0x10);
    *pBuff++=(uint8_t)((uint32_t)(ArchStart)>>0x18);

    *pBuff++=(uint8_t)((uint32_t)(ArchSize));
    *pBuff++=(uint8_t)((uint32_t)(ArchSize)>>0x08);
    *pBuff++=(uint8_t)((uint32_t)(ArchSize)>>0x10);
    *pBuff++=(uint8_t)((uint32_t)(ArchSize)>>0x18);

    *pBuff++=(uint8_t)((uint32_t)(ArchPeriod));
    *pBuff++=(uint8_t)((uint32_t)(ArchPeriod)>>0x08);

    *pBuff++=(uint8_t)((uint32_t)(ArchRecSize));
    *pBuff++=(uint8_t)((uint32_t)(ArchRecSize)>>0x08);

   // *pBuff++=(uint8_t)((uint32_t)(ArchRecCount));
   // *pBuff++=(uint8_t)((uint32_t)(ArchRecCount)>>0x08);
   // *pBuff++=(uint8_t)((uint32_t)(ArchRecCount)>>0x10);
   // *pBuff++=(uint8_t)((uint32_t)(ArchRecCount)>>0x18);
   //  pBuff+=4;//Сдесь должна бить ControlTime

    // sky 19.01.12
    *pBuff++ = (uint8_t)((uint32_t)(&arch_struct));
    *pBuff++ = (uint8_t)((uint32_t)(&arch_struct)>>0x08);
    *pBuff++ = (uint8_t)((uint32_t)(&arch_struct)>>0x10);
    *pBuff++ = (uint8_t)((uint32_t)(&arch_struct)>>0x18);
   
    *pBuff++ = (uint8_t)((uint32_t)(&arch_points));
    *pBuff++ = (uint8_t)((uint32_t)(&arch_points)>>0x08);
    *pBuff++ = (uint8_t)((uint32_t)(&arch_points)>>0x10);
    *pBuff++ = (uint8_t)((uint32_t)(&arch_points)>>0x18);
    
    CHS=CALC_CHS_16(&TxBuff[1],TxLen-3);
    *pBuff++=(uint8_t)(CHS>>0x08);
    *pBuff++=(uint8_t)(CHS);

    SlaveSendPacket(&TxBuff[0],TxLen);
    break;

    case cmdArchData:

    if((Tree.ArchiveRecCnt)&&(UsartManagerSatus==UsartManagerFree))
      {
       pBuff=&TxBuff[0];
      *pBuff++=Tree.Signature;
      *pBuff++=NetworkDeviceAddr;
      *pBuff++=Tree.Command;

      UsartManagerSatus=UsartManagerBusy;

      ArchRecCnt=(TxBuffSize-13)/ArchRecSize;//Сколько записей поместится в буфер

      if(ArchRecCnt>Tree.ArchiveRecCnt)//Если осталось меньше записей чем может поместиться в буфер
        ArchRecCnt=Tree.ArchiveRecCnt;

      TxLen=ArchRecCnt*ArchRecSize+5+8;

      *pBuff++=(uint8_t)(Tree.ArchiveRecNo);
      *pBuff++=(uint8_t)((Tree.ArchiveRecNo)>>0x08);
      *pBuff++=(uint8_t)((Tree.ArchiveRecNo)>>0x10);
      *pBuff++=(uint8_t)((Tree.ArchiveRecNo)>>0x18);

      *pBuff++=(uint8_t)(ArchRecCnt);
      *pBuff++=(uint8_t)(ArchRecCnt>>0x08);
      *pBuff++=(uint8_t)(ArchRecCnt>>0x10);
      *pBuff++=(uint8_t)(ArchRecCnt>>0x18);


      ArchRecRead(pBuff,ArchRecCnt,Tree.ArchiveRecNo);//Чтение из памяти

      pBuff=&TxBuff[TxLen-2];

      CHS=CALC_CHS_16(&TxBuff[1],TxLen-3);
      *pBuff++=(uint8_t)(CHS>>0x08);
      *pBuff++=(uint8_t)(CHS);

      Tree.ArchiveRecNo+=ArchRecCnt;
      Tree.ArchiveRecCnt-=ArchRecCnt;

      SlaveSendPacket(&TxBuff[0],TxLen);

      if(0==Tree.ArchiveRecCnt)
        Tree.Signature=0x00;
      }
    break;
#endif  //_archive_h
    
    case cmdDataTransfer:
      DTConf.Reserved = Tree.Reserved;
      DTConf.ProtType = Tree.ProtType;
      DTConf.pData    = &Tree.data[0];
      DTConf.DataCnt  = Tree.DataLen;     
      DTConf.DTState  = Tree.State;    
      RxEvent(&DTConf);                 //Передача управления внешнему обработчику
      TxLen=10+DTConf.DataCnt;           //Длина пакета для передачи
      pBuff=&TxBuff[0];      
      *pBuff++=Tree.Signature;
      Tree.Signature=0;
      *pBuff++=NetworkDeviceAddr;
      *pBuff++=Tree.Command;
      *pBuff++=DTConf.Reserved;
      *pBuff++=(uint8_t)DTConf.DTState;
      *pBuff++=(DTConf.FragCnt<<0x04)|(0x0F&DTConf.FragIdx); 
      *pBuff++=(uint8_t)DTConf.ProtType;
      *pBuff++=DTConf.DataCnt;   
      pSource=&DTConf.pData[0];
      for( i=0;i!=DTConf.DataCnt;i++)
        *pBuff++=*pSource++;  
      CHS=CALC_CHS_16(&TxBuff[1],TxLen-3);
      *pBuff++=(uint8_t)(CHS>>0x08);
      *pBuff++=(uint8_t)(CHS);
      asm("nop");
      SlaveSendPacket(&TxBuff[0],TxLen);
    break;  
    
    default:break;
    };
    }
  else if((errNone!=Tree.error)&&(Tree.Signature!=0x00))
    {

    TxLen=5;//Длина ответа в байтах (для этой комманды фиксированная)
    TxBuff[0]=Tree.Signature;
    TxBuff[1]=NetworkDeviceAddr;
    TxBuff[2]=Tree.error;

    CHS=CALC_CHS_16(&TxBuff[1],TxLen-3);
    TxBuff[3]=(uint8_t)(CHS>>0x08);
    TxBuff[4]=(uint8_t)(CHS);
    Tree.Signature=0x00;
    
	// sky!! float
	for(i=0;i!=0xFFF;i++)asm("nop");

    SlaveSendPacket(&TxBuff[0],TxLen);

    }
  }
}

void TBusTimeOutCounter(void)
{
  if ( TbState != SIGNATURE_ANALISE_STATE )
   if(Time++ > TBusTimeOut - 1)
   {
    Time=0;
    TbState=SIGNATURE_ANALISE_STATE;
   }
  
  if (TxTimeDelay > 0) TxTimeDelay--;

}











