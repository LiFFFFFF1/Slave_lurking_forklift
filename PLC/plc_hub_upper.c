#define PLC_HUB_UPPER_GLOBALS

#include "plc_hub_upper.h"

// 包含使用模块 xxx_xxx_method.h
#include "plc_hub_custom_method.h"
#include "host_hub_cc_method.h"


/***************************************************************************************
*函    数: void PLC_Hub_UpperTX_Mesg_Task(void)
*功    能: 发送进程
*参    数:       
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：
*  
*  
****************************************************************************************/
void PLC_Hub_UpperTX_Mesg_Task(void)
{

    switch(PLC_TO_HUB_Mesg_Stru.device_type)
    {
        case EM_DEVICE_DEV_HOST_MCU_RS232:                           // 四差速新协议
            PLC_Hub_CustomTX_Mesg_Ask();
            break;
            
        case EM_DEVICE_DEV_HOST_MCU_CAN:                  // 
            //Host_Hub_CAN_TX_Task();
            break;
            
        default:
            break;
    }
	
}

/***************************************************************************************
*函    数: void PLC_Hub_UpperRX_Task(void)
*功    能: 接收处理
*参    数:       
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：
*    
*  
****************************************************************************************/
void PLC_Hub_UpperRX_Task(void)
{
    
    switch(PLC_TO_HUB_Mesg_Stru.device_type)
    {
        case EM_DEVICE_DEV_HOST_MCU_RS232:                           
            PLC_Hub_CustomRX_Task();
            break;
        case EM_DEVICE_DEV_HOST_MCU_CAN:                           // 
            
            break;            
        default:
            break;
    }
}


/***************************************************************************************
*函    数: void Host_UpperRX_CAN_Handler_Task(void)
*功    能: CAN接收-中断服务
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：
****************************************************************************************/
void Host_UpperRX_CAN_Handler_Task(CanRxMsg* RxMessage)
{
    switch(PLC_TO_HUB_Mesg_Stru.device_type)
    {
        case EM_DEVICE_DEV_HOST_MCU_CAN:                           // 
            //Host_Hub_CAN_RX_Task(RxMessage);
            break;
            
        default:
            break;
    }
    
}







