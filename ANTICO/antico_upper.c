#define ANTICO_GLOBALS
#include "antico_upper.h"

// 包含使用模块 xxx_xxx_method.h
#include "antico_knd_method.h"


 

/***************************************************************************************
*函    数: void ANTICO_UpperTX_Task(void)
*功    能: 发送进程
*参    数:   
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：
*   
****************************************************************************************/
void ANTICO_UpperTX_Task(void)
{
    switch(ANTICO_Mesg_Stru.device_type)                      
    {
        case EM_ANTICO_TYPE_KND:// 
            ANTICO_KND_Send_Task();
            break;
        
        default:
            break;
    }
    
    
}

/***************************************************************************************
*函    数: void ANTICO_UpperRX_Task(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：自定义协议接收
****************************************************************************************/
void ANTICO_UpperRX_Task(void)
{
    switch(ANTICO_Mesg_Stru.device_type)
    {
        case EM_ANTICO_TYPE_KND://
	    ANTICO_KND_RX_Task();
            break;
            
        default:
            break;
    }
    
}





