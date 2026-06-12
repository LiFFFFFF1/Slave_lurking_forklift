#ifndef _MOTOR_FORK_METHOD_H
#define _MOTOR_FORK_METHOD_H


// 使用到的底层基础文件
#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"	
#include "bsp_modbusrtu.h"





/*叉齿任务处理*/  
void Fork_Task_Deal(void); 

/* CAN接收处理任务*/
extern void Motor_WC_CAN_RX_Handler(CanRxMsg* rxmessage);


#endif


