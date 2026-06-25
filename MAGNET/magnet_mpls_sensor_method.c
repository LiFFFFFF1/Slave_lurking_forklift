#include "magnet_mpls_sensor_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "magnet_sensor_upper.h"


// 私有宏定义
#define  MPLS_SENSOR_REC_LEN            11 


/***************************************************************************************
*函    数: void Magnet_Mpls_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
static void Magnet_Mpls_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
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
                    if(dat<=0x04)                                // 起始码
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
                    if(dat == 0x06)                             // 字节数
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
                    if(Serial->count>=MPLS_SENSOR_REC_LEN)
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
*函    数: void MAGNET_Mpls_COM_RX_Task(void)
*功    能: 接收处理
*参    数:  Serial-对应串口发送结构体，QUEUE_com-对应接收串口结构体     
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
void MAGNET_Mpls_COM_RX_Task(void)
{
    u16 crc_val=0;
    u8 adr_mpls=0;
    USART_STRU *Serial;
    QUEUE *QUEUE_com;
    
    if(Magnet_Upper_Stru.can_com_type == EM_SET_PORT_CAN )  // CAN-模式下
    {
        return;
    }
    if(Magnet_Upper_Stru.commun_port == USART1)
    {
        Serial    = &Usart1_Serial_Stru;
        QUEUE_com = &COM1_Serial;
    }
    else if(Magnet_Upper_Stru.commun_port == USART2)
    {
        Serial    = &Usart2_Serial_Stru;
        QUEUE_com = &COM2_Serial;
    }
    else if(Magnet_Upper_Stru.commun_port == USART3)
    {
        Serial    = &Usart3_Serial_Stru;
        QUEUE_com = &COM3_Serial;
    }
    else if(Magnet_Upper_Stru.commun_port == UART4)
    {
        Serial    = &Usart4_Serial_Stru;
        QUEUE_com = &COM4_Serial;
    }
    else if(Magnet_Upper_Stru.commun_port == UART5)
    {
        Serial    = &Usart5_Serial_Stru;
        QUEUE_com = &COM5_Serial;
    }
    else
    {
        return;
    }

    Magnet_Mpls_ReadbufData(Serial,QUEUE_com);                   // 获取对应串口对接数据

    if(Serial->recv_complete_bit & 0x8000)
    { 
        /* CRC校验判断 */
        crc_val = Bsp_ModbusRTU_CRC(Serial->recv_pbuffer,MPLS_SENSOR_REC_LEN-2);
                                                            //  先判断ID，在判断校验 
        if(Serial->recv_pbuffer[0]>=1 && Serial->recv_pbuffer[0]<=4 &&          
           (GET_LOW_BYTE(crc_val)==Serial->recv_pbuffer[MPLS_SENSOR_REC_LEN-2]) &&     
           (GET_HIGH_BYTE(crc_val)==Serial->recv_pbuffer[MPLS_SENSOR_REC_LEN-1]))
        {
            adr_mpls =Serial->recv_pbuffer[0]-1;
            if(Serial->recv_pbuffer[1] == 0x03 && Serial->recv_pbuffer[2] == 06)
            {
                                                                    // 磁段
                Magnet_Upper_Stru.segment[adr_mpls]          = Serial->recv_pbuffer[3];   
                                                                    // 左偏移值
                Magnet_Upper_Stru.offset_left[adr_mpls]      = Serial->recv_pbuffer[4];
                                                                    // 中偏移值
                Magnet_Upper_Stru.offset_middle[adr_mpls]    = Serial->recv_pbuffer[5];
                                                                    // 右偏移值
                Magnet_Upper_Stru.offset_right[adr_mpls]     = Serial->recv_pbuffer[6];
                                                                    // 点位值
                Magnet_Upper_Stru.io_low[adr_mpls]           = Serial->recv_pbuffer[7];
                Magnet_Upper_Stru.io_high[adr_mpls]          = Serial->recv_pbuffer[8];

                Magnet_Upper_Stru.magnet_comm_time[adr_mpls] = 0;
              
            }
        }
        
        Serial->recv_complete_bit =0;
    }
}





