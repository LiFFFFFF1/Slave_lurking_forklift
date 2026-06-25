#define GYR_COMM_GLOBALS
#include "gyr_comm_upper.h"

// 包含使用模块 xxx_xxx_method.h
#include "gyr_mpls_method.h"




/***************************************************************************************
*函    数: void GYR_UpperTX_Task(void)
*功    能: 发送进程
*参    数:  Bat_Mesg_Stru.commun_port需要指定电池串口端口       
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：电池支持两种：
*   第二种 自定义
****************************************************************************************/
void GYR_UpperTX_Task(void)
{
    switch(GYR_Mesg_Stru.device_type)                      
    {
        case GYR_TYPE_MPI240:                               // 自定义协议
            if(GYR_Mesg_Stru.angle_clear_bit ==1)
            {
                GYR_Mpls_ZeroSet_Task();// 清0
            }
            else
            {
                GYR_Mpls_Send_Task();//查询
            }
            break;
        
        default:
            break;
    }
    
    
}

/***************************************************************************************
*函    数: void GYR_UpperRX_Task(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：自定义协议接收
****************************************************************************************/
void GYR_UpperRX_Task(void)
{
    switch(GYR_Mesg_Stru.device_type)
    {
        case GYR_TYPE_MPI240:                               //  MPI240陀螺仪
            if(GYR_Mesg_Stru.can_com_type == GYR_COM_MODE) 
            {
                GYR_Mpls_RX_Task();
            }
            break;
            
        default:
            break;
    }
    
}

/***************************************************************************************
*函    数: void GYR_UpperRX_CAN_Handler_Task(void)
*功    能: CAN接收-中断服务
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：
****************************************************************************************/
void GYR_UpperRX_CAN_Handler_Task(CanRxMsg* RxMessage)
{
    switch(GYR_Mesg_Stru.device_type)
    {
        case GYR_TYPE_MPI240:                             
            GYR_Mpls_CAN_RX_Handler(RxMessage);         
            break;
            
        default:
            break;
    }
    
}



