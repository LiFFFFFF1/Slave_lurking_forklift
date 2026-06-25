#include "bat_comm_custom_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "bat_comm_upper.h"


/***************************************************************************************
*函    数: void Bat_CommReadHxCustomData(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取队列缓存数据
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：自定义协议接收 
****************************************************************************************/
static void Bat_CommReadHxCustomData(USART_STRU *Serial,QUEUE *QUEUE_com)
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
                if(Serial->step==0)
                {
                    if(dat == 0x7F)                         // 起始码
                    {
                       Serial->step++;
                       Serial->recv_pbuffer[0] = dat;
                       Serial->count = 1;
                    }
                }
                else if(Serial->step== 1)
                {
                    if(dat == 0x10)                             // 功能码
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
                    if(dat == 0x02)                         // VER版本
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
                else if(Serial->step== 3)
                {
                    if(dat == 0x1B)                         // 接收数据字节-长度
                    {
                       Serial->step++;
                       Serial->recv_pbuffer[2] = dat;
                       Serial->count = 4;
                    }
                    else
                    {
                        Serial->step =0;
                    }
                }
                else
                {
                    Serial->recv_pbuffer[Serial->count++] = dat;
                    if(Serial->count>=27)
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
*函    数: void Bat_Comm_CustomSend_Ask(void)
*功    能: 发送进程
*参    数:      
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：自定义协议
*    
****************************************************************************************/
void Bat_Comm_HxCustomSend_Ask(void)
{

    u8 bat_ask_buf[8]= {0};
    
                                                    // 自定义协议
    bat_ask_buf[0] = 0x7F;                          // SOI
    bat_ask_buf[1] = Bat_Mesg_Stru.device_adr;      // addr
    bat_ask_buf[2] = 0x02;                          // VER
    bat_ask_buf[3] = 0x06;                          // LEN
    
    bat_ask_buf[4] = 0x11;                          // FUN
    bat_ask_buf[5] =bat_ask_buf[0]+bat_ask_buf[1]+bat_ask_buf[2]+
                    bat_ask_buf[3]+bat_ask_buf[4];
    bat_ask_buf[5] = (~bat_ask_buf[5])+1;           // CKS
    
    Bsp_Usart_Usr_SendArray(Bat_Mesg_Stru.commun_port, bat_ask_buf,6);
    
    LED2_GREEN_UART4_TX =! LED2_GREEN_UART4_TX;
    
    if(Bat_Mesg_Stru.com_timer < 1000)  Bat_Mesg_Stru.com_timer++;
    
}

/***************************************************************************************
*函    数: void Bat_Comm_HxCustom_Handler(void)
*功    能: 接收处理
*参    数:  Serial-对应串口发送结构体，QUEUE_com-对应接收串口结构体     
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
void Bat_Comm_HxCustom_Handler(void)
{
    USART_STRU *Serial;
    QUEUE *QUEUE_com;

    uint16_t current_power =0, total_power =0;
 
    if(Bat_Mesg_Stru.commun_port == USART1)
    {
        Serial    = &Usart1_Serial_Stru;
        QUEUE_com = &COM1_Serial;
    }
    else if(Bat_Mesg_Stru.commun_port == USART2)
    {
        Serial    = &Usart2_Serial_Stru;
        QUEUE_com = &COM2_Serial;
    }
    else if(Bat_Mesg_Stru.commun_port == USART3)
    {
        Serial    = &Usart3_Serial_Stru;
        QUEUE_com = &COM3_Serial;
    }
    else if(Bat_Mesg_Stru.commun_port == UART4)
    {
        Serial    = &Usart4_Serial_Stru;
        QUEUE_com = &COM4_Serial;
    }
    else if(Bat_Mesg_Stru.commun_port == UART5)
    {
        Serial    = &Usart5_Serial_Stru;
        QUEUE_com = &COM5_Serial;
    }
    else
    {
        return;
    }
    
    Bat_CommReadHxCustomData(Serial, QUEUE_com);               // 自定义解码
    
    if(Serial->recv_complete_bit & 0x8000)
    {
                                                            // 电池电压H8---整数值-----单位0.001V
       Bat_Mesg_Stru.voltage =(u16)((Serial->recv_pbuffer[16]<<8)+Serial->recv_pbuffer[15])*10;                             
        
                                                            // 电池电流H8---整数值-----单位单位0.1A
       Bat_Mesg_Stru.current =(s16)(Serial->recv_pbuffer[10]<<8)+Serial->recv_pbuffer[9];    

                                                            // 电池SOC百分比 0-100%
       current_power = (Serial->recv_pbuffer[22]<<8)|(Serial->recv_pbuffer[21]);
       total_power  = (Serial->recv_pbuffer[24]<<8)|(Serial->recv_pbuffer[23]);
        
       Bat_Mesg_Stru.soc_percent =(current_power*100)/total_power;

       Bat_Mesg_Stru.com_timer = 0;//清零
       
       LED2_RED_UART4_RX =! LED2_RED_UART4_RX;
       
       Serial->recv_complete_bit =0;
    }
    
}
