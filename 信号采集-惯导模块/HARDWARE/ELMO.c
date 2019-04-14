
/******************************************************************************
注意:
    1。修改startup_ewarm中的中断函数注册,将CAN的中断注册
        (当然也可以不注册中断)
    2。请同时拷贝  my_can.c，my_can.h，ELMO.c，ELMO.h
       并同时将安装目录的can.c和can.h覆盖，否则编译时可能会报错
    3。elmo一次状态之间的转换几乎为3ms
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
                          CAN通信相关变量
*******************************************************************************/
/*
tCANFrameBuff ReceFrameBuff;			    //  定义帧接收缓冲区
tCANFrameBuff SendFrameBuff;                        //  定义帧发送缓冲区
tCANFrameBuff *pReceFrameBuff = &ReceFrameBuff;	    //  定义帧接收缓冲区指针
tCANFrameBuff *pSendFrameBuff = &SendFrameBuff;     //  定义帧发送缓冲区指针
*/

/*******************************************************************************
                          ELMO通信相关变量
  注意： 修改此处时，要同时修改ELMO.h中的声明
*******************************************************************************/
Elmo   node_communicate[4];
Elmo      elmo[6];                                  // 定义Elmo结构体数组,与elmo个数对应 
unsigned int Elmo_PDOCOBID[4] = {0x27f,0x37f,0x47f,0x57f};	//四个RPDO的接收COBID                             
/*******************************************************************************
                        初始化CAN硬件
*******************************************************************************/


/*******************************************************************************
入口参数: elmo          指向elmo结构体的指针
          Index         对象字典索引
          SubIndex      对象字典子索引
          Param         对象字典对应索引的 值
*******************************************************************************/
void Elmo_Write(Elmo* elmo, unsigned long Index, unsigned char SubIndex, long Param)
{
	elmo->ulNodeID = 0X7F; 
  elmo->elmoCAN.IDE = CAN_ID_STD;                    //  发送标准帧
  elmo->elmoCAN.StdId  = 0x600+elmo->ulNodeID;         //  CAN报文IDelmo->ulNodeID
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   //数据帧
  elmo->elmoCAN.DLC = 8;                            //  CAN报文数据长度
  elmo->elmoCAN.Data[0]= 0x22;                    //  CAN报文前缀 
  elmo->elmoCAN.Data[1]= Index&0xFF;              //  发送Index低字节
  elmo->elmoCAN.Data[2]= (Index&0xFF00)>>8;       //  发送Index高字节
  elmo->elmoCAN.Data[3]= SubIndex;                //  发送SubIndex
  elmo->elmoCAN.Data[4]= Param&0xFF;
  elmo->elmoCAN.Data[5]= (Param&0xFF00)>>8;
  elmo->elmoCAN.Data[6]= (Param&0xFF0000)>>16;
  elmo->elmoCAN.Data[7]= (Param&0xFF000000)>>24;  //  依次发送对象内容
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     //  调用发送报文函数
}

void my_elmo_len(Elmo* elmo, unsigned char*  x1, unsigned char len)
{
	 unsigned char i =0;
	elmo->ulNodeID = 0X7F; 
  elmo->elmoCAN.IDE = CAN_ID_STD;                    //  发送标准帧
  elmo->elmoCAN.StdId  = 0x300+elmo->ulNodeID;         //  CAN报文IDelmo->ulNodeID
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   //数据帧
  elmo->elmoCAN.DLC = len;	//  CAN报文数据长度
   for(i=0;i<len;i++)
     elmo->elmoCAN.Data[i]= x1[i];  
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     //  调用发送报文函数
}

/*******************************************************************************
入口参数:                                                  
          elmo          指向elmo结构体的指针
          Index         对象字典索引
          SubIndex      对象字典子索引
*******************************************************************************/
void Elmo_Read(Elmo* elmo, unsigned long Index, unsigned char SubIndex)
{
  g_Index = Index;
  g_SubIndex = SubIndex;
  g_ReadNodeID = elmo->ulNodeID;                     	//  修改全局变量,供中断使用
  elmo->elmoCAN.IDE = CAN_ID_STD;                    	//  发送标准帧
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					  	//  数据帧
  elmo->elmoCAN.StdId  = 0x600+elmo->ulNodeID;         	//  CAN报文ID 
  elmo->elmoCAN.DLC = 8;                            	//  CAN报文数据长度
  elmo->elmoCAN.Data[0]= 0x40;                    		//  CAN报文前缀,根据资料得到  请求获取命令字的0x40
  elmo->elmoCAN.Data[1]= Index&0xFF;              		//  发送Index低字节
  elmo->elmoCAN.Data[2]= (Index&0xFF00)>>8;       		//  发送Index高字节
  elmo->elmoCAN.Data[3]= SubIndex;                		//  发送SubIndex
  elmo->elmoCAN.Data[4]= 0;
  elmo->elmoCAN.Data[5]= 0;
  elmo->elmoCAN.Data[6]= 0;
  elmo->elmoCAN.Data[7]= 0;                       		//  对象内容保留
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     			//  调用发送报文函数
}


