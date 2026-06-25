#define BAT_COMM_UPPER_GLOBALS
#include "bat_comm_upper.h"

// 包含使用模块 xxx_xxx_method.h
#include "bat_comm_modebus_method.h"
#include "bat_comm_custom_method.h"
#include "bat_comm_mec_method.h"


/***************************************************************************************
*函    数: void Bat_Comm_UpperTX_Mesg_Task(void)
*功    能: 发送进程
*参    数:  Bat_Mesg_Stru.commun_port需要指定电池串口端口       
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：电池支持两种：
*   第二种 自定义
****************************************************************************************/
void Bat_Comm_UpperTX_Mesg_Task(void)
{ 
    
    if(Bat_Mesg_Stru.commun_port == NULL)
    {
        return;
    }
    
    switch(Bat_Mesg_Stru.device_type)                      
    {
        case BAT_TYPE_CUSTOM:                               // 自定义协议
             Bat_Comm_HxCustomSend_Ask();
            break;
        case BAT_TYPE_MODBUS:                               // 自定义modebus协议
             Bat_Comm_Modbus_Send_Ask();                            
            break;
        case BAT_TYPE_MEC:                               // 自定义modebus协议
             Bat_Comm_MecSend_Ask();                            
            break;
        default:
            break;
    }
    
    
}

/***************************************************************************************
*函    数: void Bat_Comm_UpperRX_Task(void)
*功    能: 接收处理
*参    数:  Serial-对应串口发送结构体，QUEUE_com-对应接收串口结构体     
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
void Bat_Comm_UpperRX_Task(void) 
{
   
    switch(Bat_Mesg_Stru.device_type)                      
    {
        case BAT_TYPE_CUSTOM:                               // 自定义协议
             Bat_Comm_HxCustom_Handler();
            break;
        case BAT_TYPE_MODBUS:                               // 自定义modebus协议
             Bat_Comm_Modebus_Handler();                            
            break;
        case BAT_TYPE_MEC:                               // 自定义mec协议
             Bat_Comm_Mec_Handler();                            
            break;
        default:
            break;
    }
}

