#ifndef _ENCODER_UPPER_H
#define _ENCODER_UPPER_H


// 使用到的底层基础文件
#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"	
#include "bsp_modbusrtu.h"
#include "bsp_gpio_dependency.h"

 
#ifdef ENCODER_UPPER_GLOBALS
   #define ENCODER_UPPER_EXT  
#else
   #define ENCODER_UPPER_EXT  extern
#endif
   
    
typedef enum
{
    ENCODER_TPYE_TF    = 1,// 托菲编码器:   500k, 20ms, id=1+x
    ENCODER_TPYE_TBF    = 2,// 特倍福编码器:   500k, 10ms, id=3+x
    
} ENCODER_TYPE_MODE;   


typedef enum
{
    ENCODER_TPYE_CANOPEN  = 1,                              // 自定义--CANopen
    
}ENCODER_CAN_COM_MODE;


typedef __packed struct
{
    u8 device_type;  
    u8 can_com_type;     
    u8 commun_ask_mode;
    u8 device_adr;
    u8 id_number;

    u8 save_bit;
    
    u8 err_state;
    
    u16 com_timer[2];//通信计时

    u32 currt_data[2];//当前位置(原始计数)

    u32 offset_data[2];//零位值(原始计数)
        
    s32 real_data[2];//真实位置(mm)
    
}ENCODER_UPPER_STRU;


ENCODER_UPPER_EXT ENCODER_UPPER_STRU Encoder_Mesg_Stru;



extern void Encoder_UpperTX_Task(void);

extern void Encoder_UpperRX_CAN_Task(CanRxMsg* RxMessage);

#endif


