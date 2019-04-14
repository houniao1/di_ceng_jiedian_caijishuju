
/******************************************************************************
ע��:
    1���޸�startup_ewarm�е��жϺ���ע��,��CAN���ж�ע��
        (��ȻҲ���Բ�ע���ж�)
    2����ͬʱ����  my_can.c��my_can.h��ELMO.c��ELMO.h
       ��ͬʱ����װĿ¼��can.c��can.h���ǣ��������ʱ���ܻᱨ��
    3��elmoһ��״̬֮���ת������Ϊ3ms
*******************************************************************************/

#include "ELMO.h"
#include "delay.h"
u8  Current_um = 0;
Elm0_sport sport_mode = {21,1};		
		


void Delay_100us(u32 ntime)
{
   while(ntime)
	 {
      ntime--;
     delay_us(100);
    }
     

}
		
/******************************************************************************
                          CANͨ����ر���
*******************************************************************************/
/*
tCANFrameBuff ReceFrameBuff;			    //  ����֡���ջ�����
tCANFrameBuff SendFrameBuff;                        //  ����֡���ͻ�����
tCANFrameBuff *pReceFrameBuff = &ReceFrameBuff;	    //  ����֡���ջ�����ָ��
tCANFrameBuff *pSendFrameBuff = &SendFrameBuff;     //  ����֡���ͻ�����ָ��
*/

/*******************************************************************************
                          ELMOͨ����ر���
  ע�⣺ �޸Ĵ˴�ʱ��Ҫͬʱ�޸�ELMO.h�е�����
*******************************************************************************/
Elmo   node_communicate[4];
Elmo      elmo[6];                                  // ����Elmo�ṹ������,��elmo������Ӧ 
unsigned int Elmo_PDOCOBID[4] = {0x27f,0x37f,0x47f,0x57f};	//�ĸ�RPDO�Ľ���COBID                             
/*******************************************************************************
                        ��ʼ��CANӲ��
*******************************************************************************/


/*******************************************************************************
��ڲ���: elmo          ָ��elmo�ṹ���ָ��
          Index         �����ֵ�����
          SubIndex      �����ֵ�������
          Param         �����ֵ��Ӧ������ ֵ
*******************************************************************************/
void Elmo_Write(Elmo* elmo, unsigned long Index, unsigned char SubIndex, long Param)
{
	elmo->ulNodeID = 0X7F; 
  elmo->elmoCAN.IDE = CAN_ID_STD;                    //  ���ͱ�׼֡
  elmo->elmoCAN.StdId  = 0x600+elmo->ulNodeID;         //  CAN����IDelmo->ulNodeID
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   //����֡
  elmo->elmoCAN.DLC = 8;                            //  CAN�������ݳ���
  elmo->elmoCAN.Data[0]= 0x22;                    //  CAN����ǰ׺ 
  elmo->elmoCAN.Data[1]= Index&0xFF;              //  ����Index���ֽ�
  elmo->elmoCAN.Data[2]= (Index&0xFF00)>>8;       //  ����Index���ֽ�
  elmo->elmoCAN.Data[3]= SubIndex;                //  ����SubIndex
  elmo->elmoCAN.Data[4]= Param&0xFF;
  elmo->elmoCAN.Data[5]= (Param&0xFF00)>>8;
  elmo->elmoCAN.Data[6]= (Param&0xFF0000)>>16;
  elmo->elmoCAN.Data[7]= (Param&0xFF000000)>>24;  //  ���η��Ͷ�������
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     //  ���÷��ͱ��ĺ���
}

void my_elmo_len(Elmo* elmo, unsigned char*  x1, unsigned char len)
{
	 unsigned char i =0;
	elmo->ulNodeID = 0X7F; 
  elmo->elmoCAN.IDE = CAN_ID_STD;                    //  ���ͱ�׼֡
  elmo->elmoCAN.StdId  = 0x300+elmo->ulNodeID;         //  CAN����IDelmo->ulNodeID
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   //����֡
  elmo->elmoCAN.DLC = len;	//  CAN�������ݳ���
   for(i=0;i<len;i++)
     elmo->elmoCAN.Data[i]= x1[i];  
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     //  ���÷��ͱ��ĺ���
}

/*******************************************************************************
��ڲ���:                                                  
          elmo          ָ��elmo�ṹ���ָ��
          Index         �����ֵ�����
          SubIndex      �����ֵ�������
*******************************************************************************/
void Elmo_Read(Elmo* elmo, unsigned long Index, unsigned char SubIndex)
{
  g_Index = Index;
  g_SubIndex = SubIndex;
  g_ReadNodeID = elmo->ulNodeID;                     	//  �޸�ȫ�ֱ���,���ж�ʹ��
  elmo->elmoCAN.IDE = CAN_ID_STD;                    	//  ���ͱ�׼֡
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					  	//  ����֡
  elmo->elmoCAN.StdId  = 0x600+elmo->ulNodeID;         	//  CAN����ID 
  elmo->elmoCAN.DLC = 8;                            	//  CAN�������ݳ���
  elmo->elmoCAN.Data[0]= 0x40;                    		//  CAN����ǰ׺,�������ϵõ�  �����ȡ�����ֵ�0x40
  elmo->elmoCAN.Data[1]= Index&0xFF;              		//  ����Index���ֽ�
  elmo->elmoCAN.Data[2]= (Index&0xFF00)>>8;       		//  ����Index���ֽ�
  elmo->elmoCAN.Data[3]= SubIndex;                		//  ����SubIndex
  elmo->elmoCAN.Data[4]= 0;
  elmo->elmoCAN.Data[5]= 0;
  elmo->elmoCAN.Data[6]= 0;
  elmo->elmoCAN.Data[7]= 0;                       		//  �������ݱ���
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     			//  ���÷��ͱ��ĺ���
}


