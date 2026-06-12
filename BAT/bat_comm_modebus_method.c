#include "bat_comm_modebus_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "bat_comm_upper.h"



/***************************************************************************************
*函    数: void Bat_CommReadModbusData(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
static void Bat_CommReadModbusData(USART_STRU *Serial,QUEUE *QUEUE_com)
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
                    if(dat == 0x01)                         // 起始码
                    {
                       Serial->step++;
                       Serial->recv_pbuffer[0] = dat;
                       Serial->count = 1;
                    }
                }
                else if(Serial->step== 1)
                {
                    if(dat == 0x03)                             // 功能码
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
                    if(dat == 14)                             // 接收数据字节-长度
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
                else
                {
                    Serial->recv_pbuffer[Serial->count++] = dat;
                    if(Serial->count>=19)
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
*函    数: void Bat_Comm_Modbus_Send_Ask(void)
*功    能: 发送进程
*参    数:  Bat_Mesg_Stru.commun_port需要指定电池串口端口       
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：电池支持两种：
*   第一种 标准 MODEBUS_RTU
****************************************************************************************/
void Bat_Comm_Modbus_Send_Ask(void)
{
    u16 bat_crc_val=0;
    u8 bat_ask_buf[8]= {0};
    
    
    bat_ask_buf[0] = Bat_Mesg_Stru.device_adr;      // addr
    bat_ask_buf[1] = 0x03;
    bat_ask_buf[2] = 0;                             // add_H
    bat_ask_buf[3] = 0;                             // add_L
    
    bat_ask_buf[4] = 0x00;                          // LEN_H
    bat_ask_buf[5] = 0x07;                          // LEN_L
    
    bat_crc_val = Bsp_ModbusRTU_CRC(bat_ask_buf,6); // CRC
    bat_ask_buf[6] =GET_LOW_BYTE(bat_crc_val);
    bat_ask_buf[7] =GET_HIGH_BYTE(bat_crc_val);

    Bsp_Usart_Usr_SendArray(Bat_Mesg_Stru.commun_port, bat_ask_buf,8);
    LED2_GREEN_UART4_TX =! LED2_GREEN_UART4_TX;
    
    if(Bat_Mesg_Stru.com_timer < 1000)  Bat_Mesg_Stru.com_timer++;
    
}

/***************************************************************************************
*函    数: void Bat_Comm_Modebus_Handler(void)
*功    能: 接收处理
*参    数:  Serial-对应串口发送结构体，QUEUE_com-对应接收串口结构体     
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
void Bat_Comm_Modebus_Handler(void)
{
    USART_STRU *Serial;
    QUEUE *QUEUE_com;
    u16 tem16_val=0;

 
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
    
    Bat_CommReadModbusData(Serial,QUEUE_com);               // 获取对应串口对接数据
    
    if(Serial->recv_complete_bit & 0x8000)
    { 
        /* CRC校验判断 */
        tem16_val = Bsp_ModbusRTU_CRC(Serial->recv_pbuffer,17);     
                                                            //  先判断ID，在判断校验 
        if(Serial->recv_pbuffer[0]>=1 && Serial->recv_pbuffer[0]<=4 &&          
           (GET_LOW_BYTE(tem16_val)==Serial->recv_pbuffer[17]) &&    
           (GET_HIGH_BYTE(tem16_val)==Serial->recv_pbuffer[18]))    
        {
                                                            // 电池电压H8---整数值-----单位0.01V
           Bat_Mesg_Stru.voltage =(u16)(Serial->recv_pbuffer[3]<<8)+Serial->recv_pbuffer[4];                             
            
                                                            // 电池电流H8---整数值-----单位单位0.1A
           Bat_Mesg_Stru.current =(s16)(Serial->recv_pbuffer[5]<<8)+Serial->recv_pbuffer[6];    

                                                            // 电池SOC百分比 0-100%
           Bat_Mesg_Stru.soc_percent =Serial->recv_pbuffer[16];   

           LED2_RED_UART4_RX =! LED2_RED_UART4_RX;
            
           Bat_Mesg_Stru.com_timer = 0;//清零

        }
        
        Serial->recv_complete_bit =0;
     }
    
}



