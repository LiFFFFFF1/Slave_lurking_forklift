/*
*********************************************************************************************************
*	文件名: FIFO.c
*   作 者:  赵世泉
*	版 本:  V1.4.0
*	日 期： 2022-03-24
*	备 注: 
*********************************************************************************************************
*/
#define FIFO_GLOBALS
#include "FIFO.h"

/*
*********************************************************************************************************
*	函 数 名: void FIFO_BuffInit(QUEUE *QUEUE_com)
*	功能说明: 初始化
*	形    参：uint8_t element 
*	返 回 值: 无
*
*********************************************************************************************************
*/
void FIFO_BuffInit(QUEUE *QUEUE_com)
{
    QUEUE_com->FIFO_buffer_status = 0;
	QUEUE_com->read_buffer_status = 0;
	QUEUE_com->rear = 0;
	QUEUE_com->RecLen = 0;
	QUEUE_com->FIFO_Status_FULL = 0;
}
/*
*********************************************************************************************************
*	函 数 名: FIFO_ComQueueIn
*	功能说明: 入队列
*	形    参：uint8_t element 
*	返 回 值: 无
*   使用范例: xFIFO_ComQueueIn(test);   把test存在环形队列
*********************************************************************************************************
*/
void FIFO_ComQueueIn(QUEUE *QUEUE_com,u8 element)                       
{
     if(QUEUE_com->FIFO_buffer_status == 0)
     {
		QUEUE_com->fifo_bufferA[QUEUE_com->RecLen] = element;
	 }
	 else
	 {
		QUEUE_com->fifo_bufferB[QUEUE_com->RecLen] = element;
	 }
     QUEUE_com->RecLen++;
	 if(QUEUE_com->RecLen >= QUEUE_LENGTH)//写完一个缓冲区
     {
        QUEUE_com->RecLen = 0;
        if(QUEUE_com->FIFO_buffer_status == 0)
        {
            QUEUE_com->FIFO_buffer_status = 1;
		}
		else
		{
            QUEUE_com->FIFO_buffer_status = 0;
		}
	 }
}

/*********************************************************************************************************
*	函 数 名: FIFO_ComQueueOut
*	功能说明: 出队列
*	形    参：uint8_t* element
*	返 回 值: 返回值，空为0，出队列成功为1
* 使用范例: xFIFO_ComQueueOut(&test);  出队列的数存在test变量
*********************************************************************************************************
*/
u8 FIFO_ComQueueOut(QUEUE *QUEUE_com,u8 *Data_buf)
{   
   u16 len = 0;
   u8 temp=0;
   
    if(QUEUE_com == &COM1_Serial) 
   	{
       len = DMA_GetCurrDataCounter(DMA1_Channel5);
    }
    else if(QUEUE_com == &COM2_Serial) 
   	{
       len = DMA_GetCurrDataCounter(DMA1_Channel6);
    }  
    else if(QUEUE_com == &COM3_Serial) 
   	{
       len = DMA_GetCurrDataCounter(DMA1_Channel3);
    }  
    else if(QUEUE_com == &COM4_Serial) 
   	{
       len = DMA_GetCurrDataCounter(DMA2_Channel3);
    }   
    else
   	{
       len = QUEUE_LENGTH - QUEUE_com->RecLen;
    }
   
    if(QUEUE_com->read_buffer_status == QUEUE_com->FIFO_buffer_status)
   	{
   	   QUEUE_com->rear = QUEUE_LENGTH - len;
   	}  
    else
   	{
	   QUEUE_com->rear = QUEUE_LENGTH;
    }

    if(QUEUE_com->front < QUEUE_com->rear)                  //队列中有数据
   	{
   	     if(QUEUE_com->read_buffer_status == 0)             //获取一个字节的数据
		 {                                                  
			*Data_buf = QUEUE_com->fifo_bufferA[QUEUE_com->front];
		 }
		 else
		 {
			*Data_buf = QUEUE_com->fifo_bufferB[QUEUE_com->front];
		 }
          
         QUEUE_com->front++;                                //重新调整队列的长度
         if(QUEUE_com->front >= QUEUE_LENGTH)               //读完一个缓冲区
         {
            QUEUE_com->front = 0;
            if(QUEUE_com->read_buffer_status == 0)
            {
                QUEUE_com->read_buffer_status = 1;
            }
            else
            {
                QUEUE_com->read_buffer_status = 0;
            }
         }
         temp = FIFO_OK;
    }
    else
   	{
   	   if(QUEUE_com->front > QUEUE_com->rear)
   	   {
            QUEUE_com->front = QUEUE_com->rear;
	   }
	   temp = FIFO_ERROR_EMPTY;
    }   
    return temp;//返回队列为空
}

/***************************endif*****************************************/