/*******************************************************************************
                        ��ʼ��elmo����
��ڲ���:
          elmo          ָ��elmo�ṹ���ָ��
*******************************************************************************/
void Elmo_ParamInit(Elmo* elmo)
{
   Elmo_Write(elmo, 0x6040, 0x00, 0x80);               //  ���elmo����
  Delay_100us(Delay2);
  Elmo_Write(elmo, 0x6065, 0x00, MAX_F_ERR);          //  PPMode�µ�ǰλ�ú�ʵ��λ�õ����������� 
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x607D, 0x01, MIN_P_LIMIT);        //  PPMode����Сλ�ü���
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x607D, 0x02, MAX_P_LIMIT);        //  PPMode�����λ�ü���
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x607E, 0x00, POLARITY  );         //  λ�á��ٶȼ��� (������Ҫ�޸�)
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x607F, 0x00, MAX_SPEED);          //  PPMode��PVMode�����ܴﵽ������ٶ�
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x6083, 0x00, MAX_ACC);            //  �����ٶ�
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x6084, 0x00, MAX_ACC);            //  �����ٶ�
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x6085, 0x00, MAX_QDEC);           //  ����ģʽ��,elmo����ʱ�����ͣ���ٶ�
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x6086, 0x00, MOTION_PROFILE_TYPE);//  PPMʵ�ַ�ʽ;0:Linear ramp (trapezoidal profile)     
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x6088, 0x00, 0);                  //  PTMʵ�ַ�ʽ,ELMOֻ֧�� linear ramp                 
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x2f41, 0x00, 0x02);               //  ����PPM 
  Elmo_Delay(Delay1);
}

void my_sdo_Write(Elmo* elmo, unsigned long Index, unsigned char SubIndex, unsigned char *x )
{
	unsigned char i = 0;
	elmo->ulNodeID = 0X7F; 
  elmo->elmoCAN.IDE = CAN_ID_STD;                    //  ���ͱ�׼֡
  elmo->elmoCAN.StdId  = 0x600+elmo->ulNodeID;         //  CAN����IDelmo->ulNodeID
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   //����֡
  elmo->elmoCAN.DLC = 8;                            //  CAN�������ݳ���
  elmo->elmoCAN.Data[0]= 0x22;                    //  CAN����ǰ׺ 
  elmo->elmoCAN.Data[1]= Index&0xFF;              //  ����Index���ֽ�
  elmo->elmoCAN.Data[2]= (Index&0xFF00)>>8;       //  ����Index���ֽ�
  elmo->elmoCAN.Data[3]= SubIndex;                //  ����SubIndex
  for(i=0;i<4;i++)
	  elmo->elmoCAN.Data[4+i] = x[i];
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     //  ���÷��ͱ��ĺ���
}

/*******************************************************************************
                        elmo_PDO��ʼ��ӳ��
��ڲ���:
          elmo          ָ��elmo�ṹ���ָ��
˵��	�����ڿ�����ҪŶ�õ��ٶ�ģʽ������ֻ���ٶ�ģʽ������ӳ��
			����RPDO0(0x27F)�����㲥֡��3��elmo�����յ���
				RPDOi(0x37f,0x47f,0x57f)����i(1~3)��elmoӳ��
*******************************************************************************/
void my_Elmo_PDOInit(Elmo * elmo)
{
	   	
	
			unsigned char init_pdo[][4] =  {
				 0x01,0x02,0x00,0x08,    //diaable rpdo1  
			  0x00,0x00,0x00,0x08,
				0x01,0x00,0x00,0x00,
				0x01,0x02,0x00,0x00,
			};
	
	   Elmo_NMTWrite(elmo, 0x80);								//����Pre-Operation
	   Delay_100us(Delay3);
  
	
	  my_sdo_Write(elmo,(0x1400+0),0x01,&init_pdo[0][0]);			  //������ͨѶ��������
		Delay_100us(Delay3);
		my_sdo_Write(elmo,(0x1600+0),0x00,&init_pdo[0][1]);			  //��ս���ӳ�����
		Delay_100us(Delay3);
		Elmo_Write(elmo,(0x1600+0),1,0x60ff0020);	 //RPDOi(i=0~3)��subindex 1 -> Ŀ���ٶ� elmo->ulNodeID	
		Delay_100us(Delay3);	
		my_sdo_Write(elmo,(0x1400+0),0x02,&init_pdo[0][2]);			  //������ͨѶ��������	
		Delay_100us(Delay3);		
		my_sdo_Write(elmo,(0x1400+0),0x01,&init_pdo[0][3]);			  //����ӳ��	
		Delay_100us(Delay3);		
		Elmo_NMTWrite(elmo, 0x1);				//����Pre-Operation
	
}

void MY_PDOWrite(Elmo* elmo,long value)
{

  elmo->elmoCAN.IDE = CAN_ID_STD;                    	//  ���ͱ�׼֡
  elmo->elmoCAN.StdId  = Elmo_PDOCOBID[0];	//elmo->ulNodeID;     //37f 
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   	//����֡
  elmo->elmoCAN.DLC = 4;                            	//  CAN�������ݳ���
  elmo->elmoCAN.Data[0]= value&0xFF;              //  ����Index���ֽ�
  elmo->elmoCAN.Data[1]= (value&0xFF00)>>8;       //  ����Index���ֽ�
  elmo->elmoCAN.Data[2]= (value&0xFF0000)>>16;               //  ����SubIndex
  elmo->elmoCAN.Data[3]= (value&0xFF000000)>>24;
  CAN_Transmit(CAN1, &(elmo->elmoCAN));
}




