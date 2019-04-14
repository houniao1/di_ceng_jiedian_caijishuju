#ifndef UART2_H
#define UART2_H

#include "sys.h"
#define ACC 0X01//�Ӽƿ���λ
#define GYR 0X02//���ݿ���λ
#define MAG 0X04//�ų�����λ
#define RPY 0X08//ŷ���ǿ���λ
#define Q4  0X10//��Ԫ������λ
extern u8 Rxdbuf2[100];//���ݽ��ջ�����
extern u8 Rxdcnt2 ;
void USART_Send_bytes(uint8_t *Buffer, uint8_t Length);
uint8_t CHeck(uint8_t *data);
void Senddat(unsigned char *dat, unsigned char  len );
void Uart2_ConfigBaud(unsigned int baud );
void Uart2_driver(void);
void Uart2_monitor(void);
void Uart2_Senddat(unsigned char *dat, unsigned char  len );
unsigned char ReadUart(unsigned char *buf ,unsigned char len );


extern u8  USART2_RX_BUF[200]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA_2;         		//����״̬���	


#endif 



