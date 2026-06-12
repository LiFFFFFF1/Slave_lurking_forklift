#ifndef _GYR_MPLS_METHOD_H
#define _GYR_MPLS_METHOD_H


// 使用到的底层基础文件
#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"	
#include "bsp_modbusrtu.h"

/* GYR-清0 指令*/   
extern void GYR_Mpls_ZeroSet_Task(void);  

/* GYR-发送查询任务*/
extern void GYR_Mpls_Send_Task(void);

/* GYR-接收处理任务*/
extern void GYR_Mpls_RX_Task(void);

/* GYR-CAN接收处理任务*/
extern void GYR_Mpls_CAN_RX_Handler(CanRxMsg* RxMessage);
#endif


