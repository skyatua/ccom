#ifndef _terabus_h
#define _terabus_h

//#include "variables.h"

//Last modified 16.05.11 by Ihor Bilorus
//Last modified 18.05.11 by Ihor Bilorus (added functions in TRAS)
//Last modified 23.05.11 by Ihor Bilorus (added commands cmdSetAddr, cmdGetVarList)
//Last modified 23.06.11 by Ihor Bilorus (revised command cmdWriteRAM, change TimeOutCounter)
//Last modified 01.08.11 by Ihor Bilorus (added command cmdReadRAM_EXT, added TBusDataReceiver,
//added the possibility of indirect addressing in command of cmdWriteRAM/cmdReadRAM)
//Last modified 20.12.11 by Ihor Bilorus (addef command cmdDataTransfer)
//Last modified 12.01.12 by Ihor Bilorus (modified command cmdDataTransfer)

//Модуль протокола TeraBus ver 4.0 beta
//****Краткое описание работы***
//Модуль состоит из следующих подпрограмм:
//1) TBusDataReceiver - приемник потока данных. Поток нужно передавать по одному байту.

//2) TBusFlowAnaliser - анализатор входного потока байт. Результатом работы есть дерево
//разбора входной последовательности Tree. Если байт сигнатуры (Tree.Signature)
//этого дерева не равен 0, то значит, что TBusFlowAnaliser разобрал фходной запрос и сформировал дерево

//3) TBusRun - анализирует дерево разбора и в зависимости от занесенной в него коммандыи формирует ответ,
//который потом передается по UART. Для комманды чтения архива ответ формируется по частям.
//Подпрограмму TBusRun необходимо поместить в цикл функции main();


//4) TBusTimeOutCounter - подпрограмма подсчета таймаута. Вызивать нужно с периодом 1 мс.


//Рекомендовано использовать этот модуль следующим образом:
//   ____________________
//  |    TeraBus.c (h)   |
//  |____________________| - Модуль протокола TeraBus кроссплатформенный.
//  | UsartManager.c (h) |
//  |____________________| - Менеджер раборы с последовательным портом. Кроссплатформенный.
//  |  UsartDriver.c (h) |
//  |  (нижний уровень)  | - Драйвер последовательного порта. Для каждой платформы разный
//  |____________________|

//Для правильной работы модуля нужно:
//1)Поместить TBusRun() в цикл функции main()
//2)Поместить TBusTimeOutCounter() в обаботчик таймера 1 мс.
//3)Передавать подпрограмме TBusDataReceiver принятые данные по одному байту
//4)Настроить параметр CPU_BUS_LEN в зависимости от разрядности МК

//*********************************Примечание!!!********************************
//Модуль Terabus сохраняет свою работоспособность даже если не выполнять пункт 2


#define TeraDeviceName         "UP_Dev"        //Имя прибора
#define DeviceProgrammVersion  "v4.000.01"     //Версия программы прибора
#define DeviceNetworkAddr      0x03            //Сетевой адрес прибора по умолчанию

typedef enum
{
cmdReadRAM          = 0x02, //Чтение ОЗУ
cmdWriteRAM         = 0x03, //Запись в ОЗУ
cmdReadEEPROM       = 0x12, //Чтение EEPROM
cmdWriteEEPROM      = 0x13, //Запись EEPROM
cmdReadFlash        = 0x42, //Чтение Flash
cmdWriteFlash       = 0x43, //Запись во Flash
cmdId               = 0x20, //Идентификация прибора
cmdSetAddr          = 0x21, //Установка адреса прибора
cmdArchParams       = 0x22, //Чтение конфигурации архива
cmdArchData         = 0x23, //Чтение архивных записей
cmdGetVarList       = 0x25, //Вычитывание списка переменных прибора
cmdReadRAM_EXT      = 0x26, //Расширенное чтение RAM
cmdWriteRAM_EXT     = 0x27, //Расширенная запись в ОЗУ
cmdDataTransfer     = 0x35  //Комманда транспортировки произвольных данных
}TeraBusCmd_t;

typedef enum
{
errNone             = 0x00,
errCHS              = 0xF0,
errUnknownCmd       = 0xF1,
//errInvalidCount     = 0xF2,
errInvalidType      = 0xF3,
//errInvalidDataLen   = 0xF4,
//errInvalidData      = 0xF5,
//errExecution        = 0xF6,
//errFrame            = 0xF7,
//errPending          = 0xF8
}TeraBusErr_t;

typedef enum
{
uint8_t_            = 0x00, //Беззнаковый байт
int8_t_             = 0x01, //Знаковый байт
uint16_t_           = 0x02,
int16_t_            = 0x03,
uint32_t_           = 0x04,
int32_t_            = 0x05,
float_              = 0x06,
DateTime_t_         = 0x08  //Дата и время
}VarType_t;


typedef enum
{
Request = 0x01,
Answer  = 0x02,
Next  	= 0x03,
Retry   = 0x04
}DTState_t;     


typedef enum 
{
NO_PROTOCOL         = 0x00, //Пртокол не наложен (просто данные)
BOOT_PROTOCOL       = 0x01  //Протококол обновления ПО
}ProtType_t;        //Тип протокола, наложенного на данные комманды cmdDataTransfer

