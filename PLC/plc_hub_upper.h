#ifndef _PLC_HUB_UPPER_H
#define _PLC_HUB_UPPER_H


// 使用到的底层基础文件
#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"
#include "bsp_modbusrtu.h"
#include "bsp_gpio_dependency.h"

#include "public_def_type.h"

#ifdef PLC_HUB_UPPER_GLOBALS
#define PLC_HUB_UPPER_EXT
#else
#define PLC_HUB_UPPER_EXT  extern
#endif

#define    DEFAULT_ID    0x10//CAN默认id地址

#define    T_HARBE    200//心跳间隔时间，T*5ms
#define    T_SEND_DEFAULT    6//默认上传间隔时间，T*5ms

#define    T_FLASH_SLOW    200//慢闪时间T*5ms
#define    T_FLASH_FAST    100//快闪时间T*5ms
#define    T_FLASH_BRUST    50//爆闪时间T*5ms

typedef enum
{
    EM_INIT   = 0,                       //初始
    EM_NORM   = 1,                    // 正常

} EM_CANOPEN_STATE;                          // CAN状态


typedef enum
{
    EM_LIGHT_INIT = 0,//初始状态-全灭
    EM_LIGHT_STOP = 1,//停止
    EM_LIGHT_ERROR = 2,//故障
    EM_LIGHT_BZ_OFF = 3,//避障关闭
    EM_LIGHT_BZ_NEAR = 4,//近避障
    EM_LIGHT_LOWBAT = 5,//低电压
    EM_LIGHT_IPC_NOTE = 6,//IPC下发提示
    EM_LIGHT_HAND = 7,//手动
    EM_LIGHT_TURN_L = 8,//左转
    EM_LIGHT_TURN_R = 9,//右转
    EM_LIGHT_RUN = 0x0A,//运行
    EM_LIGHT_CCW = 0x0B,//左旋转
    EM_LIGHT_CW = 0x0C,//右旋转
    EM_LIGHT_IPC_OFFLINE = 0x0D,//IPC掉线
    
    EM_LIGHT_RCS_OFFLINE = 0x10,//RCS下线中
    
    EM_LIGHT_IPC_1 = 0x11,//IPC1-
    EM_LIGHT_IPC_2 = 0x12,//IPC2-
    EM_LIGHT_IPC_3 = 0x13,//IPC3-
    EM_LIGHT_IPC_4 = 0x14,//IPC4-
    EM_LIGHT_IPC_5 = 0x15,//IPC5-
    EM_LIGHT_IPC_6 = 0x16,//IPC6-
    EM_LIGHT_IPC_7 = 0x17,//IPC7-
    EM_LIGHT_IPC_8 = 0x18,//IPC8-
    EM_LIGHT_IPC_9 = 0x19,//IPC9-

} EM_LIGHT_STATE; //三色灯状态





typedef __packed struct
{
    // 配置参数
    USART_TypeDef* commun_port;                             // 设置通信端口号,串口端口
    u8 device_type;                                         // 设备类型
    u8 commun_ask_mode;                                     // 通信应答模式

    u32 baud_rate;                                          // 波特率
    u16 wordlength;                                         // 数据宽度
    u16 parity;                                             // 校验

    //---------------------------------------------------------------
    u8 err_id;//故障ID
    u8 err_data;//故障码

    //u8 off_line;//掉线状态

    u8 set_type;                    // 设置-类型
    u8 set_cmd;                     // 设置-命令码
    u8 set_mode;                    // 设置-模式
    u8 set_start_msg;               // 设置-启停信息
    u8 set_run_dir;                 // 设置-运行方向
    u8 set_run_sped;                // 设置-运行速度
    u8 set_run_obs;                 // 设置-运行区域
    u8 set_run_branch;              // 设置-运行分支
    s8 set_hand_sped[3];            // 设置-遥控速度
    float set_wheel_cir;            // 设置-行走轮周长
    float set_gear_ratio;           // 设置-行走齿轮比
    u8 set_walk_pid[3];             // 设置-行走轮PID参数
    u8 set_custom_cmdbit;           // 设置-自定义功能位
    u8 set_run_segment;             // 设置-走形磁段检测数
    u16 set_rfid_cur_val;           // 设置-rfid当前位置值
    u16 set_wrtie_rfid_val;         // 设置-写rfid读头下地标值

    u8  set_music_type;             // 设置音乐类型
    u8  set_music_id;               // 设置播放ID曲目
    u8  set_music_volume;           // 设置播放音量大小

    u8 upload_start_msg;            // 反馈-运行信号
    s8 upload_run_magnet;           // 走形数据
    u8 upload_run_warning;          // 警告提示信息
    u16 upload_rfid_site;           // 当前地标位置
    float upload_run_mileage;       // 运行里程

    u8 upolad_general_alarm;        // 常规报警
    u8 upload_obs_alarm;            // 壁障器故障代码
    u8 upload_rfid_alarm;           // RFID故障代码
    u8 upload_motor1_alarm;         // 电机1-故障代码
    u8 upload_motor2_alarm;         // 电机2-故障代码
    u8 upload_magnet_alarm;         // 走形-故障代码


    //--------------不上传定义

    u16 recv_over_time;
    u16 over_time_set;
    u8 set_walk_locat_sped;         // 设置-行走精准停车速度

    u8 canopen_state;//can状态
    u8 light_cmd;//三色灯状态
    u8 ctrl_cmd;//控制指令
    u8 charge_cmd;//充电命令
    u8 charge_state;//充电状态
    u8 ctrl_state;//控制状态
    u8 send_time;//上传周期

    u8 inputH_state;//输入H
    u8 inputL_state;//输入L
    u8 output_state;//输出

    u8 function_Code;//功能码
    u8 function_value;//功能值

    u8 enter_iap_update_flag;//进入IAP更新标志
    u16 enter_iap_update_dalay_timer;
    
} PLC_HUB_UPPER_STRU;


PLC_HUB_UPPER_EXT PLC_HUB_UPPER_STRU PLC_TO_HUB_Mesg_Stru;



extern void PLC_Hub_UpperTX_Mesg_Task(void);

extern void PLC_Hub_UpperRX_Task(void);

/* CAN接收中断 */
extern void Host_UpperRX_CAN_Handler_Task(CanRxMsg* RxMessage);


#endif


