#ifndef __BSP_CONFIG_DEPENDENCY_H
#define __BSP_CONFIG_DEPENDENCY_H 	


#include "sys.h"

#ifdef CONFIG_ARGUMENT_GLOBALS
   #define CONFIG_ARGUMENT_EXT  
#else
   #define CONFIG_ARGUMENT_EXT  extern
#endif


//CONFIG_ARGUMENT_EXT u8 DebugRegister[50];
   
#define   CAN_RX_API_LEN    3

/* CAN接收完成处理*/
extern void(*CAN_RX_Complete_Process[CAN_RX_API_LEN])(CanRxMsg* RxMessage);



/* 拨码模式导入参数 */
extern void Bsp_Loading_Config_Parameters(void);

#endif

