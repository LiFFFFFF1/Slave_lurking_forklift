/**
  ******************************************************************************
  * @file    public_def_type.h
  * @author  zsq
  * @version  v1
  * @date    2022-10-24
  * @brief   1、
  *
  ******************************************************************************
  */

#ifndef PUBLIC_DEF_TYPE_H
#define PUBLIC_DEF_TYPE_H

#ifdef  PUBLIC_GLOBALS_EXT
#define PUBLIC_EXT
#else
#define PUBLIC_EXT extern
#endif
/*******************************************************************************
************************************ Includes **********************************
*******************************************************************************/

//
#include <stdint.h>
#include <string.h>
#include "stdbool.h"
#include "math.h"
#include <stdio.h>
#include <stdarg.h>


// 系统文件
#include "sys.h"
#include "delay.h"
#include "myiic.h"

/*******************************************************************************
******************************* Local public define ******************************
*******************************************************************************/
typedef  uint32_t  u32;
typedef  uint16_t  u16;
typedef  uint8_t   u8;

//软件版本号
//命名规则: X0Y
//X值大版本号，范围: 0-9
//Y值小版本号，范围: 0-9
#define MCU_VISION     101// 从机版本号，不超过255
/*  版本修改记录
MCU   V100       单片机KIVA 车从板
MCU   V106       增加RCS下线指示灯语-停止状态绿灯快闪-2026年2月26日18:41:58
MCU   V107	 增加爆闪，增加IPC掉线灯语-2026年3月1日09:15:35	 
MCU   V108	 增加IPC下发提示灯语-2026年3月1日18:02:54

MCU   V110	修改为潜叉从板-2026年3月19日09:39:17

*/

#define  RUN_SYSPOWER    PEout(11)              // 电源指示灯
#define  RUN_SYS         PEout(12)              // 程序运行指示灯

#define  MCU_GPIO_IN1    PAin(5)
#define  MCU_GPIO_IN2    PAin(6)
#define  MCU_GPIO_IN3    PBin(0)
#define  MCU_GPIO_IN4    PBin(1) 
#define  MCU_GPIO_IN5    PBin(2)
#define  MCU_GPIO_IN6    PEin(7)
#define  MCU_GPIO_IN7    PEin(8)
#define  MCU_GPIO_IN8    PEin(9)
#define  MCU_GPIO_IN9    PBin(9)
#define  MCU_GPIO_IN10    PBin(8)


#define  MCU_GPIO_OUT1    PAout(4)          // 
#define  MCU_GPIO_OUT2    PAout(3)          // 
#define  MCU_GPIO_OUT3    PAout(0)
#define  MCU_GPIO_OUT4    PEout(6)
#define  MCU_GPIO_OUT5    PEout(5)
#define  MCU_GPIO_OUT6    PEout(4)
#define  MCU_GPIO_OUT7    PEout(3)
#define  MCU_GPIO_OUT8    PEout(2)

#define     Output_Ctrl_Shutdown            MCU_GPIO_OUT1//一键关机
#define     Output_Light_Red                MCU_GPIO_OUT2//三色灯-红
#define     Output_Light_Green              MCU_GPIO_OUT3//三色灯-绿
#define     Output_Light_Yellow_L           MCU_GPIO_OUT4//三色灯-黄左
#define     Output_Light_Yellow_R           MCU_GPIO_OUT5//三色灯-黄右
//#define     Output_Atico_Ctrl              MCU_GPIO_OUT6//触边屏蔽控制
#define     Output_Ctrl_Charge              MCU_GPIO_OUT7//充电继电器控制
//#define     Output_Lift_PowerOn    	    MCU_GPIO_OUT8//举升驱动上电
#define     Output_Lift_Break    	    MCU_GPIO_OUT8//举升电机抱闸


#define Input_Fork_Left_Atico              MCU_GPIO_IN1//叉齿左触边-常闭
#define Input_Fork_Right_Atico            MCU_GPIO_IN2//叉齿右触边-常闭
#define Input_OBS_FR_Error			MCU_GPIO_IN3	//前右避障器-故障	
#define Input_OBS_BL_Error			MCU_GPIO_IN4	//后左避障器-故障
#define Input_OBS_BR_Error			MCU_GPIO_IN5	//后右避障器-故障

#define Input_Lift_Front_Upside         MCU_GPIO_IN1	//  前举升升到位-常闭
#define Input_Lift_Front_Downside    MCU_GPIO_IN2	//  前举升降到位-常闭
#define Input_Lift_Back_Upside          MCU_GPIO_IN3	//  后举升升到位-常闭
#define Input_Lift_Back_Downside     MCU_GPIO_IN4	//  后举升降到位-常闭

//#define   Input_Charge_End    MCU_GPIO_IN8           //  充电放行
//#define   Input_Charge_AGVstop    MCU_GPIO_IN9        //  充电-AGV停止信号


//NPN低电平有效
#define    GET_INPUT_TURE      0//输入有效电平值
#define    GET_INPUT_FALSE    1//输入无效电平值


#define    OUTPUT_TURE      1//输出有效电平值
#define    OUTPUT_FALSE    0//输出无效电平值


#define    ERR_ID_PLC    1//故障ID-主机
#define    ERR_ID_PLAYER    2//故障ID-播放器
#define    ERR_ID_MOTOR_LIFT1     3//故障ID-举升电机1-前
#define    ERR_ID_MOTOR_LIFT2     4//故障ID-举升电机2-后


//位操作公用函数
#define setbit(x,y)     x|=((u32)1<<y)  //将X的第Y位置1
#define clrbit(x,y)      x&=~((u32)1<<y) //将X的第Y位清0
#define getbit(a,b)    (((u32)a>>b)&0x01)//获取a的第b位