void MY_Elmo_PDOWrite_index(Elmo* elmo, unsigned char *cmd,u8 index,long value,u8 len)
{
	u8 value_tmp[2];
	u8 i = 0;
  elmo->elmoCAN.IDE = CAN_ID_STD;                    	//  ���ͱ�׼֡
  elmo->elmoCAN.StdId  = Elmo_PDOCOBID[1];	//elmo->ulNodeID;     //37f 
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   	//����֡
  elmo->elmoCAN.DLC = len;                            	//  CAN�������ݳ���

  elmo->elmoCAN.Data[0]= cmd[0];                   //  CAN����ǰ׺
  elmo->elmoCAN.Data[1]= cmd[1];
                   

  elmo->elmoCAN.Data[2]= 0;                   //  CAN����ǰ׺
  elmo->elmoCAN.Data[3]= index;
  elmo->elmoCAN.Data[4]= value&0xFF;              //  ����Index���ֽ�
  elmo->elmoCAN.Data[5]= (value&0xFF00)>>8;       //  ����Index���ֽ�
  elmo->elmoCAN.Data[6]= (value&0xFF0000)>>16;               //  ����SubIndex
  elmo->elmoCAN.Data[7]= (value&0xFF000000)>>24;
  CAN_Transmit(CAN1, &(elmo->elmoCAN));
}










 
void MY_Elmo_PDOWrite(Elmo* elmo, unsigned char *cmd,long value,u8 len)
{
	u8 value_tmp[2];
//	u8 i = 0;
  elmo->elmoCAN.IDE = CAN_ID_STD;                    	//  ���ͱ�׼֡
  elmo->elmoCAN.StdId  = Elmo_PDOCOBID[1];	//elmo->ulNodeID;     //37f 
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   	//����֡
  elmo->elmoCAN.DLC = len;                            	//  CAN�������ݳ���

  elmo->elmoCAN.Data[0]= cmd[0];                   //  CAN����ǰ׺
  elmo->elmoCAN.Data[1]= cmd[1];
                   

  elmo->elmoCAN.Data[2]= 0x00;                   //  CAN����ǰ׺
  elmo->elmoCAN.Data[3]= 0x00;
  elmo->elmoCAN.Data[4]= value&0xFF;              //  ����Index���ֽ�
  elmo->elmoCAN.Data[5]= (value&0xFF00)>>8;       //  ����Index���ֽ�
  elmo->elmoCAN.Data[6]= (value&0xFF0000)>>16;               //  ����SubIndex
  elmo->elmoCAN.Data[7]= (value&0xFF000000)>>24;
  CAN_Transmit(CAN1, &(elmo->elmoCAN));
}
void Speed_control_mode(long speed)
{
	Elmo_NMTWrite(elmo, 0x01);				//����Pre-Operation
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"MO",0,8);//MO=0;
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"UM",0x05,8);//UM=5
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"MO",0x01,8);//���ʹ��
	delay_ms(1000);	
  MY_Elmo_PDOWrite(elmo,"AC",10000,8);//AC=10000
	delay_ms(200);
  MY_Elmo_PDOWrite(elmo,"DC",10000,8);//DC=10000
	 delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"JV",speed,8);//
  delay_ms(1000);
	MY_Elmo_PDOWrite(elmo,"BG",0,4);

}
void Init_location_control_mode(void)
{
	Elmo_NMTWrite(elmo, 0x01);				//����Pre-Operation
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"MO",0,8);//MO=0;
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"UM",0x05,8);//UM=5 λ��ģʽ
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"MO",0x01,8);//���ʹ��
	delay_ms(100);
	MY_Elmo_PDOWrite(elmo,"DC",1000000,8);//DC=10000
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"AC",1000000,8);//AC=10000
	delay_ms(100);
	Elmo_Write(elmo, 0x60FF, 0x00, 600000);
}
void Run_pt_mode(float location)
{
	location = location*182;
  MY_Elmo_PDOWrite(elmo,"PR",location,8);
	delay_ms(2); 	
	MY_Elmo_PDOWrite(elmo,"BG",0,4);
}
void PT_mode()
{
 float sin[] = {0,3.7674,3.7526,3.7229,3.6785,3.6196,3.5465,3.4593,3.3585,3.2444,3.1175,2.9783,2.8274,2.6653,2.4927,2.3102,2.1187,1.9187,1.7112,1.497,1.2768,1.0516,0.82225,0.58965,0.35472,0.1184,-0.1184,-0.35472,-0.58965,-0.82225,-1.0516,-1.2768,-1.497,-1.7112,-1.9187,-2.1187,-2.3102,-2.4927,-2.6653,-2.8274,-2.9783,-3.1175,-3.2444,-3.3585,-3.4593,-3.5465,-3.6196,-3.6785,-3.7229,-3.7526,-3.7674,-3.7674,-3.7526,-3.7229,-3.6785,-3.6196,-3.5465,-3.4593,-3.3585,-3.2444,-3.1175,-2.9783,-2.8274,-2.6653,-2.4927,-2.3102,-2.1187,-1.9187,-1.7112,-1.497,-1.2768,-1.0516,-0.82225,-0.58965,-0.35472,-0.1184,0.1184,0.35472,0.58965,0.82225,1.0516,1.2768,1.497,1.7112,1.9187,2.1187,2.3102,2.4927,2.6653,2.8274,2.9783,3.1175,3.2444,3.3585,3.4593,3.5465,3.6196,3.6785,3.7229,3.7526
};
    int i;
		Elmo_NMTWrite(elmo, 0x01);				//����Pre-Operation
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"MO",0,8);//MO=0;
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"UM",0x05,8);
		delay_ms(100);
	  MY_Elmo_PDOWrite(elmo,"MO",0x01,8);//���ʹ��

		MY_Elmo_PDOWrite(elmo,"AC",10000000,8);//AC=10000
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"DC",10000000,8);//DC=10000
		delay_ms(100);
  Elmo_Write(elmo, 0x60FF, 0x00, 700000);
		delay_ms(200);
  MY_Elmo_PDOWrite(elmo,"PT",1,4);//����PTģʽ
	 delay_ms(200);
   MY_Elmo_PDOWrite_index(elmo,"MP",1,1,8);//����PTģʽ 
	 delay_ms(200);
	 MY_Elmo_PDOWrite_index(elmo,"MP",2,50,8);//����PTģʽ
	 delay_ms(200);
		MY_Elmo_PDOWrite_index(elmo,"MP",3,1,8);//����PTģʽ

  for(i=0;i<50;i++)
  {
      MY_Elmo_PDOWrite_index(elmo,"QP",i+1,sin[i]*182,8);//����PTģʽ
	    delay_ms(100);
		  MY_Elmo_PDOWrite_index(elmo,"QT",i+1,10,8);//����PTģʽ
		 delay_ms(100);
  }
	delay_ms(200);
  MY_Elmo_PDOWrite(elmo,"BG",0,4);
	delay_ms(200);


}
void Bigan_run()
{
  Elmo_NMTWrite(elmo, 0x01);				//����Pre-Operation
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"MO",0,8);//MO=0;
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"UM",0x02,8);//UM=2 �ٶ�ģʽ
	delay_ms(100);
	MY_Elmo_PDOWrite(elmo,"AC",10000,8);//AC=10000
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"DC",10000,8);//DC=10000
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"MO",0x01,8);//���ʹ��
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"JV",10000,8);//
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"BG",0,4);
	delay_ms(100);

	
	
	
}
void Init_PTmode( u8 mode )
{
		Current_um = 0x05;
		Elmo_NMTWrite(elmo, 0x01);				//����Pre-Operation
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"MO",0,8);//MO=0;
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"UM",0x02,8);//UM=2 �ٶ�ģʽ
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"AC",10000000,8);//AC=10000
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"DC",10000000,8);//DC=10000
		delay_ms(100);
		//MY_Elmo_PDOWrite(elmo,"JV",600000,8);//
			Elmo_Write(elmo, 0x60FF, 0x00, 600000);
		//Elmo_PDOWrite(elmo, 600000);
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"MO",0x01,8);//���ʹ��
		delay_ms(100); 
	  if(mode==1)
		{
		    sport_mode.Timie_sys = 10;
		    sport_mode.sport_Mode = 1;
			  sport_mode.index_value = 99;
		}
		else if(mode==2)
		{
				sport_mode.Timie_sys = 20;
		    sport_mode.sport_Mode = 2;
		    sport_mode.index_value = 49;
		}
}
void Location_control(float x)
{
	  x = x*182*(1000/sport_mode.Timie_sys);
   //	x = x*182*50;//�׹ؽ�
		MY_Elmo_PDOWrite(elmo,"JV",x,8);//

  // Elmo_Write(elmo, 0x60FF, 0x00, x);
   	delay_ms(2);
	  MY_Elmo_PDOWrite(elmo,"BG",0,4);
			
}
void disable_diver(void)
{

   Elmo_Write(elmo,(0x1600+0),1,0x60ff0020);	 //RPDOi(i=0~3)��subindex 1 -> Ŀ���ٶ� elmo->ulNodeID	


}

