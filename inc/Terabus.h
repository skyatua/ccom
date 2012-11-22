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

//������ ��������� TeraBus ver 4.0 beta
//****������� �������� ������***
//������ ������� �� ��������� �����������:
//1) TBusDataReceiver - �������� ������ ������. ����� ����� ���������� �� ������ �����.

//2) TBusFlowAnaliser - ���������� �������� ������ ����. ����������� ������ ���� ������
//������� ������� ������������������ Tree. ���� ���� ��������� (Tree.Signature)
//����� ������ �� ����� 0, �� ������, ��� TBusFlowAnaliser �������� ������� ������ � ����������� ������

//3) TBusRun - ����������� ������ ������� � � ����������� �� ���������� � ���� ��������� ��������� �����,
//������� ����� ���������� �� UART. ��� �������� ������ ������ ����� ����������� �� ������.
//������������ TBusRun ���������� ��������� � ���� ������� main();


//4) TBusTimeOutCounter - ������������ �������� ��������. �������� ����� � �������� 1 ��.


//������������� ������������ ���� ������ ��������� �������:
//   ____________________
//  |    TeraBus.c (h)   |
//  |____________________| - ������ ��������� TeraBus ������������������.
//  | UsartManager.c (h) |
//  |____________________| - �������� ������ � ���������������� ������. ������������������.
//  |  UsartDriver.c (h) |
//  |  (������ �������)  | - ������� ����������������� �����. ��� ������ ��������� ������
//  |____________________|

//��� ���������� ������ ������ �����:
//1)��������� TBusRun() � ���� ������� main()
//2)��������� TBusTimeOutCounter() � ��������� ������� 1 ��.
//3)���������� ������������ TBusDataReceiver �������� ������ �� ������ �����
//4)��������� �������� CPU_BUS_LEN � ����������� �� ����������� ��

//*********************************����������!!!********************************
//������ Terabus ��������� ���� ����������������� ���� ���� �� ��������� ����� 2


#define TeraDeviceName         "UP_Dev"        //��� �������
#define DeviceProgrammVersion  "v4.000.01"     //������ ��������� �������
#define DeviceNetworkAddr      0x03            //������� ����� ������� �� ���������

typedef enum
{
cmdReadRAM          = 0x02, //������ ���
cmdWriteRAM         = 0x03, //������ � ���
cmdReadEEPROM       = 0x12, //������ EEPROM
cmdWriteEEPROM      = 0x13, //������ EEPROM
cmdReadFlash        = 0x42, //������ Flash
cmdWriteFlash       = 0x43, //������ �� Flash
cmdId               = 0x20, //������������� �������
cmdSetAddr          = 0x21, //��������� ������ �������
cmdArchParams       = 0x22, //������ ������������ ������
cmdArchData         = 0x23, //������ �������� �������
cmdGetVarList       = 0x25, //����������� ������ ���������� �������
cmdReadRAM_EXT      = 0x26, //����������� ������ RAM
cmdWriteRAM_EXT     = 0x27, //����������� ������ � ���
cmdDataTransfer     = 0x35  //�������� ��������������� ������������ ������
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
uint8_t_            = 0x00, //����������� ����
int8_t_             = 0x01, //�������� ����
uint16_t_           = 0x02,
int16_t_            = 0x03,
uint32_t_           = 0x04,
int32_t_            = 0x05,
float_              = 0x06,
DateTime_t_         = 0x08  //���� � �����
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
NO_PROTOCOL         = 0x00, //������� �� ������� (������ ������)
BOOT_PROTOCOL       = 0x01  //���������� ���������� ��
}ProtType_t;        //��� ���������, ����������� �� ������ �������� cmdDataTransfer

typedef enum
{
SIGNATURE_ANALISE_STATE,   //��������� ������� ����������������� �����
DEV_ADDR_ANALISE_STATE,    //��������� ������� �������� ������ ����������
NEW_DEV_ADDR_RECEIVE_STATE,//��������� ������ ������ �������� ������ ���������� (��� �������� ����� ������)
COMMAND_ANALISE_STATE,     //��������� ������� � ������ �������� ���������
VAR_ADDR_RECEIVE_STATE,    //��������� ������ ������ ���������� ���, FLASH ��� EEPROM
VAR_CNT_RECEIVE_STATE,     //��������� ������ ����������� ���������� ������� ����� ������/����������
VAR_TYPE_ANALISE_STATE,    //��������� ������� � ������ ���� ����������
ARCH_REC_NO_RECEIVE_STATE, //��������� ������ ������ �������� ������ ������
ARCH_REC_CNT_RECEIVE_STATE,//��������� ������ ����������� �������� �������
CHS_ANALISE_STATE,         //��������� ������ � ������� ����������� �����
DATA_RECEIVE_STATE,        //��������� ������ ������ (��� ������� ������)
IDX_RAS_RECEIVE_STATE,     //��������� ������ ����������� ������ ���������� 
CNT_RAS_RECEIVE_STATE,     //��������� ������ ����������� ������������� ����������
IDX_MAS_RECEIVE_STATE,     //��������� ������ ������� ������ 
RESERVED_RECEIVE_STATE,    //��������� ������ ���������� ����� ��� �������� cmdDataTransfer  
STATE_RECEIVE_STATE,       //��������� ������ ��������� ����������� ������� cmdDataTransfer
FRAG_CNT_IDX_RECEIVE_STATE,//��������� ������ ����������� ���������� � ������ �������� ���������
PROT_TYPE_RECEIVE_STATE,   //��������� ������ ���� ���������, ����������� �� ������ �������� cmdDataTransfer  
DATA_CNT_RECEIVE_STATE     //��������� ������ ����������� ���� �������� cmdDataTransfer
}ParcerState_t;

