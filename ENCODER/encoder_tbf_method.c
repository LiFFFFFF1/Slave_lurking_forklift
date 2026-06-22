#include "encoder_tbf_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "encoder_upper.h"
#include "plc_hub_custom_method.h"
#include "myiic_upper.h"
#include "motor_upper.h"

#define ENCODER_OFFSET_READ_DELAY_CNT 15    // 20ms*15=300ms，等电源和24C64先站稳再读零位

/***************************************************************************************
*函    数: void Encoder_tbf_SendAPI(void)
*功    能:
*参    数:
*作    者:
*修改时间:
*返 回 值:
*备    注：
*
****************************************************************************************/
void Encoder_tbf_SendAPI(void)
{
    u8 buf_temp[8] = {0};
    u32 offset_temp[2] = {0};
    static u8 start_bit = 0;
    static u8 offset_read_delay_cnt =0;

    //零位读取
    if(start_bit == 0)//只执行一次
    {
        if(offset_read_delay_cnt < ENCODER_OFFSET_READ_DELAY_CNT)
        {
            offset_read_delay_cnt++;
            return;
        }

        start_bit = 1;

        myiic_read_24c64_api(0, 8, buf_temp);
        offset_temp[0] = ((u32)buf_temp[3]<<24) | ((u32)buf_temp[2]<<16) | ((u32)buf_temp[1]<<8) | buf_temp[0];
        offset_temp[1] = ((u32)buf_temp[7]<<24) | ((u32)buf_temp[6]<<16) | ((u32)buf_temp[5]<<8) | buf_temp[4];

        if(offset_temp[0] == 0xffffffff)//初次上电
        {
            buf_temp[0] = 0;
            buf_temp[1] = 0;
            buf_temp[2] = 0;
            buf_temp[3] = 0;
            myiic_write_24c64_api(0, 4, &buf_temp[0]);//写入0
            Encoder_Mesg_Stru.offset_data[0] = 0;
        }
        else Encoder_Mesg_Stru.offset_data[0] = offset_temp[0];

        if(offset_temp[1] == 0xffffffff)//初次上电
        {
            buf_temp[4] = 0;
            buf_temp[5] = 0;
            buf_temp[6] = 0;
            buf_temp[7] = 0;
            myiic_write_24c64_api(4, 4, &buf_temp[4]);//写入0
            Encoder_Mesg_Stru.offset_data[1] = 0;
        }
        else Encoder_Mesg_Stru.offset_data[1] = offset_temp[1];

    }

    //零位设置保存
    if(Encoder_Mesg_Stru.save_bit == 1)
    {
        Encoder_Mesg_Stru.save_bit = 0;

        if(Motor_Lift_Stru.unit_act_id == EM_UNIT_ACT_ID_LEFT
           &&getbit(Encoder_Mesg_Stru.err_state, 6) == 0)//指定1，无报警
        {
            Encoder_Mesg_Stru.offset_data[0] = Encoder_Mesg_Stru.currt_data[0];
            buf_temp[0] = Encoder_Mesg_Stru.offset_data[0]&0xff;
            buf_temp[1] = (Encoder_Mesg_Stru.offset_data[0]>>8)&0xff;
            buf_temp[2] = (Encoder_Mesg_Stru.offset_data[0]>>16)&0xff;
            buf_temp[3] = (Encoder_Mesg_Stru.offset_data[0]>>24)&0xff;
            myiic_write_24c64_api(0, 4, &buf_temp[0]);//写入保存
        }
        else if(Motor_Lift_Stru.unit_act_id == EM_UNIT_ACT_ID_RIGHT
                &&getbit(Encoder_Mesg_Stru.err_state, 7) == 0)//指定2，无报警
        {
            Encoder_Mesg_Stru.offset_data[1] = Encoder_Mesg_Stru.currt_data[1];
            buf_temp[4] = Encoder_Mesg_Stru.offset_data[1]&0xff;
            buf_temp[5] = (Encoder_Mesg_Stru.offset_data[1]>>8)&0xff;
            buf_temp[6] = (Encoder_Mesg_Stru.offset_data[1]>>16)&0xff;
            buf_temp[7] = (Encoder_Mesg_Stru.offset_data[1]>>24)&0xff;
            myiic_write_24c64_api(4, 4, &buf_temp[4]);//写入保存
        }
        else if(Motor_Lift_Stru.unit_act_id == EM_UNIT_ACT_ID_ALL
                &&Encoder_Mesg_Stru.err_state == 0)//指定所有，无报警
        {
            Encoder_Mesg_Stru.offset_data[0] = Encoder_Mesg_Stru.currt_data[0];
            Encoder_Mesg_Stru.offset_data[1] = Encoder_Mesg_Stru.currt_data[1];
            buf_temp[0] = Encoder_Mesg_Stru.offset_data[0]&0xff;
            buf_temp[1] = (Encoder_Mesg_Stru.offset_data[0]>>8)&0xff;
            buf_temp[2] = (Encoder_Mesg_Stru.offset_data[0]>>16)&0xff;
            buf_temp[3] = (Encoder_Mesg_Stru.offset_data[0]>>24)&0xff;
            buf_temp[4] = Encoder_Mesg_Stru.offset_data[1]&0xff;
            buf_temp[5] = (Encoder_Mesg_Stru.offset_data[1]>>8)&0xff;
            buf_temp[6] = (Encoder_Mesg_Stru.offset_data[1]>>16)&0xff;
            buf_temp[7] = (Encoder_Mesg_Stru.offset_data[1]>>24)&0xff;
            myiic_write_24c64_api(0, 8, &buf_temp[0]);//写入保存
        }

    }

    //超时检测
    if(Encoder_Mesg_Stru.com_timer[0] < 100)    Encoder_Mesg_Stru.com_timer[0]++;
    if(Encoder_Mesg_Stru.com_timer[1] < 100)    Encoder_Mesg_Stru.com_timer[1]++;

    if(Encoder_Mesg_Stru.com_timer[0] > 10)//*20ms超时
    {
        Encoder_Mesg_Stru.com_timer[0] = 0;
        setbit(Encoder_Mesg_Stru.err_state, 6);
        Encoder_Mesg_Stru.currt_data[0] = 0;//数据清零
        Encoder_Mesg_Stru.real_data[0] = 0;
    }

    if(Encoder_Mesg_Stru.com_timer[1] > 10)//*20ms超时
    {
        Encoder_Mesg_Stru.com_timer[1] = 0;
        setbit(Encoder_Mesg_Stru.err_state, 7);
        Encoder_Mesg_Stru.currt_data[1] = 0;//数据清零
        Encoder_Mesg_Stru.real_data[1] = 0;
    }

    //复位报警
    if(getbit(PLC_TO_HUB_Mesg_Stru.ctrl_cmd, 0) == 1
       &&Encoder_Mesg_Stru.err_state > 0)
    {
        Encoder_Mesg_Stru.err_state = 0;
    }

}

/***************************************************************************************
*函    数: Encoder_tbf_CAN_Isr(CanRxMsg* RxMessage)
*功    能:
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void Encoder_tbf_CAN_Isr(CanRxMsg* RxMessage)
{
    u8 i = 0;

    switch(RxMessage->StdId)
    {
        case 0x180+TBF_CAN_ID:
        case 0x180+TBF_CAN_ID+1:

            i =RxMessage->StdId - (0x180+TBF_CAN_ID);

            if(RxMessage->DLC == 0x04)
            {
                Encoder_Mesg_Stru.com_timer[i] = 0;

                //当前位置-原始值
                Encoder_Mesg_Stru.currt_data[i] = ((u32)RxMessage->Data[3]<<24)
                                                  | ((u32)RxMessage->Data[2]<<16)
                                                  | ((u32)RxMessage->Data[1]<<8)
                                                  | RxMessage->Data[0];
                // 实际值：先转成有符号再相减，避免零位附近回退 1 个脉冲时无符号下溢成大正数
                Encoder_Mesg_Stru.real_data[i] = ((s32)Encoder_Mesg_Stru.currt_data[i] - (s32)Encoder_Mesg_Stru.offset_data[i]) / 10;

            }
            break;

        default:
            break;
    }

}



