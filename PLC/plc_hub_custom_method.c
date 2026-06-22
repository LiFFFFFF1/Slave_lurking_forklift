#include "plc_hub_custom_method.h"

// 包含使用模块 xxx_xxx_upper.h

#define NEW_UP_FRAME_LENGTH   20  // 上传帧长度
#define NEW_DOWN_FRAME_LENGTH 13  // 下发帧长度

#define IAP_FRAME_LENGTH 14  // 进入串口IAP更新下发帧长度
uint32_t iap_boot_flag __attribute__((at(APP_BOOT_COMM_VAR_ADDR)));


/***************************************************************************************
*函    数:  void PLC_ReadCustomData(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能:  读取PLC上传数据，从队列数据自动匹配协议
*参    数:  根据配置COM端口，自动匹配对应队列
*作    者:  shiquan.zhao
*修改时间:
*返 回 值: 无
*备    注：自定义协议接收
****************************************************************************************/
static void PLC_Hub_CustomReadData(USART_STRU *Serial,QUEUE *QUEUE_com)
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
                
                if(Serial->step == 0)
                {
                    if(dat == 0xEE)// 起始码
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[0] = dat;
                        Serial->count = 1;
                    }
                    else if(dat == 0x48)//head-切换串口IAP更新
                    {
                        Serial->step = 0x10;
                        Serial->recv_pbuffer[0] = dat;
                        Serial->count = 1;
                    }
                    
                }
                else if(Serial->step == 1)
                {
                    if(dat == 0x51)// 起始码
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
                else if(Serial->step == 2)
                {
                    Serial->step++;
                    Serial->recv_pbuffer[2] = dat;// 读取长度字节
                    Serial->count=3;
                }
                
                else if(Serial->step == 3)//主机数据帧
                {
                    Serial->recv_pbuffer[Serial->count++] = dat;
                    if(Serial->count >= Serial->recv_pbuffer[2])
                    {
                        Serial->recv_complete_bit=Serial->count;
                        Serial->recv_complete_bit |= 0x8000;

                        Serial->step = 0;
                        Serial->count = 0;
                        return;
                    }
                }

		/////////////////切换串口IAP更新数据帧
		else if(Serial->step == 0x10)
                {
                    if(dat == 0x05)//cmd-切换串口IAP更新
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

		else if(Serial->step == 0x11)
                {
                    Serial->recv_pbuffer[Serial->count++] = dat;
                    if(Serial->count >= IAP_FRAME_LENGTH)
                    {
                        Serial->recv_complete_bit = Serial->count;
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
*函    数: void PLC_Hub_CustomRX_Task(void)
*功    能: 根据从队列获取的数据，进行数据校验
*参    数: 根据配置COM端口，自动匹配对应队列
*作    者:  shiquan.zhao
*修改时间:
*返 回 值: 无
****************************************************************************************/
void PLC_Hub_CustomRX_Task(void)
{
    USART_STRU *Serial;
    QUEUE *QUEUE_com;
    u8 xor_temp=0;
    u16 get_all_len=0;
    u16 iap_recv_data_len =0;
    u32 flash_addr_index = 0;


    if(PLC_TO_HUB_Mesg_Stru.commun_port == USART1)
    {
        Serial    = &Usart1_Serial_Stru;
        QUEUE_com = &COM1_Serial;
    }
    else if(PLC_TO_HUB_Mesg_Stru.commun_port == USART2)
    {
        Serial    = &Usart2_Serial_Stru;
        QUEUE_com = &COM2_Serial;
    }
    else if(PLC_TO_HUB_Mesg_Stru.commun_port == USART3)
    {
        Serial    = &Usart3_Serial_Stru;
        QUEUE_com = &COM3_Serial;
    }
    else if(PLC_TO_HUB_Mesg_Stru.commun_port == UART4)
    {
        Serial    = &Usart4_Serial_Stru;
        QUEUE_com = &COM4_Serial;
    }
    else if(PLC_TO_HUB_Mesg_Stru.commun_port == UART5)
    {
        Serial    = &Usart5_Serial_Stru;
        QUEUE_com = &COM5_Serial;
    }
    else
    {
        return;
    }

    PLC_Hub_CustomReadData(Serial,QUEUE_com);                   // 自定义解码

    if(Serial->recv_complete_bit & 0x8000)
    {
        get_all_len =Serial->recv_complete_bit&0x7FFF;

        if(get_all_len == NEW_DOWN_FRAME_LENGTH)//主机数据帧
        {
            xor_temp = Bsp_XorCheck(Serial->recv_pbuffer, NEW_DOWN_FRAME_LENGTH-1);

            // 结束码
            if(Serial->recv_pbuffer[NEW_DOWN_FRAME_LENGTH-1] == xor_temp)
            {
                Parse_NewProtocol_Down_Frame(Serial->recv_pbuffer);

	        PLC_TO_HUB_Mesg_Stru.over_time_set = 1000;
	        PLC_TO_HUB_Mesg_Stru.recv_over_time = 0;
            }
        }

        else if(get_all_len == IAP_FRAME_LENGTH)//切换串口IAP更新数据帧
        {
            // 设置起始地址，小端存储，低位在前
            flash_addr_index = (uint32_t)Serial->recv_pbuffer[5];
            flash_addr_index <<= 8;
            flash_addr_index += (uint32_t)Serial->recv_pbuffer[4];
            flash_addr_index <<= 8;
            flash_addr_index += (uint32_t)Serial->recv_pbuffer[3];
            flash_addr_index <<= 8;
            flash_addr_index += (uint32_t)Serial->recv_pbuffer[2];

            //数据区长度
            iap_recv_data_len = (u16)(Serial->recv_pbuffer[7]<<8)+Serial->recv_pbuffer[6];

            // XOR校验判断 
            xor_temp = Bsp_XorCheck(Serial->recv_pbuffer,(get_all_len-1));

            if(xor_temp == Serial->recv_pbuffer[get_all_len-1]//校验
                &&Serial->recv_pbuffer[get_all_len-2] == 0x58//end
                &&flash_addr_index == FLASH_APP1_START_ADDR//addr
                &&get_all_len == 14//有效长度
                &&iap_recv_data_len == 4//数据区长度
                &&Serial->recv_pbuffer[8] == 0x05// 数据区
                &&Serial->recv_pbuffer[9] == 0x05
                &&Serial->recv_pbuffer[10] == 0x05
                &&Serial->recv_pbuffer[11] == 0x05)
            {
		PLC_TO_HUB_Mesg_Stru.enter_iap_update_flag = 1;//切换串口IAP更新标志
	    }

        }
        
        Serial->recv_complete_bit=0;
    }

    //超时计时*1ms
    if(++PLC_TO_HUB_Mesg_Stru.recv_over_time > PLC_TO_HUB_Mesg_Stru.over_time_set)
    {
         PLC_TO_HUB_Mesg_Stru.recv_over_time = 0;
         PLC_TO_HUB_Mesg_Stru.light_cmd = EM_LIGHT_INIT;//灯全灭
    }


    //报警复位
    if(getbit(PLC_TO_HUB_Mesg_Stru.ctrl_cmd, 0) == 1
        &&PLC_TO_HUB_Mesg_Stru.err_data > 0)    
    {
        PLC_TO_HUB_Mesg_Stru.err_id = 0;
        PLC_TO_HUB_Mesg_Stru.err_data = 0;
    }


#if IAP_1_W5500_2_Serial == 2//串口更新
    //进入串口IAP更新
    if(PLC_TO_HUB_Mesg_Stru.enter_iap_update_flag == 1)
    {
	if(++PLC_TO_HUB_Mesg_Stru.enter_iap_update_dalay_timer > 500)//*1ms
        {
            PLC_TO_HUB_Mesg_Stru.enter_iap_update_flag = 0;//清除标志
            PLC_TO_HUB_Mesg_Stru.enter_iap_update_dalay_timer = 0;

	    iap_boot_flag = APP_BOOTLOADER_REQ_FLAG;//更新标志
	    
            __set_FAULTMASK(1);// 关闭所有中断
            NVIC_SystemReset();// 复位
        }
    }
#endif
    

}


static void Parse_NewProtocol_Down_Frame(u8 *frame)
{
    // 验证帧长度 ,   命令码
    if(frame[2] != NEW_DOWN_FRAME_LENGTH || frame[3] != 0x01)
    {
        return;
    }

    // 解析灯状态
    PLC_TO_HUB_Mesg_Stru.light_cmd = frame[4];

    // 解析控制位
    PLC_TO_HUB_Mesg_Stru.ctrl_cmd = frame[5];

    // 解析播放ID
    PLC_TO_HUB_Mesg_Stru.set_music_id = frame[6];

    //上装动作类型
    Motor_Lift_Stru.unit_type = frame[7];
    
    //上装执行id
    Motor_Lift_Stru.unit_act_id = frame[8];

    //上装控制模式
    Motor_Lift_Stru.unit_ctrl_mode = frame[9];
    
    //上装控制参数
    Motor_Lift_Stru.unit_ctrl_data = (s16)((frame[11] << 8) | frame[10]);


}



/***************************************************************************************
*函    数: void PLC_Hub_CustomTX_Mesg_Ask(void)
*功    能: HUB发送给PLC的数据处理
*参    数:
*作    者: shiquan.zhao
*修改时间:
*返 回 值:    无
*备    注： 匹配旧协议
*
****************************************************************************************/
void PLC_Hub_CustomTX_Mesg_Ask(void)
{
    static u8 send_step = 0;
    u8 upload_frame[NEW_UP_FRAME_LENGTH] = {0};

    if(PLC_TO_HUB_Mesg_Stru.commun_port == NULL)
    {
        return;
    }

    upload_frame[0] = 0xEE;                                     // 帧头
    upload_frame[1] = 0x61;
    upload_frame[2] = NEW_UP_FRAME_LENGTH;    // 总长度
    upload_frame[3] = 0x01;                                     // 命令码
    upload_frame[4] = ANTICO_Mesg_Stru.err_state | Encoder_Mesg_Stru.err_state;//                                       
    upload_frame[5] = MCU_VISION;
    upload_frame[6] = PLC_TO_HUB_Mesg_Stru.inputL_state;
    upload_frame[7] = PLC_TO_HUB_Mesg_Stru.inputH_state;
    upload_frame[8] = PLC_TO_HUB_Mesg_Stru.output_state;
    upload_frame[9] = PLC_TO_HUB_Mesg_Stru.charge_state;//充电状态
    upload_frame[10] = PLC_TO_HUB_Mesg_Stru.set_music_id;//播放音乐id
    
    upload_frame[11] = Motor_Lift_Stru.unit_type;//上装动作类型
    upload_frame[12] = Motor_Lift_Stru.unit_ctrl_mode;//上装控制模式
    upload_frame[13] = Motor_Lift_Stru.unit_act_state;//上装执行反馈状态

    if(send_step == 0)//交替发
    {
        upload_frame[14] = EM_UNIT_ACT_ID_LEFT;//1-左叉齿
        upload_frame[15] = GET_LOW_BYTE(Motor_Lift_Stru.unit_act_data[0]);//反馈参数L
        upload_frame[16] = GET_HIGH_BYTE(Motor_Lift_Stru.unit_act_data[0]);//反馈参数H
        upload_frame[17] = GET_LOW_BYTE(Motor_Lift_Stru.unit_err_code[0]);//故障码L
        upload_frame[18] = GET_HIGH_BYTE(Motor_Lift_Stru.unit_err_code[0]);//故障码H
        send_step = 1;//切换
    }
    else if(send_step == 1)
    {
        upload_frame[14] = EM_UNIT_ACT_ID_RIGHT;//2-右叉齿
        upload_frame[15] = GET_LOW_BYTE(Motor_Lift_Stru.unit_act_data[1]);//反馈参数L
        upload_frame[16] = GET_HIGH_BYTE(Motor_Lift_Stru.unit_act_data[1]);//反馈参数H
        upload_frame[17] = GET_LOW_BYTE(Motor_Lift_Stru.unit_err_code[1]);//故障码L
        upload_frame[18] = GET_HIGH_BYTE(Motor_Lift_Stru.unit_err_code[1]);//故障码H
        send_step = 0;//切换
    }

    upload_frame[19] = Bsp_XorCheck(upload_frame, NEW_UP_FRAME_LENGTH-1);

    Bsp_Usart_Usr_SendArray(PLC_TO_HUB_Mesg_Stru.commun_port, upload_frame,upload_frame[2]);

}




