#include "led.h"
#include "delay.h"
#include "sys.h"
#include "includes.h"
#include <stdio.h>
#include "Lcd_Driver.h"
#include "GUI.h"
#include "TFT_demo.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"	
#include "hc05.h"
#include "mpu6050.h"
#include "inv_mpu.h"

//-------------------------------------UCOSII任务设置----------------------------------------------
//START 任务
#define START_TASK_PRIO      						10 		//设置任务优先级,开始任务的优先级设置为最低
#define START_STK_SIZE  								64		//设置任务堆栈大小
OS_STK START_TASK_STK[START_STK_SIZE];				//任务堆栈	
void start_task(void *pdata);									//任务函数
 			   
//LED任务
#define LED_TASK_PRIO   			    			9 
#define LED_STK_SIZE  		    					64
OS_STK LED_TASK_STK[LED_STK_SIZE];
void led_task(void *pdata);

//GUI任务
#define GUI_TASK_PRIO       						8 
#define GUI_STK_SIZE  									128
OS_STK GUI_TASK_STK[GUI_STK_SIZE];
void gui_task(void *pdata);

//ADC转换任务
#define ADC_TASK_PRIO 									6
#define ADC_STK_SIZE										64
OS_STK ADC_TASK_STK[ADC_STK_SIZE];
void adc_task(void *pdata);

//MPU6050任务
#define MPU6050_TASK_PRIO								5
#define MPU6050_STK_SIZE								128
OS_STK MPU6050_TASK_STK[MPU6050_STK_SIZE];
void mpu6050_task(void *pdata);

//HC05任务
#define HC05_TASK_PRIO									7
#define HC05_STK_SIZE										64
OS_STK HC05_TASK_STK[HC05_STK_SIZE];
void hc05_task(void *pdata);


//-------------------------------------UCOSII任务设置结束----------------------------------------------

//-----------------------------------------全局变量----------------------------------------------------

#define ADC_CHLS 9 							//9路ADC
#define ADC_CNTS 10 						//每个ADC通道取10次值
u16 ADC_VALUES[ADC_CNTS][ADC_CHLS];			//存储ADC转换后M*N个数字量的数据
u16 ADC_VALUES_AVER[ADC_CHLS];					//每个ADC通道的平均值
float pitch, roll, yaw;
short pitch_s, roll_s, yaw_s;
short aacx, aacy, aacz;
short aacx_t, aacy_t, aacz_t;
//---------------------------------------全局变量定义结束---------------------------------------------

//ADC滤波
void ADC_Filter(void)
{
	u8 i, j;
	u16 sum = 0;											
	for(i=0; i < ADC_CHLS; i++)							//每个通道进行循环滤波
	{
		for(j = 0; j < ADC_CNTS; j++)					//采样N次循环
		{
			 sum += ADC_VALUES[j][i];						//计算出N次采样值的总和
    }
    ADC_VALUES_AVER[i]= sum / ADC_CNTS;  //求出通道数字量的平均值
    sum = 0;                             //清零便于下次重新计算
  }
}



//------------------------------------------主函数-----------------------------------------------------

 int main(void)
 {	
	delay_init();	    	 //延时函数初始化	
  NVIC_Configuration();	 
	uart_init(9600);
	LED_Init();		  	//初始化与LED连接的硬件接口
	Lcd_Init();
	Adc_Init();
	MyDMA_Init(DMA1_Channel1,(u32)&ADC1->DR, (u32)&ADC_VALUES, ADC_CHLS*ADC_CNTS);	//初始化DMA
	while(MPU_Init() != 0) printf("mpu init error!");
	while(mpu_dmp_init() != 0)printf("mpu dmp init error!");
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	
 }

	  
//开始任务
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
  OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
 	OSTaskCreate(led_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);						   
 	OSTaskCreate(gui_task,(void *)0,(OS_STK*)&GUI_TASK_STK[GUI_STK_SIZE-1],GUI_TASK_PRIO);	 				   
	OSTaskCreate(adc_task,(void *)0,(OS_STK*)&ADC_TASK_STK[ADC_STK_SIZE-1],ADC_TASK_PRIO);	 				   
	OSTaskCreate(mpu6050_task,(void *)0,(OS_STK*)&MPU6050_TASK_STK[MPU6050_STK_SIZE-1],MPU6050_TASK_PRIO);	 				   
	OSTaskCreate(hc05_task,(void *)0,(OS_STK*)&HC05_TASK_STK[HC05_STK_SIZE-1],HC05_TASK_PRIO);	 				   
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}

//gui任务
void gui_task(void *pdata)
{	
	char pitch_str[16];
	char roll_str[16];
	char yaw_str[16];
	LCD_LED_SET;
	Lcd_Clear(GRAY0);
	Gui_DrawFont_GBK16(5,20,BLUE,GRAY0,"pitch:");
	Gui_DrawFont_GBK16(5,40,BLUE,GRAY0," roll:");
	Gui_DrawFont_GBK16(5,60,BLUE,GRAY0," yaw :");
	while(1){
		Gui_DrawFont_GBK16(70, 20, GRAY0, GRAY0, "          ");
		Gui_DrawFont_GBK16(70, 40, GRAY0, GRAY0, "          ");
		Gui_DrawFont_GBK16(70, 60, GRAY0, GRAY0, "          ");
		
		sprintf(pitch_str, "%+4d" , pitch_s);
		sprintf(roll_str, "%+4d", roll_s);
		sprintf(yaw_str, "%+4d", yaw_s);
		Gui_DrawFont_GBK16(70, 20, RED, GRAY0, pitch_str);
		Gui_DrawFont_GBK16(70, 40, RED, GRAY0, roll_str);
		Gui_DrawFont_GBK16(70, 60, RED, GRAY0, yaw_str);
		delay_ms(50);
	}
}

//LED任务
void led_task(void *pdata)
{	  
	while(1)
	{
		LED=0;
		delay_ms(300);
		LED=1;
		delay_ms(300);
	};
}

void adc_task(void *pdata){
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能
	DMA_Cmd(DMA1_Channel1, ENABLE);				//使能DMA通道
	while(1){
		ADC_Filter();
		//printf("%d %d %d %d\n", ADC_VALUES_AVER[0], ADC_VALUES_AVER[1], ADC_VALUES_AVER[2], ADC_VALUES_AVER[3] );
		delay_ms(30);
	}
}

void mpu6050_task(void *pdata){
	
	while(1){
		if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0){
			MPU_Get_Accelerometer(&aacx,&aacy,&aacz);
			pitch_s = (short)pitch + 90;
			roll_s = (short)roll + 180;
			yaw_s = (short)yaw + 180;
			aacx_t = aacx / 10 + 3276;
			aacy_t = aacy / 10 + 3276;
			aacz_t = aacz / 10 + 3276;
			//printf("pitch=%d roll=%d yaw=%d\n", pitch_s, roll_s, yaw_s);
			//printf("AacX=%d AacY=%d AacZ=%d\n", aacx_t, aacy_t, aacz_t);
			delay_ms(10);
		}
	}
}

void hc05_task(void *pdata){
	
	while(1){
		HC05_Send_Data(aacx_t, aacy_t, aacz_t, pitch_s, roll_s, yaw_s, 
										ADC_VALUES_AVER[0], ADC_VALUES_AVER[1], ADC_VALUES_AVER[2], 
										ADC_VALUES_AVER[3], ADC_VALUES_AVER[4], ADC_VALUES_AVER[5],
										ADC_VALUES_AVER[6], ADC_VALUES_AVER[7], ADC_VALUES_AVER[8]);
		delay_ms(20);
	}
}