/*******************************************************************************
                        初始化elmo参数
入口参数:
          elmo          指向elmo结构体的指针
*******************************************************************************/
void Elmo_ParamInit(Elmo* elmo)
{
   Elmo_Write(elmo, 0x6040, 0x00, 0x80);               //  清除elmo错误
  Delay_100us(Delay2);
  Elmo_Write(elmo, 0x6065, 0x00, MAX_F_ERR);          //  PPMode下当前位置和实际位置的最大允许误差 
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x607D, 0x01, MIN_P_LIMIT);        //  PPMode下最小位置极限
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x607D, 0x02, MAX_P_LIMIT);        //  PPMode下最大位置极限
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x607E, 0x00, POLARITY  );         //  位置、速度极性 (根据需要修改)
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x607F, 0x00, MAX_SPEED);          //  PPMode和PVMode下所能达到的最大速度
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x6083, 0x00, MAX_ACC);            //  最大加速度
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x6084, 0x00, MAX_ACC);            //  最大减速度
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x6085, 0x00, MAX_QDEC);           //  各种模式下,elmo出错时的最大急停减速度
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x6086, 0x00, MOTION_PROFILE_TYPE);//  PPM实现方式;0:Linear ramp (trapezoidal profile)     
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x6088, 0x00, 0);                  //  PTM实现方式,ELMO只支持 linear ramp                 
  Elmo_Delay(Delay1);
  Elmo_Write(elmo, 0x2f41, 0x00, 0x02);               //  启动PPM 
  Elmo_Delay(Delay1);
}

void my_sdo_Write(Elmo* elmo, unsigned long Index, unsigned char SubIndex, unsigned char *x )
{
	unsigned char i = 0;
	elmo->ulNodeID = 0X7F; 
  elmo->elmoCAN.IDE = CAN_ID_STD;                    //  发送标准帧
  elmo->elmoCAN.StdId  = 0x600+elmo->ulNodeID;         //  CAN报文IDelmo->ulNodeID
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   //数据帧
  elmo->elmoCAN.DLC = 8;                            //  CAN报文数据长度
  elmo->elmoCAN.Data[0]= 0x22;                    //  CAN报文前缀 
  elmo->elmoCAN.Data[1]= Index&0xFF;              //  发送Index低字节
  elmo->elmoCAN.Data[2]= (Index&0xFF00)>>8;       //  发送Index高字节
  elmo->elmoCAN.Data[3]= SubIndex;                //  发送SubIndex
  for(i=0;i<4;i++)
	  elmo->elmoCAN.Data[4+i] = x[i];
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     //  调用发送报文函数
}

/*******************************************************************************
                        elmo_PDO初始化映射
入口参数:
          elmo          指向elmo结构体的指针
说明	：由于控制主要哦用到速度模式，所以只对速度模式进行了映射
			其中RPDO0(0x27F)用作广播帧，3个elmo都能收到；
				RPDOi(0x37f,0x47f,0x57f)被第i(1~3)个elmo映射
*******************************************************************************/
void my_Elmo_PDOInit(Elmo * elmo)
{
	   	
	
			unsigned char init_pdo[][4] =  {
				 0x01,0x02,0x00,0x08,    //diaable rpdo1  
			  0x00,0x00,0x00,0x08,
				0x01,0x00,0x00,0x00,
				0x01,0x02,0x00,0x00,
			};
	
	   Elmo_NMTWrite(elmo, 0x80);								//进入Pre-Operation
	   Delay_100us(Delay3);
  
	
	  my_sdo_Write(elmo,(0x1400+0),0x01,&init_pdo[0][0]);			  //这里是通讯参数设置
		Delay_100us(Delay3);
		my_sdo_Write(elmo,(0x1600+0),0x00,&init_pdo[0][1]);			  //清空接收映射参数
		Delay_100us(Delay3);
		Elmo_Write(elmo,(0x1600+0),1,0x60ff0020);	 //RPDOi(i=0~3)的subindex 1 -> 目标速度 elmo->ulNodeID	
		Delay_100us(Delay3);	
		my_sdo_Write(elmo,(0x1400+0),0x02,&init_pdo[0][2]);			  //这里是通讯参数设置	
		Delay_100us(Delay3);		
		my_sdo_Write(elmo,(0x1400+0),0x01,&init_pdo[0][3]);			  //启动映射	
		Delay_100us(Delay3);		
		Elmo_NMTWrite(elmo, 0x1);				//进入Pre-Operation
	
}

