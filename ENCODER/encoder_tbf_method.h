#ifndef _ENCODER_TBF_METHOD_H
#define _ENCODER_TBF_METHOD_H


#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"
#include "bsp_modbusrtu.h"

#define    TBF_CAN_ID    3//起始can id


/* 发送处理任务 */
extern void Encoder_tbf_SendAPI(void);

/* 接收处理任务 */
extern void Encoder_tbf_CAN_Isr(CanRxMsg* RxMessage);


 
#endif
 

