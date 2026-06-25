#ifndef _BAT_COMM_UPPER_H
#define _BAT_COMM_UPPER_H

// 使用到的底层基础文件
#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"	
#include "bsp_modbusrtu.h"
#include "bsp_gpio_dependency.h"


#ifdef BAT_COMM_UPPER_GLOBALS
   #define BAT_COMM_EXT  
#else
   #define BAT_COMM_EXT  extern  volatile
#endif


typedef enum
{
    BAT_TYPE_CUSTOM     = 1,                                // 自定义--协议
    BAT_TYPE_MODBUS     = 2,                                // 自定义--Modbus协议
    BAT_TYPE_MEC             = 3,                               //MEC电池---旧协议
     
}BAT_TYPE_MODE;

typedef enum
{
    BAT_CAN_MODE            = 1,                            //  CAN-通信模式
    BAT_COM_MODE            = 2,                            //  COM 模式
    
}BAT_CAN_COM_MODE;  

typedef __packed struct
{
    USART_TypeDef* commun_port;                             // 设置通信端口号,串口端口
    u8 device_type;                                         // 1-自定义协议，2-标准Modbus协议
    u8 can_com_type;                                        // 1-CAN, 2-com模式
    u8 device_adr;                                          // 标准Modbus协议设备地址
    
    u32 baud_rate;                                          // 波特率
    u16 wordlength;                                         // 数据宽度
    u16 parity; 
    
    u8 soc_percent;                                         // SOC百分比
    u16 voltage;                                            // 电压
    s16 current;                                            // 电流

    u16 com_timer;                                      //离线计时

}BAT_COMM_STRU;

BAT_COMM_EXT BAT_COMM_STRU Bat_Mesg_Stru;


/* 发送任务处理 */
extern void Bat_Comm_UpperTX_Mesg_Task(void);         

/* 接收任务处理 */
extern void Bat_Comm_UpperRX_Task(void);

#endif


