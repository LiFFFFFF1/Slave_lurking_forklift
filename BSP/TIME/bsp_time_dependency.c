#include "bsp_time_dependency.h"


/**
  * 函数功能: 通用定时器3中断初始化
  * arr：自动重装值
  * psc：时钟预分频数
  * 返 回 值: 无
  * 说    明: 
  */
void TIM3_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);        //时钟使能

	TIM_DeInit(TIM3);
	TIM_TimeBaseStructure.TIM_Period = arr; 					 // 设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 					 // 设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 	 // 设置时钟分割: 0 不分割
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  // TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 			 // 根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update ,ENABLE);                    // TIM3 使能
		         		              
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;              // TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;    // 先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;           // 从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              // IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);                              // 根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM3, ENABLE);                                       // 使能TIMx外设							 
}

/**
  * 函数功能: TIM3中断处理函数
  * 输入参数：无
  * 返 回 值: 无
  * 说    明:  
  */
void TIM3_IRQHandler(void)                                      //TIM3中断
{	
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)           //检查指定的TIM中断发生与否:TIM 中断源 
	{		            
    	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);             //清除TIMx的中断待处理位:TIM 中断源 
		

	}
}


/*
//#define TIM4_ARR           (1000-1) //定时器 4 自动重装载值      
//#define TIM4_PSC           (72-1)   //定时器 4 预分频
void Tim4_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //时钟使能
	
	TIM_DeInit(TIM4);
	TIM_TimeBaseStructure.TIM_Period = arr; 
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); 
	
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);     //清除中断标志位
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE ); //使能tim4中断源 
		
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);
		
	TIM_Cmd(TIM4, ENABLE);	
}

void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{ 
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  
		
	}	  
}*/


