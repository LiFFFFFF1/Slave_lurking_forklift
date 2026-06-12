#ifndef _MAGNET_LG_SENSOR_METHOD_H
#define _MAGNET_LG_SENSOR_METHOD_H


// 包含函数参数 芯片头文件
#include "stm32f10x.h"



// 发送查询函数
extern void MAGNET_LG_Send_Mesg_Task(void);

// COM接收处理函数
extern void MAGNET_LG_COM_RX_Task(void);

/* CAN接收中断 */
extern void Magnet_LG_CAN_RX_Handler(CanRxMsg* RxMessage);

#endif


