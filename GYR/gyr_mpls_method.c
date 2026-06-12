#include "gyr_mpls_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "gyr_comm_upper.h"

/***************************************************************************************
*函    数: void GYR_Read_MPI240Data(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：自定义协议接收
****************************************************************************************/
static void GYR_Read_MPI240Data(USART_STRU *Serial,QUEUE *QUEUE_com)
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
                    if(dat == GYR_Mesg_Stru.device_adr)     // 起始码
                    {
                       Serial->step++;
                       Serial->recv_pbuffer[0] = dat;
                       Serial->count = 1;
                    }
                }
                else if(Serial->step== 1)
                {
                    if(dat == 0x03 || dat == 0x10)          // 功能码
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
                else
                {
                    Serial->recv_pbuffer[Serial->count++] = dat;
                    if(Serial->count>=(Serial->recv_pbuffer[2]+5))
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
*函    数: void GYR_Mpls_Send_Task(void)
*功    能: modbus问询模式指令
*参    数:      
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：
*    
****************************************************************************************/
void GYR_Mpls_Send_Task(void)
{
    u16 gyr_crc_val=0;
    u8 gyr_ask_buf[8]= {0};
    
    if(GYR_Mesg_Stru.commun_ask_mode != GYR_ASK_HAND)
    {
        return;
    }
    
    if(GYR_Mesg_Stru.can_com_type == GYR_COM_MODE)
    {                                                       // adr
        gyr_ask_buf[0] = GYR_Mesg_Stru.device_adr;                        
        gyr_ask_buf[1] = 0x03;                              // cmd
        gyr_ask_buf[2] = 0x00;                              // star_H8
        gyr_ask_buf[3] = 0x04;                              // star_L8
        
        gyr_ask_buf[4] = 0x00;                              // LEN-H8
        gyr_ask_buf[5] = 0X04;                              // LEN-L8
        gyr_crc_val =Bsp_ModbusRTU_CRC(gyr_ask_buf,6);
        gyr_ask_buf[6] =GET_LOW_BYTE(gyr_crc_val);
        gyr_ask_buf[7] =GET_HIGH_BYTE(gyr_crc_val);
        
        Bsp_Usart_Usr_SendArray(GYR_Mesg_Stru.commun_port, gyr_ask_buf,8);
        LED1_GREEN_UART5_TX =! LED1_GREEN_UART5_TX;

        if(GYR_Mesg_Stru.com_timer < 1000)  GYR_Mesg_Stru.com_timer++;
    }
    
    else if(GYR_Mesg_Stru.can_com_type == GYR_CAN_MODE)
    {                                                  
        gyr_ask_buf[0] = 0x4D;                        
        gyr_ask_buf[1] = 0x04;                         
        CAN1_Send_One_Frame_Data((0x600+GYR_Mesg_Stru.device_adr),gyr_ask_buf,2);
        LED4_GREEN_CAN_TX =! LED4_GREEN_CAN_TX;
        
        if(GYR_Mesg_Stru.com_timer < 1000)  GYR_Mesg_Stru.com_timer++;
    }
    
    
}


/***************************************************************************************
*函    数: void GYR_Mpls_ZeroSet_Task(void)
*功    能: modbus-角度清零
*参    数:      
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：
*    
****************************************************************************************/
void GYR_Mpls_ZeroSet_Task(void)
{
    u16 gyr_crc_val=0;
    u8 gyr_zero_buf[13]= {0};
    
    if(GYR_Mesg_Stru.can_com_type == GYR_COM_MODE)          // COM通信-清0指令
    {
                                                            // adr
        gyr_zero_buf[0] = GYR_Mesg_Stru.device_adr;                        
        gyr_zero_buf[1] = 0x10;                             // cmd
        gyr_zero_buf[2] = 0x00;                             // star_H8
        gyr_zero_buf[3] = 0x04;                             // star_L8
        
        gyr_zero_buf[4] = 0x00;                             // LEN-H8
        gyr_zero_buf[5] = 0X02;                             // LEN-L8
        
        gyr_zero_buf[6] = 0X04; 
        gyr_zero_buf[7] = 0X43; 
        gyr_zero_buf[8] = 0X4C; 
        gyr_zero_buf[9] = 0X52; 
        gyr_zero_buf[10]= 0X00; 
        gyr_crc_val =Bsp_ModbusRTU_CRC(gyr_zero_buf,11);
        gyr_zero_buf[11] =GET_LOW_BYTE(gyr_crc_val);
        gyr_zero_buf[12] =GET_HIGH_BYTE(gyr_crc_val);

        Bsp_Usart_Usr_SendArray(GYR_Mesg_Stru.commun_port, gyr_zero_buf,13);
        LED1_GREEN_UART5_TX =! LED1_GREEN_UART5_TX;
        
    }
    
    else if(GYR_Mesg_Stru.can_com_type == GYR_CAN_MODE)     // CAN通信-清0指令
    {                                                  
        gyr_zero_buf[0] = 0xCE;                        
        gyr_zero_buf[1] = 0x04;     
        gyr_zero_buf[2] = 0x43;                        
        gyr_zero_buf[3] = 0x4C;  
        gyr_zero_buf[4] = 0x52;
        CAN1_Send_One_Frame_Data((0x600+GYR_Mesg_Stru.device_adr),gyr_zero_buf,5);
        LED4_GREEN_CAN_TX =! LED4_GREEN_CAN_TX;
    }
    
}


/***************************************************************************************
*函    数: void GYR_Mpls_RX_Task(void)
*功    能:  GYR-MPI240处理解析队列数据
*参    数:  Serial-对应串口发送结构体，QUEUE_com-对应接收串口结构体     
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
void GYR_Mpls_RX_Task(void)
{
    USART_STRU *Serial;
    QUEUE *QUEUE_com;
    u16 tem16_val=0;
    u16 get_len=0;
    
    if(GYR_Mesg_Stru.commun_port == USART1)
    {
        Serial    = &Usart1_Serial_Stru;
        QUEUE_com = &COM1_Serial;
    }
    else if(GYR_Mesg_Stru.commun_port == USART2)
    {
        Serial    = &Usart2_Serial_Stru;
        QUEUE_com = &COM2_Serial;
    }
    else if(GYR_Mesg_Stru.commun_port == USART3)
    {
        Serial    = &Usart3_Serial_Stru;
        QUEUE_com = &COM3_Serial;
    }
    else if(GYR_Mesg_Stru.commun_port == UART4)
    {
        Serial    = &Usart4_Serial_Stru;
        QUEUE_com = &COM4_Serial;
    }
    else if(GYR_Mesg_Stru.commun_port == UART5)
    {
        Serial    = &Usart5_Serial_Stru;
        QUEUE_com = &COM5_Serial;
    }
    else
    {
        return;
    }
    
    GYR_Read_MPI240Data(Serial,QUEUE_com);                  // 自定义解码
    
    if(Serial->recv_complete_bit & 0x8000)
    {
        get_len =Serial->recv_complete_bit&0x7FFF;
        /* CRC校验判断 */
        tem16_val = Bsp_ModbusRTU_CRC(Serial->recv_pbuffer,(get_len-2));                                           
        
        if((GET_LOW_BYTE(tem16_val)==Serial->recv_pbuffer[get_len-2]) &&    
           (GET_HIGH_BYTE(tem16_val)==Serial->recv_pbuffer[get_len-1]))
        {
            switch(Serial->recv_pbuffer[1])
            {
                case 0x03:                                  
                                                            // 读角度
                    GYR_Mesg_Stru.angle =(float)((s16)(Serial->recv_pbuffer[3]<<8)+Serial->recv_pbuffer[4])/10.0F;
                    GYR_Mesg_Stru.angle_rawdata = (Serial->recv_pbuffer[3]<<8)+Serial->recv_pbuffer[4];
                    
                                                            // 读角速度
                    GYR_Mesg_Stru.angle_speed =(float)((s16)(Serial->recv_pbuffer[5]<<8)+Serial->recv_pbuffer[6])/10.0F;
                    GYR_Mesg_Stru.angle_speed_rawdata = (Serial->recv_pbuffer[5]<<8)+Serial->recv_pbuffer[6];

                    GYR_Mesg_Stru.com_timer = 0;
                    
                    LED1_RED_UART5_RX =! LED1_RED_UART5_RX;
                    
                    break;
                    
                case 0x10:                                  // 清角度-返回值
                    if(GYR_Mesg_Stru.angle_clear_bit)
                    {                                       // 清零成功
                        if(Serial->recv_pbuffer[3] == 0x04)
                        {
                            GYR_Mesg_Stru.angle_clear_bit =0;   
                            LED1_RED_UART5_RX =! LED1_RED_UART5_RX;
                        }
                    }
                    break;
                    
                default:
                    break;
                    
            }
        }
        
        Serial->recv_complete_bit =0;
    }    
}

