#include "uart2.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "elmo.h" 
#include "can.h"
#include "adc.h"

extern int buf_adc_value_tmp[13];
unsigned long g_Index;            //  全局变量,需要读的对象字典的索引
unsigned long g_SubIndex;         //  全局变量,需要读的对象字典的子索引
unsigned long g_ReadNodeID;       //  全局变量,需要读的节点ID
unsigned int Can_RX_STA = 0;
unsigned char Can_REC_LEN = 200;
u8 fished_can = 0;
u8 can_Node_buff[200];
u8 back_up_adc[8];
Can_node   node_comm;

u8 my_addr =2  ;
u8 buf_adc_value[13];
int foot_total;
extern u8 send_rad[8];
u8 force_angle[8]={0};
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
//CAN初始化
//tsjw:重新同步跳跃时间单元.范围:1~3; CAN_SJW_1tq	 CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq
//tbs2:时间段2的时间单元.范围:1~8;
//tbs1:时间段1的时间单元.范围:1~16;	  CAN_BS1_1tq ~CAN_BS1_16tq
//brp :波特率分频器.范围:1~1024;(实际要加1,也就是1~1024) tq=(brp)*tpclk1
//注意以上参数任何一个都不能设为0,否则会乱.
//波特率=Fpclk1/((tsjw+tbs1+tbs2)*brp);
//mode:0,普通模式;1,回环模式;1                                1 8 7 1   1
//Fpclk1的时钟在初始化的时候设置为36M,如果设置CAN_Normal_Init(1,8,7,5,1);
//则波特率为:36M/((1+8+7)*5)=450Kbps
//返回值:0,初始化OK;
//    其他,初始化失败;
#define CAN_RX0_INT_ENABLE	1 //使能RX0中断
u8 CAN_Mode_Init(u8 tsjw,u8 tbs2,u8 tbs1,u16 brp,u8 mode)
{                    //1          //4       //13

	GPIO_InitTypeDef GPIO_InitStructure; 
	CAN_InitTypeDef        CAN_InitStructure;
 	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
#if CAN_RX0_INT_ENABLE 
   	NVIC_InitTypeDef  NVIC_InitStructure;
#endif

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//使能PORTA时钟	                   											 

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//使能CAN1时钟	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
	GPIO_Init(GPIOA, &GPIO_InitStructure);		//初始化IO

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化IO

	//CAN单元设置
	CAN_InitStructure.CAN_TTCM=DISABLE;						 //非时间触发通信模式  //
	CAN_InitStructure.CAN_ABOM=DISABLE;						 //软件自动离线管理	 //
	CAN_InitStructure.CAN_AWUM=DISABLE;						 //睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)//
	CAN_InitStructure.CAN_NART=ENABLE;						 	//禁止报文自动传送 //
	CAN_InitStructure.CAN_RFLM=DISABLE;						 //报文不锁定,新的覆盖旧的 // 
	CAN_InitStructure.CAN_TXFP=DISABLE;						 //优先级由报文标识符决定 //
	CAN_InitStructure.CAN_Mode= mode;	         //模式设置： mode:0,普通模式;1,回环模式; //
	//设置波特率
	CAN_InitStructure.CAN_SJW=tsjw;				//重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位  CAN_SJW_1tq	 CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq
	CAN_InitStructure.CAN_BS1=tbs1; //Tbs1=tbs1+1个时间单位CAN_BS1_1tq ~CAN_BS1_16tq
	CAN_InitStructure.CAN_BS2=tbs2;//Tbs2=tbs2+1个时间单位CAN_BS2_1tq ~	CAN_BS2_8tq
	CAN_InitStructure.CAN_Prescaler=brp;            //分频系数(Fdiv)为brp+1	//
	CAN_Init(CAN1, &CAN_InitStructure);            // 初始化CAN1 

//CAN_FilterInitStructure.CAN_FilterIdHigh=(((u32)0x5678<<3)&0xFFFF0000)>>16;
//CAN_FilterInitStructure.CAN_FilterIdLow=(((u32)0x5678<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF;
//  CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0xFFFF;
//  CAN_FilterInitStructure.CAN_FilterMaskIdLow=0xFFFF;
	CAN_FilterInitStructure.CAN_FilterNumber=0;	  //过滤器0
	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask; 
	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; //32位 
	CAN_FilterInitStructure.CAN_FilterIdHigh=(((u32)my_addr<<21)&0xFFFF0000)>>16;////32位ID
	CAN_FilterInitStructure.CAN_FilterIdLow=(((u32)my_addr<<21)|CAN_ID_STD|CAN_RTR_DATA)&0xFFFF;;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0xFFFF;//32位MASK
	CAN_FilterInitStructure.CAN_FilterMaskIdLow=0xFFFF;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;//过滤器0关联到FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE; //激活过滤器0

  	CAN_FilterInit(&CAN_FilterInitStructure);//滤波器初始化
#if CAN_RX0_INT_ENABLE
	
	CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);//FIFO0消息挂号中断允许.		    
  
  	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;     // 主优先级为1
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;            // 次优先级为0
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
#endif
	return 0;
}