void MY_PDOWrite(Elmo* elmo,long value)
{

  elmo->elmoCAN.IDE = CAN_ID_STD;                    	//  发送标准帧
  elmo->elmoCAN.StdId  = Elmo_PDOCOBID[0];	//elmo->ulNodeID;     //37f 
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   	//数据帧
  elmo->elmoCAN.DLC = 4;                            	//  CAN报文数据长度
  elmo->elmoCAN.Data[0]= value&0xFF;              //  发送Index低字节
  elmo->elmoCAN.Data[1]= (value&0xFF00)>>8;       //  发送Index高字节
  elmo->elmoCAN.Data[2]= (value&0xFF0000)>>16;               //  发送SubIndex
  elmo->elmoCAN.Data[3]= (value&0xFF000000)>>24;
  CAN_Transmit(CAN1, &(elmo->elmoCAN));
}




void MY_Elmo_PDOWrite_index(Elmo* elmo, unsigned char *cmd,u8 index,long value,u8 len)
{
	u8 value_tmp[2];
	u8 i = 0;
  elmo->elmoCAN.IDE = CAN_ID_STD;                    	//  发送标准帧
  elmo->elmoCAN.StdId  = Elmo_PDOCOBID[1];	//elmo->ulNodeID;     //37f 
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   	//数据帧
  elmo->elmoCAN.DLC = len;                            	//  CAN报文数据长度

  elmo->elmoCAN.Data[0]= cmd[0];                   //  CAN报文前缀
  elmo->elmoCAN.Data[1]= cmd[1];
                   

  elmo->elmoCAN.Data[2]= 0;                   //  CAN报文前缀
  elmo->elmoCAN.Data[3]= index;
  elmo->elmoCAN.Data[4]= value&0xFF;              //  发送Index低字节
  elmo->elmoCAN.Data[5]= (value&0xFF00)>>8;       //  发送Index高字节
  elmo->elmoCAN.Data[6]= (value&0xFF0000)>>16;               //  发送SubIndex
  elmo->elmoCAN.Data[7]= (value&0xFF000000)>>24;
  CAN_Transmit(CAN1, &(elmo->elmoCAN));
}










 
void MY_Elmo_PDOWrite(Elmo* elmo, unsigned char *cmd,long value,u8 len)
{
	u8 value_tmp[2];
//	u8 i = 0;
  elmo->elmoCAN.IDE = CAN_ID_STD;                    	//  发送标准帧
  elmo->elmoCAN.StdId  = Elmo_PDOCOBID[1];	//elmo->ulNodeID;     //37f 
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   	//数据帧
  elmo->elmoCAN.DLC = len;                            	//  CAN报文数据长度

  elmo->elmoCAN.Data[0]= cmd[0];                   //  CAN报文前缀
  elmo->elmoCAN.Data[1]= cmd[1];
                   

  elmo->elmoCAN.Data[2]= 0x00;                   //  CAN报文前缀
  elmo->elmoCAN.Data[3]= 0x00;
  elmo->elmoCAN.Data[4]= value&0xFF;              //  发送Index低字节
  elmo->elmoCAN.Data[5]= (value&0xFF00)>>8;       //  发送Index高字节
  elmo->elmoCAN.Data[6]= (value&0xFF0000)>>16;               //  发送SubIndex
  elmo->elmoCAN.Data[7]= (value&0xFF000000)>>24;
  CAN_Transmit(CAN1, &(elmo->elmoCAN));
}
void Speed_control_mode(long speed)
{
	Elmo_NMTWrite(elmo, 0x01);				//进入Pre-Operation
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"MO",0,8);//MO=0;
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"UM",0x05,8);//UM=5
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"MO",0x01,8);//电机使能
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
	Elmo_NMTWrite(elmo, 0x01);				//进入Pre-Operation
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"MO",0,8);//MO=0;
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"UM",0x05,8);//UM=5 位置模式
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"MO",0x01,8);//电机使能
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
		Elmo_NMTWrite(elmo, 0x01);				//进入Pre-Operation
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"MO",0,8);//MO=0;
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"UM",0x05,8);
		delay_ms(100);
	  MY_Elmo_PDOWrite(elmo,"MO",0x01,8);//电机使能

		MY_Elmo_PDOWrite(elmo,"AC",10000000,8);//AC=10000
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"DC",10000000,8);//DC=10000
		delay_ms(100);
  Elmo_Write(elmo, 0x60FF, 0x00, 700000);
		delay_ms(200);
  MY_Elmo_PDOWrite(elmo,"PT",1,4);//设置PT模式
	 delay_ms(200);
   MY_Elmo_PDOWrite_index(elmo,"MP",1,1,8);//设置PT模式 
	 delay_ms(200);
	 MY_Elmo_PDOWrite_index(elmo,"MP",2,50,8);//设置PT模式
	 delay_ms(200);
		MY_Elmo_PDOWrite_index(elmo,"MP",3,1,8);//设置PT模式

  for(i=0;i<50;i++)
  {
      MY_Elmo_PDOWrite_index(elmo,"QP",i+1,sin[i]*182,8);//设置PT模式
	    delay_ms(100);
		  MY_Elmo_PDOWrite_index(elmo,"QT",i+1,10,8);//设置PT模式
		 delay_ms(100);
  }
	delay_ms(200);
  MY_Elmo_PDOWrite(elmo,"BG",0,4);
	delay_ms(200);


}
void Bigan_run()
{
  Elmo_NMTWrite(elmo, 0x01);				//进入Pre-Operation
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"MO",0,8);//MO=0;
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"UM",0x02,8);//UM=2 速度模式
	delay_ms(100);
	MY_Elmo_PDOWrite(elmo,"AC",10000,8);//AC=10000
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"DC",10000,8);//DC=10000
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"MO",0x01,8);//电机使能
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"JV",10000,8);//
	delay_ms(100);
  MY_Elmo_PDOWrite(elmo,"BG",0,4);
	delay_ms(100);

	
	
	
}
void Init_PTmode( u8 mode )
{
		Current_um = 0x05;
		Elmo_NMTWrite(elmo, 0x01);				//进入Pre-Operation
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"MO",0,8);//MO=0;
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"UM",0x02,8);//UM=2 速度模式
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"AC",10000000,8);//AC=10000
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"DC",10000000,8);//DC=10000
		delay_ms(100);
		//MY_Elmo_PDOWrite(elmo,"JV",600000,8);//
			Elmo_Write(elmo, 0x60FF, 0x00, 600000);
		//Elmo_PDOWrite(elmo, 600000);
		delay_ms(100);
		MY_Elmo_PDOWrite(elmo,"MO",0x01,8);//电机使能
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
   //	x = x*182*50;//踝关节
		MY_Elmo_PDOWrite(elmo,"JV",x,8);//

  // Elmo_Write(elmo, 0x60FF, 0x00, x);
   	delay_ms(2);
	  MY_Elmo_PDOWrite(elmo,"BG",0,4);
			
}
void disable_diver(void)
{

   Elmo_Write(elmo,(0x1600+0),1,0x60ff0020);	 //RPDOi(i=0~3)的subindex 1 -> 目标速度 elmo->ulNodeID	


}

