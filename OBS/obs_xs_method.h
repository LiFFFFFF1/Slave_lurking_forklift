#ifndef _OBS_XS_METHOD_H
#define _OBS_XS_METHOD_H


// 使用到的底层基础文件
#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"	
#include "bsp_modbusrtu.h"



/* OBS-发送查询任务*/
extern void Obs_AreaXs_Send_Task(void);

/*OBS-接收处理任务*/
extern void Obs_AreaXs_RX_Task(void);

/* OBS-IO输出任务*/
extern void Obs_AreaXs_IO_Send_Task(void);

/* OBS-IO输入任务*/
extern void Obs_AreaXs_IO_RX_Task(void);

#endif


