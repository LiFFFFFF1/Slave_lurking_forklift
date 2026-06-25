#include "magnet_lg_sensor_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "magnet_sensor_upper.h"


// 私有宏定义
#define  LG_SENSOR_REC_LEN            11

static u8 lg_canopen_send_number=0;

/***************************************************************************************
*函    数: void Magnet_LG_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
static void Magnet_LG_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
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
                    if(dat <= 0x04)                                // 起始码
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
                    if(Serial->count >= LG_SENSOR_REC_LEN)      // 字节数+5
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
*函    数: void MAGNET_LG_COM_RX_Task(void)
*功    能: 接收处理
*参    数:  Serial-对应串口发送结构体，QUEUE_com-对应接收串口结构体
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void MAGNET_LG_COM_RX_Task(void)
{
    u8 left_temp = 0;
    u8 middle_temp = 0;
    u8 right_temp = 0;
    u16 crc_val=0;
    u8 adr_mpls=0;
    USART_STRU *Serial;
    QUEUE *QUEUE_com;

    if(Magnet_Upper_Stru.can_com_type == EM_SET_PORT_CAN)   // CAN-模式下
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

    Magnet_LG_ReadbufData(Serial,QUEUE_com);                   // 获取对应串口对接数据

    if(Serial->recv_complete_bit & 0x8000)
    {
        /* CRC校验判断 */
        crc_val = Bsp_ModbusRTU_CRC(Serial->recv_pbuffer,LG_SENSOR_REC_LEN-2);
        //  先判断ID，在判断校验
        if(Serial->recv_pbuffer[0] >= 1 && Serial->recv_pbuffer[0] <= 4 &&
           (GET_LOW_BYTE(crc_val)==Serial->recv_pbuffer[LG_SENSOR_REC_LEN-2]) &&
           (GET_HIGH_BYTE(crc_val)==Serial->recv_pbuffer[LG_SENSOR_REC_LEN-1]))
        {
            adr_mpls =Serial->recv_pbuffer[0] - 1;

            if(Serial->recv_pbuffer[1] == 0x03 && Serial->recv_pbuffer[2] == 0x06)
            {
                // 磁段
                Magnet_Upper_Stru.segment[adr_mpls] = Serial->recv_pbuffer[3];
                // 左偏移值
                Magnet_Upper_Stru.offset_left[adr_mpls] = Serial->recv_pbuffer[4];
                // 中偏移值
                Magnet_Upper_Stru.offset_middle[adr_mpls] = Serial->recv_pbuffer[5];
                // 右偏移值
                Magnet_Upper_Stru.offset_right[adr_mpls] = Serial->recv_pbuffer[6];;

                //错误码
                //Magnet_Upper_Stru.error_state[adr_mpls] = Serial->recv_pbuffer[6];

                // 点位值
                Magnet_Upper_Stru.io_high[adr_mpls] = Serial->recv_pbuffer[7];
                Magnet_Upper_Stru.io_low[adr_mpls] = Serial->recv_pbuffer[8];

                Magnet_Upper_Stru.magnet_comm_time[adr_mpls] = 0;


                //分支选择后上传
                if(PLC_TO_HUB_Mesg_Stru.set_run_branch == EM_AGV_BRANCH_LEFT)//左分支
                {
                    if(Magnet_Upper_Stru.segment[adr_mpls] == 3)                //3段磁条
                        PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_right[adr_mpls];
                    else if(Magnet_Upper_Stru.segment[adr_mpls] == 2)////2段磁条
                        PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_middle[adr_mpls];
                    else////1段磁条
                        PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_left[adr_mpls];
                }
                else if(PLC_TO_HUB_Mesg_Stru.set_run_branch == EM_AGV_BRANCH_RIGHT)//右分支
                {
                    if(Magnet_Upper_Stru.segment[adr_mpls] == 3)                //3段磁条
                        PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_left[adr_mpls];
                    else if(Magnet_Upper_Stru.segment[adr_mpls] == 2)////2段磁条
                        PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_left[adr_mpls];
                    else////1段磁条
                        PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_left[adr_mpls];
                }
                else//未指定默认中分支
                {
                    if(Magnet_Upper_Stru.segment[adr_mpls] == 3)//3段磁条
                    {
                        left_temp = abs((s8)Magnet_Upper_Stru.offset_left[adr_mpls]);
                        middle_temp = abs((s8)Magnet_Upper_Stru.offset_middle[adr_mpls]);
                        right_temp = abs((s8)Magnet_Upper_Stru.offset_right[adr_mpls]);

                        if(left_temp < middle_temp&&left_temp < right_temp)//左绝对值最小
                        {
                            PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_left[adr_mpls];
                        }
                        else if(middle_temp <= left_temp&&middle_temp <= right_temp)//中绝对值最小
                        {
                            PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_middle[adr_mpls];
                        }
                        else if(right_temp < left_temp&&right_temp < middle_temp)//右绝对值最小
                        {
                            PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_right[adr_mpls];
                        }
                    }

                    else if(Magnet_Upper_Stru.segment[adr_mpls] == 2)////2段磁条
                    {
                        //绝对值比较
                        if(abs((s8)Magnet_Upper_Stru.offset_left[adr_mpls]) <= abs((s8)Magnet_Upper_Stru.offset_middle[adr_mpls]))
                        {
                            PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_left[adr_mpls];
                        }
                        else
                            PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_middle[adr_mpls];
                    }

                    else////1段磁条
                        PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_left[adr_mpls];

                }

                PLC_TO_HUB_Mesg_Stru.set_run_segment = Magnet_Upper_Stru.segment[adr_mpls];

            }
        }

        Serial->recv_complete_bit =0;
    }
}