void Can_node_send_byte(u8 addr,u8 function ,u8 *pda,u8 len)
{
	int i;
	Can_node  node_communicate;
	node_communicate.elmoCAN.IDE = CAN_ID_STD;                    //  发送标准帧
	node_communicate.elmoCAN.StdId  = addr;         //  CAN报文IDelmo->ulNodeID
	node_communicate.elmoCAN.RTR=CAN_RTR_DATA;					   //数据帧
	node_communicate.elmoCAN.DLC = 8; 
	node_communicate.elmoCAN.Data[0] =my_addr;
	node_communicate.elmoCAN.Data[1] = function;
	node_communicate.elmoCAN.Data[2] = len;
	
  for(i=0;i<len;i++)//发送数据
	  node_communicate.elmoCAN.Data[i+3] = pda[i];
	for(;i<5;i++)//表示发送的数据不到4个字节则补零
	  node_communicate.elmoCAN.Data[len+i+3] = 0;
	CAN_Transmit(CAN1, &(node_communicate.elmoCAN));     //  调用发送报文函数
}
void Can_send_rad(u8 addr,u8 *pda)
{
	int i;
	Can_node  node_communicate;
	node_communicate.elmoCAN.IDE = CAN_ID_STD;                    //  发送标准帧
	node_communicate.elmoCAN.StdId  = addr;         //  CAN报文IDelmo->ulNodeID
	node_communicate.elmoCAN.RTR=CAN_RTR_DATA;					   //数据帧
	node_communicate.elmoCAN.DLC = 8; 
	node_communicate.elmoCAN.Data[0] = my_addr;
	node_communicate.elmoCAN.Data[1] = 25;
	for(i=0;i<6;i++)
	{
	    node_communicate.elmoCAN.Data[i+2] = pda[i];
	}
	
	CAN_Transmit(CAN1, &(node_communicate.elmoCAN));     //  调用发送报文函�
}
void Can_send_small_leg_force(u8 addr,u8 *pda)
{
	int i;
	Can_node  node_communicate;
	node_communicate.elmoCAN.IDE = CAN_ID_STD;                    //  发送标准帧
	node_communicate.elmoCAN.StdId  = addr;         //  CAN报文IDelmo->ulNodeID
	node_communicate.elmoCAN.RTR=CAN_RTR_DATA;					   //数据帧
	node_communicate.elmoCAN.DLC = 8; 
	node_communicate.elmoCAN.Data[0] = my_addr;
	node_communicate.elmoCAN.Data[1] = 24 ;
	for(i=0;i<6;i++)
	{
	    node_communicate.elmoCAN.Data[i+2] = pda[i];
	}
	
	CAN_Transmit(CAN1, &(node_communicate.elmoCAN));     //  调用发送报文函�
}
void Can_send_adc(u8 addr,u8 *pda)
{
	int i;
	Can_node  node_communicate;
	node_communicate.elmoCAN.IDE = CAN_ID_STD;                    //  发送标准帧
	node_communicate.elmoCAN.StdId  = addr;         //  CAN报文IDelmo->ulNodeID
	node_communicate.elmoCAN.RTR=CAN_RTR_DATA;					   //数据帧
	node_communicate.elmoCAN.DLC = 8; 
	node_communicate.elmoCAN.Data[0] = my_addr;
	node_communicate.elmoCAN.Data[1] = 22;
	for(i=0;i<6;i++)
	{
	    node_communicate.elmoCAN.Data[i+2] = pda[i];
	}
	
	CAN_Transmit(CAN1, &(node_communicate.elmoCAN));     //  调用发送报文函�
}
void Respond_node(char node )
{
  u8  rep[8] = {0};
	rep[0] = 'O';
	rep[1] = 'K';
	rep[2] = my_addr;
	rep[3] = 0;
	Can_node_send_byte(1,11,rep,4);
}

