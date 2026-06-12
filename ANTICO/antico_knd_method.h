#ifndef _ANTICO_ZS_METHOD_H
#define _ANTICO_ZS_METHOD_H


// 使用到的底层基础文件
#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"
#include "bsp_modbusrtu.h"
#include "plc_hub_custom_method.h"

/* 发送查询任务*/
extern void ANTICO_KND_Send_Task(void); 

/* 接收处理任务*/
extern void ANTICO_KND_RX_Task(void);

#endif