/***************************************************************************************
*函    数: void MAGNET_LG_Send_Mesg_Task(void)
*功    能: 查询模式
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
*备    注：此函数根据每调用一次发送一次查询通信指令(查询模式有效)
****************************************************************************************/
void MAGNET_LG_Send_Mesg_Task(void)
{
    static u8 std_send_id=0;
    u8 u8_index=0;
    u8 magnet_canopen_buf[8]= {0};
    u8 magnet_send_ask_buf[4][8]=
    {
        {0x04,0x03,0x00,0x04,0x00,0x03,0x44,0x5F},
        {0x05,0x03,0x00,0x04,0x00,0x03,0x45,0x8E},
        {0x06,0x03,0x00,0x04,0x00,0x03,0x45,0xBD},
        {0x07,0x03,0x00,0x04,0x00,0x03,0x44,0x6C},
    };

    //离线计时
    for(u8_index=0; u8_index<Magnet_Upper_Stru.id_number; u8_index++)
    {
        if(Magnet_Upper_Stru.magnet_comm_time[u8_index] < 3)        // *15ms
        {
            Magnet_Upper_Stru.magnet_comm_time[u8_index]++;
        }
        else
        {
            Magnet_Upper_Stru.offline |= (1<<u8_index);              // 掉线标志

            Magnet_Upper_Stru.segment[u8_index] =0;
            // 左偏移值
            Magnet_Upper_Stru.offset_left[u8_index]    = 0;
            // 中偏移值
            Magnet_Upper_Stru.offset_middle[u8_index]  = 0;
            // 中偏移值
            Magnet_Upper_Stru.offset_right[u8_index]  = 0;

            // 上传磁段
            PLC_TO_HUB_Mesg_Stru.set_run_segment =0;

        }

    }


    //报警检测
    if(Magnet_Upper_Stru.offline)                                    // 离线
    {
        PLC_TO_HUB_Mesg_Stru.upload_magnet_alarm = 0xff;
    }
    else if(Magnet_Upper_Stru.error_state[0])                        // 故障代码
    {
        PLC_TO_HUB_Mesg_Stru.upload_magnet_alarm = Magnet_Upper_Stru.error_state[0];
    }

    if(PLC_TO_HUB_Mesg_Stru.set_cmd == 0x02 && PLC_TO_HUB_Mesg_Stru.upload_magnet_alarm)                         // 复位
    {
        PLC_TO_HUB_Mesg_Stru.upload_magnet_alarm = 0;
        Magnet_Upper_Stru.error_state[0] = 0;
        Magnet_Upper_Stru.offline = 0;
        Magnet_Upper_Stru.magnet_comm_time[0] = 0;
        Magnet_Upper_Stru.magnet_comm_time[1] = 0;
        Magnet_Upper_Stru.magnet_comm_time[2] = 0;
        Magnet_Upper_Stru.magnet_comm_time[3] = 0;

        Magnet_Upper_Stru.canpen_state =0;//重新启动节点
    }

    if(Magnet_Upper_Stru.can_com_type == EM_SET_PORT_CAN)            // CAN模式下-里格仅支持CANOpen
    {
        switch(Magnet_Upper_Stru.canpen_state)
        {
            case 0:

                magnet_canopen_buf[0] =0x01;                    // 01-启动节点命令
                magnet_canopen_buf[1] =std_send_id+4;           // 00-所有地址，xx-对应地址
                magnet_canopen_buf[2] =0x00;
                magnet_canopen_buf[3] =0x00;
                magnet_canopen_buf[4] =0x00;
                magnet_canopen_buf[5] =0x00;
                magnet_canopen_buf[6] =0x00;
                magnet_canopen_buf[7] =0x00;
                CAN1_Send_One_Frame_Data(0x00, magnet_canopen_buf, 2);

                lg_canopen_send_number ++;

                if(lg_canopen_send_number >= 4)                 //  发送多次启动节点
                {
                    lg_canopen_send_number =0;
                    if((std_send_id+1) < Magnet_Upper_Stru.id_number)
                    {
                        std_send_id++;
                    }
                    else
                    {
                        Magnet_Upper_Stru.canpen_state++;
                    }
                }
                break;
            case 1:
                break;
            default:
                break;
        }

    }
    else if(Magnet_Upper_Stru.can_com_type == EM_SET_PORT_COM)       // RS485模式下
    {
        // 主动上传-退出-未配置端口
        if(Magnet_Upper_Stru.commun_port == NULL || Magnet_Upper_Stru.commun_ask_mode != EM_DEV_ASK_HAND)
        {
            return;
        }

        Bsp_Usart_Usr_SendArray(Magnet_Upper_Stru.commun_port, &magnet_send_ask_buf[std_send_id][0], 8);


        std_send_id++;
        if(std_send_id >= Magnet_Upper_Stru.id_number)
        {
            std_send_id =0;
        }

    }

}

