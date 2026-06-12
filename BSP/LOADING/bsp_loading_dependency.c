#define CONFIG_ARGUMENT_GLOBALS
#include "bsp_loading_dependency.h"
#include "delay.h"

// 底层基础
#include "bsp_can_dependency.h"
#include "bsp_gpio_dependency.h"
#include "bsp_time_dependency.h"
#include "bsp_usart_dependency.h"


// 包含使用模块 xxx_xxx_upper.h

#include "magnet_sensor_upper.h"
#include "plc_hub_upper.h"
#include "rfid_upper.h"
#include "motor_upper.h"
#include "antico_upper.h"
#include "encoder_upper.h"

#include "public_def_type.h"


/* CAN接收完成处理*/
void(*CAN_RX_Complete_Process[CAN_RX_API_LEN])(CanRxMsg* RxMessage);



/**
  * 函数功能: 参数配置加载
  * 返 回 值: 无
  * 说    明:
*/
void Bsp_Loading_Config_Parameters(void)
{

    //-------------------------COM端口配置----------------------------
    //上位机配置 
    PLC_TO_HUB_Mesg_Stru.device_type  = EM_DEVICE_DEV_HOST_MCU_RS232;// 设备参数配置
    PLC_TO_HUB_Mesg_Stru.over_time_set = 10000;//主机超时时间*1ms
    
    //防撞配置
    ANTICO_Mesg_Stru.device_type = EM_ANTICO_TYPE_KND;
    ANTICO_Mesg_Stru.can_com_type = EM_ANTICO_COM_MODE;
    
    //--------------------------COM end--------------------------


    //----------------CAN端口配置-------------------------
    //叉齿机构
    Motor_Lift_Stru.device_type = EM_DEVICE_DEV_MOTOR_FORK_WC2;
    Motor_Lift_Stru.can_com_type = EM_SET_PORT_CAN;
    Motor_Lift_Stru.fork_motor_max_rpm =3000.0f;
    Motor_Lift_Stru.fork_gear_ratio =20.0f;
    Motor_Lift_Stru.fork_wheel_diameter_mm =95.0f;
    Motor_Lift_Stru.fork_drv_pulse_per_rev =10000.0f;
    Motor_Lift_Stru.fork_max_speed_mm_s =(Motor_Lift_Stru.fork_motor_max_rpm/Motor_Lift_Stru.fork_gear_ratio)*
                                         3.14f*Motor_Lift_Stru.fork_wheel_diameter_mm/60.0f;

    //编码器
    Encoder_Mesg_Stru.device_type = ENCODER_TPYE_TBF;
    Encoder_Mesg_Stru.can_com_type = EM_SET_PORT_CAN;
    //----------------CAN end-----------------------------




    /*********************以下为具体参数配置*********************/

    /*-------------------------------上位机通信---------------------------------*/
    switch(PLC_TO_HUB_Mesg_Stru.device_type)
    {
        case EM_DEVICE_DEV_HOST_MCU_RS232:
            PLC_TO_HUB_Mesg_Stru.commun_ask_mode = EM_DEV_ASK_AUTO;    //主动上传模式
            PLC_TO_HUB_Mesg_Stru.commun_port   = USART3;                // 映射
            PLC_TO_HUB_Mesg_Stru.baud_rate      = 115200;
            PLC_TO_HUB_Mesg_Stru.wordlength     = USART_WordLength_8b;  // 数据长度
            PLC_TO_HUB_Mesg_Stru.parity         = USART_Parity_No;      // 无校验

            Bsp_Usart_Config_Init(PLC_TO_HUB_Mesg_Stru.commun_port, PLC_TO_HUB_Mesg_Stru.baud_rate,
                              PLC_TO_HUB_Mesg_Stru.wordlength, PLC_TO_HUB_Mesg_Stru.parity);
            break;
            
        case EM_DEVICE_DEV_HOST_MCU_CAN:
            PLC_TO_HUB_Mesg_Stru.commun_ask_mode = EM_DEV_CANOPEN;    //主动上传模式
            //CAN1_Init(6);// 3M/6=500K
            CAN_RX_Complete_Process[0] = Host_UpperRX_CAN_Handler_Task;
            
            break;
            
        default:
            break;

    }
    

    /*-------------------------------防撞通信---------------------------------*/
    switch(ANTICO_Mesg_Stru.device_type)
    {
	case EM_ANTICO_TYPE_KND:
    	    ANTICO_Mesg_Stru.commun_ask_mode = EM_ANTICO_ASK_MODBUS;
    	    ANTICO_Mesg_Stru.device_adr = 0x01;
            
            ANTICO_Mesg_Stru.commun_port   = UART5;                // 映射
            ANTICO_Mesg_Stru.baud_rate      = 9600;
            ANTICO_Mesg_Stru.wordlength     = USART_WordLength_8b;  // 数据长度
            ANTICO_Mesg_Stru.parity         = USART_Parity_No;      // 无校验

            Bsp_Usart_Config_Init(ANTICO_Mesg_Stru.commun_port, ANTICO_Mesg_Stru.baud_rate,
                              ANTICO_Mesg_Stru.wordlength, ANTICO_Mesg_Stru.parity);
            break;
            
        default:
            break;
    }


    //---------------------------------举升驱动器控制---------------------
    switch(Motor_Lift_Stru.device_type)
    {
        case EM_DEVICE_DEV_MOTOR_LIFT_TY2:
        case EM_DEVICE_DEV_MOTOR_FORK_WC2:
            if(Motor_Lift_Stru.can_com_type == EM_SET_PORT_CAN)      //  CAN通信
            {
                Motor_Lift_Stru.commun_ask_mode   =  EM_DEV_CANOPEN;    //
                CAN_RX_Complete_Process[1] = Motor_UpperRX_CAN_Handler_Task;
            }
            break;

        default:
            break;
    }

    //--------------------------------end---------------------------------------------

    //-----------------------------编码器------------------------------------------------
    switch(Encoder_Mesg_Stru.device_type)
    {
        case ENCODER_TPYE_TBF:
            if(Encoder_Mesg_Stru.can_com_type == EM_SET_PORT_CAN)      //  CAN通信
            {
                Encoder_Mesg_Stru.commun_ask_mode = EM_DEV_CANOPEN;    //
                CAN_RX_Complete_Process[2] = Encoder_UpperRX_CAN_Task;
            }
            break;
            
        default:
            break;
    }
    //--------------------------------end---------------------------------------------



    //485-1,  UART4设置初始化-  零动播放器
    Bsp_Usart_Config_Init(UART4, 9600, USART_WordLength_8b, USART_Parity_No);



    //CAN初始化配置
    if(PLC_TO_HUB_Mesg_Stru.device_type == EM_DEVICE_DEV_HOST_MCU_CAN
        ||Encoder_Mesg_Stru.can_com_type == EM_SET_PORT_CAN
            ||Motor_Lift_Stru.can_com_type == EM_SET_PORT_CAN)// || Motor_Lift_Stru.device_type
    {
        CAN1_Init(6);// 3M/6=500K
    }


}

