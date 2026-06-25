#ifndef _MOTOR_UPPER_H
#define _MOTOR_UPPER_H


#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"
#include "bsp_modbusrtu.h"
#include "bsp_gpio_dependency.h"
#include "public_def_type.h"


#ifdef MOTOR_DRIVE_GLOBALS
#define MOTOR_DEV_EXT 
#else
#define MOTOR_DEV_EXT  extern 
#endif

#define    LIFT_CANID_1    1//举升1  can_id
#define    LIFT_CANID_2    2//举升2  can_id

#define    FORK_CANID    1//叉齿起始 can_id


#define    INPUT_UP_TIMESET    25//上升输入消抖时间*20ms
#define    INPUT_DOWN_TIMESET    10//下降输入消抖时间*20ms

//举升升降超时时间*20ms
#define    LIFT_RUN_OVERTIME    1500

//举升到位默认高度mm
#define    DEFAULT_LIFT_HIGH_DATA    300

//举升默认速度*0.1rpm
#define    DEFAULT_LIFT_SPEED    5000

//举升电机方向
#define    LIFT_DIRET	-1


typedef enum
{
    EM_UNIT_ACT_ID_ALL     = 0,                   // 所有同步
    EM_UNIT_ACT_ID_LEFT    = 1,                   // 指定id1
    EM_UNIT_ACT_ID_RIGHT   = 2,                   // 指定id2

} EM_UNIT_ACT_ID;                                 // 上装执行ID


typedef __packed struct
{
    USART_TypeDef* commun_port;                   // 设置通信端口号,串口端口
    u8 can_com_type;                              // 1-CAN, 2-com模式
    u8 device_type;                               //
    u8 device_adr;                                // 设备地址
    u8 commun_ask_mode;                           // 通信应答模式

    u32 baud_rate;                                // 波特率
    u16 wordlength;                               // 数据宽度
    u16 parity;

    //u8 input_timer[4];
    u16 comm_timer[2];                            // 通信计时
    u16 run_timer;						//电机启动计时
    u16 speed_timer;					//速度检测计时
    u16 power_on_delay;					//上电延时计时器
        
    u16 current_data[2];					//电流
    u16 err_data[2];						//故障码
    s32 real_speed[2];					//当前速度
    s32 set_speed_front;					//设置速度-前
    s32 set_speed_back;					//设置速度-后
    s16 high_real_data[2];				//当前举升高度

    u8 all_lift_state;						//举升总状态
    u8 lift_state;						//前后举升状态
    u8 lift_action_set;					//举升动作设置
        
    s16 lift_offset[2];				    //举升零位偏移值
    s16 lift_target_highdata[2];		//举升目标高度

    float highdata_ratio;			    //举升高度系数，电机转1圈对应高度值

    u8 lift_en;							//举升使能

    u8 can_state;						//CAN设备网络状态，掉线1

    u8 act_syn;							//举升同步=0,   异步=1

    u8 unit_type;				// 上装动作类型
    u8 unit_act_id;			    // 上装执行id
    u8 unit_ctrl_mode;			// 上装控制模式
    s16 unit_ctrl_data;			// 上装控制参数

    u8 unit_act_state;			// 上装执行反馈状态
    s16 unit_act_data[2];	    // 上装执行反馈数据
    u16 unit_err_code[2];		// 上装故障代码
    
    u8 can_run_step;
    u16 can_send_cnt;
    u8 heart_node_cnt;
    
    s32 fork_dst_spd[2];        // 目标速度
    u16 fork_can_time[2];
    u16 fork_word_status[2];
    s16 fork_real_speed[2];     // 当前叉齿电机转速，单位 rpm
    u8  fork_location_cnt;

    float fork_motor_max_rpm;       // 叉齿行走电机最高转速，单位 rpm
    float fork_gear_ratio;          // 叉齿行走减速比
    float fork_wheel_diameter_mm;   // 叉齿行走轮直径，单位 mm
    float fork_drv_pulse_per_rev;   // 驱动器速度单位对应的每转计数
    float fork_max_speed_mm_s;      // 叉齿最大线速度，单位 mm/s
	
} MOTOR_DEV_STRU;

MOTOR_DEV_EXT MOTOR_DEV_STRU Motor_Lift_Stru;

/* 对外接口-发送处理任务 */
extern void Motor_UpperTX_Task(void);

/* 对外接口-接收处理任务 */
extern void Motor_UpperRX_Task(void);

/* CAN中断接收处理 */
extern void Motor_UpperRX_CAN_Handler_Task(CanRxMsg* RxMessage);

#endif


