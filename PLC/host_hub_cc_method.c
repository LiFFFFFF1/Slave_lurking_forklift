#include "host_hub_cc_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "plc_hub_upper.h"
#include "public_def_type.h"


/***************************************************************************************
*函    数: void Host_Hub_CAN_TX_Task(void)
*功    能: HUB发送给host  的数据处理
*参    数:
*作    者:
*修改时间:
*返 回 值:    无
*备    注：
*
****************************************************************************************/
void Host_Hub_CAN_TX_Task(void)
{
    u8 send_buf[8] = {0};
    static u16 send_timer = 0;

    //上传状态
    switch(PLC_TO_HUB_Mesg_Stru.canopen_state)
    {
        case EM_INIT:
            if(++send_timer > T_HARBE)//间隔3s
            {
                send_timer = 0;

                send_buf[0] = 0x00;
                CAN1_Send_One_Frame_Data(0x700+DEFAULT_ID, send_buf, 1);
            }

            break;

        case EM_NORM:
            if(PLC_TO_HUB_Mesg_Stru.send_time == 0)
                PLC_TO_HUB_Mesg_Stru.send_time = T_SEND_DEFAULT;

            if(++send_timer > PLC_TO_HUB_Mesg_Stru.send_time)
            {
                send_timer = 0;

                send_buf[0] = 0x01;
                send_buf[1] = PLC_TO_HUB_Mesg_Stru.light_cmd;
                send_buf[2] = PLC_TO_HUB_Mesg_Stru.inputL_state;
                send_buf[3] = PLC_TO_HUB_Mesg_Stru.inputH_state;
                send_buf[4] = PLC_TO_HUB_Mesg_Stru.output_state;
                send_buf[5] = PLC_TO_HUB_Mesg_Stru.ctrl_state;
                send_buf[6] = PLC_TO_HUB_Mesg_Stru.err_data;
                send_buf[7] = Bsp_XorCheck(send_buf, 7);
                CAN1_Send_One_Frame_Data(0x180+DEFAULT_ID, send_buf, 8);
            }
            break;

        default:
            break;

    }


}


/***************************************************************************************
*函    数: void Host_Hub_CAN_RX_Task(CanRxMsg* RxMessage)
*功    能: CAN接收处理
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void Host_Hub_CAN_RX_Task(CanRxMsg* RxMessage)
{
    u8 crc_data = 0;

    switch(RxMessage->StdId)
    {
        case 0x000:
            if(RxMessage->DLC == 0x02)
            {
                //启动节点
                if(RxMessage->Data[0] == 0x01&&
                   (RxMessage->Data[1] == 0x00||RxMessage->Data[1] == DEFAULT_ID))
                {
                    PLC_TO_HUB_Mesg_Stru.canopen_state = EM_NORM;
                }

                //关闭节点
                if(RxMessage->Data[0] == 0x02&&
                   (RxMessage->Data[1] == 0x00||RxMessage->Data[1] == DEFAULT_ID))
                {
                    PLC_TO_HUB_Mesg_Stru.canopen_state = EM_INIT;
                }
            }
            break;

        case 0x200+DEFAULT_ID:
            if(RxMessage->DLC == 0x08&&RxMessage->Data[0] == 0x01)
            {
                crc_data = Bsp_XorCheck(RxMessage->Data, 7);

                //校验通过
                if(crc_data == RxMessage->Data[7])
                {
                    if(RxMessage->Data[1] > 0)
                        PLC_TO_HUB_Mesg_Stru.light_cmd = RxMessage->Data[1];

                    PLC_TO_HUB_Mesg_Stru.ctrl_cmd = RxMessage->Data[2];

                    PLC_TO_HUB_Mesg_Stru.send_time = RxMessage->Data[3];

                }
            }

            break;

        default:
            break;  
    }
}





