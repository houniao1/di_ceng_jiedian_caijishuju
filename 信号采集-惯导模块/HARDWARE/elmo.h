

/*******************************************************************************/

#ifndef _ELMO_H
#define _ELMO_H

#include "can.h"

/*******************************************************************************
   ���������Ҫ������Ӧ�ĵĵ���޸�
*******************************************************************************/
#define MAX_TORQUE          2000              //  �û��������Ť��,���㷽��ͬ��
#define MAX_CURRENT         5000              //  �û�����������,���㷽����
#define RATE_CURRENT        5000              //  ��������(mA)

/*******************************************************************************
                            ���ú궨��
*******************************************************************************/
#define CANBAUD_SET         CANBAUD_1M        //  ����CANͨ�ŵĲ�����
#define MOTION_PROFILE_TYPE 0                 //  ƽ���˶���ʽ;0:linear ramp
#define MAX_F_ERR           1000000           //  ����������
#define MIN_P_LIMIT        -1000000000        //  ��Сλ�ü���
#define MAX_P_LIMIT         1000000000        //  ���λ�ü���
#define MAX_SPEED           1000000              //  ��������ٶ�
#define MAX_QDEC            1000000000        //  ������ֹͣ���ٶ�
#define MAX_ACC             1000000            //  �����ٶ�
#define POLARITY            0xC0              //  0x00: �ٶ�λ�ö�����  1
                                              //  0xc0: �ٶ�λ�ö����� -1
                                              //  0x80: �ٶȳ���  1��λ�ó��� -1
                                              //  0x40: �ٶȳ��� -1��λ�ó���  1
/*******************************************************************************
                             ���ú궨��
*******************************************************************************/
#define ABS_POS             0x1f              //  PPMODE�����з�ʽ(runMode):����λ��
#define ABS_POS_IMM         0x3f              //  PPMODE�����з�ʽ(runMode):����λ��,��������3
#define REL_POS_IMM         0x7f              //  PPMODE�����з�ʽ(runMode):���λ��,��������7
#define REL_POS             0x5f              //  PPMODE�����з�ʽ(runMode):���λ��

/*******************************************************************************
                           �ڲ��궨��
*******************************************************************************/
#define Delay1              40                //  ��ʱ400us
#define Delay2              80                //  ��ʱ800us
#define Delay3              200               //  ��ʱ2ms
#define Delay_Read          150
#define Delay_Velocity		10				  //100us
#define Elmo_Delay(Time)   delay_us(Time)    //  ��ʱ10us�궨��


/*******************************************************************************
                 Elmo��ؽṹ��д��ͬEpos
*******************************************************************************/


//  ƽ���ٶ�ģʽ�ṹ��
typedef struct _tPVMode
{
  long Speed;             //  ƽ���ٶ�ģʽ�����ٶ�ֵ
  long Acc;               //  ƽ���ٶ�ģʽ���ٶ�ֵ
  long Dec;               //  ƽ���ٶ�ģʽ���ٶ�ֵ
  long QSDec;             //  ƽ���ٶ�ģʽ��ͣ���ٶ�ֵ
}tPVMode;

//  ƽ��λ��ģʽ�ṹ��
typedef struct _tPPMode
{
  long Position;          //  ƽ��λ��ģʽ����λ��ֵ
  long Speed;             //  ƽ��λ��ģʽ�ٶ�
  long Acc;               //  ƽ��λ��ģʽ���ٶ�
  long Dec;               //  ƽ��λ��ģʽ���ٶ�
  long QSDec;             //  ƽ��λ��ģʽ��ͣ���ٶ�
}tPPMode;

//  ƽ��Ť��ģʽ�ṹ��
typedef struct _tPTMode
{
  long Torque;            //  ƽ��Ť����������ֵ
}tPTMode;

//  ����ģʽ�ܺͽṹ��
typedef struct _tMode
{
  tPPMode    PPMode;       //  ƽ��λ��ģʽ
  tPVMode    PVMode;       //  ƽ���ٶ�ģʽ
  tPTMode    PTMode;       //  ƽ��Ť��ģʽ
  void*     CurMode;       //  ָ��ǰģʽ��ָ��
}tMode;

//  Elmo�ṹ��,ÿ��Elmo��Ӧһ���ýṹ��
typedef struct _Elmo
{
  unsigned long ulNodeID; //  elmo����
  CanTxMsg     elmoCAN;  //  elmo���Ľṹ��,������elmoд������
  tMode         Mode;     //  elmo����ģʽ
}Elmo;

typedef struct Elmo_s
{
    u8    Timie_sys;
    u8     sport_Mode;     //  elmo����ģʽ
	  u8    index_value;
}Elm0_sport;


/*******************************************************************************
                               ��������
*******************************************************************************/
extern Elmo   node_communicate[4];
extern Elmo           elmo[6];
extern  Elm0_sport sport_mode;
/*
extern tCANFrameBuff ReceFrameBuff;		//  ����֡���ջ�����
extern tCANFrameBuff SendFrameBuff;             //  ����֡���ͻ�����
extern tCANFrameBuff *pReceFrameBuff;	        //  ����֡���ջ�����ָ��
extern tCANFrameBuff *pSendFrameBuff;           //  ����֡���ͻ�����ָ��
*/
/*******************************************************************************  
                               �������� 
*******************************************************************************/

void Change_mode(Elmo* elmo,  unsigned char mode);
void my_elmo_len(Elmo* elmo, unsigned char*  x1, unsigned char len);
void my_elmo(Elmo* elmo, unsigned char x1, unsigned char x2, unsigned char x3,
unsigned char x4,unsigned char x5, unsigned char x6,unsigned char x7,unsigned char x8);
void MY_NMTWrite(Elmo* elmo, long cmd);
extern void Elmo_ParamInit(Elmo* elmo);		  /* ��ʼ��elmo����	*/	  
extern void Elmo_Init(Elmo* elmo, unsigned long elmoNum);	  /* ����ELMO�ĳ�ʼ�� ������һ��������*/
extern void Elmo_Write(Elmo* elmo, unsigned long Index, unsigned char SubIndex, long Param);
extern void Elmo_Read(Elmo* elmo, unsigned long Index, unsigned char SubIndex);
extern void Elmo_RunPVM(Elmo* elmo, long speed, long acc, long dec, long QSDec);
extern void Elmo_RunPPM(Elmo* elmo, long speed, long position, long acc, long dec, long runMode);
extern void Elmo_RunPTM(Elmo* elmo, long torque);
extern void Elmo_QuickStop(Elmo* elmo);
extern long Elmo_PositionGet(Elmo* elmo);
extern unsigned long Elmo_PPMReached(Elmo* elmo);
void Elmo_NMTWrite(Elmo* elmo, long cmd);
//extern void Elmo_PDOMapping(Elmo* elmo,ElmoObject *RPDO ,ElmoObject *AppObj);
extern void Elmo_PDOWrite(Elmo* elmo, long Data);
extern void Elmo_NMTWrite(Elmo* elmo, long cmd);
void Run_pt_mode(float location);
void Init_location_control_mode(void);
void Init_PTmode( u8 mode );
void Location_control(float x);
void Bigan_run();
void PT_mode();
void MY_PDOWrite(Elmo* elmo,long value);
void Speed_control_mode(long speed);
void MY_Elmo_PDOWrite(Elmo* elmo, unsigned char *cmd,long value,u8 len);
void my_Elmo_PDOInit(Elmo * elmo);
#endif
