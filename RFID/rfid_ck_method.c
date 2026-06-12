#define RFID_CK_GLOBALS
#include "rfid_ck_method.h"

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
static void RFID_CK_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
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
                    if(dat == '$')                               
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[0] = dat;
                        Serial->count = 1;
                    }
                    
                }
                
                else if(Serial->step== 1)//
                {
                    if(dat == 'F')                          
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
                                
                else if(Serial->step== 2)
                {
                    Serial->recv_pbuffer[Serial->count++] = dat;
                    
                    if(Serial->count >= RFID_CK_REC_LEN)
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
void RFID_CK_232_RX_Complete_Handler(u8 port_com)
{
    u8 xor_val=0;
    u16 xor_ascll_H = 0, xor_ascll_L = 0;
    USART_STRU *Serial;
    QUEUE *QUEUE_com;
    
    switch(port_com)
    {
        default:
            return;
        case 1:
            Serial    = &Usart1_Serial_Stru;
            QUEUE_com = &COM1_Serial;
            break;
        case 2:
            Serial    = &Usart2_Serial_Stru;
            QUEUE_com = &COM2_Serial;
            break;
        case 3:
            Serial    = &Usart3_Serial_Stru;
            QUEUE_com = &COM3_Serial;
            break;
        case 4:
            Serial    = &Usart4_Serial_Stru;
            QUEUE_com = &COM4_Serial;
            break;
        case 5:
            Serial    = &Usart5_Serial_Stru;
            QUEUE_com = &COM5_Serial;
            break;
    }

    RFID_CK_ReadbufData(Serial, QUEUE_com);                   // 获取对应串口对接数据

    if(Serial->recv_complete_bit & 0x8000)
    { 
        /* XOR校验判断 */
        xor_val = Bsp_XorCheck(&Serial->recv_pbuffer[1], RFID_CK_REC_LEN-4);
        xor_ascll_H = Bsp_HexToASCII(xor_val>>4);
        xor_ascll_L = Bsp_HexToASCII(xor_val&0x0f);
        
        // 校验 通过
        if(Serial->recv_pbuffer[RFID_CK_REC_LEN-1] == '#'&&
            xor_ascll_L==Serial->recv_pbuffer[RFID_CK_REC_LEN-2] &&     
                xor_ascll_H==Serial->recv_pbuffer[RFID_CK_REC_LEN-3])
        {
            //RFID编号
            RFID_Upper_Mesg_Stru.read_number   = (Serial->recv_pbuffer[13]-0x30)*1000;   
            RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[14]-0x30)*100;   
            RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[15]-0x30)*10;   
            RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[16]-0x30)*1;  

            //上传
            if(RFID_Upper_Mesg_Stru.read_number)
            PLC_TO_HUB_Mesg_Stru.upload_rfid_site = RFID_Upper_Mesg_Stru.read_number;
            
            PLC_TO_HUB_Mesg_Stru.upload_run_mileage = 0;  //里程清零           
        }
        
        Serial->recv_complete_bit =0;
        
    }
    
}

void RFID_CK_232_Write_Send(void)//写卡时发送数据
{
    
    
    RFID_Upper_Mesg_Stru.commun_timer =0;
    RFID_Upper_Mesg_Stru.offline_state = 0;               // 在线

    //写卡
    if(RFID_Upper_Mesg_Stru.rfid_mode == RFID_WRITE_ONLY_MODE)
    {
        RFID_Upper_Mesg_Stru.write_number = 0;            // 清零
        RFID_Upper_Mesg_Stru.rfid_mode = RFID_READ_ONLY_MODE;

    }

}



