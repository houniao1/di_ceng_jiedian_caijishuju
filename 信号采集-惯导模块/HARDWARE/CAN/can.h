#ifndef __CAN_H
#define __CAN_H	 
#include "sys.h"	  
 //////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//CAN���� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/11
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

//CAN����RX0�ж�ʹ��
//#define CAN_RX0_INT_ENABLE	1		//0,��ʹ��;1,ʹ��.								    


typedef struct node
{
  unsigned long ulNodeID; //  elmo����
  CanTxMsg     elmoCAN;  //  elmo���Ľṹ��,������elmoд������
}Can_node;


void Can_send_rad(u8 addr,u8 *pda);
extern unsigned long g_Index;            //  ȫ�ֱ���,��Ҫ���Ķ����ֵ������
extern unsigned long g_SubIndex;         //  ȫ�ֱ���,��Ҫ���Ķ����ֵ��������
extern unsigned long g_ReadNodeID;       //  ȫ�ֱ���,��Ҫ���Ľڵ�ID
void Respond_node(char node );
void Test_Can(void);
void can_diver(void);
u8 CAN_Mode_Init(u8 tsjw,u8 tbs2,u8 tbs1,u16 brp,u8 mode);//CAN��ʼ��
 void Can_send_adc(u8 addr,u8 *pda);
u8 Can_Send_Msg(u8* msg,u8 len);						//��������
void Senddat_tonode(u8 node_addr,int*buf,int len);
u8 Can_Receive_Msg(u8 *buf);							//��������
#endif

















