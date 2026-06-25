/**
  ******************************************************************************
  * @file    myiic_m24c64_method.h
  * @author  zsq
  * @version V1.1.0
  * @date    14-04-2014
  * @brief   操作M24C64存储对外接口函数
  *
 @verbatim
  ******************************************************************************
  */
#ifndef __M24C64_IIC_H
#define __M24C64_IIC_H

#include "myiic_private_datatype.h"


#define AT24C01		127 
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191        // 最大存储字节数
#define AT24C128	16383
#define AT24C256	32767



//extern u8 device_m24C64_check(void);
extern void device_m24C64_read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);
extern void device_m24C64_write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);


#endif

