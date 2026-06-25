#include "music_method.h"

// 包含使用模块 xxx_xxx_upper.h


/***************************************************************************************
*函    数: void music_com_read_buf(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
*备    注：自定义协议接收
****************************************************************************************/
static void music_com_read_buf(USART_STRU *Serial,QUEUE *QUEUE_com)
{
    u8 dat=0,readstatus=0;

    while(1)
    {
        readstatus = FIFO_ComQueueOut(QUEUE_com,&dat);

        if(readstatus == FIFO_OK)
        {
            Serial->the_over_time =0;

            switch(Serial->step)
            {
                case 0:
                    Serial->recv_pbuffer[0] = dat;
                    if(dat == 0x01)
                    {
                        Serial->step = 1;
                        Serial->count =1;
                    }
                    else
                    {
                        Serial->count =0;
                    }
                    break;
                case 1:
                    Serial->recv_pbuffer[1] = dat;
                    Serial->count =2;
                    if(dat==0x06 || dat==0x10)
                    {
                        Serial->step  = 2;
                    }
                    else
                    {
                        Serial->count = 0;
                        Serial->step  = 0;
                    }
                    break;
                case 2:                     // 0x03+0x06
                    Serial->recv_pbuffer[Serial->count++] = dat;
                    if(Serial->count >= 8)
                    {
                        Serial->recv_complete_bit = Serial->count;
                        Serial->recv_complete_bit |= 0x8000;
                        Serial->step = 0;
                        Serial->count = 0;
                        return;
                    }
                    break;
            }
        }
        else if(readstatus == FIFO_ERROR_EMPTY)
        {
            Serial->the_over_time++;
            if(Serial->the_over_time>=10)
            {
                Serial->the_over_time =0;
                Serial->step = 0;
                Serial->count = 0;
            }
            return;
        }
    }
}


/***************************************************************************************
*函    数: void Obs_AreaXs_RX_Task(void)
*功    能:
*参    数:  Serial-对应串口发送结构体，QUEUE_com-对应接收串口结构体
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void Music_Play_Send_API(u8 music_type,u8 music_volume)
{
    static u8 _upload_music_number=0xFF, music_send_cnt=0;
    u16 u16_crc_temp=0;
    u8 ack_buf[15]= {0};

    //接收
    music_com_read_buf(&Usart4_Serial_Stru,&COM4_Serial);            // 自定义解码
    
    if(Usart4_Serial_Stru.recv_complete_bit & 0x8000)
    {
        u16_crc_temp = Bsp_ModbusRTU_CRC(Usart4_Serial_Stru.recv_pbuffer,6);

        if((Usart4_Serial_Stru.recv_pbuffer[6] == GET_LOW_BYTE(u16_crc_temp)) &&
           (Usart4_Serial_Stru.recv_pbuffer[7] == GET_HIGH_BYTE(u16_crc_temp)))
        {
            if(music_type ==0) // 萨驰播放器
            {
                _upload_music_number = Usart4_Serial_Stru.recv_pbuffer[5];    // 音乐编号
            }
        }
        Usart4_Serial_Stru.recv_complete_bit =0;
    }

    //发送
    switch(music_type)
    {
        case 0:// 萨驰485
            if(_upload_music_number !=PLC_TO_HUB_Mesg_Stru.set_music_id)
            {
                music_send_cnt++;
            }
            else
            {
                music_send_cnt =0;
            }
            
            if(music_send_cnt>=5)                   // 50ms*
            {
                music_send_cnt =0;

                ack_buf[0] = 0x01;                  // 首码
                ack_buf[1] = 0x06;                  // 功能码
                ack_buf[2] = 0x00;                  // ADD-H
                ack_buf[3] = 0x04;                  // ADD-L--04-播放文件夹 05-音量 0-255
                ack_buf[4] = 0x00;                  // ADD-H
                ack_buf[5] = PLC_TO_HUB_Mesg_Stru.set_music_id;     // ADD-L---对应播放文件
                u16_crc_temp =Bsp_ModbusRTU_CRC(ack_buf, 6);
                ack_buf[6] = GET_LOW_BYTE(u16_crc_temp);             // CRC-H
                ack_buf[7] = GET_HIGH_BYTE(u16_crc_temp);            // CRC-L
                Bsp_Usart_Usr_SendArray(UART4, ack_buf, 8);
            }
            break;
            
        case 1:// 零动485+音量控制
            if(++music_send_cnt>=5)                // 50ms*
            {
                music_send_cnt =0;

                ack_buf[0] = 0x01;                  // 首码
                ack_buf[1] = 0x10;                  // 功能码
                ack_buf[2] = 0x00;                  // ADD-H
                ack_buf[3] = 0x03;                  // ADD-L--

                ack_buf[4] = 0x00;                  // -H
                ack_buf[5] = 0x03;                  // -L--寄存器个数
                ack_buf[6] = 0x06;                  // 字节数

                ack_buf[7] = 0x00;                  // ADD-H--03
                ack_buf[8]= PLC_TO_HUB_Mesg_Stru.set_music_id;                  // ADD-L---对应播放文件

                ack_buf[9] = 0x00;                  // ADD-H---04
                ack_buf[10] = music_volume;   // 音量控制 0-100

                ack_buf[11] = 0x00;                 // ADD-H
                ack_buf[12] = 0x00;                 // ADD-L-循环模式 00-循环，01-单次

                u16_crc_temp =Bsp_ModbusRTU_CRC(ack_buf, 13);
                ack_buf[13] = GET_LOW_BYTE(u16_crc_temp);               // CRC-H
                ack_buf[14] = GET_HIGH_BYTE(u16_crc_temp);              // CRC-L
                Bsp_Usart_Usr_SendArray(UART4, ack_buf, 15);
                
            }
            break;
            
        default:
            music_send_cnt =0;
            return;
            
    }
    
}



