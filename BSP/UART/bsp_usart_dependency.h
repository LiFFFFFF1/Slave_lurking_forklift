#ifndef __BSP_USART_DEPENDENCY_H
#define __BSP_USART_DEPENDENCY_H
#include "stdio.h"	

#include "FIFO.h"
#include "sys.h" 

#ifdef USART_GLOBALS
   #define USART_EXT  
#else
   #define USART_EXT  extern
#endif

   
#define USART_REC_LEN  QUEUE_LENGTH  	//定义最大接收字节数 200
   
typedef __packed struct
{
    u16 recv_complete_bit;
    
	u8  recv_pbuffer[USART_REC_LEN];

	u8  step;
	u16 the_over_time;
	u16 count;
    
}USART_STRU;  
                                                            // 定义结构体
USART_EXT USART_STRU Usart1_Serial_Stru,Usart2_Serial_Stru,Usart3_Serial_Stru,Usart4_Serial_Stru,Usart5_Serial_Stru;        
 


/* 串口#1接收完成回调函数*/
//extern void(*Usart1_RX_Complete_CallBack)(uint8_t *buffer, uint8_t buffer_Len);

/* 串口#1发送完成回调函数*/
//extern void(*Usart1_TX_Complete_CallBack)(void);


   
extern void Bsp_Usart_Config_Init(USART_TypeDef* USARTx,u32 baud_rate,u16 wordlength,u16 parity);
extern void Bsp_Uart_Dma_SendCheck(void);

extern void Bsp_Usart_Usr_SendArray(USART_TypeDef* USARTx, u8 *data, u16 len);
   

#endif






