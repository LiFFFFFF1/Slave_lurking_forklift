#ifndef _ENCODER_TF_METHOD_H
#define _ENCODER_TF_METHOD_H


#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"
#include "bsp_modbusrtu.h"

/* 发送处理任务 */
extern void Encoder_tf_SendAPI(void);

/* 接收处理任务 */
extern void Encoder_tf_CAN_Isr(CanRxMsg* RxMessage);
 


#endif
 

