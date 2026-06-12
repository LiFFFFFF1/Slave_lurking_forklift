#include "magnet_xs_sensor_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "magnet_sensor_upper.h"

  
// 私有宏定义
#define  XS_SENSOR_REC_LEN            21 

static u16 xs_canopen_send_number=0;

/***************************************************************************************
*函    数: void Magnet_Xs_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
static void Magnet_Xs_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
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
                    if(dat == 0x04)                             // 功能码
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
                    if(dat == 0x10)                             // 字节数
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
                    if(Serial->count>=XS_SENSOR_REC_LEN)      // 字节数+5
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
*函    数: void MAGNET_Xs_COM_RX_Task(void)
*功    能: 接收处理
*参    数:  Serial-对应串口发送结构体，QUEUE_com-对应接收串口结构体     
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
void MAGNET_Xs_COM_RX_Task(void)
{
    u16 crc_val=0;
    u16 xs_segment = 0;
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

    Magnet_Xs_ReadbufData(Serial,QUEUE_com);                   // 获取对应串口对接数据

    if(Serial->recv_complete_bit & 0x8000)
    { 
        /* CRC校验判断 */
        crc_val = Bsp_ModbusRTU_CRC(Serial->recv_pbuffer,XS_SENSOR_REC_LEN-2);
                                                            //  先判断ID，在判断校验 
        if(Serial->recv_pbuffer[0]>=1 && Serial->recv_pbuffer[0]<=4 &&          
           (GET_LOW_BYTE(crc_val)==Serial->recv_pbuffer[XS_SENSOR_REC_LEN-2]) &&     
           (GET_HIGH_BYTE(crc_val)==Serial->recv_pbuffer[XS_SENSOR_REC_LEN-1]))
        {
            adr_mpls =Serial->recv_pbuffer[0]-1;
            if(Serial->recv_pbuffer[1] == 0x04 && Serial->recv_pbuffer[2] == 0x10)
            {
                // 兴颂上传的原始磁段
                xs_segment = (u16)(Serial->recv_pbuffer[5]<<8)+Serial->recv_pbuffer[6]; 

                if(xs_segment == 0)//无
                {
                    Magnet_Upper_Stru.segment[adr_mpls] = 0;
                    Magnet_Upper_Stru.offset_left[adr_mpls] = 0;
                    Magnet_Upper_Stru.offset_middle[adr_mpls] = 0;
                    Magnet_Upper_Stru.offset_right[adr_mpls] = 0;
                }
                else if(xs_segment == 1 || xs_segment == 2 || xs_segment == 4)////1根磁条
                {
                    Magnet_Upper_Stru.segment[adr_mpls] = 1;
                    Magnet_Upper_Stru.offset_left[adr_mpls] = (u16)(Serial->recv_pbuffer[15]<<8)+Serial->recv_pbuffer[16];
                    Magnet_Upper_Stru.offset_middle[adr_mpls] = 0;
                    Magnet_Upper_Stru.offset_right[adr_mpls] = 0;
                }
                else if(xs_segment == 3)////2根磁条-左中
                {
                    Magnet_Upper_Stru.segment[adr_mpls] = 2;
                    Magnet_Upper_Stru.offset_left[adr_mpls] = (u16)(Serial->recv_pbuffer[15]<<8)+Serial->recv_pbuffer[16];
                    Magnet_Upper_Stru.offset_middle[adr_mpls] = (u16)(Serial->recv_pbuffer[13]<<8)+Serial->recv_pbuffer[14];
                    Magnet_Upper_Stru.offset_right[adr_mpls] = 0;
                }
                else if(xs_segment == 6)////2根磁条-右中
                {
                    Magnet_Upper_Stru.segment[adr_mpls] = 2;
                    Magnet_Upper_Stru.offset_left[adr_mpls] = (u16)(Serial->recv_pbuffer[17]<<8)+Serial->recv_pbuffer[18];
                    Magnet_Upper_Stru.offset_middle[adr_mpls] = (u16)(Serial->recv_pbuffer[15]<<8)+Serial->recv_pbuffer[16];
                    Magnet_Upper_Stru.offset_right[adr_mpls] = 0;
                }
                else if(xs_segment == 7)////3根磁条-左中右
                {
                    Magnet_Upper_Stru.segment[adr_mpls] = 3;
                    Magnet_Upper_Stru.offset_left[adr_mpls] = (u16)(Serial->recv_pbuffer[17]<<8)+Serial->recv_pbuffer[18];
                    Magnet_Upper_Stru.offset_middle[adr_mpls] = (u16)(Serial->recv_pbuffer[15]<<8)+Serial->recv_pbuffer[16];
                    Magnet_Upper_Stru.offset_right[adr_mpls] = (u16)(Serial->recv_pbuffer[13]<<8)+Serial->recv_pbuffer[14];
                }
                
                // 磁段
                //Magnet_Upper_Stru.segment[adr_mpls]          = (u16)(Serial->recv_pbuffer[5]<<8)+Serial->recv_pbuffer[6];   
                // 左偏移值
                //Magnet_Upper_Stru.offset_left[adr_mpls]      = (u16)(Serial->recv_pbuffer[13]<<8)+Serial->recv_pbuffer[14];
                // 中偏移值
                //Magnet_Upper_Stru.offset_middle[adr_mpls]    = (u16)(Serial->recv_pbuffer[15]<<8)+Serial->recv_pbuffer[16];
                // 右偏移值
                //Magnet_Upper_Stru.offset_right[adr_mpls]     = (u16)(Serial->recv_pbuffer[17]<<8)+Serial->recv_pbuffer[18];
                // 点位值
                //Magnet_Upper_Stru.io_low[adr_mpls]           = Serial->recv_pbuffer[7];
                //Magnet_Upper_Stru.io_high[adr_mpls]          = Serial->recv_pbuffer[8];

                Magnet_Upper_Stru.magnet_comm_time[adr_mpls] = 0; 

                
            }
        }
        
        Serial->recv_complete_bit =0;
    }
}





