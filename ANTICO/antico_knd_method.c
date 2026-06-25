#include "antico_knd_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "antico_upper.h"

/***************************************************************************************
*函    数: void ANTICO_Read_KND_Data(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
*备    注：
****************************************************************************************/
static void ANTICO_Read_KND_Data(USART_STRU *Serial,QUEUE *QUEUE_com)
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
                    if(dat == ANTICO_Mesg_Stru.device_adr)     // 起始码
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[0] = dat;
                        Serial->count = 1;
                    }
                }

                else if(Serial->step== 1)
                {
                    if(dat == 0x03)          // 功能码 || dat == 0x10
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
                    if(dat == 0x08)          //长度
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
*函    数: void ANTICO_KND_Send_Task(void)
*功    能: modbus问询模式指令
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
*备    注：
*
****************************************************************************************/
void ANTICO_KND_Send_Task(void)
{
    u16 antico_crc_val=0;
    u8 antico_ask_buf[8]= {0};
    u8 fork_tip_touch_flag = 0;

    //上电延时8秒，等待模块准备就绪
    if(ANTICO_Mesg_Stru.power_on_delay < 160)//160*50ms=8000ms=8s
    {
        ANTICO_Mesg_Stru.power_on_delay++;
        return;//延时期间直接返回，不执行任何举升操作
    }

    if(ANTICO_Mesg_Stru.commun_ask_mode != EM_ANTICO_ASK_MODBUS)
    {
        return;
    }

    if(ANTICO_Mesg_Stru.can_com_type == EM_ANTICO_COM_MODE)
    {
        // adr
        antico_ask_buf[0] = ANTICO_Mesg_Stru.device_adr;
        antico_ask_buf[1] = 0x03;                              // cmd-read

        antico_ask_buf[2] = 0x02;                              // star_H8
        antico_ask_buf[3] = 0x58;                              // star_L8

        antico_ask_buf[4] = 0x00;                              // LEN-H8
        antico_ask_buf[5] = 0X04;                              // LEN-L8

        antico_crc_val =Bsp_ModbusRTU_CRC(antico_ask_buf,6);
        antico_ask_buf[6] =GET_LOW_BYTE(antico_crc_val);//0xc4
        antico_ask_buf[7] =GET_HIGH_BYTE(antico_crc_val);//0x62

        Bsp_Usart_Usr_SendArray(ANTICO_Mesg_Stru.commun_port, antico_ask_buf,8);
    }

    if((PLC_TO_HUB_Mesg_Stru.inputL_state & 0x03) != 0x03)
    {
        fork_tip_touch_flag = 1;
    }


    if(getbit(PLC_TO_HUB_Mesg_Stru.ctrl_cmd, 5) == 0)//触边未屏蔽
    {
        //超时计时
        if(++ANTICO_Mesg_Stru.com_timer > 20)//x50ms
        {
            ANTICO_Mesg_Stru.com_timer = 0;
            setbit(ANTICO_Mesg_Stru.err_state, 1);//触边模块掉线
            ANTICO_Mesg_Stru.get_adc_voltage[0] = 0;//数据清零
            ANTICO_Mesg_Stru.get_adc_voltage[1] = 0;
        }

        //触边断线检测
        if(ANTICO_Mesg_Stru.get_adc_voltage[0] < ADC_VOLT_MIN)
	    ANTICO_Mesg_Stru.lineoff_timer[0] ++;
        else ANTICO_Mesg_Stru.lineoff_timer[0] = 0;

        if(ANTICO_Mesg_Stru.lineoff_timer[0] > 20)//x50ms
        {
	    ANTICO_Mesg_Stru.lineoff_timer[0] = 0;
	    setbit(ANTICO_Mesg_Stru.err_state, 2);//车体触边断线
        }

        if(ANTICO_Mesg_Stru.get_adc_voltage[1] < ADC_VOLT_MIN)
	    ANTICO_Mesg_Stru.lineoff_timer[1] ++;
        else ANTICO_Mesg_Stru.lineoff_timer[1] = 0;

        if(ANTICO_Mesg_Stru.lineoff_timer[1] > 20)//x50ms
        {
	    ANTICO_Mesg_Stru.lineoff_timer[1] = 0;
            if(ANTICO_Mesg_Stru.tray_connect_state == 1)//托盘已连接
	        setbit(ANTICO_Mesg_Stru.err_state, 3);//托盘触边断线
        }
    
        //触边触发检测
        if(ANTICO_Mesg_Stru.get_adc_voltage[0] > ADC_VOLT_MAX)
        {
	    setbit(ANTICO_Mesg_Stru.err_state, 4);//车体触边触发
        }

        if((ANTICO_Mesg_Stru.get_adc_voltage[1] > ADC_VOLT_MAX
            && ANTICO_Mesg_Stru.tray_connect_state == 1)
            || fork_tip_touch_flag == 1)
        {
	       setbit(ANTICO_Mesg_Stru.err_state, 5);//托盘触边触发/叉尖触边复用
        }
        
    }
    else if(getbit(PLC_TO_HUB_Mesg_Stru.ctrl_cmd, 5) == 1)//触边屏蔽
    {
        ANTICO_Mesg_Stru.com_timer = 0;
        clrbit(ANTICO_Mesg_Stru.err_state, 1);//触边模块掉线-屏蔽

	ANTICO_Mesg_Stru.lineoff_timer[0] = 0;
	clrbit(ANTICO_Mesg_Stru.err_state, 2);//车体触边断线-屏蔽

	ANTICO_Mesg_Stru.lineoff_timer[1] = 0;
	clrbit(ANTICO_Mesg_Stru.err_state, 3);//托盘触边断线-屏蔽

	clrbit(ANTICO_Mesg_Stru.err_state, 4);//车体触边触发-屏蔽
	clrbit(ANTICO_Mesg_Stru.err_state, 5);//叉尺触边触发-屏蔽

    }
    
    
    //报警复位
    if(getbit(PLC_TO_HUB_Mesg_Stru.ctrl_cmd, 0) == 1
        &&ANTICO_Mesg_Stru.err_state > 0)  
    {
        ANTICO_Mesg_Stru.err_state = 0;
    }
    
}


/***************************************************************************************
*函    数: void ANTICO_KND_RX_Task(void)
*功    能:  处理解析队列数据
*参    数:  Serial-对应串口发送结构体，QUEUE_com-对应接收串口结构体
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void ANTICO_KND_RX_Task(void)
{
    USART_STRU *Serial;
    QUEUE *QUEUE_com;
    u16 tem16_val=0;
    u16 get_len=0;

    if(ANTICO_Mesg_Stru.commun_port == USART1)
    {
        Serial    = &Usart1_Serial_Stru;
        QUEUE_com = &COM1_Serial;
    }
    else if(ANTICO_Mesg_Stru.commun_port == USART2)
    {
        Serial    = &Usart2_Serial_Stru;
        QUEUE_com = &COM2_Serial;
    }
    else if(ANTICO_Mesg_Stru.commun_port == USART3)
    {
        Serial    = &Usart3_Serial_Stru;
        QUEUE_com = &COM3_Serial;
    }
    else if(ANTICO_Mesg_Stru.commun_port == UART4)
    {
        Serial    = &Usart4_Serial_Stru;
        QUEUE_com = &COM4_Serial;
    }
    else if(ANTICO_Mesg_Stru.commun_port == UART5)
    {
        Serial    = &Usart5_Serial_Stru;
        QUEUE_com = &COM5_Serial;
    }
    else
    {
        return;
    }

    ANTICO_Read_KND_Data(Serial,QUEUE_com);                  // 自定义解码

    if(Serial->recv_complete_bit & 0x8000)
    {
        get_len =Serial->recv_complete_bit&0x7FFF;

        /* CRC校验判断 */
        tem16_val = Bsp_ModbusRTU_CRC(Serial->recv_pbuffer,(get_len-2));

        if((GET_LOW_BYTE(tem16_val)==Serial->recv_pbuffer[get_len-2]) &&
           (GET_HIGH_BYTE(tem16_val)==Serial->recv_pbuffer[get_len-1]))
        {
            ANTICO_Mesg_Stru.com_timer = 0;

	    ANTICO_Mesg_Stru.get_adc_data[0] = 
                (Serial->recv_pbuffer[5]<<24)+(Serial->recv_pbuffer[6]<<16)+(Serial->recv_pbuffer[3]<<8)+Serial->recv_pbuffer[4];
	    ANTICO_Mesg_Stru.get_adc_data[1] = 
                (Serial->recv_pbuffer[9]<<24)+(Serial->recv_pbuffer[10]<<16)+(Serial->recv_pbuffer[7]<<8)+Serial->recv_pbuffer[8];

            ANTICO_Mesg_Stru.get_adc_voltage[0] = Bsp_U32ToFloat(ANTICO_Mesg_Stru.get_adc_data[0])/1000;
            ANTICO_Mesg_Stru.get_adc_voltage[1] = Bsp_U32ToFloat(ANTICO_Mesg_Stru.get_adc_data[1])/1000;
                
        }

        Serial->recv_complete_bit =0;
    }

}