/*******************************************************************************
                        elmo_PDO初始化映射
入口参数:
          elmo          指向elmo结构体的指针
说明	：由于控制主要哦用到速度模式，所以只对速度模式进行了映射
			其中RPDO0(0x27F)用作广播帧，3个elmo都能收到；
				RPDOi(0x37f,0x47f,0x57f)被第i(1~3)个elmo映射
*******************************************************************************/
void Elmo_PDOInit(Elmo * elmo)
{

	   	Elmo_NMTWrite(elmo, 0x80);								//进入Pre-Operation
	   	Delay_100us(Delay3);
      
//	    Elmo_Write(elmo,(0x1400+0),0x01,0x8000037f);			//RPDOi准备映射  elmo->ulNodeID
//	    Delay_100us(Delay3);
//	
//	
//	   /*有效PDO个数置0，映射无效，映射可重映射*/	   
//		 Elmo_Write(elmo,(0x1600+0),0,0x80);			//RPDOi准备映射  elmo->ulNodeID
//	   Delay_100us(Delay3);
//	  
//	//RPDO0 1600 1映射的时6040 00 即状态
//	   Elmo_Write(elmo,(0x1600+0),1,0x60400010);	 //RPDOi(i=0~3)的subindex 1 -> 目标速度 elmo->ulNodeID
//	   	Delay_100us(Delay3);
//	   /*Object映射-速度模式*/	//这里映射速度
//	 //  	Elmo_Write(elmo,(0x1600+0),2,0x60ff0020);	 //RPDOi(i=0~3)的subindex 1 -> 目标速度 elmo->ulNodeID
//	  // 	Delay_100us(Delay3);
//	
//	    //这里映射差不模式时的位置 60c1 01 四个字节的数据
//	   	Elmo_Write(elmo,(0x1600+0),3,0x60C10120);	 //RPDOi(i=0~3)的subindex 1 -> 目标速度 elmo->ulNodeID
//	   	Delay_100us(Delay3);
//	  
//	  	    	   	/*有效PDO个数置2，映射使能*/	
//		  Elmo_Write(elmo,(0x1600+0),0x00,0X01);			 //映射结束
//		  Delay_100us(Delay3);
//	    
//			Elmo_Write(elmo,(0x1400+0),0x01,0x0000027f);			//RPDOi准备映射  elmo->ulNodeID
//	    Delay_100us(Delay3);
//TPDO映射
      
	 
		 Elmo_Write(elmo,(0x1800+0),1,0x800001ff);			//不使能发送
	   Delay_100us(Delay3);
	  
	//TPDO0 1600 1映射的时6040 00 即状态
	   Elmo_Write(elmo,(0x1A00+0),0,0);	 //清除映射
	   	Delay_100us(Delay3);

	
	    //映射状态字6041
	   	Elmo_Write(elmo,(0x1A00+0),1,0x60410010);	 
	   	Delay_100us(Delay3);
	  
		//实际位置的映射
	   	Elmo_Write(elmo,(0x1A00+0),2,0x60640020);	
	   	Delay_100us(Delay3);
			//更改映射数目
	   	Elmo_Write(elmo,(0x1A00+0),0,2);
	   	Delay_100us(Delay3);  
	  	    	  
		  Elmo_Write(elmo,(0x1800+0),0x02,0X4);			 //同步映射
		  Delay_100us(Delay3);
	    
			Elmo_Write(elmo,(0x1800+0),0x01,0x000001ff);			//使能TPDO1映射
	    Delay_100us(Delay3);
      

//RPDO
     Elmo_Write(elmo,(0x1400+0),1,0x8000027f);			//不使能接收
	   Delay_100us(Delay3);

	    Elmo_Write(elmo,(0x1600+0),0,0);	 //清除映射
	   	Delay_100us(Delay3);

	
	    //映射状态字6041
	   	//Elmo_Write(elmo,(0x1600+0),1,0x60ff0020);	 
	   	//Delay_100us(Delay3);
	  
		//实际位置的映射  60c1
	   	//Elmo_Write(elmo,(0x1600+0),2,0x60c10120);	
	   	//Delay_100us(Delay3);
	  	Elmo_Write(elmo,(0x1600),1,0x60ff0020);	 //映射速度值
		   Delay_100us(Delay3);	
			//更改映射数目
	   	Elmo_Write(elmo,(0x1600+0),0,1);
	   	Delay_100us(Delay3);  
	  	    	  
		  Elmo_Write(elmo,(0x1400+0),0x02,0X04);			 //同步映射
		  Delay_100us(Delay3);
	    
			Elmo_Write(elmo,(0x1400+0),0x01,0x0000027f);			//使能RPDO1映射
	    Delay_100us(Delay3);
      
			Elmo_NMTWrite(elmo, 0x1);				//进入Pre-Operation
	    Delay_100us(Delay3);
      Elmo_Delay(Delay2);
		
}

