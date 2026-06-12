#ifndef _RFID_CK_METHOD_H
#define _RFID_CK_METHOD_H

#include "sys.h"
#include "FIFO.h"
#include "stdio.h"

#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"
#include "bsp_modbusrtu.h"

#ifdef RFID_CK_GLOBALS
#define RFID_CK_EXT
#else
#define RFID_CK_EXT  extern
#endif 


#define  RFID_CK_REC_LEN            20  //晨控RFID读卡后上传数据长度


void RFID_CK_232_RX_Complete_Handler(u8 port_com);
RFID_CK_EXT void RFID_CK_232_Write_Send(void);

#endif


