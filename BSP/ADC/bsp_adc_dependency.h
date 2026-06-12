#ifndef __ADC_H
#define __ADC_H


/* 包含头文件 ----------------------------------------------------------------*/
#include "sys.h"

/* 类型定义 ------------------------------------------------------------------*/
/* 宏定义 --------------------------------------------------------------------*/
// 注意：用作ADC采集的IO必须没有复用，否则采集电压会有影响
/********************ADC输入通道（引脚）配置**************************/

#define ADC1_Channel_Count 2   //采集通道数量   
#define ADC_Filter_Count   16  //ADC采集数字滤波个数

extern __IO uint16_t ADC_ConvertedValue[ADC_Filter_Count][ADC1_Channel_Count];             //采集数据存放数组，二维数组，16行 2列

extern uint16_t Current_Num;                                             //当前采集数量

extern uint32_t ADC_Channel_Sum[ADC1_Channel_Count];                     //每一通道的采样总值
extern uint16_t ADC_Channel_Max[ADC1_Channel_Count];                     //每一通道的采样最大值
extern uint16_t ADC_Channel_Min[ADC1_Channel_Count];                     //每一通道的采样最小值
extern uint16_t ADC_Everage_Value[ADC1_Channel_Count];                   //ADC采样，数字滤波后的最终平均数据


void ADC1_Init(void);

//ADC 采集 DAM传输完成中断回调函数
extern void (*ADC_DMA_Trans_Complete_CallBack)(void);

//void Get_Max(uint16_t * p,int length);//得到数组最大值

//void Get_Min(uint16_t * p,int length);//得到数组最小值

//void Filter_Handler(void);            //滤波算法，滑动 + 中位 + 平均滤波法

#endif


