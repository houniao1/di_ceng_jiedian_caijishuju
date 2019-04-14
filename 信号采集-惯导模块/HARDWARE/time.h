#ifndef TIME_H
#define TIME_H

#include "stm32f10x.h"


void NVIC_Time(void);
void Init_Time(u16 arr, u16 pse);//arr计数器的初值， pse分频系数

#endif