/*******************************************************************************
  函数描述：
             初始化后，ELMO已经处于使能状态，目的是方便各个模式之间的切
             换因为ELMO支持使能状态下的切换
  入口参数:
            elmo          指向elmo结构体的指针
            elmoNum       要初始化的ELMO个数
*******************************************************************************/
void Elmo_Init(Elmo* elmo, unsigned long elmoNum)
{
   // unsigned long i;

    tMode tModeInit={0,0,0,0,0,0,0,0,0,0,(void*)0};  //  初始化各种模式的参数
    CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_7tq,1,CAN_Mode_Normal);//CAN初始化环回模式,波特率450Kbps 
	Elmo_Delay(Delay3);
	//Elmo_PDOInit(&elmo[0]);
    //for (i=0; i<=elmoNum; i++)
   // {
	   	elmo[0].ulNodeID = 127;							//结点号
       	elmo[0].Mode = tModeInit;
		Elmo_PDOInit(&elmo[0]);							//PDO映射初始化
		Elmo_ParamInit(&elmo[0]);
  //  }
		node_communicate[0].ulNodeID = 1;//这个数组用来节点通信
		node_communicate[1].ulNodeID = 2;
		node_communicate[2].ulNodeID = 3;
		
}

/*******************************************************************************
入口参数: elmo          指向elmo结构体的指针
          Data			期望速度
*******************************************************************************/
void Elmo_PDOWrite(Elmo* elmo, long Data)
{
  elmo->elmoCAN.IDE = CAN_ID_STD;                    	//  发送标准帧
  elmo->elmoCAN.StdId  = Elmo_PDOCOBID[0];	//elmo->ulNodeID;     //37f 
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					   	//数据帧
  elmo->elmoCAN.DLC = 4;                            	//  CAN报文数据长度
  elmo->elmoCAN.Data[0]= Data&0xff;                    //  CAN报文前缀
  elmo->elmoCAN.Data[1]= (Data&0xff00)>>8;
  elmo->elmoCAN.Data[2]= (Data&0xff0000)>>16;
  elmo->elmoCAN.Data[3]= (Data&0xff000000)>>24;
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     //  调用发送报文函数
}


