#define OBS_AREA_GLOBALS
#include "obs_area_upper.h"

// 包含使用模块 xxx_xxx_method.h
#include "obs_xs_method.h"
#include "obs_sick_method.h"



/***************************************************************************************
*函    数: void Obs_Area_UpperTX_Task(void)
*功    能: 发送进程
*参    数:       
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：电池支持两种：
*   第二种 自定义
****************************************************************************************/
void Obs_Area_UpperTX_Task(void)
{
    switch(OBS_Mesg_Stru.device_type)                      
    {
        case EM_DEVICE_DEV_OBS_XS_RS485:                               // 自定义协议
            Obs_AreaXs_Send_Task();								 //查询
            break;
            
        case EM_DEVICE_DEV_OBS_SICK_IO: 
            Obs_AreaSICK_IO_Send_Task();
            break;
            
        default:
            break;
    }
    
    
}

/***************************************************************************************
*函    数: void Obs_Area_UpperRX_Task(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：自定义协议接收
****************************************************************************************/
void Obs_Area_UpperRX_Task(void)
{
    switch(OBS_Mesg_Stru.device_type)
    {
        case EM_DEVICE_DEV_OBS_XS_RS485:                               //  
            if(OBS_Mesg_Stru.can_com_type == EM_SET_PORT_COM) 
            {
                Obs_AreaXs_RX_Task();
            }
            break;

        case EM_DEVICE_DEV_OBS_SICK_IO:                               //  
            Obs_AreaSICK_IO_RX_Task();
            break;
            
        default:
            break;
    }
    
}



