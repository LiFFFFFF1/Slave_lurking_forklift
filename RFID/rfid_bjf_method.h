#ifndef _RFID_BJF_METHOD_H
#define _RFID_BJF_METHOD_H

#include "sys.h"
#include "FIFO.h"
#include "stdio.h"

#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"
#include "bsp_modbusrtu.h"

#ifdef RFID_BJF_GLOBALS
#define RFID_BJF_EXT
#else
#define RFID_BJF_EXT  extern
#endif  


#define  RFID_BJF_REC_LEN            7  //倍加福RFID读卡后上传数据长度


RFID_BJF_EXT void RFID_BJF_485_RX_Complete_Handler(void);
RFID_BJF_EXT void RFID_BJF_485_Write_Send(void);


#endif


