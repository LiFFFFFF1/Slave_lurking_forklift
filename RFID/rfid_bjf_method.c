#define RFID_BJF_GLOBALS
#include "rfid_bjf_method.h"
#include "delay.h"

// 包含使用模块 xxx_xxx_upper.h
#include "rfid_upper.h"

// 私有定义
//static u8 ChangeMode_Bit;//读写卡模式转换标志位

/***************************************************************************************
*函    数: void RFID_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存数据一帧
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
static void RFID_BJF_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
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
                    if(dat == 0x30)//读卡返回数据
                    {
                        Serial->step = 1;
                        Serial->recv_pbuffer[0] = dat;
                        Serial->count = 1;
                    }

                    else if(dat == 0x35)//另外一帧返回数据
                    {
                        Serial->step = 11;
                        Serial->recv_pbuffer[0] = dat;
                        Serial->count = 1;
                    }
                }

                else if(Serial->step== 1)//
                {
                    if(dat >= 0x30 && dat <= 0x39)
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
                    if(dat >= 0x30 && dat <= 0x39)
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
                    if(dat >= 0x30 && dat <= 0x39)
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

                else if(Serial->step == 4)//
                {
                    if(dat >= 0x30 && dat <= 0x39)
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

                else if(Serial->step == 5) //
                {
                    if(dat == 0x23)
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[5] = dat;
                        Serial->count = 6;
                    }
                    else
                    {
                        Serial->step = 0;
                    }
                }
                
                else if(Serial->step == 6) //
                {
                    if(dat == 0x0D)
                    {
                        Serial->recv_pbuffer[6] = dat;
                        
                        Serial->recv_complete_bit = Serial->count;
                        Serial->recv_complete_bit |= 0x8000;

                        Serial->step = 0;
                        Serial->count = 0;
                        return;
                    }
                    else
                    {
                        Serial->step = 0;
                    }
                }


                ////////////////////////////////////

                else if(Serial->step== 11)//另外一帧返回数据
                {
                    if(dat == 0x23)
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

                else if(Serial->step== 12)//
                {
                    if(dat == 0x0d)
                    {
                        Serial->recv_pbuffer[2] = dat;

                        Serial->recv_complete_bit=Serial->count;
                        Serial->recv_complete_bit |= 0x8000;

                        Serial->step = 0;
                        Serial->count = 0;
                        return;

                    }
                    else
                    {
                        Serial->step =0;
                    }

                }

            }

            else if(readstatus == FIFO_ERROR_EMPTY)
            {
                Serial->the_over_time++;
                if(Serial->the_over_time >= 100)
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
void RFID_BJF_485_RX_Complete_Handler(void)
{
    //u16 crc_val=0;
    USART_STRU *Serial;
    QUEUE *QUEUE_com;

    /* 非COM模式下 */
    if(RFID_Upper_Mesg_Stru.can_com_type != EM_SET_PORT_COM)
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

    RFID_BJF_ReadbufData(Serial,QUEUE_com);                   // 获取对应串口对接数据

    if(Serial->recv_complete_bit & 0x8000)
    {
        if(RFID_Upper_Mesg_Stru.init_state == 0x0a)//初始化完成后
        {
            if(RFID_Upper_Mesg_Stru.rfid_mode == RFID_READ_ONLY_MODE)//读卡时返回数据
            {
                if(Serial->recv_pbuffer[0] == 0x30&&Serial->recv_pbuffer[5] == 0x23&&Serial->recv_pbuffer[6] == 0x0d)
                {
                    //RFID编号
                    RFID_Upper_Mesg_Stru.read_number   = (Serial->recv_pbuffer[1]-0x30)*1000;
                    RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[2]-0x30)*100;
                    RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[3]-0x30)*10;
                    RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[4]-0x30)*1;
                   
                }
            }
        }

        //初始化成功收到数据
        if(RFID_Upper_Mesg_Stru.init_state == 2 || RFID_Upper_Mesg_Stru.init_state == 4)
        {
            //初始化时无卡，返回0x35 0x23 0x0D
            if(Serial->recv_pbuffer[0] == 0x35 && Serial->recv_pbuffer[1] == 0x23 && Serial->recv_pbuffer[2] == 0x0D)
            {
                RFID_Upper_Mesg_Stru.init_state = 0x0a; //初始化完成标志
            }
            
            //初始化时有卡，返回实际卡号
            else if(Serial->recv_pbuffer[0] == 0x30 && Serial->recv_pbuffer[5] == 0x23 && Serial->recv_pbuffer[6] == 0x0d)
            {
                RFID_Upper_Mesg_Stru.init_state = 0x0a; //初始化完成标志
                //RFID编号
                RFID_Upper_Mesg_Stru.read_number   = (Serial->recv_pbuffer[1] - 0x30) * 1000;
                RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[2] - 0x30) * 100;
                RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[3] - 0x30) * 10;
                RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[4] - 0x30) * 1;
            }
        }

        Serial->recv_complete_bit = 0;

    }

}

