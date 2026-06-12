#ifndef __BSP_GPIO_DEPENDENCY_H
#define __BSP_GPIO_DEPENDENCY_H

#include "sys.h"


/* 初始化端口IO */
extern void Bsp_GPIO_Pin_Init(void);

extern void Bsp_Read_AlarmIO(void);

extern void Bsp_IO_Deal_Task(void);

#endif

