#include "sys.h"
#include "uart.h"
#include "uart2.h"
#include "string.h"
#include "delay.h"
#include "usart.h"


uint8_t stata_reg=0;//接收状态寄存器
uint8_t data1_buf[7]={0},data2_buf[7]={0},data3_buf[7]={0},

data4_buf[7]={0},data5_buf[9]={0};
extern u8 send_rad[8];
u8 Rxdbuf2[100];//数据接收缓冲区
u8 Rxdcnt2 = 0;
u8 Cmdarrived2 = 0;
//u8  my_addr[2]={0X00};//4662是水位节点


void Uart2_ConfigBaud(unsigned int baud )
{
	//GPIO????	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//??USART2,GPIOA??
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //?? GPIOA??
	
	USART_DeInit(USART2);  //????1

	//USART2_TX   PA.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//??????
	GPIO_Init(GPIOA, &GPIO_InitStructure); //???PA9
	
	//USART2_RX	  PA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//????
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //???PA10
	
	//Usart2 NVIC ??
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//?????3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;		//????3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ????
	NVIC_Init(&NVIC_InitStructure);	//??????????VIC???
	
	//USART ?????
	
	USART_InitStructure.USART_BaudRate = baud;//?????9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//???8?????
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//?????
	USART_InitStructure.USART_Parity = USART_Parity_No;//??????
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//????????
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//????
	
	USART_Init(USART2, &USART_InitStructure); //?????
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//????
	USART_Cmd(USART2, ENABLE);                    //???? 
	
}
void USART_send_byte(uint8_t byte)
{
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);//等待发送完成
	USART2->DR=byte;	
}
void USART_Send_bytes(uint8_t *Buffer, uint8_t Length)
{
	uint8_t i=0;
	while(i<Length)
	{
		USART_send_byte(Buffer[i++]);
	}
}
uint8_t CHeck(uint8_t *data)
{
  uint8_t i=0,flag=0,length=0,sum=0x5a+0x5a;
	if(stata_reg)
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_9);//LED亮
		if(stata_reg&ACC)//判断加数度数据接收完毕
		{
			stata_reg^=ACC;//清标志位
			flag=0x15;//记录功能字节
			length=6;//记录数据长度
			/*转移数据到data，避免串口中断对data1_buf的影响*/
		  memcpy(data,data1_buf,sizeof(data1_buf));
		  sum=sum+flag+length;//累加帧头、功能字节、数据长度
		}
		else if(stata_reg&GYR)
		{
			stata_reg^=GYR;
			flag=0x25;
			length=6;
		  memcpy(data,data2_buf,sizeof(data2_buf));
		  sum=sum+flag+length;
		}
		else if(stata_reg&MAG)
		{
			stata_reg^=MAG;
			flag=0x35;
			length=6;
		  memcpy(data,data3_buf,sizeof(data3_buf));
		  sum=sum+flag+length;
		}
		else if(stata_reg&RPY)
		{
			stata_reg^=RPY;
			flag=0x45;
			length=6;
		  memcpy(data,data4_buf,sizeof(data4_buf));
		  sum=sum+flag+length;
		}
		else if(stata_reg&Q4)
		{
			stata_reg^=Q4;
			flag=0x65;
			length=8;
		  memcpy(data,data5_buf,sizeof(data5_buf));
		  sum=sum+flag+length;
		}
		for(i=0;i<length;i++)//累加数据
		{
		 sum+=data[i];
		}
		if(sum!=data[i])//判断校验和是否正确
		return 0;
		else
			return flag;//返回功能字节	
	}
	else
		return 0;
}



//串口监测函数，检测数据时间否接受完毕
void Uart2_monitor(void) 
{
	static u8 backcnt = 0;
	static u8  tir30ms = 0;//监控计数
	if (Rxdcnt2 > 0)//说明数据还没发送完
	{
		if (Rxdcnt2 != backcnt )
		{
		     tir30ms = 0;
			 backcnt = Rxdcnt2; //保存这次的值	
		}
		else
		{
			tir30ms++;
			if(tir30ms >= 30)
			{	
			    tir30ms = 0;
			    Cmdarrived2 = 1;  //命令到达的标志
			}
		}


	}
	else
	{
		backcnt = 0;			

	}	

}

//把一帧数据读出来，buf使用来存放数据的缓冲区，len是要读的长度
u8  ReadUart2(u8 *buf ,u8 len )
{
	u8 i = 0;
   if (len > Rxdcnt2)//接收到的数据长度小于要求的长度
	{	
	    len = Rxdcnt2; 	
	}
	else
	{
	   
	}
	for(i=0; i<len ; i++)
	{
		*buf++ = Rxdbuf2[i];   		
	}
  	Rxdcnt2 = 0 ;//清零计数器
    return len; 
}

//串口发送数据
void Uart2_Senddat(unsigned char *dat, unsigned char  len )
{
	while(len--)
	{
	   USART_SendData(USART2, *dat++);
	   while(USART_GetFlagStatus(USART2,USART_FLAG_TC) != SET);
	}

}

void Uart2_Action(u8* buf,  u8  len)
{
	u16 temp_addr = 0;
  u8 int_buf[ 6 ] = {0xC0, 0x12, 0x36, 0x1A, 0x17, 0xC4 };
	if(buf[0] == 'X')//设置主模块的地址
	 {   
	 }

}
//串口驱动函数，负责把数据读出来并执行相应的操作
void Uart2_driver(void)
{
	 u8 len  = 0;
	 u8 buf[60];
	 
	 if(Cmdarrived2==1)
	 {
		 Cmdarrived2 = 0; //命令到达标志
	
	   len = ReadUart2(buf,sizeof(buf));
		 //Uart2_Action(buf,len);
	 }

}


void USART2_IRQHandler(void)
{
   u8 Res = 0;
	 static uint8_t rebuf[13]={0},i=0;
	 static u8 x=0;
   if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0
   {
   	 Res =USART_ReceiveData(USART2);//(USART1->DR);	//读取接收到的数据
	   if(Rxdcnt2 < 100)
	   {
		    Rxdbuf2[Rxdcnt2++] = Res;
	   }
		 if(Rxdcnt2==11 && Rxdbuf2[0]==Rxdbuf2[1])
		 {
		     	send_rad[0] = Rxdbuf2[4]; 
					send_rad[1] = Rxdbuf2[5]; 
			    Rxdcnt2 = 0;
		 }
		 else if(Rxdcnt2>11 )
		 {
		 
		   Rxdcnt2 =0;
		 }
	} 
} 

