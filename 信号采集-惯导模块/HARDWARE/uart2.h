#ifndef UART2_H
#define UART2_H

#include "sys.h"
#define ACC 0X01//加计控制位
#define GYR 0X02//陀螺控制位
#define MAG 0X04//磁场控制位
#define RPY 0X08//欧拉角控制位
#define Q4  0X10//四元数控制位
extern u8 Rxdbuf2[100];//数据接收缓冲区
extern u8 Rxdcnt2 ;
void USART_Send_bytes(uint8_t *Buffer, uint8_t Length);
uint8_t CHeck(uint8_t *data);
void Senddat(unsigned char *dat, unsigned char  len );
void Uart2_ConfigBaud(unsigned int baud );
void Uart2_driver(void);
void Uart2_monitor(void);
void Uart2_Senddat(unsigned char *dat, unsigned char  len );
unsigned char ReadUart(unsigned char *buf ,unsigned char len );


extern u8  USART2_RX_BUF[200]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA_2;         		//接收状态标记	


#endif 