void Get_adc()
{
  int temp_adc  = 0;
  u8 send_len  = 0;
	int i;
  foot_total = 0;
#ifdef foot		

//0	 左脚三个值
   temp_adc = Get_Adc_Average(ADC_Channel_0,10);
   buf_adc_value[0] = (u8)(temp_adc>>8);
   buf_adc_value[1] = (u8)(temp_adc );	
   
   foot_total =	0;
//1  
   temp_adc = Get_Adc_Average(ADC_Channel_1,10); 
   buf_adc_value[2] = (u8)(temp_adc>>8);
   buf_adc_value[3] = (u8) (temp_adc );
   foot_total+=	temp_adc;	
//5	
   temp_adc = Get_Adc_Average(ADC_Channel_5,10);
   buf_adc_value[4] = (u8)(temp_adc>>8);
   buf_adc_value[5] = (u8) (temp_adc);	
   //foot_total +=	temp_adc;	 
  
// 左膝拉力
//6  
   temp_adc = Get_Adc_Average(ADC_Channel_6,10); 
   buf_adc_value[6] = (u8)(temp_adc>>8);
   buf_adc_value[7] = (u8) (temp_adc );	//1	
	 temp_adc = 0;
////7  
	temp_adc = Get_Adc_Average(ADC_Channel_7,10);
   buf_adc_value[8] = (u8)(temp_adc>>8);
   buf_adc_value[9] = (u8) (temp_adc );		
   temp_adc = 0;	 
//PB08 通道
   temp_adc = Get_Adc_Average(ADC_Channel_8,10); 
   buf_adc_value[10] = (u8)(temp_adc>>8);
   buf_adc_value[11] = (u8) (temp_adc );	
	 temp_adc = 0;
#endif 

#ifdef leg_big		

//6  
   temp_adc = Get_Adc_Average(ADC_Channel_6,10); 
   buf_adc_value[6] = (u8)(temp_adc>>8);
   buf_adc_value[7] = (u8)(temp_adc );	//1	
	 temp_adc = 0;
//7  
	 temp_adc = Get_Adc_Average(ADC_Channel_7,10);
   buf_adc_value[2] = (u8)(temp_adc>>8);
   buf_adc_value[3] = (u8)(temp_adc );		
   temp_adc = 0;	 
//PB08 通道
   temp_adc = Get_Adc_Average(ADC_Channel_8,10); 
   buf_adc_value[4] = (u8)(temp_adc>>8);
   buf_adc_value[5] = (u8)(temp_adc);	
	 temp_adc = 0;
	 
#endif 

#ifdef back		



//6  
   temp_adc = Get_Adc_Average(ADC_Channel_6,10); 
   buf_adc_value[0] = (u8)(temp_adc>>8);
   buf_adc_value[1] = (u8)(temp_adc );	//1	
	 temp_adc = 0;
//7  
	temp_adc = Get_Adc_Average(ADC_Channel_7,10);
   buf_adc_value[2] = (u8)(temp_adc>>8);
   buf_adc_value[3] = (u8)(temp_adc );		
   temp_adc = 0;	 
//PB08 通道
   temp_adc = Get_Adc_Average(ADC_Channel_8,10); 
   buf_adc_value[4] = (u8)(temp_adc>>8);
   buf_adc_value[5] = (u8)(temp_adc );	
	 temp_adc = 0;
	 
	 
#endif 

}
void Can_Action(void)		
{
	   u8 i = 0;
	   u8 buf[8] = {0};
		 u8 data_bufe[3];
		int temp_adc  = 0;
		u8 send_len  = 0;
		switch(can_Node_buff[1])//指令码
		{
		  case 11: //表示节点在请求命令
           // Respond_node(can_Node_buff[0]);
			   for(i=0;i<can_Node_buff[2];i++)
			      buf[i] = can_Node_buff[3+i];
			      Can_node_send_byte(1,11,buf,can_Node_buff[2]);
			break;
			case 22:		//返回足底数据	，背部数据	
			   Can_send_adc(1,back_up_adc);
			   for(i=0;i<6;i++)
           back_up_adc[i] =  buf_adc_value[i];	
			break;
			case 24:		//返回陀螺仪和拉力值
				 Can_send_small_leg_force(1,force_angle);
			   //for(i=0;i<6;i++)
			   
			//小腿拉力值
         force_angle[2] =  buf_adc_value[6];	
				 force_angle[3] =  buf_adc_value[7];		
         data_bufe[0] = 0xa5;
			   data_bufe[1] = 0x95;
			   data_bufe[2] = 0x3a;
			 
			// 陀螺仪数据
			   force_angle[4] =  send_rad[0];	
				 force_angle[5] =  send_rad[1]; 
			   USART_Send_bytes(data_bufe,3);//发送欧拉角数据输出指令	
			break;

			//组合形式			
	//		case 24:		
				//Can_send_adc(1,buf_adc_value);				  
//只有小腿节点有			
       //  delay_us(2);
        // Can_send_small_leg_force(1,&buf_adc_value[6]);				
			 case 25:		  
        // Can_send_rad(1,send_rad);		
			 break;
			 default: break;
			
		}
			

}
void can_diver(void)
{
   if(fished_can == 1)//接收完成
	 {
	     	fished_can = 0;
		    LED1 = ~LED1;
        Can_Action();
	 }

}

 
#if CAN_RX0_INT_ENABLE	//使能RX0中断
//中断服务函数			    
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  CanRxMsg RxMessage;
  unsigned char  i=0;
	
	CAN_Receive(CAN1, 0, &RxMessage);
	  for(i=0;i<8;i++)
	  {
			can_Node_buff[i]=RxMessage.Data[i] ;
	  }	
	  Can_Action();
  // fished_can =1;		
}
#endif