/*******************************************************************************
                        elmo_PDO��ʼ��ӳ��
��ڲ���:
          elmo          ָ��elmo�ṹ���ָ��
˵��	�����ڿ�����ҪŶ�õ��ٶ�ģʽ������ֻ���ٶ�ģʽ������ӳ��
			����RPDO0(0x27F)�����㲥֡��3��elmo�����յ���
				RPDOi(0x37f,0x47f,0x57f)����i(1~3)��elmoӳ��
*******************************************************************************/
void Elmo_PDOInit(Elmo * elmo)
{

	   	Elmo_NMTWrite(elmo, 0x80);								//����Pre-Operation
	   	Delay_100us(Delay3);
      
//	    Elmo_Write(elmo,(0x1400+0),0x01,0x8000037f);			//RPDOi׼��ӳ��  elmo->ulNodeID
//	    Delay_100us(Delay3);
//	
//	
//	   /*��ЧPDO������0��ӳ����Ч��ӳ�����ӳ��*/	   
//		 Elmo_Write(elmo,(0x1600+0),0,0x80);			//RPDOi׼��ӳ��  elmo->ulNodeID
//	   Delay_100us(Delay3);
//	  
//	//RPDO0 1600 1ӳ���ʱ6040 00 ��״̬
//	   Elmo_Write(elmo,(0x1600+0),1,0x60400010);	 //RPDOi(i=0~3)��subindex 1 -> Ŀ���ٶ� elmo->ulNodeID
//	   	Delay_100us(Delay3);
//	   /*Objectӳ��-�ٶ�ģʽ*/	//����ӳ���ٶ�
//	 //  	Elmo_Write(elmo,(0x1600+0),2,0x60ff0020);	 //RPDOi(i=0~3)��subindex 1 -> Ŀ���ٶ� elmo->ulNodeID
//	  // 	Delay_100us(Delay3);
//	
//	    //����ӳ��ģʽʱ��λ�� 60c1 01 �ĸ��ֽڵ�����
//	   	Elmo_Write(elmo,(0x1600+0),3,0x60C10120);	 //RPDOi(i=0~3)��subindex 1 -> Ŀ���ٶ� elmo->ulNodeID
//	   	Delay_100us(Delay3);
//	  
//	  	    	   	/*��ЧPDO������2��ӳ��ʹ��*/	
//		  Elmo_Write(elmo,(0x1600+0),0x00,0X01);			 //ӳ�����
//		  Delay_100us(Delay3);
//	    
//			Elmo_Write(elmo,(0x1400+0),0x01,0x0000027f);			//RPDOi׼��ӳ��  elmo->ulNodeID
//	    Delay_100us(Delay3);
//TPDOӳ��
      
	 
		 Elmo_Write(elmo,(0x1800+0),1,0x800001ff);			//��ʹ�ܷ���
	   Delay_100us(Delay3);
	  
	//TPDO0 1600 1ӳ���ʱ6040 00 ��״̬
	   Elmo_Write(elmo,(0x1A00+0),0,0);	 //���ӳ��
	   	Delay_100us(Delay3);

	
	    //ӳ��״̬��6041
	   	Elmo_Write(elmo,(0x1A00+0),1,0x60410010);	 
	   	Delay_100us(Delay3);
	  
		//ʵ��λ�õ�ӳ��
	   	Elmo_Write(elmo,(0x1A00+0),2,0x60640020);	
	   	Delay_100us(Delay3);
			//����ӳ����Ŀ
	   	Elmo_Write(elmo,(0x1A00+0),0,2);
	   	Delay_100us(Delay3);  
	  	    	  
		  Elmo_Write(elmo,(0x1800+0),0x02,0X4);			 //ͬ��ӳ��
		  Delay_100us(Delay3);
	    
			Elmo_Write(elmo,(0x1800+0),0x01,0x000001ff);			//ʹ��TPDO1ӳ��
	    Delay_100us(Delay3);
      

//RPDO
     Elmo_Write(elmo,(0x1400+0),1,0x8000027f);			//��ʹ�ܽ���
	   Delay_100us(Delay3);

	    Elmo_Write(elmo,(0x1600+0),0,0);	 //���ӳ��
	   	Delay_100us(Delay3);

	
	    //ӳ��״̬��6041
	   	//Elmo_Write(elmo,(0x1600+0),1,0x60ff0020);	 
	   	//Delay_100us(Delay3);
	  
		//ʵ��λ�õ�ӳ��  60c1
	   	//Elmo_Write(elmo,(0x1600+0),2,0x60c10120);	
	   	//Delay_100us(Delay3);
	  	Elmo_Write(elmo,(0x1600),1,0x60ff0020);	 //ӳ���ٶ�ֵ
		   Delay_100us(Delay3);	
			//����ӳ����Ŀ
	   	Elmo_Write(elmo,(0x1600+0),0,1);
	   	Delay_100us(Delay3);  
	  	    	  
		  Elmo_Write(elmo,(0x1400+0),0x02,0X04);			 //ͬ��ӳ��
		  Delay_100us(Delay3);
	    
			Elmo_Write(elmo,(0x1400+0),0x01,0x0000027f);			//ʹ��RPDO1ӳ��
	    Delay_100us(Delay3);
      
			Elmo_NMTWrite(elmo, 0x1);				//����Pre-Operation
	    Delay_100us(Delay3);
      Elmo_Delay(Delay2);
		
}

