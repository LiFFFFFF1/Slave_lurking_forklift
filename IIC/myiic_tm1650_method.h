/**
  ******************************************************************************
  * @file    myiic_tm1650_method.h
  * @author  zsq
  * @version V1.1.0
  * @date    14-04-2014
  * @brief   操作tm1650外接口函数
  *
 @verbatim
  ******************************************************************************
  */
#ifndef __TM1650_IIC_H
#define __TM1650_IIC_H

#include "myiic_private_datatype.h"


#define	SDA_COMMAND	    0X40
#define	DISP_COMMAND	0x80
#define	ADDR_COMMAND	0XC0

#define TM1650_BRIGHT1       0x11   /*1级亮度,开LED*/
#define TM1650_BRIGHT2       0x21   /*2级亮度,开LED*/
#define TM1650_BRIGHT3       0x31   /*3级亮度,开LED*/
#define TM1650_BRIGHT4       0x41   /*4级亮度,开LED*/
#define TM1650_BRIGHT5       0x51   /*5级亮度,开LED*/
#define TM1650_BRIGHT6       0x61   /*6级亮度,开LED*/
#define TM1650_BRIGHT7       0x71   /*7级亮度,开LED*/
#define TM1650_BRIGHT8       0x01   /*8级亮度,开LED*/
#define TM1650_DSP_OFF       0x00   /*关LED*/

#define TM1650_DIG1     0
#define TM1650_DIG2     1
#define TM1650_DIG3     2
#define TM1650_DIG4     3


extern void tm1650_iic_init_display(void);
extern void tm1650_iic_write_display(u8 dig,u8 set_data);


#endif
