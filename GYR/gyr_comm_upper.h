#ifndef _GYR_COMM_UPPER_H
#define _GYR_COMM_UPPER_H


#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"	
#include "bsp_modbusrtu.h"
#include "bsp_gpio_dependency.h"

#ifdef GYR_COMM_GLOBALS
   #define GYR_COMM_EXT  
#else
   #define GYR_COMM_EXT  extern
#endif
   
   
typedef enum
{
    GYR_TYPE_MPI240     = 1,                                // 曼普拉斯-MPI240陀螺仪

    
}GYR_TYPE_MODE;   
   
typedef enum
{
    GYR_CAN_MODE            = 1,                            //  CAN-通信模式
    GYR_COM_MODE            = 2,                            //  COM-模式
    
}GYR_CAN_COM_MODE;  

typedef enum
{
    GYR_ASK_AUTO   = 1,                                     //  主动上传
    GYR_ASK_HAND   = 2,                                     //  问询模式
    
}GYR_ASK_MODE; 

typedef __packed struct
{
    USART_TypeDef* commun_port;                             // 设置通信端口号,串口端口
    u8 can_com_type;                                        // 1-CAN, 2-com模式
    u8 device_type;                                         // 
    u8 device_adr;                                          // 设备地址
    u8 commun_ask_mode;                                     // 通信应答模式
    
    u32 baud_rate;                                          // 波特率
    u16 wordlength;                                         // 数据宽度
    u16 parity; 
    
    float angle;                                            // 当前角度
    float angle_speed;                                      // 当前角速度
    bool angle_clear_bit;                                   // 清零标志

    u16 angle_rawdata;                                  //角度-陀螺仪上传原始数据
    u16 angle_speed_rawdata;                        //角速度-陀螺仪上传原始数据

    u16 com_timer;                                      //通信计时

}GYR_COMM_STRU;

GYR_COMM_EXT GYR_COMM_STRU GYR_Mesg_Stru;

/* GYR对外接口-发送处理任务 */
extern void GYR_UpperTX_Task(void);

/* GYR对外接口-接收处理任务 */
extern void GYR_UpperRX_Task(void);

/* GYR-CAN中断接收处理 */
extern void GYR_UpperRX_CAN_Handler_Task(CanRxMsg* RxMessage);

#endif