typedef enum
{
SIGNATURE_ANALISE_STATE,   //Состояние анализа синхронизирующего байта
DEV_ADDR_ANALISE_STATE,    //Состояние анализа сетевого адреса устройства
NEW_DEV_ADDR_RECEIVE_STATE,//Состояние приема нового сетевого адреса устройства (для комманды смены адреса)
COMMAND_ANALISE_STATE,     //Состояние анализа и приема комманды протокола
VAR_ADDR_RECEIVE_STATE,    //Состояние приема адреса переменной ОЗУ, FLASH или EEPROM
VAR_CNT_RECEIVE_STATE,     //Состояние приема колличества переменных которые нужно читать/записывать
VAR_TYPE_ANALISE_STATE,    //Состояние анализа и приема типа переменных
ARCH_REC_NO_RECEIVE_STATE, //Состояние приема номера архивной записи записи
ARCH_REC_CNT_RECEIVE_STATE,//Состояние приема колличества архивных записей
CHS_ANALISE_STATE,         //Состояние приема и анализа контрольной суммы
DATA_RECEIVE_STATE,        //Состояние приема данный (для комманд записи)
IDX_RAS_RECEIVE_STATE,     //Состояние приема порядкового номера переменной 
CNT_RAS_RECEIVE_STATE,     //Состояние приема колличества запрашиваемых переменных
IDX_MAS_RECEIVE_STATE,     //Состояние приема индекса масива 
RESERVED_RECEIVE_STATE,    //Состояние приема резервного байта для комманды cmdDataTransfer  
STATE_RECEIVE_STATE,       //Состояние приема состояния выполенения команды cmdDataTransfer
FRAG_CNT_IDX_RECEIVE_STATE,//Состояние приема колличества фрагментов и номера текущего фрагмента
PROT_TYPE_RECEIVE_STATE,   //Состояние приема типа протокола, наложенного на данные комманды cmdDataTransfer  
DATA_CNT_RECEIVE_STATE     //Состояние приема колличества байт комманды cmdDataTransfer
}ParcerState_t;

typedef enum 
{
DT_FREE_RUN                 = 0x00,
DT_WAIT_SLAVE_RESPONSE      = 0x01,
DT_SLAVE_RESPONSE_COMPLETED = 0x02
}DtTransferState_t;


typedef struct
{
DtTransferState_t   State;    //Признак того, что данные уже можно отправлять  
uint8_t             DevAddr;  //Сетевой адрес прибора
uint8_t             Reserved; //Зарезервированный байт для комманды cmdDataTransfer
DTState_t           DTState;  //Состояние выполнения 
uint8_t             FragCnt;  //Колличество фрагментов
uint8_t             FragIdx;  //Номер фрагмента
ProtType_t          ProtType; //Тип протокола, наложенного на данные комманды cmdDataTransfer
uint8_t             *pData;   //Указатель на данные
uint8_t             DataCnt;  //Длина принятых даных
}DTConf_t;


//Дерево синтаксического разбора входного потока
//Дерево синтаксического разбора входного потока
typedef struct
{
uint8_t       data[256];    //Принятые данные (для комманд записи в ОЗУ, EEPROM, FLASH, cmdDataTransfer)
uint8_t       Signature;    //Байт синхронизации 0х50 (так же используется как признак завершенности разбора)
uint8_t       DevAddr;      //Принятый адрес устройства
uint8_t       NewDevAddr;   //Новый адрес устройства (при комманде смены адреса)
TeraBusCmd_t  Command;      //Комманда протокола
uint8_t       Reserved;     //Зарезервированный байт для комманды cmdDataTransfer
DTState_t     State;        //Состояние выполнения команды  cmdDataTransfer
uint8_t       FragCnt;      //Колличество фрагментов
uint8_t       FragIdx;      //Номер фрагмента
ProtType_t    ProtType;     //Тип протокола, наложенного на данные комманды cmdDataTransfer
uint16_t      IdxRAS;       //Индекс переменной в структуре RAS
uint16_t      CntRAS;       //Колличество переменных начиная с IdxRAS
uint16_t      IdxMAS;       //Индекс внутри массива
uint32_t      VarAddr;      //Адрес переменной которую читают или в которую пишут
uint8_t       VarCnt;       //Колличество переменных которые нужно прочитать или записать
VarType_t     VarType;      //Тип переменных которые нужно прочитать или записать
uint8_t       VarSize;      //Размер переменной, которую нужно прочитать/записать VarSize = sizeof(VarType)
uint32_t      ArchiveRecNo; //Номер записи в архиве, начиная с которой нужно прочитать архив
uint16_t      ArchiveRecCnt;//Ко-ство записей в архиве, которые нужно прочитать  начиная с ArchiveRecNo
uint8_t       DataLen;      //Длина данных которые нужно вычитать или записать dataLen=VarCnt*sizeof(VarType)
uint16_t      chs;          //Принятая контрольная сумма
TeraBusErr_t  error;        //Сообщение об ошибке
}Tree_t;

//********************************************************************************
//**************Настройка модуля для разных микроконтроллеров*********************
//********************************************************************************
//#define CPU_BUS_LEN uint16_t //Для МК с 16-битной адресацией в памяти программ *
#define CPU_BUS_LEN uint16_t //Для МК с 32-битной адресацией в памяти программ   *
//********************************************************************************


#define TBusTimeOut 1000 //x10ms ~200ms //Таймаут

extern uint8_t NetworkDeviceAddr;//Сетевой адрес прибора

extern void TBusDataReceiver(uint8_t byte);//Прием байт (передавать по одному байту)

extern void TBusRun(void);//Формирователь ответов по протоколу TeraBus
                          //Вставить в цикл функции main
extern void TBusTimeOutCounter(void);//Вызывать с периодом в 1 мс

#endif