/***************************************************************************************
*函    数: void Magnet_LG_CAN_RX_Handler(CanRxMsg* RxMessage)
*功    能: 里格走形-CAN接收处理
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
*          低字节在前，高位在后
****************************************************************************************/
void Magnet_LG_CAN_RX_Handler(CanRxMsg* RxMessage)
{
    u8 can_mpls_adr=0;
    u8 left_temp = 0;
    u8 middle_temp = 0;
    u8 right_temp = 0;

    switch(RxMessage->StdId)
    {
        case 0x184:
//        case 0x185:
//        case 0x186:
//        case 0x187:
            can_mpls_adr = 0;

            /* LG_CANOPEN 模式*/
            {
                Magnet_Upper_Stru.offline &= ~(1<<can_mpls_adr);       // 在线标志
                Magnet_Upper_Stru.magnet_comm_time[can_mpls_adr] = 0;

                //段数
                Magnet_Upper_Stru.segment[can_mpls_adr] = RxMessage->Data[1];
                // 左偏移值
                Magnet_Upper_Stru.offset_left[can_mpls_adr] = ((u16)(RxMessage->Data[3]<<8)+RxMessage->Data[2]);               
                // 中偏移值
                Magnet_Upper_Stru.offset_middle[can_mpls_adr] = ((u16)(RxMessage->Data[5]<<8)+RxMessage->Data[4]);
                // 右偏移值
                Magnet_Upper_Stru.offset_right[can_mpls_adr] = ((u16)(RxMessage->Data[7]<<8)+RxMessage->Data[6]);

                // 分支选择
                switch(PLC_TO_HUB_Mesg_Stru.set_run_branch)
                {
                    case EM_AGV_BRANCH_LEFT:        // 左分支
                        if(Magnet_Upper_Stru.segment[can_mpls_adr] == 3)                    // 3段磁条
                        {
                            PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_right[can_mpls_adr];
                        }
                        else if(Magnet_Upper_Stru.segment[can_mpls_adr] == 2)               // 2段磁条
                        {
                            PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_middle[can_mpls_adr];
                        }
                        else                                                                // 1段磁条
                        {
                            PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_left[can_mpls_adr];
                        }
                        break;
                    case EM_AGV_BRANCH_RIGHT:       //  右分支
                        PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_left[can_mpls_adr];
                        break;
                    default:                        //   默认-中分支                
                        if(Magnet_Upper_Stru.segment[can_mpls_adr] == 3)		            //  3段磁条
                        {
                            left_temp   = abs((s8)Magnet_Upper_Stru.offset_left[can_mpls_adr]);
                            middle_temp = abs((s8)Magnet_Upper_Stru.offset_middle[can_mpls_adr]);
                            right_temp  = abs((s8)Magnet_Upper_Stru.offset_right[can_mpls_adr]);

                            if(left_temp < middle_temp && left_temp < right_temp)           //  左绝对值最小
                            {
                                PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_left[can_mpls_adr];
                            }
                            else if(middle_temp <= left_temp && middle_temp <= right_temp)  //  中绝对值最小
                            {
                                PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_middle[can_mpls_adr];
                            }
                            else if(right_temp < left_temp && right_temp < middle_temp)     // 右绝对值最小
                            {
                                PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_right[can_mpls_adr];
                            }

                        }
                        else if(Magnet_Upper_Stru.segment[can_mpls_adr] == 2)               // 2段磁条
                        {
                            //绝对值比较
                            if(abs((s8)Magnet_Upper_Stru.offset_left[can_mpls_adr]) <= abs((s8)Magnet_Upper_Stru.offset_middle[can_mpls_adr]))
                            {
                                PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_left[can_mpls_adr];
                            }
                            else
                                PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_middle[can_mpls_adr];
                        }
                        else                                                            //1段磁条
                        {
                            PLC_TO_HUB_Mesg_Stru.upload_run_magnet = Magnet_Upper_Stru.offset_left[can_mpls_adr];
                        }
                        break;
                }

                PLC_TO_HUB_Mesg_Stru.set_run_segment = Magnet_Upper_Stru.segment[can_mpls_adr];

            }
            break;

        case 0x284:
//        case 0x285:
//        case 0x286:
//        case 0x287:
            can_mpls_adr = 0;

            /* LG_CANOPEN 模式*/
            {
                Magnet_Upper_Stru.offline &= ~(1<<can_mpls_adr);                        // 在线标志
                Magnet_Upper_Stru.magnet_comm_time[can_mpls_adr] = 0;

                // 里格老款协议
                /*
                Magnet_Upper_Stru.err_code[can_mpls_adr] = RxMessage->Data[0];          // 错误代码
                Magnet_Upper_Stru.err_data[can_mpls_adr] = RxMessage->Data[1];          // 错误参数
                Magnet_Upper_Stru.magnet_width[can_mpls_adr] = RxMessage->Data[2];      // 磁条宽度
                Magnet_Upper_Stru.io_num[can_mpls_adr] = RxMessage->Data[3];            // 传感器点位总数

                Magnet_Upper_Stru.new_io_low_low[can_mpls_adr] = RxMessage->Data[4];    // 点位信息
                Magnet_Upper_Stru.new_io_low_high[can_mpls_adr] = RxMessage->Data[5];   //
                Magnet_Upper_Stru.new_io_high_low[can_mpls_adr] = RxMessage->Data[6];   //
                Magnet_Upper_Stru.new_io_high_high[can_mpls_adr] = RxMessage->Data[7];  //
                
                //满磁
                if(Magnet_Upper_Stru.new_io_low_low[can_mpls_adr]   == 0xff &&
                   Magnet_Upper_Stru.new_io_low_high[can_mpls_adr]  == 0xff &&
                   Magnet_Upper_Stru.new_io_high_low[can_mpls_adr]  == 0x03 &&
                   Magnet_Upper_Stru.new_io_high_high[can_mpls_adr] == 0x00)
                {
                    PLC_TO_HUB_Mesg_Stru.set_run_segment = 1;                           //  磁段=  1
                    PLC_TO_HUB_Mesg_Stru.upload_run_magnet = 0;                         //  偏距=  0
                }
                
                */
                
                // 里格新款-V2.1协议
                Magnet_Upper_Stru.new_io_low_low[can_mpls_adr] = RxMessage->Data[0];    //  点位信息
                Magnet_Upper_Stru.new_io_low_high[can_mpls_adr] = RxMessage->Data[1];   //
                Magnet_Upper_Stru.new_io_high_low[can_mpls_adr] = RxMessage->Data[2];   //
                Magnet_Upper_Stru.new_io_high_high[can_mpls_adr] = RxMessage->Data[3];  //
                Magnet_Upper_Stru.io_num[can_mpls_adr] = RxMessage->Data[4];            //  传感器点位总数
                Magnet_Upper_Stru.magnet_width[can_mpls_adr] = RxMessage->Data[5];      //  磁条宽度
                
                Magnet_Upper_Stru.err_code[can_mpls_adr] = RxMessage->Data[6];          //  错误代码
                Magnet_Upper_Stru.err_data[can_mpls_adr] = RxMessage->Data[7];          //  错误参数
                

                

                if(Magnet_Upper_Stru.err_code[can_mpls_adr])    //非0-锁定故障
                    Magnet_Upper_Stru.error_state[can_mpls_adr] = Magnet_Upper_Stru.err_code[can_mpls_adr];

            }


        default:
            break;

    }

}



