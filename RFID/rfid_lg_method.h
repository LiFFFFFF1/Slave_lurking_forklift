#ifndef _RFID_LG_METHOD_H
#define _RFID_LG_METHOD_H

#include "sys.h"
#include "FIFO.h"
#include "stdio.h"

#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"
#include "bsp_modbusrtu.h"

#ifdef RFID_LG_GLOBALS
#define RFID_LG_EXT
#else
#define RFID_LG_EXT  extern
#endif 


#define  RFID_LG_REC_LEN1            33  //里格RFID读卡后上传数据长度
#define  RFID_LG_REC_LEN2            25  //里格RFID无卡时上传数据长度

void RFID_LG_485_RX_Complete_Handler(u8 port_com);
RFID_LG_EXT void RFID_LG_485_Write_Send(void);
    

#endif


