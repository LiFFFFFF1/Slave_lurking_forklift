#include "obs_xs_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "obs_area_upper.h"
#include "plc_hub_upper.h"


/***************************************************************************************
*函    数: void Obs_Com_Read_XSData(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
*备    注：自定义协议接收
****************************************************************************************/
static void Obs_Com_Read_XSData(USART_STRU *Serial,QUEUE *QUEUE_com)
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
                    Serial->recv_pbuffer[0] = dat;
                    if(dat == OBS_Mesg_Stru.device_adr)     // 起始码
                    {
                        Serial->step++;
                        Serial->count = 1;
                    }
                }
                else if(Serial->step== 1)
                {
                    Serial->recv_pbuffer[1] = dat;
                    if(dat == 0x04)               // 功能码
                    {
                        Serial->step =2;
                        Serial->count = 2;
                    }
                    else if(dat == 0x10)              // 功能码
                    {
                        Serial->step =3;
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
                    if(Serial->count>=(Serial->recv_pbuffer[2]+5))
                    {

                        Serial->recv_complete_bit=Serial->count;
                        Serial->recv_complete_bit |= 0x8000;

                        Serial->step = 0;
                        Serial->count = 0;
                        return;
                    }
                }
                else if(Serial->step== 3)  // 0x10
                {
                    Serial->recv_pbuffer[Serial->count++] = dat;
                    if(Serial->count>=8)
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
                if(Serial->the_over_time>=10)
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
*函    数: void Obs_AreaXs_Send_Task(void)
*功    能: modbus问询模式指令
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
*备    注：
*
****************************************************************************************/
void Obs_AreaXs_Send_Task(void)
{
    u16 obs_crc_val=0;
    u8 obs_ask_buf[11]= {0};
    static u8 _obs_send_num=0;

    if(OBS_Mesg_Stru.can_com_type == EM_SET_PORT_COM)
    {
        // adr
        if(_obs_send_num == 0)                                  // 查询避障状态
        {
            obs_ask_buf[0] = OBS_Mesg_Stru.device_adr;
            obs_ask_buf[1] = 0x04;                              // cmd
            obs_ask_buf[2] = 0x03;                              // star_H8
            obs_ask_buf[3] = 0xE8;                              // star_L8-----1000

            obs_ask_buf[4] = 0x00;                              // 寄存器个数H8
            obs_ask_buf[5] = 0x05;                              // 寄存器个数L8
            obs_crc_val =Bsp_ModbusRTU_CRC(obs_ask_buf,6);
            obs_ask_buf[6] =GET_LOW_BYTE(obs_crc_val);
            obs_ask_buf[7] =GET_HIGH_BYTE(obs_crc_val);

            Bsp_Usart_Usr_SendArray(OBS_Mesg_Stru.commun_port, obs_ask_buf,8);
            _obs_send_num =1;
        }
        else                                                    // 设置区域
        {
            obs_ask_buf[0] = OBS_Mesg_Stru.device_adr;
            obs_ask_buf[1] = 0x10;                              // cmd
            obs_ask_buf[2] = 0x07;                              // star_H8
            obs_ask_buf[3] = 0xD1;                              // star_L8-----2001

            obs_ask_buf[4] = 0x00;                              // 寄存器个数H8
            obs_ask_buf[5] = 0x01;                              // 寄存器个数L8

            obs_ask_buf[6] = 0x02;                              // 数据长度

            obs_ask_buf[7] = 0x00;                              // 数据段



            obs_ask_buf[8] = PLC_TO_HUB_Mesg_Stru.set_run_obs;        // 数据段

            obs_crc_val =Bsp_ModbusRTU_CRC(obs_ask_buf,9);
            obs_ask_buf[9] =GET_LOW_BYTE(obs_crc_val);
            obs_ask_buf[10] =GET_HIGH_BYTE(obs_crc_val);

            Bsp_Usart_Usr_SendArray(OBS_Mesg_Stru.commun_port, obs_ask_buf,11);

            _obs_send_num =0;
        }

        if(OBS_Mesg_Stru.com_timer < 200)
        {
            OBS_Mesg_Stru.com_timer++;
        }
        if(OBS_Mesg_Stru.com_timer == 100)//掉线
        {
            PLC_TO_HUB_Mesg_Stru.upload_obs_alarm =0xFF;
        }

        //清零
        if(PLC_TO_HUB_Mesg_Stru.set_cmd == 0x02 && PLC_TO_HUB_Mesg_Stru.upload_obs_alarm)           //  复位
        {
            PLC_TO_HUB_Mesg_Stru.upload_obs_alarm = 0;
            OBS_Mesg_Stru.com_timer = 0;
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
void Obs_AreaXs_RX_Task(void)
{
    USART_STRU *Serial;
    QUEUE *QUEUE_com;
    u16 tem16_val=0;
    u16 get_len=0;

    if(OBS_Mesg_Stru.commun_port == USART1)
    {
        Serial    = &Usart1_Serial_Stru;
        QUEUE_com = &COM1_Serial;
    }
    else if(OBS_Mesg_Stru.commun_port == USART2)
    {
        Serial    = &Usart2_Serial_Stru;
        QUEUE_com = &COM2_Serial;
    }
    else if(OBS_Mesg_Stru.commun_port == USART3)
    {
        Serial    = &Usart3_Serial_Stru;
        QUEUE_com = &COM3_Serial;
    }
    else if(OBS_Mesg_Stru.commun_port == UART4)
    {
        Serial    = &Usart4_Serial_Stru;
        QUEUE_com = &COM4_Serial;
    }
    else if(OBS_Mesg_Stru.commun_port == UART5)
    {
        Serial    = &Usart5_Serial_Stru;
        QUEUE_com = &COM5_Serial;
    }
    else
    {
        return;
    }

    Obs_Com_Read_XSData(Serial,QUEUE_com);                  // 自定义解码

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

                case 0x04:      // 读数据
                    OBS_Mesg_Stru.com_timer =0;

                    OBS_Mesg_Stru.upload_cur_obs    = Serial->recv_pbuffer[4];      // 当前通道
                    OBS_Mesg_Stru.upload_out1_state = Serial->recv_pbuffer[6];      // 最外
                    OBS_Mesg_Stru.upload_out2_state = Serial->recv_pbuffer[8];      // 中间
                    OBS_Mesg_Stru.upload_out3_state = Serial->recv_pbuffer[10];     // 最内


                    if(OBS_Mesg_Stru.upload_out3_state)
                    {
                        PLC_TO_HUB_Mesg_Stru.upload_run_warning |=0x01;
                    }
                    else
                    {
                        PLC_TO_HUB_Mesg_Stru.upload_run_warning &=0xFE;
                    }
                    if(OBS_Mesg_Stru.upload_out1_state)
                    {
                        PLC_TO_HUB_Mesg_Stru.upload_run_warning |=0x02;
                    }
                    else
                    {
                        PLC_TO_HUB_Mesg_Stru.upload_run_warning &=0xFD;
                    }
                    PLC_TO_HUB_Mesg_Stru.upload_obs_alarm   = Serial->recv_pbuffer[12];

                    break;
                case 0x10:      // 写数据
                    OBS_Mesg_Stru.com_timer =0;

                    break;
                default:
                    break;

            }
        }

        Serial->recv_complete_bit =0;
    }
}


/***************************************************************************************
*函    数: void Obs_AreaXs_IO_Send_Task(void)
*功    能: 兴颂避障器输出IO-设置避障区域
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void Obs_AreaXs_IO_Send_Task(void)
{
    switch(PLC_TO_HUB_Mesg_Stru.set_run_obs)
    {
        case 1:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 0;
            break;

        case 2:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 0;
            break;

        case 3:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 0;
            break;

        case 4:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 0;
            break;

        case 5:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 0;
            break;

        case 6:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 0;
            break;

        case 7:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 0;
            break;

        case 8:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 1;
            break;

        case 9:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 1;
            break;

        case 10:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 1;
            break;

        case 11:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 1;
            break;

        case 12:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 1;
            break;

        case 13:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 1;
            break;

        case 14:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 1;
            break;

        case 15:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 1;
            break;

        default:
            break;
    }

    OBS_Mesg_Stru.upload_cur_obs = OBS_XS_IO_IN4*8+OBS_XS_IO_IN3*4+
                                   OBS_XS_IO_IN2*2+OBS_XS_IO_IN1;// 当前通道

    //清零
    if(PLC_TO_HUB_Mesg_Stru.set_cmd == 0x02 && PLC_TO_HUB_Mesg_Stru.upload_obs_alarm)           //  复位
    {
        PLC_TO_HUB_Mesg_Stru.upload_obs_alarm = 0;

    }

}

/***************************************************************************************
*函    数: void Obs_AreaXs_IO_RX_Task(void)
*功    能: 兴颂避障器输入IO-避障状态
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void Obs_AreaXs_IO_RX_Task(void)
{
    OBS_Mesg_Stru.upload_out1_state = OBS_XS_IO_OUT1;       // 最外
    OBS_Mesg_Stru.upload_out2_state = OBS_XS_IO_OUT2;       // 中间
    OBS_Mesg_Stru.upload_out3_state = OBS_XS_IO_OUT3;       // 最内

    if(OBS_Mesg_Stru.upload_out3_state)                     // 近避障
    {
        PLC_TO_HUB_Mesg_Stru.upload_run_warning |=0x01;
    }
    else
    {
        PLC_TO_HUB_Mesg_Stru.upload_run_warning &=0xFE;
    }

    if(OBS_Mesg_Stru.upload_out1_state)                     // 远避障
    {
        PLC_TO_HUB_Mesg_Stru.upload_run_warning |=0x02;
    }
    else
    {
        PLC_TO_HUB_Mesg_Stru.upload_run_warning &=0xFD;
    }

    PLC_TO_HUB_Mesg_Stru.upload_obs_alarm = OBS_XS_IO_OUT4;//障碍物故障

}


