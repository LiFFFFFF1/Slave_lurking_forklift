#ifndef _HOST_HUB_CC_METHOD_H
#define _HOST_HUB_CC_METHOD_H

// 包含函数参数 芯片头文件
#include "stm32f10x.h"



/* 发送处理任务 */
extern void Host_Hub_CAN_TX_Task(void);
  
 /* 接收处理任务 */
extern void Host_Hub_CAN_RX_Task(CanRxMsg* RxMessage);


#endif


