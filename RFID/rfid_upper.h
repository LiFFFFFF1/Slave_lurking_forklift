#ifndef _RFID_UPPER_H
#define _RFID_UPPER_H

// 使用到的底层基础文件
#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"
#include "bsp_modbusrtu.h"
#include "bsp_gpio_dependency.h"

#include "plc_hub_upper.h"

#ifdef RFID_UPPER_GLOBALS
    #define RFID_UPPER_EXT
#else
    #define RFID_UPPER_EXT  extern
#endif



typedef enum
{
    RFID_READ_ONLY_MODE     = 1,
    RFID_WRITE_ONLY_MODE    = 2,
    
}EM_MODE;



typedef __packed struct
{
    USART_TypeDef* commun_port;                             // 设置通信端口号,串口端口
    
    u8 id_number;                                           // 设备id个数
    u32 baud_rate;                                          // 波特率
    u16 wordlength;                                         // 数据宽度
    u16 parity;                                             // 校验

    u8 rfid_type;                                 			// RFID类型
    u8 rfid_mode;                                      		// 读/写模式
    u8 can_com_type;                                        // 模式
    u8 commun_ask_mode;                                     // 通信应答模式，1-主动上传，2-问答模式

    u8 init_state;                                          // 初始化标志
    u8 offline_state;                        				// 离线状态
    u16 commun_timer;                   					// 通信计时

    u16 read_number;                                     	// 读取的RFID编号
    u16 write_number;                                    	// 写入的RFID编号

    u16 id_number_history[10];                              // 读取的RFID编号历史记录  

} SENSOR_RFID_STRU;

RFID_UPPER_EXT SENSOR_RFID_STRU  RFID_Upper_Mesg_Stru;




extern void RFID_UpperTX_Mesg_Task(void);
extern void RFID_UpperRX_Task(void); 

#endif


