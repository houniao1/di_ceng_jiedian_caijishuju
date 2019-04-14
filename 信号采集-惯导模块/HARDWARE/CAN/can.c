#include "uart2.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "elmo.h" 
#include "can.h"
#include "adc.h"

extern int buf_adc_value_tmp[13];
unsigned long g_Index;            //  ȫ�ֱ���,��Ҫ���Ķ����ֵ������
unsigned long g_SubIndex;         //  ȫ�ֱ���,��Ҫ���Ķ����ֵ��������
unsigned long g_ReadNodeID;       //  ȫ�ֱ���,��Ҫ���Ľڵ�ID
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
//CAN��ʼ��
//tsjw:����ͬ����Ծʱ�䵥Ԫ.��Χ:1~3; CAN_SJW_1tq	 CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq
//tbs2:ʱ���2��ʱ�䵥Ԫ.��Χ:1~8;
//tbs1:ʱ���1��ʱ�䵥Ԫ.��Χ:1~16;	  CAN_BS1_1tq ~CAN_BS1_16tq
//brp :�����ʷ�Ƶ��.��Χ:1~1024;(ʵ��Ҫ��1,Ҳ����1~1024) tq=(brp)*tpclk1
//ע�����ϲ����κ�һ����������Ϊ0,�������.
//������=Fpclk1/((tsjw+tbs1+tbs2)*brp);
//mode:0,��ͨģʽ;1,�ػ�ģʽ;1                                1 8 7 1   1
//Fpclk1��ʱ���ڳ�ʼ����ʱ������Ϊ36M,�������CAN_Normal_Init(1,8,7,5,1);
//������Ϊ:36M/((1+8+7)*5)=450Kbps
//����ֵ:0,��ʼ��OK;
//    ����,��ʼ��ʧ��;
#define CAN_RX0_INT_ENABLE	1 //ʹ��RX0�ж�
u8 CAN_Mode_Init(u8 tsjw,u8 tbs2,u8 tbs1,u16 brp,u8 mode)
{                    //1          //4       //13

	GPIO_InitTypeDef GPIO_InitStructure; 
	CAN_InitTypeDef        CAN_InitStructure;
 	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
#if CAN_RX0_INT_ENABLE 
   	NVIC_InitTypeDef  NVIC_InitStructure;
#endif

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//ʹ��PORTAʱ��	                   											 

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//ʹ��CAN1ʱ��	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);		//��ʼ��IO

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��IO

	//CAN��Ԫ����
	CAN_InitStructure.CAN_TTCM=DISABLE;						 //��ʱ�䴥��ͨ��ģʽ  //
	CAN_InitStructure.CAN_ABOM=DISABLE;						 //����Զ����߹���	 //
	CAN_InitStructure.CAN_AWUM=DISABLE;						 //˯��ģʽͨ���������(���CAN->MCR��SLEEPλ)//
	CAN_InitStructure.CAN_NART=ENABLE;						 	//��ֹ�����Զ����� //
	CAN_InitStructure.CAN_RFLM=DISABLE;						 //���Ĳ�����,�µĸ��Ǿɵ� // 
	CAN_InitStructure.CAN_TXFP=DISABLE;						 //���ȼ��ɱ��ı�ʶ������ //
	CAN_InitStructure.CAN_Mode= mode;	         //ģʽ���ã� mode:0,��ͨģʽ;1,�ػ�ģʽ; //
	//���ò�����
	CAN_InitStructure.CAN_SJW=tsjw;				//����ͬ����Ծ���(Tsjw)Ϊtsjw+1��ʱ�䵥λ  CAN_SJW_1tq	 CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq
	CAN_InitStructure.CAN_BS1=tbs1; //Tbs1=tbs1+1��ʱ�䵥λCAN_BS1_1tq ~CAN_BS1_16tq
	CAN_InitStructure.CAN_BS2=tbs2;//Tbs2=tbs2+1��ʱ�䵥λCAN_BS2_1tq ~	CAN_BS2_8tq
	CAN_InitStructure.CAN_Prescaler=brp;            //��Ƶϵ��(Fdiv)Ϊbrp+1	//
	CAN_Init(CAN1, &CAN_InitStructure);            // ��ʼ��CAN1 

//CAN_FilterInitStructure.CAN_FilterIdHigh=(((u32)0x5678<<3)&0xFFFF0000)>>16;
//CAN_FilterInitStructure.CAN_FilterIdLow=(((u32)0x5678<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF;
//  CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0xFFFF;
//  CAN_FilterInitStructure.CAN_FilterMaskIdLow=0xFFFF;
	CAN_FilterInitStructure.CAN_FilterNumber=0;	  //������0
	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask; 
	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; //32λ 
	CAN_FilterInitStructure.CAN_FilterIdHigh=(((u32)my_addr<<21)&0xFFFF0000)>>16;////32λID
	CAN_FilterInitStructure.CAN_FilterIdLow=(((u32)my_addr<<21)|CAN_ID_STD|CAN_RTR_DATA)&0xFFFF;;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0xFFFF;//32λMASK
	CAN_FilterInitStructure.CAN_FilterMaskIdLow=0xFFFF;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;//������0������FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE; //���������0

  	CAN_FilterInit(&CAN_FilterInitStructure);//�˲�����ʼ��
#if CAN_RX0_INT_ENABLE
	
	CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);//FIFO0��Ϣ�Һ��ж�����.		    
  
  	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;     // �����ȼ�Ϊ1
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;            // �����ȼ�Ϊ0
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
#endif
	return 0;
}