void MY_NMTWrite(Elmo* elmo, long cmd)
{
	elmo->ulNodeID = 0X10;
  elmo->elmoCAN.IDE = CAN_ID_STD;                   //  发送标准帧  00 01 00
  elmo->elmoCAN.StdId  = 0X00;         				//  NMT  COB-ID为0
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					//数据帧
  elmo->elmoCAN.DLC = 2;                            //  CAN报文数据长度
  elmo->elmoCAN.Data[0]= 0X01;                    	//  CAN报文前缀
  elmo->elmoCAN.Data[1]= 0X00;		 
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     //  调用发送报文函数	
}



/*******************************************************************************
		   写NMT
*******************************************************************************/
void Elmo_NMTWrite(Elmo* elmo, long cmd)
{
  elmo->elmoCAN.IDE = CAN_ID_STD;                   //  发送标准帧
  elmo->elmoCAN.StdId  = 0;         				//  NMT  COB-ID为0
  elmo->elmoCAN.RTR=CAN_RTR_DATA;					//数据帧
  elmo->elmoCAN.DLC = 2;                            //  CAN报文数据长度
  elmo->elmoCAN.Data[0]= cmd;                    	//  CAN报文前缀
  elmo->elmoCAN.Data[1]= elmo->ulNodeID;		 
  CAN_Transmit(CAN1, &(elmo->elmoCAN));     //  调用发送报文函数	
}
void Change_mode(Elmo* elmo, unsigned char mode)
{
    if(mode==1||mode==3||mode==6 )
		{
		
      Elmo_Write(elmo, 0x6040, 0x00, 0x06);        //  Shutdown
      Elmo_Delay(Delay2);
      Elmo_Write(elmo, 0x6060, 0x00, mode);        //  模式设定
      Elmo_Delay(Delay3);
			Elmo_Write(elmo, 0x6040, 0x00, 0x07);        //  SwitchOn
			Elmo_Delay(Delay3); 
			Elmo_Write(elmo, 0x6040, 0x00, 0x0f);        //  电机使能
	 		Elmo_Delay(Delay3);
		}
}
/*******************************************************************************
入口参数：
          speed              目标速度，单位是：counts/sec   
          acc                加速度
          dec                减速度
          QSDec              急停减速度
使用示例：Elmo_RunPVM(&elmo[0],-100000,300000,300000, 40000);        
*******************************************************************************/
//速度位置pv
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
  if (elmo->Mode.PVMode.Acc != acc)              // 设置加速度
  {
    elmo->Mode.PVMode.Acc = acc;
    Elmo_Write(elmo, 0x6083, 0x00, acc);
    Elmo_Delay(Delay1);
  }
  if (elmo->Mode.PVMode.Dec != dec)              // 设置减速度
  {
    elmo->Mode.PVMode.Dec = dec;
    Elmo_Write(elmo, 0x6084, 0x00, dec);
    Elmo_Delay(Delay1);
  }
  if (elmo->Mode.PVMode.QSDec != QSDec)          //  设置急停减速度
  {
    elmo->Mode.PVMode.QSDec = QSDec;
    Elmo_Write(elmo, 0x6085, 0x00, QSDec);
    Elmo_Delay(Delay1);  
  }  
  if (elmo->Mode.PVMode.Speed != speed)          //  设置目标速度，同时启动PVM
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
 入口参数：
           speed        最大速度（必须大于0）
           position     目标位置，单位：counts  
           acc          加速度 
           dec          减速度 
           runMode      P平滑运动模式的运行模式，详细如下：
                           ABS_POS       绝对位置
                           ABS_POS_IMM   绝对位置,立即运行
                           REL_POS_IMM   相对位置,立即运行
                           REL_POS       相对位置
