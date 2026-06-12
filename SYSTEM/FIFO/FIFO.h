#ifndef _FIFO_H_
#define _FIFO_H_

#ifdef FIFO_GLOBALS
   #define FIFO__EXT  
#else
   #define FIFO__EXT  extern
#endif

   #include "sys.h"
   #include <string.h>
   #include <stdio.h>	
   #include <stdarg.h>	 

#define FIFO_OK                 0
#define FIFO_ERROR_PARAM        1
#define FIFO_ERROR_MEM          2
#define FIFO_ERROR_FULL         3
#define FIFO_ERROR_EMPTY        4
#define FIFO_ERROR_BUSY         5
 
#define QUEUE_LENGTH           255

typedef struct
{
	unsigned int   front;                                   //队列头 	  
	unsigned int   rear;                                    //队列尾	
	unsigned int   RecLen;                                  //接收数据长度
	unsigned char  fifo_bufferA[QUEUE_LENGTH];
	unsigned char  fifo_bufferB[QUEUE_LENGTH];
	unsigned char  FIFO_Status_FULL;                        //队列满状态位
	unsigned char  FIFO_buffer_status;                      //缓冲区状态  
	unsigned char  read_buffer_status;                      //出队列读缓冲区数据
    volatile unsigned char  send_lock;
}QUEUE;
                                                            //接受串口
FIFO__EXT QUEUE COM1_Serial,COM2_Serial,COM3_Serial,COM4_Serial,COM5_Serial,COM6_Serial;      
                                                            //发送串口
FIFO__EXT QUEUE Serialusart1,Serialusart2,Serialusart3,Serialusart4,Serialusart5,Serialusart6;

FIFO__EXT void FIFO_ComQueueIn(QUEUE *QUEUE_com,u8 element) ;
FIFO__EXT u8   FIFO_ComQueueOut(QUEUE *QUEUE_com,u8 *Data_buf);
FIFO__EXT void FIFO_BuffInit(QUEUE *QUEUE_com);

#endif
/***************************endif*****************************************/