void Can_node_send_byte(u8 addr,u8 function ,u8 *pda,u8 len)
{
	int i;
	Can_node  node_communicate;
	node_communicate.elmoCAN.IDE = CAN_ID_STD;                    //  ���ͱ�׼֡
	node_communicate.elmoCAN.StdId  = addr;         //  CAN����IDelmo->ulNodeID
	node_communicate.elmoCAN.RTR=CAN_RTR_DATA;					   //����֡
	node_communicate.elmoCAN.DLC = 8; 
	node_communicate.elmoCAN.Data[0] =my_addr;
	node_communicate.elmoCAN.Data[1] = function;
	node_communicate.elmoCAN.Data[2] = len;
	
  for(i=0;i<len;i++)//��������
	  node_communicate.elmoCAN.Data[i+3] = pda[i];
	for(;i<5;i++)//��ʾ���͵����ݲ���4���ֽ�����
	  node_communicate.elmoCAN.Data[len+i+3] = 0;
	CAN_Transmit(CAN1, &(node_communicate.elmoCAN));     //  ���÷��ͱ��ĺ���
}
void Can_send_rad(u8 addr,u8 *pda)
{
	int i;
	Can_node  node_communicate;
	node_communicate.elmoCAN.IDE = CAN_ID_STD;                    //  ���ͱ�׼֡
	node_communicate.elmoCAN.StdId  = addr;         //  CAN����IDelmo->ulNodeID
	node_communicate.elmoCAN.RTR=CAN_RTR_DATA;					   //����֡
	node_communicate.elmoCAN.DLC = 8; 
	node_communicate.elmoCAN.Data[0] = my_addr;
	node_communicate.elmoCAN.Data[1] = 25;
	for(i=0;i<6;i++)
	{
	    node_communicate.elmoCAN.Data[i+2] = pda[i];
	}
	
	CAN_Transmit(CAN1, &(node_communicate.elmoCAN));     //  ���÷��ͱ��ĺ��
}
void Can_send_small_leg_force(u8 addr,u8 *pda)
{
	int i;
	Can_node  node_communicate;
	node_communicate.elmoCAN.IDE = CAN_ID_STD;                    //  ���ͱ�׼֡
	node_communicate.elmoCAN.StdId  = addr;         //  CAN����IDelmo->ulNodeID
	node_communicate.elmoCAN.RTR=CAN_RTR_DATA;					   //����֡
	node_communicate.elmoCAN.DLC = 8; 
	node_communicate.elmoCAN.Data[0] = my_addr;
	node_communicate.elmoCAN.Data[1] = 24 ;
	for(i=0;i<6;i++)
	{
	    node_communicate.elmoCAN.Data[i+2] = pda[i];
	}
	
	CAN_Transmit(CAN1, &(node_communicate.elmoCAN));     //  ���÷��ͱ��ĺ��
}
void Can_send_adc(u8 addr,u8 *pda)
{
	int i;
	Can_node  node_communicate;
	node_communicate.elmoCAN.IDE = CAN_ID_STD;                    //  ���ͱ�׼֡
	node_communicate.elmoCAN.StdId  = addr;         //  CAN����IDelmo->ulNodeID
	node_communicate.elmoCAN.RTR=CAN_RTR_DATA;					   //����֡
	node_communicate.elmoCAN.DLC = 8; 
	node_communicate.elmoCAN.Data[0] = my_addr;
	node_communicate.elmoCAN.Data[1] = 22;
	for(i=0;i<6;i++)
	{
	    node_communicate.elmoCAN.Data[i+2] = pda[i];
	}
	
	CAN_Transmit(CAN1, &(node_communicate.elmoCAN));     //  ���÷��ͱ��ĺ��
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

//0	 �������ֵ
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
  
// ��ϥ����
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
//PB08 ͨ��
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
//PB08 ͨ��
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
//PB08 ͨ��
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
		switch(can_Node_buff[1])//ָ����
		{
		  case 11: //��ʾ�ڵ�����������
           // Respond_node(can_Node_buff[0]);
			   for(i=0;i<can_Node_buff[2];i++)
			      buf[i] = can_Node_buff[3+i];
			      Can_node_send_byte(1,11,buf,can_Node_buff[2]);
			break;
			case 22:		//�����������	����������	
			   Can_send_adc(1,back_up_adc);
			   for(i=0;i<6;i++)
           back_up_adc[i] =  buf_adc_value[i];	
			break;
			case 24:		//���������Ǻ�����ֵ
				 Can_send_small_leg_force(1,force_angle);
			   //for(i=0;i<6;i++)
			   
			//С������ֵ
         force_angle[2] =  buf_adc_value[6];	
				 force_angle[3] =  buf_adc_value[7];		
         data_bufe[0] = 0xa5;
			   data_bufe[1] = 0x95;
			   data_bufe[2] = 0x3a;
			 
			// ����������
			   force_angle[4] =  send_rad[0];	
				 force_angle[5] =  send_rad[1]; 
			   USART_Send_bytes(data_bufe,3);//����ŷ�����������ָ��	
			break;

			//�����ʽ			
	//		case 24:		
				//Can_send_adc(1,buf_adc_value);				  
//ֻ��С�Ƚڵ���			
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
   if(fished_can == 1)//�������
	 {
	     	fished_can = 0;
		    LED1 = ~LED1;
        Can_Action();
	 }

}

 
#if CAN_RX0_INT_ENABLE	//ʹ��RX0�ж�
//�жϷ�����			    
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

//can����һ������(�̶���ʽ:IDΪ0X12,��׼֡,����֡)	
//len:���ݳ���(���Ϊ8)				     
//msg:����ָ��,���Ϊ8���ֽ�.
//����ֵ:0,�ɹ�;
//		 ����,ʧ��;
u8 Can_Send_Msg(u8* msg,u8 len)
{	
  u8 mbox;
  u16 i=0;
  CanTxMsg TxMessage;
  TxMessage.StdId=0x12; // ��׼��ʶ��Ϊ0
  TxMessage.ExtId=0x12;	// ������չ��ʾ����29λ��
  TxMessage.IDE=0;// ʹ����չ��ʶ��
  TxMessage.RTR=0; // ��Ϣ����Ϊ����֡��һ֡8λ
  TxMessage.DLC=len;							 // ������֡��Ϣ
  for(i=0;i<len;i++)
  TxMessage.Data[i]=msg[i];				 // ��һ֡��Ϣ          
  mbox= CAN_Transmit(CAN1, &TxMessage);   
  i=0;
  while((CAN_TransmitStatus(CAN1, mbox)==CAN_TxStatus_Failed)&&(i<0XFFF))i++;	//�ȴ����ͽ���
  if(i>=0XFFF)return 1;
  return 0;		

}
//can�ڽ������ݲ�ѯ
//buf:���ݻ�����;	 
//����ֵ:0,�����ݱ��յ�;
//		 ����,���յ����ݳ���;
u8 Can_Receive_Msg(u8 *buf)
{		   		   
 	u32 i;
	CanRxMsg RxMessage;
    if( CAN_MessagePending(CAN1,CAN_FIFO0)==0)return 0;		//û�н��յ�����,ֱ���˳� 
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);//��ȡ����	
    for(i=0;i<8;i++)
    buf[i]=RxMessage.Data[i];  
	return RxMessage.DLC;	
}