/*******************************************************************************
  ����������
             ��ʼ����ELMO�Ѿ�����ʹ��״̬��Ŀ���Ƿ������ģʽ֮�����
             ����ΪELMO֧��ʹ��״̬�µ��л�
  ��ڲ���:
            elmo          ָ��elmo�ṹ���ָ��
            elmoNum       Ҫ��ʼ����ELMO����
*******************************************************************************/
void Elmo_Init(Elmo* elmo, unsigned long elmoNum)
{
   // unsigned long i;

    tMode tModeInit={0,0,0,0,0,0,0,0,0,0,(void*)0};  //  ��ʼ������ģʽ�Ĳ���
    CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_7tq,1,CAN_Mode_Normal);//CAN��ʼ������ģʽ,������450Kbps 
	Elmo_Delay(Delay3);
	//Elmo_PDOInit(&elmo[0]);
    //for (i=0; i<=elmoNum; i++)
   // {
	   	elmo[0].ulNodeID = 127;							//����
       	elmo[0].Mode = tModeInit;
		Elmo_PDOInit(&elmo[0]);							//PDOӳ���ʼ��
		Elmo_ParamInit(&elmo[0]);
  //  }
		node_communicate[0].ulNodeID = 1;//������������ڵ�ͨ��
		node_communicate[1].ulNodeID = 2;
		node_communicate[2].ulNodeID = 3;
		
}

/*******************************************************************************
��ڲ���: elmo          ָ��elmo�ṹ���ָ��
          Data			�����ٶ�
*******************************************************************************/
void Elmo_PDOWrite(Elmo* elmo, long Data)
{
  elmo->elmoCAN.IDE = CAN_ID_STD;                    	//  ���ͱ�׼֡
  elmo->elmoCAN.StdId  = Elmo_PDOCOBID[0];	//elmo->ulNodeID;     //37f 
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   	//����֡
  elmo->elmoCAN.DLC = 4;                            	//  CAN�������ݳ���
  elmo->elmoCAN.Data[0]= Data&0xff;                    //  CAN����ǰ׺
  elmo->elmoCAN.Data[1]= (Data&0xff00)>>8;
  elmo->elmoCAN.Data[2]= (Data&0xff0000)>>16;
  elmo->elmoCAN.Data[3]= (Data&0xff000000)>>24;
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     //  ���÷��ͱ��ĺ���
}


void MY_NMTWrite(Elmo* elmo, long cmd)
{
	elmo->ulNodeID = 0X10;
  elmo->elmoCAN.IDE = CAN_ID_STD;                   //  ���ͱ�׼֡  00 01 00
  elmo->elmoCAN.StdId  = 0X00;         				//  NMT  COB-IDΪ0
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					//����֡
  elmo->elmoCAN.DLC = 2;                            //  CAN�������ݳ���
  elmo->elmoCAN.Data[0]= 0X01;                    	//  CAN����ǰ׺
  elmo->elmoCAN.Data[1]= 0X00;		 
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     //  ���÷��ͱ��ĺ���	
}



