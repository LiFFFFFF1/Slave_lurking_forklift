/***
  ******************************************************************************
  * @file    myiic_upper.h
  * @author  zsq
  * @version V1.1.0
  * @date    10-04-2022
  * @brief   文件包含IIC对外所有函数接口
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************  
***/ 
#ifndef _MYIIC_UPPER_H_
#define _MYIIC_UPPER_H_


#include "stm32f10x.h"

extern void Myiic_init_api(void);                                   // 24c64  IIC硬件初始化              

extern int myiic_read_24c64_api(u16 addr, u16 lenth,u8 *redbuf);    // 24C64  读取参数接口
extern int myiic_write_24c64_api(u16 addr, u16 lenth,u8 *pbuf);     // 24c64  写入参数接口



#endif