/***************************************************************************************
*函    数: void GYR_Mpls_CAN_RX_Handler(USART_TypeDef* USARTx,u8 id_code)
*功    能: GYR-CAN接收处理
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
void GYR_Mpls_CAN_RX_Handler(CanRxMsg* RxMessage)
{

    if(RxMessage->StdId == (0x580+GYR_Mesg_Stru.device_adr))
    {
        /* MPLS_CAN 问询模式*/
        if(GYR_Mesg_Stru.commun_ask_mode == GYR_ASK_HAND && 
           GYR_Mesg_Stru.can_com_type == GYR_CAN_MODE &&
           GYR_Mesg_Stru.device_type  == GYR_TYPE_MPI240 &&
           (RxMessage->DLC == 0X06) && (RxMessage->Data[0] == 0x4D) && (RxMessage->Data[1] == 0x04))
        {
            GYR_Mesg_Stru.angle = (float)((s16)(RxMessage->Data[2]<<8)+RxMessage->Data[3])/10.0F;
            GYR_Mesg_Stru.angle_rawdata = (RxMessage->Data[2]<<8)+RxMessage->Data[3];
            
            GYR_Mesg_Stru.angle_speed = (float)((s16)(RxMessage->Data[4]<<8)+RxMessage->Data[5])/10.0F;
            GYR_Mesg_Stru.angle_speed_rawdata = (RxMessage->Data[4]<<8)+RxMessage->Data[5];
                
            GYR_Mesg_Stru.com_timer = 0;
            LED4_RED_CAN_RX =! LED4_RED_CAN_RX;

        }
        
        /* MPLS_CAN 问询模式--清0 回复 */
        else if(GYR_Mesg_Stru.commun_ask_mode == GYR_ASK_HAND && 
           GYR_Mesg_Stru.can_com_type == GYR_CAN_MODE &&
           GYR_Mesg_Stru.device_type  == GYR_TYPE_MPI240 &&
           (RxMessage->DLC == 0X05))
        {
            if((RxMessage->Data[0] == 0xCE) && (RxMessage->Data[1] == 0x04) &&
               (RxMessage->Data[2] == 0x43) && (RxMessage->Data[3] == 0x4C) && (RxMessage->Data[4] == 0x52))
            {
                GYR_Mesg_Stru.angle_clear_bit =0;
                LED4_RED_CAN_RX =! LED4_RED_CAN_RX;
            }
        }
        
        /* MPLS_CAN 主动上传模式*/
        else if(GYR_Mesg_Stru.commun_ask_mode == GYR_ASK_AUTO && 
                GYR_Mesg_Stru.can_com_type == GYR_CAN_MODE &&
                GYR_Mesg_Stru.device_type  == GYR_TYPE_MPI240 && 
                RxMessage->DLC == 0X04)
        {
            GYR_Mesg_Stru.angle = (float)((s16)(RxMessage->Data[0]<<8)+RxMessage->Data[1])/10.0F;
            GYR_Mesg_Stru.angle_rawdata = (RxMessage->Data[0]<<8)+RxMessage->Data[1];
            
            GYR_Mesg_Stru.angle_speed = (float)((s16)(RxMessage->Data[2]<<8)+RxMessage->Data[3])/10.0F;
            GYR_Mesg_Stru.angle_speed_rawdata = (RxMessage->Data[2]<<8)+RxMessage->Data[3];

            LED4_RED_CAN_RX =! LED4_RED_CAN_RX;
        }
        
    }
}



