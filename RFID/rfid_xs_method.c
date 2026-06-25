#define RFID_XS_GLOBALS
#include "rfid_xs_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "rfid_upper.h"

// 私有定义
static u8 XS_Write_SendBuf[16] = {0};

/***************************************************************************************
*函    数: void RFID_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存数据一帧
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
static void RFID_XS_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
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
                    if(dat == 0x52)                               
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[0] = dat;
                        Serial->count = 1;
                    }
                    
                }
                
                else if(Serial->step== 1)//
                {
                    if(dat == 0x43)                          
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
                    if(dat == 0x6f)                             
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
                    if(dat == 0x64)                             
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

                else if(Serial->step== 4)//
                {
                    if(dat == 0x65)                             
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[4] = dat;
                        Serial->count = 5;
                    }
                    else
                    {
                        Serial->step =0;
                    }
                }
                
                else if(Serial->step== 5)//
                {
                    Serial->recv_pbuffer[Serial->count++] = dat;
                    if(Serial->count>=RFID_XS_REC_LEN)
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
*函    数: void RFID_485_RX_Complete_Handler(void)
*功    能: 接收处理
*参    数:   
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
void RFID_XS_485_RX_Complete_Handler(void)
{
    u16 crc_val=0;
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

    RFID_XS_ReadbufData(Serial,QUEUE_com);                   // 获取对应串口对接数据

    if(Serial->recv_complete_bit & 0x8000)
    { 
        /* CRC校验判断 */
        crc_val = Bsp_ModbusRTU_CRC(Serial->recv_pbuffer,RFID_XS_REC_LEN-2);
        // 校验 通过
        if(GET_LOW_BYTE(crc_val)==Serial->recv_pbuffer[RFID_XS_REC_LEN-2] &&     
           GET_HIGH_BYTE(crc_val)==Serial->recv_pbuffer[RFID_XS_REC_LEN-1])
        {
            //RFID编号
            RFID_Upper_Mesg_Stru.read_number  = (Serial->recv_pbuffer[8]&0x0f)*1000;   
            RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[9]&0x0f)*100;   
            RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[10]&0x0f)*10;   
            RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[11]&0x0f)*1;   
			
           
        }
        
        Serial->recv_complete_bit =0;
        
    }
    
}

/***************************************************************************************
*函    数: void RFID_XS_485_Write_Send(void)
*功    能: 
*参    数:    
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
void RFID_XS_485_Write_Send(void)//写卡时发送数据
{
    u16 crctemp = 0;
    
    if(RFID_Upper_Mesg_Stru.rfid_mode == RFID_WRITE_ONLY_MODE)
    {
        if(RFID_Upper_Mesg_Stru.write_number > 0 && RFID_Upper_Mesg_Stru.write_number < 9999)
        {  
            XS_Write_SendBuf[0] = 0x57;
            XS_Write_SendBuf[1] = 0x43;
            XS_Write_SendBuf[2] = 0x6f;
            XS_Write_SendBuf[3] = 0x64;
            XS_Write_SendBuf[4] = 0x65;
            XS_Write_SendBuf[5] = 0x01;
            XS_Write_SendBuf[6] = RFID_Upper_Mesg_Stru.write_number / 1000;//编号
            XS_Write_SendBuf[7] = (RFID_Upper_Mesg_Stru.write_number % 1000) / 100;//编号
            XS_Write_SendBuf[8] = (RFID_Upper_Mesg_Stru.write_number % 1000) % 100 / 10;//编号
            XS_Write_SendBuf[9] = (RFID_Upper_Mesg_Stru.write_number % 1000) % 100 % 10;//编号
            XS_Write_SendBuf[10] = 0x00;
            XS_Write_SendBuf[11] = 0x00;
            XS_Write_SendBuf[12] = 0x00;
            XS_Write_SendBuf[13] = 0x00;

            crctemp = Bsp_ModbusRTU_CRC(XS_Write_SendBuf,14);
            XS_Write_SendBuf[14] = crctemp&0x00ff;
            XS_Write_SendBuf[15] = (crctemp>>8)&0x00ff;
            
            Bsp_Usart_Usr_SendArray(RFID_Upper_Mesg_Stru.commun_port, &XS_Write_SendBuf[0],16);//发送数据
           
            
            RFID_Upper_Mesg_Stru.write_number = 0;//清零
            
        }

        RFID_Upper_Mesg_Stru.rfid_mode = RFID_READ_ONLY_MODE;
        
    }

}






