使用示例：Elmo_RunPPM(&elmo[0],100000,1000000,300000,300000,REL_POS_IMM);
*/
void U_Elmo_RunPPM(Elmo* elmo, long speed, long position, long acc, long dec, long runMode)
{  
	
	 		 signed int buff[] = {13	,12,	13,	18	,264	,383	,525	,60	,80	,91	,
		 102,	108	,114
     };
	 u8 i=0;
	
   MY_Elmo_PDOWrite(elmo,"PT",1,4);//设置PT模式
	 delay_ms(200);
   MY_Elmo_PDOWrite_index(elmo,"MP",1,1,8);//设置PT模式 
	 delay_ms(200);
	 MY_Elmo_PDOWrite_index(elmo,"MP",2,13,8);//设置PT模式
	 delay_ms(200);
		MY_Elmo_PDOWrite_index(elmo,"MP",3,1,8);//设置PT模式
  for(i=0;i<13;i++)
  {
      MY_Elmo_PDOWrite_index(elmo,"QP",i+1,buff[i]/100,8);//设置PT模式
	    delay_ms(200);
		  MY_Elmo_PDOWrite_index(elmo,"QT",i+1,5*i,8);//设置PT模式
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
    Elmo_Write(elmo, 0x6040, 0x00, 0x0f);         //  Enable  Operation 多啦了这个 可以用的  必定有其特殊用处
    Elmo_Delay(Delay3);
  }
  if (elmo->Mode.PPMode.Speed != speed)           //  设置最大速度
  {
    elmo->Mode.PPMode.Speed = speed;
    Elmo_Write(elmo, 0x6081, 0x00, speed);
    Elmo_Delay(Delay2);
  }

  if(elmo->Mode.PPMode.Acc != acc)                //  设置加速度 加速度的限制
  {
    elmo->Mode.PPMode.Acc = acc;
    Elmo_Write(elmo, 0x6083, 0x00, acc);
    Elmo_Delay(Delay2);
  }
  if(elmo->Mode.PPMode.Dec != dec)                //  设置减速度
  { 
    elmo->Mode.PPMode.Dec = dec;
    Elmo_Write(elmo, 0x6084, 0x00, dec);
    Elmo_Delay(Delay2);
  }
    if(elmo->Mode.PPMode.Position != position)      //  设置目标位置
  {
		position = position*182;
    elmo->Mode.PPMode.Position = position;
    Elmo_Write(elmo, 0x607A, 0x00, position);
    Elmo_Delay(Delay2);
  }
  Elmo_Delay(Delay2);
  Elmo_Write(elmo, 0x6040, 0x00, runMode);        //  选择运行模式，同时启动PPM
  Elmo_Delay(Delay2);
  Elmo_Write(elmo, 0x6040, 0x00, 0x0f);           //  Enable  Operation
  Elmo_Delay(Delay2);
  Elmo_Write(elmo, 0x6040, 0x00, 0x0f);           //  Enable  Operation
  Elmo_Delay(Delay2);
	MY_Elmo_PDOWrite(elmo,"BG",0,8);//设置PT模式
}

