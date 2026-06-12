#ifndef _OBS_AREA_UPPER_H
#define _OBS_AREA_UPPER_H


#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"	
#include "bsp_modbusrtu.h"
#include "bsp_gpio_dependency.h"
#include "plc_hub_upper.h"

#include "public_def_type.h"

#ifdef OBS_AREA_GLOBALS
   #define OBS_AREA_EXT  
#else
   #define OBS_AREA_EXT  extern
#endif

////兴颂避障器-IO口定义
//#define OBS_XS_IO_IN1   MCU_GPIO_OUT3           //避障器输入1 -区域选择1
//#define OBS_XS_IO_IN2   MCU_GPIO_OUT4           //避障器输入2 -区域选择2
//#define OBS_XS_IO_IN3   MCU_GPIO_OUT5           //避障器输入3 -区域选择3
//#define OBS_XS_IO_IN4   MCU_GPIO_OUT6           //避障器输入4 -区域选择4

//#define OBS_XS_IO_OUT1   MCU_GPIO_IN5           //避障器输出1-最外
//#define OBS_XS_IO_OUT2   MCU_GPIO_IN6           //避障器输出2-中间
//#define OBS_XS_IO_OUT3   MCU_GPIO_IN7           //避障器输出3-最内
//#define OBS_XS_IO_OUT4   MCU_GPIO_IN8           //避障器输出4-故障


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
	
	u8 com_timer;
	
	
	
	u8 upload_cur_obs;										// 壁障器上报当前避障区域
	u8 upload_out1_state;									// 壁障器输出OUT1--最外
	u8 upload_out2_state;									// 壁障器输出OUT2--中间
	u8 upload_out3_state;									// 壁障器输出OUT3--最内
	
	
    
}OBS_COMM_STRU;

OBS_AREA_EXT OBS_COMM_STRU OBS_Mesg_Stru;

/* OBS对外接口-发送处理任务 */
extern void Obs_Area_UpperTX_Task(void);

/* OBS对外接口-接收处理任务 */
extern void Obs_Area_UpperRX_Task(void);


#endif


