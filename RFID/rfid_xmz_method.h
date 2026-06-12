#ifndef _RFID_XMZ_METHOD_H
#define _RFID_XMZ_METHOD_H

#include "sys.h"
#include "FIFO.h"
#include "stdio.h"

#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"
#include "bsp_modbusrtu.h"

#ifdef RFID_XMZ_GLOBALS
#define RFID_XMZ_EXT
#else
#define RFID_XMZ_EXT  extern
#endif 


#define  RFID_XMZ_REC_LEN            10  //西门子RFID读卡后上传数据长度


RFID_XMZ_EXT void RFID_XMZ_232_RX_Complete_Handler(void);


#endif


