#ifndef _RFID_XS_METHOD_H
#define _RFID_XS_METHOD_H

#include "sys.h"
#include "FIFO.h"
#include "stdio.h"

#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"
#include "bsp_modbusrtu.h"

#ifdef RFID_XS_GLOBALS
#define RFID_XS_EXT
#else
#define RFID_XS_EXT  extern
#endif 


#define  RFID_XS_REC_LEN            18  //兴颂RFID读卡后上传数据长度


RFID_XS_EXT void RFID_XS_485_RX_Complete_Handler(void);
RFID_XS_EXT void RFID_XS_485_Write_Send(void);


#endif