void Elmo_RunPPM(Elmo* elmo, long speed, long position, long acc, long dec, long runMode)
{
  if (elmo->Mode.CurMode != &elmo->Mode.PPMode)
  {
    elmo->Mode.CurMode =& elmo->Mode.PVMode; 
    elmo->Mode.PVMode.Speed=0;
    Elmo_Write(elmo, 0x6040, 0x00, 0x06);        //  Shutdown
    Elmo_Delay(Delay2);
    Elmo_Write(elmo, 0x6060, 0x00, 0x01);        //  Enter PpMode 位置模式
    Elmo_Delay(Delay3);
    Elmo_Write(elmo, 0x6040, 0x00, 0x07);        //  SwitchOn
    Elmo_Delay(Delay3); 
    Elmo_Write(elmo, 0x6040, 0x00, 0x0f);        //  电机使能
    Elmo_Delay(Delay3);
  }
  if (elmo->Mode.PPMode.Speed != speed)           //  设置最大速度
  {
		
    elmo->Mode.PPMode.Speed = speed;
    Elmo_Write(elmo, 0x6081, 0x00, speed);
    Elmo_Delay(Delay2);
  }

  if(elmo->Mode.PPMode.Acc != acc)                //  设置加速度
  {
    elmo->Mode.PPMode.Acc = acc;
    Elmo_Write(elmo, 0x6083, 0x00, acc);
    Elmo_Delay(Delay2);
  }
  if(elmo->Mode.PPMode.Dec != dec)                //  设置减速度
  { 
    elmo->Mode.PPMode.Dec = dec;
    Elmo_Write(elmo, 0x6084, 0x00, dec);
    Elmo_Delay(Delay2);
  }
    if(elmo->Mode.PPMode.Position != position)      //  设置目标位置
  {
		position  = position*182;
    elmo->Mode.PPMode.Position = position;
    Elmo_Write(elmo, 0x607A, 0x00, position);
    Elmo_Delay(Delay2);
  }
  Elmo_Delay(Delay2);
  Elmo_Write(elmo, 0x6040, 0x00, runMode);        //  选择运行模式，同时启动PPM
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
//    Elmo_Write(elmo, 0x6060, 0x00, 0x01);        //  Enter PpMode 位置模式
//    Elmo_Delay(Delay3);
//    Elmo_Write(elmo, 0x6040, 0x00, 0x07);        //  SwitchOn
//    Elmo_Delay(Delay3); 
//    Elmo_Write(elmo, 0x6040, 0x00, 0x0f);        //  电机使能
//    Elmo_Delay(Delay3);
// // }
// // if (elmo->Mode.PPMode.Speed != speed)           //  设置最大速度
//  //{
//    //elmo->Mode.PPMode.Speed = speed;
//    Elmo_Write(elmo, 0x6081, 0x00, speed);//0x6081设置速度
//    Elmo_Delay(Delay2);
// // }

//  //if(elmo->Mode.PPMode.Acc != acc)                //  设置加速度
// // {
//    elmo->Mode.PPMode.Acc = acc;
//    Elmo_Write(elmo, 0x6083, 0x00, acc);
//    Elmo_Delay(Delay2);
//  //}
//  //if(elmo->Mode.PPMode.Dec != dec)                //  设置减速度
//  //{ 
//    elmo->Mode.PPMode.Dec = dec;
//    Elmo_Write(elmo, 0x6084, 0x00, dec);
//    Elmo_Delay(Delay2);
//  //}
//  //  if(elmo->Mode.PPMode.Position != position)      //  设置目标位置
//  //{
//    elmo->Mode.PPMode.Position = position;
//    Elmo_Write(elmo, 0x607A, 0x00, position);
//    Elmo_Delay(Delay2);
//  //}
////  Elmo_Delay(Delay2);
////  Elmo_Write(elmo, 0x6040, 0x00, runMode);        //  选择运行模式，同时启动PPM
////  Elmo_Delay(Delay2);
////  Elmo_Write(elmo, 0x6040, 0x00, 0x0f);           //  Enable  Operation
////  Elmo_Delay(Delay2);
////  Elmo_Write(elmo, 0x6040, 0x00, 0x0f);           //  Enable  Operation
////  Elmo_Delay(Delay2);
//}

/*******************************************************************************
入口参数:
          elmo          指向elmo结构体的指针
          torque        扭矩，范围：0 -> 2000(与用户设置的MAX_TORQUE有关)
   torque=DemandCurrent*1000/MotoRateCurrent 在6075中定义的 
使用示例: Elmo_RunPTM(&elmo[0],100);
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
      Elmo_Write(elmo, 0x6071, 0x00, torque); //  设置期望的扭矩
   //Elmo_PDOWrite(elmo,torque,torque);
      Elmo_Delay(Delay1);
  }
   Elmo_Delay(Delay3);
}


/*******************************************************************************
入口参数: elmo          指向elmo结构体的指针
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
  
  elmo->Mode = tModeInit;                   //  复位参数
}


/*******************************************************************************
//                       读取PPM是否到位                                 
入口参数: epos          指向epos结构体的指针                            
出口参数: Reached       PPM是否到位的标志量                             
                        0        没有到位                                  
                        Halt = 0: Target position not reached             
                        Halt = 1: Axle decelerates                        
                        1        到位                                      
                        Halt = 0: Target position reached                 
                        Halt = 1: Velocity of axle is 0                   
                        1           到位                                
                        0           没有到位                            
返回值:   无                                                            
*******************************************************************************/
/*
long Elmo_PositionGet(Elmo* elmo)
{
  tCANFrame FrameBuff;
  long Value;
  
  Elmo_Read(elmo, 0x6064, 0x0000);                                              //  发送读码盘的命令
  Elmo_Delay(Delay_Read);                                                       //  延时一段时间接收
  while (CAN_ucReadRcvBuffer(pReceFrameBuff, &FrameBuff) != EMPTY);             //  等待缓冲区中有数据
  Value = (FrameBuff.Data[7]<<24) | (FrameBuff.Data[6]<<16) |
          (FrameBuff.Data[5]<<8)  | (FrameBuff.Data[4]);
  return Value;
}

//============================================================================//
//==                        读取PPM是否到位                                 ==//
//============================================================================//
//==入口参数: epos          指向epos结构体的指针                            ==//
//==出口参数: Reached       PPM是否到位的标志量                             ==//
//==                     0        没有到位                                  ==//
//==                      Halt = 0: Target position not reached             ==//
//==                      Halt = 1: Axle decelerates                        ==//
//==                     1        到位                                      ==//
//==                      Halt = 0: Target position reached                 ==//
//==                      Halt = 1: Velocity of axle is 0                   ==//
//==                     1           到位                                ==//
//==                     0           没有到位                            ==//
//==返回值:   无                                                            ==//
//============================================================================//
unsigned long Elmo_PPMReached(Elmo* elmo)
{
  tCANFrame FrameBuff;
  unsigned long Reached; 
  Elmo_Read(elmo, 0x6041, 0x0000);                                   //  发送读码盘的命令
  Elmo_Delay(Delay3); 
  while (CAN_ucReadRcvBuffer(pReceFrameBuff, &FrameBuff) != EMPTY);  //  等待缓冲区中有数据
  Reached = (FrameBuff.Data[5]>>2)&0x01;
  return Reached;
}
*/
/******************************* END FILE *************************************/
