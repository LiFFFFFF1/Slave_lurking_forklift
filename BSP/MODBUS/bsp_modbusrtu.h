#ifndef _BSP_MODBUSRTU_H_
#define _BSP_MODBUSRTU_H_

#include "sys.h"
#include "bsp_usart_dependency.h"	

#include "public_def_type.h"




u16 Bsp_ModbusRTU_CRC( unsigned char * pucFrame, unsigned short len);
u8 Bsp_CRC8_Check(u8* p_buffer, u8 buf_size);
u8 Bsp_XorCheck(u8* buf,u8 Lenth);
u8 Bsp_HexToASCII(u8  data_hex);
float Bsp_U32ToFloat(u32 data);

#endif

/***************************endif*****************************************/