/***************************************************************************************
*函    数: void MAGNET_Send_CMD_Ask_Magnet(void)
*功    能: 查询模式
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：此函数根据每调用一次发送一次查询通信指令(查询模式有效)
****************************************************************************************/
void MAGNET_Mpls_Send_Mesg_Task(void)
{
    static u8 std_id=4,std_rs485_id=0;
    u8 magnet_can_ask_buf[2]={0x4D,0x04};
    u8 magnet_send_ask_buf[4][8]= {
        {0x01,0x03,0x00,0x04,0x00,0x03,0x44,0x0A},
        {0x02,0x03,0x00,0x04,0x00,0x03,0x44,0x39},
        {0x03,0x03,0x00,0x04,0x00,0x03,0x45,0xE8},
        {0x04,0x03,0x00,0x04,0x00,0x03,0x44,0x5F},
    };
    
    if(Magnet_Upper_Stru.magnet_comm_time[0]<1000)  Magnet_Upper_Stru.magnet_comm_time[0]++;
    if(Magnet_Upper_Stru.magnet_comm_time[1]<1000)  Magnet_Upper_Stru.magnet_comm_time[1]++;
    if(Magnet_Upper_Stru.magnet_comm_time[2]<1000)  Magnet_Upper_Stru.magnet_comm_time[2]++;
    if(Magnet_Upper_Stru.magnet_comm_time[3]<1000)  Magnet_Upper_Stru.magnet_comm_time[3]++;
    
    
    // 主动上传-退出
    if(Magnet_Upper_Stru.commun_ask_mode !=EM_DEV_ASK_HAND)                
    {
        return;
    }
    
    if(Magnet_Upper_Stru.can_com_type == EM_SET_PORT_CAN )  // CAN模式下
    {
        CAN1_Send_One_Frame_Data((0x600+std_id),magnet_can_ask_buf,2);
       
        
        std_id++;
        if(std_id>=(4+Magnet_Upper_Stru.id_number))         // 根据设置地址设备个数，自动匹配
        {
            std_id =4;
        }
    }                                                      
    
    else if(Magnet_Upper_Stru.can_com_type == EM_SET_PORT_COM )      // RS485模式下     
    {
        
        if(Magnet_Upper_Stru.commun_port == NULL)           // 未配置端口
        {
            return;
        }
        
        Bsp_Usart_Usr_SendArray(Magnet_Upper_Stru.commun_port, &magnet_send_ask_buf[std_rs485_id][0],8);
       
        
        std_rs485_id++;
        if(std_rs485_id>=Magnet_Upper_Stru.id_number)
        {
            std_rs485_id =0;
        }
        
    }
}

/***************************************************************************************
*函    数: void Magnet_MPLS_CAN_RX_Handler(USART_TypeDef* USARTx,u8 id_code)
*功    能: 曼普拉斯-CAN接收处理
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
void Magnet_MPLS_CAN_RX_Handler(CanRxMsg* RxMessage)
{
    u8 can_mpls_adr=0;
    
    switch(RxMessage->StdId)
    {
        case 0x584:
        case 0x585:
        case 0x586:
        case 0x587:
            can_mpls_adr = RxMessage->StdId-0x584;
            /* MPLS_CAN 问询模式*/
            if(Magnet_Upper_Stru.commun_ask_mode == EM_DEV_ASK_HAND && 
               Magnet_Upper_Stru.can_com_type == EM_SET_PORT_CAN &&
               Magnet_Upper_Stru.device_type  == EM_DEVICE_DEV_MAGNET_MPLS &&
               (RxMessage->DLC == 0X08) && (RxMessage->Data[0] == 0x4D) && (RxMessage->Data[1] == 0x04))
            {
                Magnet_Upper_Stru.segment[can_mpls_adr]          = RxMessage->Data[2];   
                                                                    // 左偏移值
                Magnet_Upper_Stru.offset_left[can_mpls_adr]      = RxMessage->Data[3];
                                                                    // 中偏移值
                Magnet_Upper_Stru.offset_middle[can_mpls_adr]    = RxMessage->Data[4];
                                                                    // 右偏移值
                Magnet_Upper_Stru.offset_right[can_mpls_adr]     = RxMessage->Data[5];
                                                                    // 点位值
                Magnet_Upper_Stru.io_low[can_mpls_adr]           = RxMessage->Data[6];
                Magnet_Upper_Stru.io_high[can_mpls_adr]          = RxMessage->Data[7];

                Magnet_Upper_Stru.magnet_comm_time[can_mpls_adr] = 0;
               
            }
            
            /* MPLS_CAN 主动上传模式*/
            else if(Magnet_Upper_Stru.commun_ask_mode == EM_DEV_ASK_AUTO && 
                    Magnet_Upper_Stru.can_com_type == EM_SET_PORT_CAN &&
                    Magnet_Upper_Stru.device_type  == EM_DEVICE_DEV_MAGNET_MPLS && 
                    RxMessage->DLC == 0X06)
            {
                Magnet_Upper_Stru.segment[can_mpls_adr]          = RxMessage->Data[0];   
                                                                    // 左偏移值
                Magnet_Upper_Stru.offset_left[can_mpls_adr]      = RxMessage->Data[1];
                                                                    // 中偏移值
                Magnet_Upper_Stru.offset_middle[can_mpls_adr]    = RxMessage->Data[2];
                                                                    // 右偏移值
                Magnet_Upper_Stru.offset_right[can_mpls_adr]     = RxMessage->Data[3];
                                                                    // 点位值
                Magnet_Upper_Stru.io_low[can_mpls_adr]           = RxMessage->Data[4];
                Magnet_Upper_Stru.io_high[can_mpls_adr]          = RxMessage->Data[5];

                Magnet_Upper_Stru.magnet_comm_time[can_mpls_adr] = 0;
               
            }
            break;
            
        default:
            break;
            
    }
    
}