/*******************************************************************************
		   дNMT
*******************************************************************************/
void Elmo_NMTWrite(Elmo* elmo, long cmd)
{
  elmo->elmoCAN.IDE = CAN_ID_STD;                   //  ���ͱ�׼֡
  elmo->elmoCAN.StdId  = 0;         				//  NMT  COB-IDΪ0
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					//����֡
  elmo->elmoCAN.DLC = 2;                            //  CAN�������ݳ���
  elmo->elmoCAN.Data[0]= cmd;                    	//  CAN����ǰ׺
  elmo->elmoCAN.Data[1]= elmo->ulNodeID;		 
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     //  ���÷��ͱ��ĺ���	
}
void Change_mode(Elmo* elmo, unsigned char mode)
{
    if(mode==1||mode==3||mode==6 )
		{
		
      Elmo_Write(elmo, 0x6040, 0x00, 0x06);        //  Shutdown
      Elmo_Delay(Delay2);
      Elmo_Write(elmo, 0x6060, 0x00, mode);        //  ģʽ�趨
      Elmo_Delay(Delay3);
			Elmo_Write(elmo, 0x6040, 0x00, 0x07);        //  SwitchOn
			Elmo_Delay(Delay3); 
			Elmo_Write(elmo, 0x6040, 0x00, 0x0f);        //  ���ʹ��
	 		Elmo_Delay(Delay3);
		}
}
/*******************************************************************************
��ڲ�����
          speed              Ŀ���ٶȣ���λ�ǣ�counts/sec   
          acc                ���ٶ�
          dec                ���ٶ�
          QSDec              ��ͣ���ٶ�
ʹ��ʾ����Elmo_RunPVM(&elmo[0],-100000,300000,300000, 40000);        
*******************************************************************************/
//�ٶ�λ��pv
void Elmo_RunPVM(Elmo* elmo,  long speed, long acc, long dec, long QSDec)
{  
  if (elmo->Mode.CurMode != &elmo->Mode.PVMode)
  {

    elmo->Mode.CurMode =& elmo->Mode.PVMode; 
    elmo->Mode.PVMode.Speed=0;
    Elmo_Write(elmo, 0x6040, 0x00, 0x06);        //  Shutdown
    Elmo_Delay(Delay2);
    Elmo_Write(elmo, 0x6060, 0x00, 0x03);        //  Enter PVMode
    Elmo_Delay(Delay3);
    Elmo_Write(elmo, 0x6040, 0x00, 0x07);        //  SwitchOn
    Elmo_Delay(Delay3); 
    Elmo_Write(elmo, 0x6040, 0x00, 0x0f);        //  Enable  Operation
    Elmo_Delay(Delay3);
  }
  if (elmo->Mode.PVMode.Acc != acc)              // ���ü��ٶ�
  {
    elmo->Mode.PVMode.Acc = acc;
    Elmo_Write(elmo, 0x6083, 0x00, acc);
    Elmo_Delay(Delay1);
  }
  if (elmo->Mode.PVMode.Dec != dec)              // ���ü��ٶ�
  {
    elmo->Mode.PVMode.Dec = dec;
    Elmo_Write(elmo, 0x6084, 0x00, dec);
    Elmo_Delay(Delay1);
  }
  if (elmo->Mode.PVMode.QSDec != QSDec)          //  ���ü�ͣ���ٶ�
  {
    elmo->Mode.PVMode.QSDec = QSDec;
    Elmo_Write(elmo, 0x6085, 0x00, QSDec);
    Elmo_Delay(Delay1);  
  }  
  if (elmo->Mode.PVMode.Speed != speed)          //  ����Ŀ���ٶȣ�ͬʱ����PVM
  {
    elmo->Mode.PVMode.Speed = speed;
   Elmo_Write(elmo, 0x60FF, 0x00, speed);
   // Elmo_PDOWrite(elmo, speed);
    Elmo_Delay(Delay_Velocity);
  }
  Elmo_Write(elmo, 0x6040, 0x00, 0x0f);           //  Enable  Operation
  Elmo_Delay(Delay2);
	Elmo_Delay(Delay2);
	Elmo_Delay(Delay2);
  Elmo_Write(elmo, 0x6040, 0x00, 0x0f);           //  Enable  Operation
  Elmo_Delay(Delay2);

}

/*******************************************************************************
 ��ڲ�����
           speed        ����ٶȣ��������0��
           position     Ŀ��λ�ã���λ��counts  
           acc          ���ٶ� 
           dec          ���ٶ� 
           runMode      Pƽ���˶�ģʽ������ģʽ����ϸ���£�
                           ABS_POS       ����λ��
                           ABS_POS_IMM   ����λ��,��������
                           REL_POS_IMM   ���λ��,��������
                           REL_POS       ���λ��
ʹ��ʾ����Elmo_RunPPM(&elmo[0],100000,1000000,300000,300000,REL_POS_IMM);
*/
void U_Elmo_RunPPM(Elmo* elmo, long speed, long position, long acc, long dec, long runMode)
{  
	
	 		 signed int buff[] = {13	,12,	13,	18	,264	,383	,525	,60	,80	,91	,
		 102,	108	,114
     };
	 u8 i=0;
	
   MY_Elmo_PDOWrite(elmo,"PT",1,4);//����PTģʽ
	 delay_ms(200);
   MY_Elmo_PDOWrite_index(elmo,"MP",1,1,8);//����PTģʽ 
	 delay_ms(200);
	 MY_Elmo_PDOWrite_index(elmo,"MP",2,13,8);//����PTģʽ
	 delay_ms(200);
		MY_Elmo_PDOWrite_index(elmo,"MP",3,1,8);//����PTģʽ
  for(i=0;i<13;i++)
  {
      MY_Elmo_PDOWrite_index(elmo,"QP",i+1,buff[i]/100,8);//����PTģʽ
	    delay_ms(200);
		  MY_Elmo_PDOWrite_index(elmo,"QT",i+1,5*i,8);//����PTģʽ
		  delay_ms(200);
  }

	delay_ms(200);	
  if (elmo->Mode.CurMode != &elmo->Mode.PPMode)
  {
    elmo->Mode.CurMode = &elmo->Mode.PPMode;  
    Elmo_Write(elmo, 0x6040, 0x00, 0x06);         //  Shutdown 26 8
    Elmo_Delay(Delay3); 
		Elmo_Write(elmo, 0x6060, 0x00, 0x03);         //  Enter PPMode 
    Elmo_Delay(Delay3);
    Elmo_Write(elmo, 0x6040, 0x00, 0x07);         //  SwitchOn 3Switch ON and  Enable Operation
    Elmo_Delay(Delay3);
    Elmo_Write(elmo, 0x6040, 0x00, 0x0f);         //  Enable  Operation ��������� �����õ�  �ض����������ô�
    Elmo_Delay(Delay3);
  }
  if (elmo->Mode.PPMode.Speed != speed)           //  ��������ٶ�
  {
    elmo->Mode.PPMode.Speed = speed;
    Elmo_Write(elmo, 0x6081, 0x00, speed);
    Elmo_Delay(Delay2);
  }

  if(elmo->Mode.PPMode.Acc != acc)                //  ���ü��ٶ� ���ٶȵ�����
  {
    elmo->Mode.PPMode.Acc = acc;
    Elmo_Write(elmo, 0x6083, 0x00, acc);
    Elmo_Delay(Delay2);
  }
  if(elmo->Mode.PPMode.Dec != dec)                //  ���ü��ٶ�
  { 
    elmo->Mode.PPMode.Dec = dec;
    Elmo_Write(elmo, 0x6084, 0x00, dec);
    Elmo_Delay(Delay2);
  }
    if(elmo->Mode.PPMode.Position != position)      //  ����Ŀ��λ��
  {
		position = position*182;
    elmo->Mode.PPMode.Position = position;
    Elmo_Write(elmo, 0x607A, 0x00, position);
    Elmo_Delay(Delay2);
  }
  Elmo_Delay(Delay2);
  Elmo_Write(elmo, 0x6040, 0x00, runMode);        //  ѡ������ģʽ��ͬʱ����PPM
  Elmo_Delay(Delay2);
  Elmo_Write(elmo, 0x6040, 0x00, 0x0f);           //  Enable  Operation
  Elmo_Delay(Delay2);
  Elmo_Write(elmo, 0x6040, 0x00, 0x0f);           //  Enable  Operation
  Elmo_Delay(Delay2);
	MY_Elmo_PDOWrite(elmo,"BG",0,8);//����PTģʽ
}

