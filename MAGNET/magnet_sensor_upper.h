#ifndef _MAGNET_UPPER_SENSOR_H
#define _MAGNET_UPPER_SENSOR_H


// 使用到的底层基础文件
#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"
#include "bsp_modbusrtu.h"
#include "bsp_gpio_dependency.h"

#include "plc_hub_upper.h"

#ifdef MAGNET_UPPER_GLOBALS
#define MAGNET_UPPER_EXT
#else
#define MAGNET_UPPER_EXT  extern
#endif



typedef enum
{
    EM_AGV_BRANCH_LINE   =0,       // 直行
    EM_AGV_BRANCH_LEFT   =1,       // 左转
    EM_AGV_BRANCH_RIGHT  =2,       // 右转

} EM_AGV_BRANCH_DIR;              // 运动分支


typedef __packed struct
{
    USART_TypeDef* commun_port;                             // 设置通信端口号,串口端口
    u8 device_type;                                         // 设备型号-类型
    u8 can_com_type;                                        // 1-CAN, 2-RS485模式
    u8 commun_ask_mode;                                     // 通信应答模式，1-主动上传，2-问答模式

    u8 canpen_state;                                        // canopen状态

    u8 id_number;                                           // 设备id个数
    u32 baud_rate;                                          // 波特率
    u16 wordlength;                                         // 数据宽度
    u16 parity;                                             // 校验

    u8 segment[4];                                          // 磁条段数
    //u8 group_number[4];                                   // 组号

    s8 offset_left[4];                                      // 第1段值
    s8 offset_middle[4];                                    // 第2段值
    s8 offset_right[4];                                     // 第3段值

//    s16 new_offset_left[4];                                      // 新协议-第1段值
//    s16 new_offset_middle[4];                                    //  新协议-第2段值
//    s16 new_offset_right[4];                                     //  新协议-第3段值

    u8 err_code[4];					//错误代码
    u8 err_data[4];					//错误参数
    u8 magnet_width[4];				//磁条宽度
    u8 io_num[4];					//传感器点位总数

    u8 new_io_low_low[4];			//点位信息1-低低
    u8 new_io_low_high[4];			//点位信息2-低高
    u8 new_io_high_low[4];			//点位信息3-高低
    u8 new_io_high_high[4];			//点位信息4-高高
    
    u8 io_low[4];                                           // 点位值
    u8 io_high[4];
    u32 magnet_comm_time[4];                                // 超时检查

    u8 branch_set;                                          //分支设置
    u8 error_state[4];                                      //错误状态
    s8 datatemp;

    u8 offline;                                             // 离线标志
    u8 canopen_recv_bit;                                    // can接收标志

} MAGNET_UPPER_STRU;

MAGNET_UPPER_EXT MAGNET_UPPER_STRU  Magnet_Upper_Stru;


/* 发送任务处理 */
extern void MAGNET_UpperTX_Mesg_Task(void);

/* 接收任务处理 */
extern void MAGNET_UpperRX_Task(void);

/* 走形-CAN中断接收处理 */
extern void MAGNET_UpperRX_CAN_Handler_Task(CanRxMsg* RxMessage);

#endif


