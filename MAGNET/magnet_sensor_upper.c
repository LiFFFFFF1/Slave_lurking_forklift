#define MAGNET_UPPER_GLOBALS
#include "magnet_sensor_upper.h"

// 包含使用模块 xxx_xxx_method.h
#include "magnet_mpls_sensor_method.h"
#include "magnet_xs_sensor_method.h"
#include "magnet_lg_sensor_method.h"


/***************************************************************************************
*函    数: void MAGNET_UpperTX_Mesg_Task(void)
*功    能: 查询模式
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：此函数根据每调用一次发送一次查询通信指令(查询模式有效)
****************************************************************************************/
void MAGNET_UpperTX_Mesg_Task(void)
{
    switch(Magnet_Upper_Stru.device_type)
    {
        /*case MAGNET_TPYE_MPLS:                              // MPLS走形传感器
            MAGNET_Mpls_Send_Mesg_Task();
            break;
        case EM_DEVICE_DEV_MAGNET_XS:                                // 兴颂-走形传感器
            MAGNET_Xs_Send_Mesg_Task();
            break;
		*/
		case EM_DEVICE_DEV_MAGNET_LG:								// 里格-走形
			MAGNET_LG_Send_Mesg_Task();						// 
			break;
        default:
            break;
    }
    
}

/***************************************************************************************
*函    数: void MAGNET_UpperRX_Task(void)
*功    能: 接收处理
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：
****************************************************************************************/
void MAGNET_UpperRX_Task(void)
{
    switch(Magnet_Upper_Stru.device_type)
    {
        /*case MAGNET_TPYE_MPLS:                              // MPLS走形传感器
            MAGNET_Mpls_COM_RX_Task();
            break;
        case EM_DEVICE_DEV_MAGNET_XS:                                // 兴颂-走形传感器
            MAGNET_Xs_COM_RX_Task();
            break;*/
		case EM_DEVICE_DEV_MAGNET_LG:								// 里格-走形
			MAGNET_LG_COM_RX_Task();
			break;
        default:
            break;
    }
    
}

/***************************************************************************************
*函    数: void MAGNET_UpperRX_CAN_Handler_Task(void)
*功    能: CAN接收-中断服务
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：
****************************************************************************************/
void MAGNET_UpperRX_CAN_Handler_Task(CanRxMsg* RxMessage)
{
    switch(Magnet_Upper_Stru.device_type)
    {
        /*case MAGNET_TPYE_MPLS:                              // MPLS走形传感器
            Magnet_MPLS_CAN_RX_Handler(RxMessage);
            break;
        case EM_DEVICE_DEV_MAGNET_XS:                                // 兴颂-走形传感器
            Magnet_XS_CAN_RX_Handler(RxMessage);
            break;*/
		case EM_DEVICE_DEV_MAGNET_LG:								// 里格-走形
			Magnet_LG_CAN_RX_Handler(RxMessage);
			break;
        default:
            break;
    }
    
}
