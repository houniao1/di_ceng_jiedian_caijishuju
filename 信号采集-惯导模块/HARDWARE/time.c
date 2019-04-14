#include "sys.h"
#include "time.h"

 //��ʱ����ʼ��ʵ�飬��Ҫ����ʱ�ӵĳ�ʼ������ʱ���Ĳ�������Ƶϵ���������ķ�ʽ
 // ����Ϊ��������жϣ��ж����ȼ����ã�ʹ�ܶ�ʱ��
void Init_Time(u16 arr, u16 pse)//arr�������ĳ�ֵ�� pse��Ƶϵ��
{   
	  TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStruct;
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//ʹ�ܶ�ʱ��ʱ��72M
	  
	  TIM_TimeBaseInitStruct.TIM_Period = arr;//�����ĳ�ֵ
	  TIM_TimeBaseInitStruct.TIM_Prescaler = pse;//��Ƶϵ��
	  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;// ����Ϊ���¼���ģʽ
	  TIM_TimeBaseInitStruct.TIM_ClockDivision  = TIM_CKD_DIV1;//����ʱ�ӷָ�
	   
	  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);//��ʼ����ʱ���Ļ�������
	   
	  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);//��������ж�
	
	  NVIC_Time();//�������ж�
	  TIM_Cmd(TIM3 , ENABLE);//ʹ�ܶ�ʱ��
	
}

void NVIC_Time()
{   
	  NVIC_InitTypeDef NVIC_InitStruct;
	  NVIC_InitStruct.NVIC_IRQChannel  = TIM3_IRQn;//������ʱ��3���ж�
	  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority  = 2;
	  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//ʹ��ͨ��
	  NVIC_Init(&NVIC_InitStruct);//�����ж����ȼ��������

}
//void TIM3_IRQHandler()
//{
//		if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET)
//		{ 
//			LED1 = ~LED1; 
//			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	//??????
//    }
//}
//


