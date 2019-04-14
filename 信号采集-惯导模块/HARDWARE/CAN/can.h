#ifndef __CAN_H
#define __CAN_H	 
#include "sys.h"	  
 //////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//CAN驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/11
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

//CAN接收RX0中断使能
//#define CAN_RX0_INT_ENABLE	1		//0,不使能;1,使能.								    


typedef struct node
{
  unsigned long ulNodeID; //  elmo结点号
  CanTxMsg     elmoCAN;  //  elmo报文结构体,用于向elmo写入数据
}Can_node;


void Can_send_rad(u8 addr,u8 *pda);
extern unsigned long g_Index;            //  全局变量,需要读的对象字典的索引
extern unsigned long g_SubIndex;         //  全局变量,需要读的对象字典的子索引
extern unsigned long g_ReadNodeID;       //  全局变量,需要读的节点ID
void Respond_node(char node );
void Test_Can(void);
void can_diver(void);
u8 CAN_Mode_Init(u8 tsjw,u8 tbs2,u8 tbs1,u16 brp,u8 mode);//CAN初始化
 void Can_send_adc(u8 addr,u8 *pda);
u8 Can_Send_Msg(u8* msg,u8 len);						//发送数据
void Senddat_tonode(u8 node_addr,int*buf,int len);
u8 Can_Receive_Msg(u8 *buf);							//接收数据
#endif

