#define GET_LOW_BYTE(i)   ((u8)(i) )
#define GET_HIGH_BYTE(i)  ((u8)( (i)>>8 & 0x00ff ))

#define MAKE_SHORT(a, b)  ((u16) (a<<8 | b))
#define MAKE_INT(a,b,c,d) ((u32) (a<<24 | b<<16 | c<<8 | d))

/*******************************************************************************
******************************* Local public data ******************************
*******************************************************************************/


typedef enum
{
    EM_DEV_ASK_AUTO   = 1,                       //  主动上传
    EM_DEV_ASK_HAND   = 2,                       //  问询模式
    EM_DEV_CANOPEN    = 3,                       //  CANOPEN模式
    EM_DEV_USE_CAN    = 4,                       //  自定义CAN模式

} EM_DEV_PORT_ASK_MODE;                          // 端口-应答模式


typedef enum
{
    EM_SET_PORT_NONE     = 0,                   // 无端口配置
    EM_SET_PORT_CAN      = 1,                   //  CAN-通信模式
    EM_SET_PORT_COM      = 2,                   //  COM模式


} EM_SET_CAN_COM_TYPE;                          // 通信接口


typedef enum
{
    EM_SET_NON     = 0,                   // 无效
    EM_SET_SLIDER_ON      = 1,       //  地充-开启
    EM_SET_SLIDER_OFF      = 2,       //  地充-关闭
    EM_SET_EXTENDER_ON      = 3,     //  侧充-开启
    EM_SET_EXTENDER_OFF      = 4,     //  侧充-关闭

} EM_CHARGE_CMD;                          // 充电命令


typedef enum
{
    EM_NON     = 0,                   // 未知
    EM_CHARGING_ON      = 1,       // 充电中
    EM_CHARGING_OFF      = 2,       // 充电停止

} EM_CHARGE_STATE;                          // 充电状态

typedef enum
{
    EM_LIFT_ACT_START      = 1,       		// 上电启动
    EM_LIFT_ACT_RESET      = 10,       		// 复位
    EM_LIFT_ACT_STOP       = 20,                  //停止
    EM_LIFT_ACT_RUN_UP  = 30,       		// 上升
    EM_LIFT_ACT_RUN_DOWN  = 40,       	// 下降
} EM_LIFT_ACTION; //举升动作

typedef enum
{
    EM_LIFT_STATE_STOP     = 0,                       //停止
    EM_LIFT_STATE_UPSIDE          = 1,       			// 上升到位
    EM_LIFT_STATE_DOWNSIDE      = 2,       			// 下降到位
    EM_LIFT_STATE_RUNING     		= 4,       		// 举升过程中
    
} EM_LIFT_STATE;                          //举升状态


typedef enum
{
    EM_MOTOR_MODE_PP    = 1,                       //位置模式
    EM_MOTOR_MODE_PV    = 3,       		   //速度模式
    
} EM_MOTOR_MODE;                          //电机控制模式



typedef enum
{
    EM_DEVICE_DEV_WHEEL_XS       = 1,            //  兴颂行走驱动设备
    
    EM_DEVICE_DEV_MOTOR_LIFT_TY2       = 5,            //  举升驱动器-同毅*2
    EM_DEVICE_DEV_MOTOR_FORK_WC2       = 6,            //  叉齿驱动器-纬创*2

    EM_DEVICE_DEV_MAGNET_LG      = 20,           //  里格磁导航设备
    EM_DEVICE_DEV_MAGNET_MPLS    = 21,
    EM_DEVICE_DEV_MAGNET_XS      = 22,

    EM_DEVICE_DEV_RFID_LG        = 30,           //  里格RFID设备
    EM_DEVICE_DEV_RFID_CK        = 31,           //  晨控RFID设备

    EM_DEVICE_DEV_OBS_XS_RS485           = 40,           //  兴颂-壁障器RS485设备
    EM_DEVICE_DEV_OBS_SICK_IO          = 41,           //  SICK-壁障器IO设备

    EM_DEVICE_DEV_HOST_MCU       = 50,           //  主机设备-单片机

    EM_DEVICE_DEV_HOST_MCU_CAN       = 51,           //  主机设备-单片机叉车

    EM_DEVICE_DEV_HOST_MCU_RS232    =52,

} EM_DEVICE_DEV_TYPE;                           // 设备类型




/*
        增量式PID
PID =Kp[e(k)-e(k-1)]+Ki*e(k)+kd*[e(k)-2e(k-1)+e(k-2)]
e(k)-e(k-1):这次误差-上次误差
e(k):误差
e(k)-2e(k-1)+e(k-2):这次误差-2*上次误差+上上次误差

调整方法：
1  先比例，再积分，最后再把微分加；
2  增大比例，减小积分
*/
typedef __packed struct
{
    s8 curren_err;   // 当前误差
    s8 last_err;     // 上一次误差
    s8 previous_err; // 上上次误差

    float ukp;        // kp系数
    float uki;        // Ki系数 =Kp*(uCyc/uTi)
    float ukd;        // kd系数 =Kp*(uTd/uCyc)


//   float uTi;   // 积分时间常数
//   float uTd;   // 微分时间常数
//   float uCyc;  // 采用周期
//   u8 uCyc;         // 采用周期

    float throsd;     // 阀值

    double outval;     // 输出+代表需要增加，-代表减小
} PID_VAL_STRU;


// PID调节
extern void public_pid_read_val(s8 NowPlace, s8 Point,PID_VAL_STRU *set_cur_pid);


#endif
/********************************* end of file ********************************/