void Elmo_RunPPM(Elmo* elmo, long speed, long position, long acc, long dec, long runMode)
{
  if (elmo->Mode.CurMode != &elmo->Mode.PPMode)
  {
    elmo->Mode.CurMode =& elmo->Mode.PVMode; 
    elmo->Mode.PVMode.Speed=0;
    Elmo_Write(elmo, 0x6040, 0x00, 0x06);        //  Shutdown
    Elmo_Delay(Delay2);
    Elmo_Write(elmo, 0x6060, 0x00, 0x01);        //  Enter PpMode λ��ģʽ
    Elmo_Delay(Delay3);
    Elmo_Write(elmo, 0x6040, 0x00, 0x07);        //  SwitchOn
    Elmo_Delay(Delay3); 
    Elmo_Write(elmo, 0x6040, 0x00, 0x0f);        //  ���ʹ��
    Elmo_Delay(Delay3);
  }
  if (elmo->Mode.PPMode.Speed != speed)           //  ��������ٶ�
  {
		
    elmo->Mode.PPMode.Speed = speed;
    Elmo_Write(elmo, 0x6081, 0x00, speed);
    Elmo_Delay(Delay2);
  }

  if(elmo->Mode.PPMode.Acc != acc)                //  ���ü��ٶ�
  {
    elmo->Mode.PPMode.Acc = acc;
    Elmo_Write(elmo, 0x6083, 0x00, acc);
    Elmo_Delay(Delay2);
  }
  if(elmo->Mode.PPMode.Dec != dec)                //  ���ü��ٶ�
  { 
    elmo->Mode.PPMode.Dec = dec;
    Elmo_Write(elmo, 0x6084, 0x00, dec);
    Elmo_Delay(Delay2);
  }
    if(elmo->Mode.PPMode.Position != position)      //  ����Ŀ��λ��
  {
		position  = position*182;
    elmo->Mode.PPMode.Position = position;
    Elmo_Write(elmo, 0x607A, 0x00, position);
    Elmo_Delay(Delay2);
  }
  Elmo_Delay(Delay2);
  Elmo_Write(elmo, 0x6040, 0x00, runMode);        //  ѡ������ģʽ��ͬʱ����PPM
  Elmo_Delay(Delay2);
  Elmo_Write(elmo, 0x6040, 0x00, 0x0f);           //  Enable  Operation
  Elmo_Delay(Delay2);
  Elmo_Write(elmo, 0x6040, 0x00, 0x0f);           //  Enable  Operation
  Elmo_Delay(Delay2);


}


//*******************************************************************************/
//void Elmo_RunPPM(Elmo* elmo, long speed, long position, long acc, long dec, long runMode)
//{  
//  //if (elmo->Mode.CurMode != &elmo->Mode.PPMode)
//  //{
//    elmo->Mode.CurMode =& elmo->Mode.PVMode; 
//    elmo->Mode.PVMode.Speed=0;
//    Elmo_Write(elmo, 0x6040, 0x00, 0x06);        //  Shutdown
//    Elmo_Delay(Delay2);
//    Elmo_Write(elmo, 0x6060, 0x00, 0x01);        //  Enter PpMode λ��ģʽ
//    Elmo_Delay(Delay3);
//    Elmo_Write(elmo, 0x6040, 0x00, 0x07);        //  SwitchOn
//    Elmo_Delay(Delay3); 
//    Elmo_Write(elmo, 0x6040, 0x00, 0x0f);        //  ���ʹ��
//    Elmo_Delay(Delay3);
// // }
// // if (elmo->Mode.PPMode.Speed != speed)           //  ��������ٶ�
//  //{
//    //elmo->Mode.PPMode.Speed = speed;
//    Elmo_Write(elmo, 0x6081, 0x00, speed);//0x6081�����ٶ�
//    Elmo_Delay(Delay2);
// // }

