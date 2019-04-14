#include "sys.h"
#include "time.h"

 //定时器初始化实验，主要包括时钟的初始化，定时器的参数，分频系数，计数的方式
 // 设置为允许更新中断，中断优先级设置，使能定时器
void Init_Time(u16 arr, u16 pse)//arr计数器的初值， pse分频系数
{   
	  TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStruct;
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//使能定时器时钟72M
	  
	  TIM_TimeBaseInitStruct.TIM_Period = arr;//计数的初值
	  TIM_TimeBaseInitStruct.TIM_Prescaler = pse;//分频系数
	  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;// 设置为向下计数模式
	  TIM_TimeBaseInitStruct.TIM_ClockDivision  = TIM_CKD_DIV1;//设置时钟分割
	   
	  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);//初始化定时器的基本参数
	   
	  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);//允许更新中断
	
	  NVIC_Time();//开启总中断
	  TIM_Cmd(TIM3 , ENABLE);//使能定时器
	
}

void NVIC_Time()
{   
	  NVIC_InitTypeDef NVIC_InitStruct;
	  NVIC_InitStruct.NVIC_IRQChannel  = TIM3_IRQn;//开启定时器3的中断
	  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority  = 2;
	  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//使能通道
	  NVIC_Init(&NVIC_InitStruct);//设置中断优先级分组管理

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