//can发送一组数据(固定格式:ID为0X12,标准帧,数据帧)	
//len:数据长度(最大为8)				     
//msg:数据指针,最大为8个字节.
//返回值:0,成功;
//		 其他,失败;
u8 Can_Send_Msg(u8* msg,u8 len)
{	
  u8 mbox;
  u16 i=0;
  CanTxMsg TxMessage;
  TxMessage.StdId=0x12; // 标准标识符为0
  TxMessage.ExtId=0x12;	// 设置扩展标示符（29位）
  TxMessage.IDE=0;// 使用扩展标识符
  TxMessage.RTR=0; // 消息类型为数据帧，一帧8位
  TxMessage.DLC=len;							 // 发送两帧信息
  for(i=0;i<len;i++)
  TxMessage.Data[i]=msg[i];				 // 第一帧信息          
  mbox= CAN_Transmit(CAN1, &TxMessage);   
  i=0;
  while((CAN_TransmitStatus(CAN1, mbox)==CAN_TxStatus_Failed)&&(i<0XFFF))i++;	//等待发送结束
  if(i>=0XFFF)return 1;
  return 0;		

}
//can口接收数据查询
//buf:数据缓存区;	 
//返回值:0,无数据被收到;
//		 其他,接收的数据长度;
u8 Can_Receive_Msg(u8 *buf)
{		   		   
 	u32 i;
	CanRxMsg RxMessage;
    if( CAN_MessagePending(CAN1,CAN_FIFO0)==0)return 0;		//没有接收到数据,直接退出 
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);//读取数据	
    for(i=0;i<8;i++)
    buf[i]=RxMessage.Data[i];  
	return RxMessage.DLC;	
}