/***************************************************************************************
*函    数: void RFID_BJF_485_Send(void)
*功    能: 初始化及写卡时发送数据
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void RFID_BJF_485_Write_Send(void)
{
    static u8 timer = 0;
    static u8 BJF_SaveCMD_SendBuf[5] = {0x43, 0x53, 0x31, 0x23, 0x0d};
    static u8 BJF_Readmode_SendBuf[10] = {0x45, 0x52, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x23, 0x0d};
    static u8 BJF_Write_SendBuf[14] = {0};

    if(RFID_Upper_Mesg_Stru.rfid_mode == RFID_WRITE_ONLY_MODE)
    {
        if(RFID_Upper_Mesg_Stru.write_number > 0 && RFID_Upper_Mesg_Stru.write_number < 9999)//写入地标值
        {
            BJF_Write_SendBuf[0] = 0x45;
            BJF_Write_SendBuf[1] = 0x57;
            BJF_Write_SendBuf[2] = 0x30;
            BJF_Write_SendBuf[3] = 0x30;
            BJF_Write_SendBuf[4] = 0x30;
            BJF_Write_SendBuf[5] = 0x30;
            BJF_Write_SendBuf[6] = 0x30;
            BJF_Write_SendBuf[7] = 0x31;
            BJF_Write_SendBuf[8] = 0x30+(RFID_Upper_Mesg_Stru.write_number / 1000);//编号:千位
            BJF_Write_SendBuf[9] = 0x30+((RFID_Upper_Mesg_Stru.write_number % 1000) / 100);//编号:百位
            BJF_Write_SendBuf[10] =0x30+((RFID_Upper_Mesg_Stru.write_number % 1000) % 100 / 10);//编号:十位
            BJF_Write_SendBuf[11] =0x30+((RFID_Upper_Mesg_Stru.write_number % 1000) % 100 % 10);//编号:个位
            BJF_Write_SendBuf[12] = 0x23;
            BJF_Write_SendBuf[13] = 0x0d;

            Bsp_Usart_Usr_SendArray(RFID_Upper_Mesg_Stru.commun_port, &BJF_Write_SendBuf[0], 14);//发送数据
            

            RFID_Upper_Mesg_Stru.rfid_mode = RFID_READ_ONLY_MODE;

            RFID_Upper_Mesg_Stru.write_number = 0;//清零
            RFID_Upper_Mesg_Stru.init_state = 0;//重新初始化

        }

    }

    //初始化时或写卡后转化成读卡模式
    if(RFID_Upper_Mesg_Stru.init_state == 0)
    {
        if(++timer > 10)//*50ms
        {
            timer = 0;
            Bsp_Usart_Usr_SendArray(RFID_Upper_Mesg_Stru.commun_port, &BJF_SaveCMD_SendBuf[0], 5);//发送数据
           
            RFID_Upper_Mesg_Stru.init_state = 1;
        }
    }
    else if(RFID_Upper_Mesg_Stru.init_state == 1)
    {
        if(++timer > 10)
        {
            timer = 0;
            Bsp_Usart_Usr_SendArray(RFID_Upper_Mesg_Stru.commun_port, &BJF_Readmode_SendBuf[0], 10);//发送数据
            
            RFID_Upper_Mesg_Stru.init_state = 2;
        }
    }
    else if(RFID_Upper_Mesg_Stru.init_state == 2)
    {
        if(++timer > 10)
        {
            timer = 0;
            Bsp_Usart_Usr_SendArray(RFID_Upper_Mesg_Stru.commun_port, &BJF_SaveCMD_SendBuf[0], 5);//发送数据
           
            RFID_Upper_Mesg_Stru.init_state = 3;
        }
    }
    else if(RFID_Upper_Mesg_Stru.init_state == 3)
    {
        if(++timer > 10)
        {
            timer = 0;
            Bsp_Usart_Usr_SendArray(RFID_Upper_Mesg_Stru.commun_port, &BJF_Readmode_SendBuf[0], 10);//发送数据
         
            RFID_Upper_Mesg_Stru.init_state = 4;
        }
    }
    else if(RFID_Upper_Mesg_Stru.init_state == 4)//超时
    {
        if(++timer > 50)//*50ms
        {
            timer = 0;
            RFID_Upper_Mesg_Stru.init_state = 0xff;//超时标志
        }
    }


}






















