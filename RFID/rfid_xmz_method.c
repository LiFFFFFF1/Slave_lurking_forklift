#define RFID_XMZ_GLOBALS
#include "rfid_xmz_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "rfid_upper.h"


/***************************************************************************************
*函    数: void RFID_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存数据一帧
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
static void RFID_XMZ_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
{
    u8 dat=0,readstatus=0;

    if((Serial->recv_complete_bit & 0x8000) == 0)
    {
        while(1)
        {
            readstatus = FIFO_ComQueueOut(QUEUE_com,&dat);

            if(readstatus == FIFO_OK)
            {
                Serial->the_over_time =0;
                
                if(Serial->step==0)//
                {
                    if(dat >= 0x30&&dat <= 0x39)                               
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[0] = dat;
                        Serial->count = 1;
                    }
                    
                }
                
                else if(Serial->step== 1)//
                {
                    if(dat >= 0x30&&dat <= 0x39)                          
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[1] = dat;
                        Serial->count = 2;

                    }
                    else
                    {
                        Serial->step =0;
                    }
                }

                else if(Serial->step== 2)//
                {
                    if(dat >= 0x30&&dat <= 0x39)                          
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[2] = dat;
                        Serial->count = 3;

                    }
                    else
                    {
                        Serial->step =0;
                    }
                }

                else if(Serial->step== 3)//
                {
                    if(dat >= 0x30&&dat <= 0x39)                          
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[3] = dat;
                        Serial->count = 4;

                    }
                    else
                    {
                        Serial->step =0;
                    }
                }
                
                else if(Serial->step== 4)
                {
                    Serial->recv_pbuffer[Serial->count++] = dat;
                    
                    if(Serial->count >= RFID_XMZ_REC_LEN)
                    {

                        Serial->recv_complete_bit=Serial->count;
                        Serial->recv_complete_bit |= 0x8000;

                        Serial->step = 0;
                        Serial->count = 0;
                        return;
                        
                    }
                }
                
            }
            
            else if(readstatus == FIFO_ERROR_EMPTY)
            {
                Serial->the_over_time++;
                if(Serial->the_over_time>=100)
                {
                    Serial->the_over_time =0;
                    Serial->step = 0;
                    Serial->count = 0;
                }
                return ;
            }
            
        }

    }
    
}


/***************************************************************************************
*函    数: void RFID_232_RX_Complete_Handler(void)
*功    能: 接收处理
*参    数:   
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
void RFID_XMZ_232_RX_Complete_Handler(void)
{
    USART_STRU *Serial;
    QUEUE *QUEUE_com;
    
    /* 非COM模式下 */
    if(RFID_Upper_Mesg_Stru.can_com_type != EM_SET_PORT_COM )
    {
        return;
    }
    
    if(RFID_Upper_Mesg_Stru.commun_port == USART1)
    {
        Serial    = &Usart1_Serial_Stru;
        QUEUE_com = &COM1_Serial;
    }
    else if(RFID_Upper_Mesg_Stru.commun_port == USART2)
    {
        Serial    = &Usart2_Serial_Stru;
        QUEUE_com = &COM2_Serial;
    }
    else if(RFID_Upper_Mesg_Stru.commun_port == USART3)
    {
        Serial    = &Usart3_Serial_Stru;
        QUEUE_com = &COM3_Serial;
    }
    else if(RFID_Upper_Mesg_Stru.commun_port == UART4)
    {
        Serial    = &Usart4_Serial_Stru;
        QUEUE_com = &COM4_Serial;
    }
    else if(RFID_Upper_Mesg_Stru.commun_port == UART5)
    {
        Serial    = &Usart5_Serial_Stru;
        QUEUE_com = &COM5_Serial;
    }
    else
    {
        return;
    }

    RFID_XMZ_ReadbufData(Serial, QUEUE_com);                   // 获取对应串口对接数据

    if(Serial->recv_complete_bit & 0x8000)
    {         
        // 校验 通过
        if(Serial->recv_pbuffer[RFID_XMZ_REC_LEN-1] == 0x0A
            &&Serial->recv_pbuffer[RFID_XMZ_REC_LEN-2] == 0x0D)
        {
            //RFID编号
            RFID_Upper_Mesg_Stru.read_number   = (Serial->recv_pbuffer[0]-0x30)*1000;   
            RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[1]-0x30)*100;   
            RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[2]-0x30)*10;   
            RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[3]-0x30)*1;   
          
        }
        
        Serial->recv_complete_bit =0;
        
    }
    
}




