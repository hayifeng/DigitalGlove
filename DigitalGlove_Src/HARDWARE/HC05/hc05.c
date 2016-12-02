#include "hc05.h"

void USART1_Send_Char(u8 c)
{
	while((USART1->SR&0X40)==0);//等待上一次发送完毕   
	USART1->DR=c;   	
} 

void USART1_Send_Data(u8 fun,u8*data,u8 len){
	u8 send_buf[34];
	u8 i;
	send_buf[len+3] = 0;									//校验数置零
	send_buf[0] = 0X88;										//帧头
	send_buf[1] = fun;										//功能码
	send_buf[2] = len;										//数据长度
	for(i=0; i < len; i++)send_buf[3+i] = data[i];							//复制数据
	for(i=0; i < len+3; i++)send_buf[len+3] += send_buf[i];			//计算校验和	
	for(i=0; i<len+4; i++)USART1_Send_Char(send_buf[i]);				//发送数据 
}

void HC05_Send_Data(short aacx, short aacy, short aacz, short pitch, short roll, short yaw, 
										short finger1, short finger2, short finger3, short finger4, short finger5,
										short joint1, short joint2, short joint3, short joint4)
{
	u8 tbuf[30]; 
	tbuf[0] = (aacx>>8)&0XFF;
	tbuf[1] = aacx&0XFF;
	tbuf[2] = (aacy>>8)&0XFF;
	tbuf[3] = aacy&0XFF;
	tbuf[4] = (aacz>>8)&0XFF;
	tbuf[5] = aacz&0XFF; 
	tbuf[6] =(pitch>>8)&0XFF;
	tbuf[7] = pitch&0XFF;
	tbuf[8] = (roll>>8)&0XFF;
	tbuf[9] = roll&0XFF;
	tbuf[10] = (yaw>>8)&0XFF;
	tbuf[11] = yaw&0XFF;
	tbuf[12] = (finger1>>8)&0XFF;
	tbuf[13] = finger1&0XFF;
	tbuf[14] = (finger2>>8)&0XFF;
	tbuf[15] = finger2&0XFF;
	tbuf[16] = (finger3>>8)&0XFF;
	tbuf[17] = finger3&0XFF; 
	tbuf[18] = (finger4>>8)&0XFF;
	tbuf[19] = finger4&0XFF;
	tbuf[20] = (finger5>>8)&0XFF;
	tbuf[21] = finger5&0XFF;
	tbuf[22] = (joint1>>8)&0XFF;
	tbuf[23] = joint1&0XFF;
	tbuf[24] = (joint2>>8)&0XFF;
	tbuf[25] = joint2&0XFF;
	tbuf[26] = (joint3>>8)&0XFF;
	tbuf[27] = joint3&0XFF;
	tbuf[28] = (joint4>>8)&0XFF;
	tbuf[29] = joint4&0XFF; 
	
	USART1_Send_Data(0XA1,tbuf,30);
}	
