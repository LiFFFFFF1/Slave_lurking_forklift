#include "encoder_tf_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "encoder_upper.h"




/***************************************************************************************
*函    数: void Encoder_tf_SendAPI(void)
*功    能:
*参    数:
*作    者:
*修改时间:
*返 回 值:
*备    注：
* 
****************************************************************************************/
void Encoder_tf_SendAPI(void)
{
    if(Encoder_Mesg_Stru.com_timer[0] < 100)    Encoder_Mesg_Stru.com_timer[0]++;
    if(Encoder_Mesg_Stru.com_timer[1] < 100)    Encoder_Mesg_Stru.com_timer[1]++;

    if(Encoder_Mesg_Stru.com_timer[0] > 10)//*20ms
    {
        setbit(Encoder_Mesg_Stru.err_state, 0);
        Encoder_Mesg_Stru.currt_data[0] = 0;//数据清零
    }
    else    clrbit(Encoder_Mesg_Stru.err_state, 0);

    if(Encoder_Mesg_Stru.com_timer[1] > 10)//*20ms
    {
        setbit(Encoder_Mesg_Stru.err_state, 1);
        Encoder_Mesg_Stru.currt_data[1] = 0;//数据清零
    }
    else    clrbit(Encoder_Mesg_Stru.err_state, 1);

}

/***************************************************************************************
*函    数: Encoder_tf_CAN_Isr(CanRxMsg* RxMessage)
*功    能:
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void Encoder_tf_CAN_Isr(CanRxMsg* RxMessage)
{
    u8 i = 0;

    switch(RxMessage->StdId)
    {
        case 0x181:
        case 0x182:
            i =RxMessage->StdId - 0x181;

            if(RxMessage->DLC == 0x04)
            {
                Encoder_Mesg_Stru.com_timer[i] = 0;
                //当前位置
                Encoder_Mesg_Stru.currt_data[i] = (RxMessage->Data[3]<<24) + (RxMessage->Data[2]<<16)
                                                  + (RxMessage->Data[1]<<8) + RxMessage->Data[0];
            }
            break;

        default:
            break;
    }

}



