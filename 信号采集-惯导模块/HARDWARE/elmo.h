

/*******************************************************************************/

#ifndef _ELMO_H
#define _ELMO_H

#include "can.h"

/*******************************************************************************
   电机参数：要根据相应的的电机修改
*******************************************************************************/
#define MAX_TORQUE          2000              //  用户允许最大扭矩,计算方法同下
#define MAX_CURRENT         5000              //  用户允许最大电流,计算方法：
#define RATE_CURRENT        5000              //  电机额定电流(mA)

/*******************************************************************************
                            配置宏定义
*******************************************************************************/
#define CANBAUD_SET         CANBAUD_1M        //  设置CAN通信的波特率
#define MOTION_PROFILE_TYPE 0                 //  平滑运动方式;0:linear ramp
#define MAX_F_ERR           1000000           //  最大允许误差
#define MIN_P_LIMIT        -1000000000        //  最小位置极限
#define MAX_P_LIMIT         1000000000        //  最大位置极限
#define MAX_SPEED           1000000              //  最大运行速度
#define MAX_QDEC            1000000000        //  最大出错停止减速度
#define MAX_ACC             1000000            //  最大加速度
#define POLARITY            0xC0              //  0x00: 速度位置都乘以  1
                                              //  0xc0: 速度位置都乘以 -1
                                              //  0x80: 速度乘以  1，位置乘以 -1
                                              //  0x40: 速度乘以 -1，位置乘以  1
/*******************************************************************************
                             调用宏定义
*******************************************************************************/
#define ABS_POS             0x1f              //  PPMODE的运行方式(runMode):绝对位置
#define ABS_POS_IMM         0x3f              //  PPMODE的运行方式(runMode):绝对位置,立即运行3
#define REL_POS_IMM         0x7f              //  PPMODE的运行方式(runMode):相对位置,立即运行7
#define REL_POS             0x5f              //  PPMODE的运行方式(runMode):相对位置

/*******************************************************************************
                           内部宏定义
*******************************************************************************/
#define Delay1              40                //  延时400us
#define Delay2              80                //  延时800us
#define Delay3              200               //  延时2ms
#define Delay_Read          150
#define Delay_Velocity		10				  //100us
#define Elmo_Delay(Time)   delay_us(Time)    //  延时10us宏定义


/*******************************************************************************
                 Elmo相关结构体写法同Epos
*******************************************************************************/


//  平滑速度模式结构体
typedef struct _tPVMode
{
  long Speed;             //  平滑速度模式期望速度值
  long Acc;               //  平滑速度模式加速度值
  long Dec;               //  平滑速度模式减速度值
  long QSDec;             //  平滑速度模式急停减速度值
}tPVMode;

//  平滑位置模式结构体
typedef struct _tPPMode
{
  long Position;          //  平滑位置模式期望位置值
  long Speed;             //  平滑位置模式速度
  long Acc;               //  平滑位置模式加速度
  long Dec;               //  平滑位置模式减速度
  long QSDec;             //  平滑位置模式急停减速度
}tPPMode;

//  平滑扭矩模式结构体
typedef struct _tPTMode
{
  long Torque;            //  平滑扭矩期望电流值
}tPTMode;

//  各种模式总和结构体
typedef struct _tMode
{
  tPPMode    PPMode;       //  平滑位置模式
  tPVMode    PVMode;       //  平滑速度模式
  tPTMode    PTMode;       //  平滑扭矩模式
  void*     CurMode;       //  指向当前模式的指针
}tMode;

//  Elmo结构体,每个Elmo对应一个该结构体
typedef struct _Elmo
{
  unsigned long ulNodeID; //  elmo结点号
  CanTxMsg     elmoCAN;  //  elmo报文结构体,用于向elmo写入数据
  tMode         Mode;     //  elmo各种模式
}Elmo;

typedef struct Elmo_s
{
    u8    Timie_sys;
    u8     sport_Mode;     //  elmo各种模式
	  u8    index_value;
}Elm0_sport;


/*******************************************************************************
                               变量声明
*******************************************************************************/
extern Elmo   node_communicate[4];
extern Elmo           elmo[6];
extern  Elm0_sport sport_mode;
/*
extern tCANFrameBuff ReceFrameBuff;		//  定义帧接收缓冲区
extern tCANFrameBuff SendFrameBuff;             //  定义帧发送缓冲区
extern tCANFrameBuff *pReceFrameBuff;	        //  定义帧接收缓冲区指针
extern tCANFrameBuff *pSendFrameBuff;           //  定义帧发送缓冲区指针
*/
/*******************************************************************************  
                               函数声明 
*******************************************************************************/

void Change_mode(Elmo* elmo,  unsigned char mode);
void my_elmo_len(Elmo* elmo, unsigned char*  x1, unsigned char len);
void my_elmo(Elmo* elmo, unsigned char x1, unsigned char x2, unsigned char x3,
unsigned char x4,unsigned char x5, unsigned char x6,unsigned char x7,unsigned char x8);
void MY_NMTWrite(Elmo* elmo, long cmd);
extern void Elmo_ParamInit(Elmo* elmo);		  /* 初始化elmo参数	*/	  
extern void Elmo_Init(Elmo* elmo, unsigned long elmoNum);	  /* 所有ELMO的初始化 调用这一个就行了*/
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