typedef enum 
{
DT_FREE_RUN                 = 0x00,
DT_WAIT_SLAVE_RESPONSE      = 0x01,
DT_SLAVE_RESPONSE_COMPLETED = 0x02
}DtTransferState_t;


typedef struct
{
DtTransferState_t   State;    //������� ����, ��� ������ ��� ����� ����������  
uint8_t             DevAddr;  //������� ����� �������
uint8_t             Reserved; //����������������� ���� ��� �������� cmdDataTransfer
DTState_t           DTState;  //��������� ���������� 
uint8_t             FragCnt;  //����������� ����������
uint8_t             FragIdx;  //����� ���������
ProtType_t          ProtType; //��� ���������, ����������� �� ������ �������� cmdDataTransfer
uint8_t             *pData;   //��������� �� ������
uint8_t             DataCnt;  //����� �������� �����
}DTConf_t;


//������ ��������������� ������� �������� ������
//������ ��������������� ������� �������� ������
typedef struct
{
uint8_t       data[256];    //�������� ������ (��� ������� ������ � ���, EEPROM, FLASH, cmdDataTransfer)
uint8_t       Signature;    //���� ������������� 0�50 (��� �� ������������ ��� ������� ������������� �������)
uint8_t       DevAddr;      //�������� ����� ����������
uint8_t       NewDevAddr;   //����� ����� ���������� (��� �������� ����� ������)
TeraBusCmd_t  Command;      //�������� ���������
uint8_t       Reserved;     //����������������� ���� ��� �������� cmdDataTransfer
DTState_t     State;        //��������� ���������� �������  cmdDataTransfer
uint8_t       FragCnt;      //����������� ����������
uint8_t       FragIdx;      //����� ���������
ProtType_t    ProtType;     //��� ���������, ����������� �� ������ �������� cmdDataTransfer
uint16_t      IdxRAS;       //������ ���������� � ��������� RAS
uint16_t      CntRAS;       //����������� ���������� ������� � IdxRAS
uint16_t      IdxMAS;       //������ ������ �������
uint32_t      VarAddr;      //����� ���������� ������� ������ ��� � ������� �����
uint8_t       VarCnt;       //����������� ���������� ������� ����� ��������� ��� ��������
VarType_t     VarType;      //��� ���������� ������� ����� ��������� ��� ��������
uint8_t       VarSize;      //������ ����������, ������� ����� ���������/�������� VarSize = sizeof(VarType)
uint32_t      ArchiveRecNo; //����� ������ � ������, ������� � ������� ����� ��������� �����
uint16_t      ArchiveRecCnt;//��-���� ������� � ������, ������� ����� ���������  ������� � ArchiveRecNo
uint8_t       DataLen;      //����� ������ ������� ����� �������� ��� �������� dataLen=VarCnt*sizeof(VarType)
uint16_t      chs;          //�������� ����������� �����
TeraBusErr_t  error;        //��������� �� ������
}Tree_t;

//********************************************************************************
//**************��������� ������ ��� ������ �����������������*********************
//********************************************************************************
//#define CPU_BUS_LEN uint16_t //��� �� � 16-������ ���������� � ������ �������� *
#define CPU_BUS_LEN uint16_t //��� �� � 32-������ ���������� � ������ ��������   *
//********************************************************************************


#define TBusTimeOut 1000 //x10ms ~200ms //�������

extern uint8_t NetworkDeviceAddr;//������� ����� �������

extern void TBusDataReceiver(uint8_t byte);//����� ���� (���������� �� ������ �����)

extern void TBusRun(void);//������������� ������� �� ��������� TeraBus
                          //�������� � ���� ������� main
extern void TBusTimeOutCounter(void);//�������� � �������� � 1 ��

#endif

