#include "obs_sick_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "obs_area_upper.h"
#include "plc_hub_upper.h"

/***************************************************************************************
*函    数: void Obs_AreaSICK_IO_Send_Task(void)
*功    能: SICK避障器输出IO-设置避障区域
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void Obs_AreaSICK_IO_Send_Task(void)
{
    switch(PLC_TO_HUB_Mesg_Stru.set_run_obs)
    {
        case 1:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 0;
            break;
            
        case 2:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 0;
            break;

        case 3:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 0;
            break;

        case 4:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 0;
            break;

        case 5:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 0;
            break;

        case 6:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 0;
            break;

        case 7:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 0;
            break;

        case 8:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 0;
            break;

        case 9:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 1;
            break;

        case 10:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 1;
            break;

        case 11:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 1;
            break;

        case 12:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 0;
            OBS_XS_IO_IN4 = 1;
            break;

        case 13:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 1;
            break;

        case 14:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 0;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 1;
            break;

        case 15:
            OBS_XS_IO_IN1 = 0;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 1;
            break;

        case 16:
            OBS_XS_IO_IN1 = 1;
            OBS_XS_IO_IN2 = 1;
            OBS_XS_IO_IN3 = 1;
            OBS_XS_IO_IN4 = 1;
            break;

        default:
            break;
    }

    OBS_Mesg_Stru.upload_cur_obs = OBS_XS_IO_IN4*8+OBS_XS_IO_IN3*4+
                                   OBS_XS_IO_IN2*2+OBS_XS_IO_IN1+1;// 当前通道

}

/***************************************************************************************
*函    数: void Obs_AreaSICK_IO_RX_Task(void)
*功    能: 兴颂避障器输入IO-避障状态
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void Obs_AreaSICK_IO_RX_Task(void)
{
    static u16 start_timer = 0;
    static u16 timer = 0;
    
    OBS_Mesg_Stru.upload_out1_state = OBS_XS_IO_OUT3;      // 最外
    OBS_Mesg_Stru.upload_out2_state = OBS_XS_IO_OUT2;      // 中间
    OBS_Mesg_Stru.upload_out3_state = OBS_XS_IO_OUT1;     // 最内

    if(OBS_Mesg_Stru.upload_out3_state)                   // 远避障
    {
        PLC_TO_HUB_Mesg_Stru.upload_run_warning |=0x01;
    }
    else
    {
        PLC_TO_HUB_Mesg_Stru.upload_run_warning &=0xFE;
    }

    if(OBS_Mesg_Stru.upload_out1_state)//近避障
    {
        PLC_TO_HUB_Mesg_Stru.upload_run_warning |=0x02;
    }
    else
    {
        PLC_TO_HUB_Mesg_Stru.upload_run_warning &=0xFD;
    }
    
    //障碍物故障
    if(start_timer < 50000)    start_timer ++;
    if(start_timer >= 20000&&OBS_XS_IO_OUT4)//启动20s后滤波
    {
        if(timer < 1000)    timer ++;//*1ms
    }
    else timer = 0;
    
    if(timer > 200)    PLC_TO_HUB_Mesg_Stru.upload_obs_alarm = 1;//故障

    //清零
    if(PLC_TO_HUB_Mesg_Stru.set_cmd == 0x02 && PLC_TO_HUB_Mesg_Stru.upload_obs_alarm)           //  复位
    {
        PLC_TO_HUB_Mesg_Stru.upload_obs_alarm = 0;
	timer = 0;
    }

}