/***************************************************************************************
*函    数: void MAGNET_Xs_Send_Mesg_Task(void)
*功    能: 查询模式
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：此函数根据每调用一次发送一次查询通信指令(查询模式有效)
****************************************************************************************/
void MAGNET_Xs_Send_Mesg_Task(void)
{
    
    static u8 std_id=4,std_rs485_id=0;
    u8 magnet_canopen_buf[8]={0};
    u8 magnet_send_ask_buf[4][8]= {
        {0x01,0x04,0x03,0xE8,0x00,0x08,0x71,0xBC},
        {0x02,0x04,0x03,0xE8,0x00,0x08,0x71,0x8F},
        {0x03,0x04,0x03,0xE8,0x00,0x08,0x70,0x5E},
        {0x04,0x04,0x03,0xE8,0x00,0x08,0x71,0xE9},
    };
    
    if(Magnet_Upper_Stru.magnet_comm_time[0]<1000)  Magnet_Upper_Stru.magnet_comm_time[0]++;
    if(Magnet_Upper_Stru.magnet_comm_time[1]<1000)  Magnet_Upper_Stru.magnet_comm_time[1]++;
    if(Magnet_Upper_Stru.magnet_comm_time[2]<1000)  Magnet_Upper_Stru.magnet_comm_time[2]++;
    if(Magnet_Upper_Stru.magnet_comm_time[3]<1000)  Magnet_Upper_Stru.magnet_comm_time[3]++;
    
    
    if(Magnet_Upper_Stru.can_com_type == EM_SET_PORT_CAN )  // CAN模式下
    {
	if(++xs_canopen_send_number >= 10)//*10ms发送间隔
        {
		xs_canopen_send_number = 0;
        
		magnet_canopen_buf[0] =0x88;
        	magnet_canopen_buf[1] =Magnet_Upper_Stru.branch_set;//分支
        	CAN1_Send_One_Frame_Data((0x80+std_id), magnet_canopen_buf, 2);

	        std_id++;
                // 根据设置地址设备个数，自动匹配
	        if(std_id >= (4+Magnet_Upper_Stru.id_number))  std_id =4;       

	}
        
        /*
        switch(Magnet_Upper_Stru.canpen_state)
        {
            case 0:                                        // 配置1801-02-同步类型
                magnet_canopen_buf[0] =0x2F;               //发送数据为1个字节
                magnet_canopen_buf[1] =0x01;               //对象索引-1801-TPDO2
                magnet_canopen_buf[2] =0x18;               // 
                magnet_canopen_buf[3] =0x02;               // 子索引-02
                magnet_canopen_buf[4] =0xFE;               //254 异步
                magnet_canopen_buf[5] =0x00;
                magnet_canopen_buf[6] =0x00;
                magnet_canopen_buf[7] =0x00;
                CAN1_Send_One_Frame_Data((0x600+std_id),magnet_canopen_buf,8);
                
                xs_canopen_send_number++;
                if(xs_canopen_send_number >= 20)            //发送多次
                {
                    xs_canopen_send_number =0;
                    Magnet_Upper_Stru.canpen_state++;
                }
                
               
                
                break;
            case 1:                                        // 配置1801-03-发送最小间隔
                magnet_canopen_buf[0] =0x2F;
                magnet_canopen_buf[1] =0x01;
                magnet_canopen_buf[2] =0x18;               // 1801
                magnet_canopen_buf[3] =0x03;               // 03-子索引
                magnet_canopen_buf[4] =0x64;               // 10ms
                magnet_canopen_buf[5] =0x00;
                magnet_canopen_buf[6] =0x00;
                magnet_canopen_buf[7] =0x00;
                CAN1_Send_One_Frame_Data((0x600+std_id),magnet_canopen_buf,8);
                
                xs_canopen_send_number++;
                if(xs_canopen_send_number >= 20)            //发送多次
                {
                    xs_canopen_send_number =0;
                    Magnet_Upper_Stru.canpen_state++;
                }
                
               
                
                break;
                
            case 2:                                        // 配置1801-05-发送周期
                magnet_canopen_buf[0] =0x2B;
                magnet_canopen_buf[1] =0x01;
                magnet_canopen_buf[2] =0x18;               // 1801
                magnet_canopen_buf[3] =0x05;               // 05-子索引
                magnet_canopen_buf[4] =0x0A;               // 10ms
                magnet_canopen_buf[5] =0x00;
                magnet_canopen_buf[6] =0x00;
                magnet_canopen_buf[7] =0x00;
                CAN1_Send_One_Frame_Data((0x600+std_id),magnet_canopen_buf,8);
                
                xs_canopen_send_number++;
                if(xs_canopen_send_number >= 20)            //发送多次
                {
                    xs_canopen_send_number =0;
                    Magnet_Upper_Stru.canpen_state++;
                }
                
               
                
                break;
                
            case 3:
                magnet_canopen_buf[0] =0x01;               // 01-启动节点命令
                magnet_canopen_buf[1] =0x00;               // 00-所有地址，xx-对应地址
                magnet_canopen_buf[2] =0x00;               
                magnet_canopen_buf[3] =0x00;               
                magnet_canopen_buf[4] =0x00;               
                magnet_canopen_buf[5] =0x00;
                magnet_canopen_buf[6] =0x00;
                magnet_canopen_buf[7] =0x00;
                CAN1_Send_One_Frame_Data(0x00, magnet_canopen_buf, 2);
                
                xs_canopen_send_number++;
                if(xs_canopen_send_number >= 5)            //发送多次
                {
                    xs_canopen_send_number =0;
                    Magnet_Upper_Stru.canpen_state++;
                }
                
               
                
                break;
                                
            default:
                break;
                
        }
	
        
        if(Magnet_Upper_Stru.canpen_state < 4)
        {
            std_id++;
            if(std_id >= (4+Magnet_Upper_Stru.id_number))         // 根据设置地址设备个数，自动匹配
            {
                std_id =4;
            }
        }
        */
        
    }                
    
    else if(Magnet_Upper_Stru.can_com_type == EM_SET_PORT_COM )      // RS485模式下     
    {
        // 主动上传-退出-未配置端口
        if(Magnet_Upper_Stru.commun_port == NULL || Magnet_Upper_Stru.commun_ask_mode !=EM_DEV_ASK_HAND)
        {
            return;
        }

        Bsp_Usart_Usr_SendArray(Magnet_Upper_Stru.commun_port, &magnet_send_ask_buf[std_rs485_id][0],8);
       
        
        std_rs485_id++;
        if(std_rs485_id >= Magnet_Upper_Stru.id_number)
        {
            std_rs485_id =0;
        }
        
    }
    
}