//  //if(elmo->Mode.PPMode.Acc != acc)                //  ���ü��ٶ�
// // {
//    elmo->Mode.PPMode.Acc = acc;
//    Elmo_Write(elmo, 0x6083, 0x00, acc);
//    Elmo_Delay(Delay2);
//  //}
//  //if(elmo->Mode.PPMode.Dec != dec)                //  ���ü��ٶ�
//  //{ 
//    elmo->Mode.PPMode.Dec = dec;
//    Elmo_Write(elmo, 0x6084, 0x00, dec);
//    Elmo_Delay(Delay2);
//  //}
//  //  if(elmo->Mode.PPMode.Position != position)      //  ����Ŀ��λ��
//  //{
//    elmo->Mode.PPMode.Position = position;
//    Elmo_Write(elmo, 0x607A, 0x00, position);
//    Elmo_Delay(Delay2);
//  //}
////  Elmo_Delay(Delay2);
////  Elmo_Write(elmo, 0x6040, 0x00, runMode);        //  ѡ������ģʽ��ͬʱ����PPM
////  Elmo_Delay(Delay2);
////  Elmo_Write(elmo, 0x6040, 0x00, 0x0f);           //  Enable  Operation
////  Elmo_Delay(Delay2);
////  Elmo_Write(elmo, 0x6040, 0x00, 0x0f);           //  Enable  Operation
////  Elmo_Delay(Delay2);
//}

/*******************************************************************************
��ڲ���:
          elmo          ָ��elmo�ṹ���ָ��
          torque        Ť�أ���Χ��0 -> 2000(���û����õ�MAX_TORQUE�й�)
   torque=DemandCurrent*1000/MotoRateCurrent ��6075�ж���� 
ʹ��ʾ��: Elmo_RunPTM(&elmo[0],100);
*******************************************************************************/
void Elmo_RunPTM(Elmo* elmo,long torque)
{
  if (elmo->Mode.CurMode != &elmo->Mode.PTMode)
  {
    elmo->Mode.CurMode = &elmo->Mode.PTMode;  
    elmo->Mode.PTMode.Torque=0;   
    Elmo_Write(elmo, 0x6060, 0x00, 0x04);     //  Enter PTMode
    Elmo_Delay(Delay3);
    Elmo_Write(elmo, 0x6040, 0x00, 0x06);     //  ShutDown    
    Elmo_Delay(Delay3);
    Elmo_Write(elmo, 0x6040, 0x00, 0x07);     //  SwitchOn
    Elmo_Delay(Delay3);
    Elmo_Write(elmo, 0x6040, 0x00, 0x0f);     //  Enable Operation
    Elmo_Delay(Delay3);
 }
  if (elmo->Mode.PTMode.Torque != torque)
  {
      elmo->Mode.PTMode.Torque = torque;
      Elmo_Write(elmo, 0x6071, 0x00, torque); //  ����������Ť��
   //Elmo_PDOWrite(elmo,torque,torque);
      Elmo_Delay(Delay1);
  }
   Elmo_Delay(Delay3);
}


/*******************************************************************************
��ڲ���: elmo          ָ��elmo�ṹ���ָ��
*******************************************************************************/
void Elmo_QuickStop(Elmo* elmo)
{
  tMode tModeInit={0,0,0,0,0,0,0,0,0,0,(void*)0};
  
  Elmo_Delay(Delay3);     
  Elmo_Write(elmo, 0x6040, 0x00, 0x05);     //  quickstop
  Elmo_Delay(Delay3);
  Elmo_Write(elmo, 0x6040, 0x00, 0x06);     //  shut down
  Elmo_Delay(Delay3);
  Elmo_Write(elmo, 0x6040, 0x00, 0x07);     //  switch on
  Elmo_Delay(Delay3);
  Elmo_Write(elmo, 0x6040, 0x00, 0x0f);     //  Enable Operation
  Elmo_Delay(Delay3);
  
  elmo->Mode = tModeInit;                   //  ��λ����
}


/*******************************************************************************
//                       ��ȡPPM�Ƿ�λ                                 
��ڲ���: epos          ָ��epos�ṹ���ָ��                            
���ڲ���: Reached       PPM�Ƿ�λ�ı�־��                             
                        0        û�е�λ                                  
                        Halt = 0: Target position not reached             
                        Halt = 1: Axle decelerates                        
                        1        ��λ                                      
                        Halt = 0: Target position reached                 
                        Halt = 1: Velocity of axle is 0                   
                        1           ��λ                                
                        0           û�е�λ                            
����ֵ:   ��                                                            
*******************************************************************************/
/*
long Elmo_PositionGet(Elmo* elmo)
{
  tCANFrame FrameBuff;
  long Value;
  
  Elmo_Read(elmo, 0x6064, 0x0000);                                              //  ���Ͷ����̵�����
  Elmo_Delay(Delay_Read);                                                       //  ��ʱһ��ʱ�����
  while (CAN_ucReadRcvBuffer(pReceFrameBuff, &FrameBuff) != EMPTY);             //  �ȴ���������������
  Value = (FrameBuff.Data[7]<<24) | (FrameBuff.Data[6]<<16) |
          (FrameBuff.Data[5]<<8)  | (FrameBuff.Data[4]);
  return Value;
}

//============================================================================//
//==                        ��ȡPPM�Ƿ�λ                                 ==//
//============================================================================//
//==��ڲ���: epos          ָ��epos�ṹ���ָ��                            ==//
//==���ڲ���: Reached       PPM�Ƿ�λ�ı�־��                             ==//
//==                     0        û�е�λ                                  ==//
//==                      Halt = 0: Target position not reached             ==//
//==                      Halt = 1: Axle decelerates                        ==//
//==                     1        ��λ                                      ==//
//==                      Halt = 0: Target position reached                 ==//
//==                      Halt = 1: Velocity of axle is 0                   ==//
//==                     1           ��λ                                ==//
//==                     0           û�е�λ                            ==//
//==����ֵ:   ��                                                            ==//
//============================================================================//
unsigned long Elmo_PPMReached(Elmo* elmo)
{
  tCANFrame FrameBuff;
  unsigned long Reached; 
  Elmo_Read(elmo, 0x6041, 0x0000);                                   //  ���Ͷ����̵�����
  Elmo_Delay(Delay3); 
  while (CAN_ucReadRcvBuffer(pReceFrameBuff, &FrameBuff) != EMPTY);  //  �ȴ���������������
  Reached = (FrameBuff.Data[5]>>2)&0x01;
  return Reached;
}
*/
/******************************* END FILE *************************************/
