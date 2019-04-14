#include "usart.h"
#include "stm32f10x.h"
#include "led.h"
#include "can.h" 
#include "elmo.h" 
#include "delay.h"
 extern u8 Cmdarrived;
 extern u8 enable_driver ;
 extern  u8 indx_cnt;
 u8 Rxdcnt = 0;
 u8 Rxdbuf[100];
//串口初始化，主要包括IO时钟初始化PA9,PA10端口复用，串口初始化，串口复位
//串口1
void Init_Uart(uint32_t baud)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
 	USART_DeInit(USART1);  //复位串口1
	 //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10

   //Usart1 NVIC 配置

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = baud;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART1, &USART_InitStructure); //初始化串口
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(USART1, ENABLE);                    //使能串口 


}

//初始化IO 串口2 
//bound:波特率
void uart_init2(u32 bound)
	{
    //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART2，GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
 	USART_DeInit(USART2);  //复位串口1
	 //USART1_TX   PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
   
    //USART1_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10

   //Usart1 NVIC 配置

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART2, &USART_InitStructure); //初始化串口
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(USART2, ENABLE);                    //使能串口 

}



void UART1_Send_DATA(u8 Data)
{
	 USART_SendData(USART1, Data);
	 while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);//等待发送完成
}



//串口中断1函数
void USART1_IRQHandler(void)
{
   u8 Res = 0;
   if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0
   	{
   	   Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
	   if(Rxdcnt < 100)
	   {
		  Rxdbuf[Rxdcnt++] = Res;
	   }
	} 
   
   
}

void usart1_senddat( u8 *buf,  u8 len)
{
    u8 t = 0;
	
   for(t=0; t<len; t++)
 	 {
 		  USART_SendData(USART2, buf[t]);//向串口1发送数据
 	    while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
 	 }

}
u8  ReadUart(u8 *buf ,u8 len )
{
	u8 i = 0;
   if (len > Rxdcnt)//接收到的数据长度小于要求的长度
	{	
	    len = Rxdcnt; 	
	}
	else
	{
	   
	}
	for(i=0; i<len ; i++)
	{
		*buf++ = Rxdbuf[i];   		
	}
  	Rxdcnt = 0 ;//清零计数器
    return len; 
}
void Uart1_Action(u8* buf,  u8  len)
{

	  if(buf[0]=='B'&&buf[1]=='G')
		{
		  // Elmo_NMTWrite(elmo, 0x80);		//进入Pre-Operation
	    // delay_ms(100);
		 //  Elmo_RunPPM(&elmo[0],10000,100000,1000,100,REL_POS_IMM);	
			//Elmo_RunPPM(Elmo* elmo, long speed, long position, long acc, long dec, long runMode)
		   //LED1 = ~LED1;
			enable_driver = 1;
			indx_cnt = 0;
		}
		else if (buf[0]=='S'&&buf[1]=='T') 
		{
		    Elmo_QuickStop(&elmo[0]);
		    enable_driver = 0;
		}
		else if (buf[0]=='P'&&buf[1]=='V') 
		{
     //(Elmo* elmo,  long speed, long acc, long dec, long QSDec)
		  Elmo_RunPVM(&elmo[0],80000,90000,90000, 40000);  
		}	
		else if (buf[0]=='P'&&buf[1]=='P') 
		{
			 // unsigned long speed ; 	
			//Elmo_RunPPM(elmo, 500, 9,300000,300000,  ABS_POS_IMM);
     PT_mode();
		}	
		 else if (buf[0]=='C') 
		{
			Elmo_Write(elmo, 0x6040, 0x00, 0x80);               //  清除elmo错误
			delay_ms(100);
		 	Elmo_NMTWrite(elmo, 0x80);		//进入Pre-Operation
	     delay_ms(100);
     //(Elmo* elmo,  long speed, long acc, long dec, long QSDec)
		 // Elmo_RunPVM(&elmo[0],-100000,300000,300000, 40000);  
		  // Elmo_RunPPM(elmo, 50000, 300000,300000,300000,  ABS_POS_IMM);
		   Change_mode(elmo, 3);
			 delay_ms(100);
			 Elmo_Write(elmo, 0x60FF, 0x00, 15000);
		  	Elmo_Read(&elmo[0],0x1400,1);
		}	
	 else if (buf[0]=='Q') 
		{
			  short x = -32764; 
        printf("char字节数为:%d\r\n",sizeof(char)); 
        printf("int的字节数为:%d\r\n",sizeof(int));
			  
			  printf("short的字节数为:%d\r\n",sizeof(short));
			  printf("long字节数为:%d\r\n",sizeof(long));
			printf("short x :%d\r\n",x);
		}
	 else if (buf[0]=='X') 
	 {
		 if(buf[1] =='1')
        Bigan_run();
		 else if(buf[1] =='2')
        PT_mode();
		 else if(buf[1] == '3')
      //Elmo_RunPPM(elmo, 100000,360,30000,30000, ABS_POS);
		     Elmo_RunPPM(&elmo[0], 10000,360,50000,50000,REL_POS_IMM);
		 else  if(buf[1] == '4')//正弦
		 {
         // Init_PTmode(1);
			 Init_location_control_mode();
			 				sport_mode.Timie_sys = 10;
		    sport_mode.sport_Mode = 1;
		    sport_mode.index_value = 100;
		 } 
		else  if(buf[1] == '5')
     {
        //  Init_PTmode(2);
			  Init_location_control_mode();
			 	sport_mode.Timie_sys = 20;
		    sport_mode.sport_Mode = 2;
		    sport_mode.index_value = 49;
		 }
		 else  if(buf[1] == '6')
     {
        //  Init_PTmode(2);
			  Init_location_control_mode();
			 	sport_mode.Timie_sys = 20;
		    sport_mode.sport_Mode = 3;
		    sport_mode.index_value = 52;
		 }
	 }
 }	 
void Uart1_driver(void)
{
	 u8 len  = 0;
	 u8 buf[60];
	 
	 if(Cmdarrived==1)
	 {
		Cmdarrived = 0; //命令到达标志
		LED1 = ~LED1;
	  len = ReadUart(buf,sizeof(buf));
		Uart1_Action(buf, len);
	 }
   
}


//串口监测函数，检测数据时间否接受完毕
void Uart1_monitor(void) 
{
	static u8 backcnt = 0;
	static u8  tir30ms = 0;//监控计数
	if (Rxdcnt > 0)//说明数据还没发送完
	{
		if (Rxdcnt != backcnt )
		{
		     tir30ms = 0;
			 backcnt = Rxdcnt; //保存这次的值	
		}
		else
		{
			tir30ms++;
			if(tir30ms >= 30)
			{	
			    tir30ms = 0;
			    Cmdarrived = 1;  //命令到达的标志
			}
		}


	}
	else
	{
		backcnt = 0;			

	}	

}