/***************************************************************************************
*函    数: void Magnet_XS_CAN_RX_Handler(CanRxMsg* RxMessage)
*功    能: 兴颂走形-CAN接收处理
*参    数:         
*作    者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
void Magnet_XS_CAN_RX_Handler(CanRxMsg* RxMessage)
{
    u8 can_mpls_adr=0;
    
    switch(RxMessage->StdId)
    {
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
            can_mpls_adr = RxMessage->StdId-4;
            //XS_CAN_HS协议
            if(Magnet_Upper_Stru.commun_ask_mode == EM_DEV_USE_CAN && 
               Magnet_Upper_Stru.can_com_type == EM_SET_PORT_CAN &&
               Magnet_Upper_Stru.device_type  == EM_DEVICE_DEV_MAGNET_XS && 
               RxMessage->DLC == 0x04)
            {
                //传感器状态
                Magnet_Upper_Stru.error_state[can_mpls_adr] = RxMessage->Data[0];   
                
                //磁条感应状态
                if(RxMessage->Data[1] == 7)
                	Magnet_Upper_Stru.segment[can_mpls_adr] = 3;//3段磁条
                else if(RxMessage->Data[1] == 3||RxMessage->Data[1] == 6)
                	Magnet_Upper_Stru.segment[can_mpls_adr] = 2;//2段磁条
                else if(RxMessage->Data[1] > 0)
                    Magnet_Upper_Stru.segment[can_mpls_adr] = 1;//1段磁条
                else if(RxMessage->Data[1] == 0)
                    Magnet_Upper_Stru.segment[can_mpls_adr] = 0;//0段磁条
                    
                // 偏移值
                //Magnet_Upper_Stru.datatemp = RxMessage->Data[3];
                Magnet_Upper_Stru.offset_left[can_mpls_adr]   = (u8)(RxMessage->Data[3]+(u16)(RxMessage->Data[2]<<8));

                Magnet_Upper_Stru.magnet_comm_time[can_mpls_adr] = 0;
                
                
            }             
            break;
             
        /*
        case 0x584:
        case 0x585:
        case 0x586:
        case 0x587:
             can_mpls_adr = RxMessage->StdId-0x584;
             if(Magnet_Upper_Stru.commun_ask_mode == EM_DEV_CANOPEN && 
                Magnet_Upper_Stru.can_com_type == EM_SET_PORT_CAN &&
                Magnet_Upper_Stru.device_type  == EM_DEVICE_DEV_MAGNET_XS)
             {
                Magnet_Upper_Stru.magnet_comm_time[can_mpls_adr] = 0;
               
             }
             break;
           
        
        case 0x284:
        case 0x285:
        case 0x286:
        case 0x287:
            can_mpls_adr = RxMessage->StdId-0x284;
            
            //XS_CANOPEN 模式
            if(Magnet_Upper_Stru.commun_ask_mode == EM_DEV_CANOPEN && 
               Magnet_Upper_Stru.can_com_type == EM_SET_PORT_CAN &&
               Magnet_Upper_Stru.device_type  == EM_DEVICE_DEV_MAGNET_XS && 
               RxMessage->DLC == 0x08)
            {
                Magnet_Upper_Stru.segment[can_mpls_adr]         = RxMessage->Data[1];   
                                                                    // 左偏移值
                Magnet_Upper_Stru.offset_left[can_mpls_adr]     = (u8)(RxMessage->Data[2]+(u16)(RxMessage->Data[3]<<8));
                                                                    // 中偏移值
                Magnet_Upper_Stru.offset_middle[can_mpls_adr]   = (u8)(RxMessage->Data[4]+(u16)(RxMessage->Data[5]<<8));
                                                                    // 右偏移值
                Magnet_Upper_Stru.offset_right[can_mpls_adr]    = (u8)(RxMessage->Data[6]+(u16)(RxMessage->Data[7]<<8));

                Magnet_Upper_Stru.magnet_comm_time[can_mpls_adr] = 0;
              
            }
            break;
        */  
            
        default:
            break;
            
    }
    
}

