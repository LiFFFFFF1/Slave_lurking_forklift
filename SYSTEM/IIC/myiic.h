/**
  ******************************************************************************
  * @file    myiic.h
  * @author  zsq
  * @version V1.1.0
  * @date    10-04-2022
  * @brief   文件包含IIC对外所有函数接口
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************  
  */ 
#ifndef _MYIIC_H_
#define _MYIIC_H_

#include "sys.h" 
#include "delay.h"


#define    delay_us(x)    Bsp_Delay_US(x)
#define    delay_ms(x)    Bsp_Delay_MS(x)

//IIC所有操作函数
extern void Bsp_My_IIC_Init(void);                                                  //初始化IIC的IO口				 
extern void Bsp_My_IIC_Start(void);				                                    //发送IIC开始信号
extern void Bsp_My_IIC_Stop(void);	  			                                    //发送IIC停止信号
extern void Bsp_My_IIC_Send_Byte(u8 txd);			                                //IIC发送一个字节
extern u8 Bsp_My_IIC_Read_Byte(unsigned char ack);                                  //IIC读取一个字节
extern u8 Bsp_My_IIC_Wait_Ack(void); 				                                //IIC等待ACK信号
extern void Bsp_My_IIC_Ack(void);					                                //IIC发送ACK信号
extern void Bsp_My_IIC_NAck(void);				                                    //IIC不发送ACK信号

extern void Bsp_My_IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
extern u8 Bsp_My_IIC_Read_One_Byte(u8 daddr,u8 addr);	  


#endif

