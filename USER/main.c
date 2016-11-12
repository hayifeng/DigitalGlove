#include "led.h"
#include "delay.h"
#include "sys.h"
#include "includes.h"
#include "Lcd_Driver.h"
#include "GUI.h"
#include "TFT_demo.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"	 

//-------------------------------------UCOSII任务设置----------------------------------------------
//START 任务
#define START_TASK_PRIO      						10 		//设置任务优先级,开始任务的优先级设置为最低
#define START_STK_SIZE  								64		//设置任务堆栈大小
OS_STK START_TASK_STK[START_STK_SIZE];				//任务堆栈	
void start_task(void *pdata);									//任务函数
 			   
//LED0任务
#define LED0_TASK_PRIO   			    			7 
#define LED0_STK_SIZE  		    					64
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
void led0_task(void *pdata);

//LED1任务
#define LED1_TASK_PRIO       						6 
#define LED1_STK_SIZE  									64
OS_STK LED1_TASK_STK[LED1_STK_SIZE];
void led1_task(void *pdata);

//adc转换任务
#define ADC_TASK_PRIO 									8
#define ADC_STK_SIZE										64
OS_STK ADC_TASK_STK[ADC_STK_SIZE];
void adc_task(void *pdata);
//-------------------------------------UCOSII任务设置结束----------------------------------------------

//-----------------------------------------全局变量----------------------------------------------------

#define ADC_CHLS 9 							//9路ADC
#define ADC_CNTS 10 						//每个ADC通道取10次值
u16 ADC_VALUES[ADC_CNTS][ADC_CHLS];			//存储ADC转换后M*N个数字量的数据
u16 ADC_VALUES_AVER[ADC_CHLS];					//每个ADC通道的平均值

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
	LED_Init();		  	//初始化与LED连接的硬件接口
	Lcd_Init();
	Adc_Init();
	MyDMA_Init(DMA1_Channel1,(u32)&ADC1->DR, (u32)&ADC_VALUES, ADC_CHLS*ADC_CNTS);	//初始化DMA
	uart_init(9600);
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
 	OSTaskCreate(led0_task,(void *)0,(OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],LED0_TASK_PRIO);						   
 	OSTaskCreate(led1_task,(void *)0,(OS_STK*)&LED1_TASK_STK[LED1_STK_SIZE-1],LED1_TASK_PRIO);	 				   
	OSTaskCreate(adc_task,(void *)0,(OS_STK*)&ADC_TASK_STK[ADC_STK_SIZE-1],ADC_TASK_PRIO);	 				   
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}

//LED0任务
void led0_task(void *pdata)
{	 	
	while(1){
		TFT_Test_Demo();
	}
	
	//OSTimeDly(60);
}

//LED1任务
void led1_task(void *pdata)
{	  
	while(1)
	{
		LED=0;
		delay_ms(300);
		LED=1;
		delay_ms(300);
		//OSTimeDly(30);
	};
}

void adc_task(void *pdata){
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能
	DMA_Cmd(DMA1_Channel1, ENABLE);				//使能DMA通道
	while(1){
		ADC_Filter();
		printf("%d %d %d\n", ADC_VALUES_AVER[1], ADC_VALUES_AVER[3], ADC_VALUES_AVER[8]);
		OSTimeDly(30);
	}
}



