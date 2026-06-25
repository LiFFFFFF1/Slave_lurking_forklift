
#define ENCODER_UPPER_GLOBALS
#include "encoder_upper.h"

// 包含使用模块 xxx_xxx_method.h
#include "encoder_tf_method.h"
#include "encoder_tbf_method.h"


/***************************************************************************************
*函    数: void Encoder_UpperTX_Task(void)
*功    能: 发送进程
*参    数:       
*作    者: 
*修改时间: 
*返 回 值: 无 
*备    注： 
*  
*  
****************************************************************************************/
void Encoder_UpperTX_Task(void)
{
    switch(Encoder_Mesg_Stru.device_type)
    {
        default:
            break;
            
        case ENCODER_TPYE_TF:
            Encoder_tf_SendAPI();
            break;

        case ENCODER_TPYE_TBF:
            Encoder_tbf_SendAPI();
            break;            
    }
    
}

/***************************************************************************************
*函    数: void Encoder_UpperRX_CAN_Task(void)
*功    能: 接收处理
*参    数:       
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：
*    
*  
****************************************************************************************/
void Encoder_UpperRX_CAN_Task(CanRxMsg* RxMessage)
{
    switch(Encoder_Mesg_Stru.device_type)
    {
        default:
            break;
            
        case ENCODER_TPYE_TF:
            Encoder_tf_CAN_Isr(RxMessage);
            break;
            
        case ENCODER_TPYE_TBF:          // 
            Encoder_tbf_CAN_Isr(RxMessage);
            break;
            
    }
    
}



