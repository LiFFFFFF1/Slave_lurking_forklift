#define MOTOR_DRIVE_GLOBALS
#include "motor_upper.h"

// 包含使用模块 xxx_xxx_method.h
#include "motor_lift_method.h"
#include "motor_fork_method.h"


 
/***************************************************************************************
*函    数: void Motor_UpperTX_Task(void)
*功    能: 发送进程
*参    数:  
*作    者:
*修改时间:
*返 回 值: 无
*备    注：
*   
****************************************************************************************/
void Motor_UpperTX_Task(void)
{
    switch(Motor_Lift_Stru.device_type)
    {
        case EM_DEVICE_DEV_MOTOR_LIFT_TY2:
            Lift_Task_Deal();
            break;
        case EM_DEVICE_DEV_MOTOR_FORK_WC2:
            Fork_Task_Deal();
            break;
        default:
            break;
    }


}

/***************************************************************************************
*函    数: void Motor_UpperRX_Task(void)
*功    能: 
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
*备    注：自定义协议接收
****************************************************************************************/
void Motor_UpperRX_Task(void)
{
    switch(Motor_Lift_Stru.device_type)
    {
        case EM_DEVICE_DEV_MOTOR_LIFT_TY2:                               
            break;
        case EM_DEVICE_DEV_MOTOR_FORK_WC2:                               
            break;
        default:
            break;
    }

}

/***************************************************************************************
*函    数: void Motor_UpperRX_CAN_Handler_Task(CanRxMsg* RxMessage)
*功    能: CAN接收-中断服务
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
*备    注：
****************************************************************************************/
void Motor_UpperRX_CAN_Handler_Task(CanRxMsg* RxMessage)
{
    switch(Motor_Lift_Stru.device_type)
    {
        case EM_DEVICE_DEV_MOTOR_LIFT_TY2:
            Motor_TY_CAN_RX_Handler(RxMessage);
            break;
        case EM_DEVICE_DEV_MOTOR_FORK_WC2:
            Motor_WC_CAN_RX_Handler(RxMessage);
            break;
        default:
            break;
    }

}



