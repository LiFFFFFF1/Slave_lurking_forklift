#ifndef _ANTICO_UPPER_H
#define _ANTICO_UPPER_H


#include "bsp_usart_dependency.h"	
#include "bsp_modbusrtu.h" 
#include "bsp_gpio_dependency.h"

#ifdef ANTICO_GLOBALS
   #define ANTICO_EXIT  
#else
   #define ANTICO_EXIT  extern 
#endif

#define    ADC_VOLT_MIN    0.1f//触边断线最小电压/v
#define    ADC_VOLT_MAX    2.2f//触边触发报警电压/v

#define   INPUT_TURE     0//输入有效
#define   INPUT_FAUSE   1//输入无效
   
typedef enum
{
    EM_ANTICO_TYPE_KND     = 1,     // 康耐德PAX0200-模拟量检测模块
    
}ANTICO_TYPE_MODE;   
   
typedef enum
{
    EM_ANTICO_CAN_MODE            = 1,                            //  CAN-通信模式
    EM_ANTICO_COM_MODE            = 2,                            //  COM-模式485
    
}ANTICO_CAN_COM_MODE;  

typedef enum
{
    EM_ANTICO_ASK_AUTO   = 1,                                     //  主动上传
    EM_ANTICO_ASK_HAND   = 2,                                     //  问询模式
    EM_ANTICO_ASK_MODBUS   = 3,                                //  MODBUS协议
    
}ANTICO_ASK_MODE; 



typedef __packed struct
{
    USART_TypeDef* commun_port;                             // 设置通信端口号,串口端口

    u8 device_type;                                             // 
    u8 can_com_type;                                        // 1-CAN, 2-com模式
    u8 device_adr;                                          // 设备地址
    u8 commun_ask_mode;                          // 通信应答模式
    
    u32 baud_rate;                                          // 波特率
    u16 wordlength;                                         // 数据宽度
    u16 parity; 

    u16 power_on_delay;
    u16 com_timer;                                      //通信计时

    u16 lineoff_timer[2];
    u16 act_timer[2];

    u8 tray_connect_state;			//托盘连接状态
    u8 err_state;			   		    //错误信息

    u32 get_adc_data[2];				//模拟量电压原始数据
    float get_adc_voltage[2];			//模拟量电压数据单位/v

    //u8 real_state;					    //防撞状态
    

}ANTICO_COMM_STRU;

ANTICO_EXIT ANTICO_COMM_STRU ANTICO_Mesg_Stru;



/* GYR对外接口-发送处理任务 */
extern void ANTICO_UpperTX_Task(void);

/* GYR对外接口-接收处理任务 */
extern void ANTICO_UpperRX_Task(void);



#endif


