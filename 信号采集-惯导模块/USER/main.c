#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "can.h" 
#include "elmo.h" 
#include "uart.h"
#include "time.h" 
#include "uart2.h"
#include "adc.h"

int foot_l;

u8 enable_driver = 0;
u8 Cmdarrived = 0;
u8 indx_cnt = 0;
u8 send_rad[8];
extern u8 my_addr;
extern u8 buf_adc_value[13];
int buf_adc_value_tmp[13];

void Usart_Receive_Data(u8 *buf,u8 *len);

int main(void)
 {	
	 
	u8 key = 0;
	u8 len =0;
	u8 i = 0;
	u8 cnt =0;
	u8 data_buf[12];
	int16_t ROLL=0,PITCH=0,YAW=0;
	int16_t rpy[3]={0},Acc[3]={0},Gyr[3]={0},Mag[3]={0},Q[4]={0};	
	int value[12] = {0};
	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	//Init_Time(40,7199);       //以10KHZ的频率，计数1ms定时间中断
	uart_init2(115200);	 	//串口初始化为9600

	//LED_Init();		  		//初始化与LED连接的硬件接口
	//KEy_Init();				//按键初始化		 	 
	//Elmo_Init(elmo,3); 
	//500k
	//CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_4tq,CAN_BS1_3tq,2,CAN_Mode_Normal);
	//时钟为1M
	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_4tq,CAN_BS1_3tq,1,CAN_Mode_Normal);//CAN初始化环回模式,波特率450Kbps 
	Adc_Init();		  		//ADC初始�  �
	delay_ms(10);
	//printf("node is  : %d \r\n",my_addr);
//	do 
 //{
//	 len = 0;
	 
  // USART_Send_bytes(data_buf,3);//发送欧拉角数据输出指令	
   
	// delay_ms(10);
	 
	// Usart_Receive_Data(data_buf,&len);
	 
// }while(len==0);
	
	
	while(1)
	{
    
		Get_adc();
    		
//		rpy[0] = (send_rad[0]<<8)|send_rad[1];
//		rpy[1] = (send_rad[2]<<8)|send_rad[3];
//		rpy[2] = (send_rad[4]<<8)|send_rad[5];
//		
//		if(Acc[0] != rpy[0] || rpy[2] != Acc[2] || Acc[1] != rpy[1])
//		{ 
//			  for( i=0;i<3;i++)
//			   Acc[i] = rpy[i];
//			   key = 0;
//		}
//		else
//		{
//		  key++;
//			
//			if(key >= 3)
//			{ 
//				key = 0;
//				data_buf[0]=0xa5; 

//				data_buf[1]=0x45;

//				data_buf[2]=0xea;
//			  USART_Send_bytes(data_buf,3);//发送欧拉角数据输出指令	 
//			    //delay_ms(10);
//			  	//USART_Send_bytes(data_buf,3);//发送欧拉角数据输出指令
//			   //Reset_Handler();
//			}
//		}
		
	}
}

void TIM3_IRQHandler()
{

	if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET)
	{ 	
        Get_adc();  		
	}
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	
}
void Usart_Receive_Data(u8 *buf,u8 *len)
{
	u8 rxlen = Rxdcnt2;
	u8 i=0;
	*len=0;				//默认为0
	delay_ms(4);		//等待10ms,连续超过10ms没有接收到一个数据,则认为接收结束
	if(rxlen==Rxdcnt2 && rxlen)//接收到了数据,且接收完成了
	{
		if(rxlen > 12 ) rxlen = 0;
		for(i=0;i<rxlen;i++)
		{
			buf[i]=Rxdbuf2[i];	
		}		
		*len=Rxdcnt2;	//记录本次数据长度
		Rxdcnt2=0;		//清零
	}
}

