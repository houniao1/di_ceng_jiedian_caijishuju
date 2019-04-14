#include "sys.h"
#include "uart.h"
#include "uart2.h"
#include "string.h"
#include "delay.h"
#include "usart.h"


uint8_t stata_reg=0;//����״̬�Ĵ���
uint8_t data1_buf[7]={0},data2_buf[7]={0},data3_buf[7]={0},

data4_buf[7]={0},data5_buf[9]={0};
extern u8 send_rad[8];
u8 Rxdbuf2[100];//���ݽ��ջ�����
u8 Rxdcnt2 = 0;
u8 Cmdarrived2 = 0;
//u8  my_addr[2]={0X00};//4662��ˮλ�ڵ�


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
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);//�ȴ��������
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
		GPIO_ResetBits(GPIOB,GPIO_Pin_9);//LED��
		if(stata_reg&ACC)//�жϼ��������ݽ������
		{
			stata_reg^=ACC;//���־λ
			flag=0x15;//��¼�����ֽ�
			length=6;//��¼���ݳ���
			/*ת�����ݵ�data�����⴮���ж϶�data1_buf��Ӱ��*/
		  memcpy(data,data1_buf,sizeof(data1_buf));
		  sum=sum+flag+length;//�ۼ�֡ͷ�������ֽڡ����ݳ���
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
		for(i=0;i<length;i++)//�ۼ�����
		{
		 sum+=data[i];
		}
		if(sum!=data[i])//�ж�У����Ƿ���ȷ
		return 0;
		else
			return flag;//���ع����ֽ�	
	}
	else
		return 0;
}



//���ڼ�⺯�����������ʱ���������
void Uart2_monitor(void) 
{
	static u8 backcnt = 0;
	static u8  tir30ms = 0;//��ؼ���
	if (Rxdcnt2 > 0)//˵�����ݻ�û������
	{
		if (Rxdcnt2 != backcnt )
		{
		     tir30ms = 0;
			 backcnt = Rxdcnt2; //������ε�ֵ	
		}
		else
		{
			tir30ms++;
			if(tir30ms >= 30)
			{	
			    tir30ms = 0;
			    Cmdarrived2 = 1;  //�����ı�־
			}
		}


	}
	else
	{
		backcnt = 0;			

	}	

}

//��һ֡���ݶ�������bufʹ����������ݵĻ�������len��Ҫ���ĳ���
u8  ReadUart2(u8 *buf ,u8 len )
{
	u8 i = 0;
   if (len > Rxdcnt2)//���յ������ݳ���С��Ҫ��ĳ���
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
  	Rxdcnt2 = 0 ;//���������
    return len; 
}

//���ڷ�������
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
	if(buf[0] == 'X')//������ģ��ĵ�ַ
	 {   
	 }

}
//����������������������ݶ�������ִ����Ӧ�Ĳ���
void Uart2_driver(void)
{
	 u8 len  = 0;
	 u8 buf[60];
	 
	 if(Cmdarrived2==1)
	 {
		 Cmdarrived2 = 0; //������־
	
	   len = ReadUart2(buf,sizeof(buf));
		 //Uart2_Action(buf,len);
	 }

}


void USART2_IRQHandler(void)
{
   u8 Res = 0;
	 static uint8_t rebuf[13]={0},i=0;
	 static u8 x=0;
   if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0
   {
   	 Res =USART_ReceiveData(USART2);//(USART1->DR);	//��ȡ���յ�������
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

