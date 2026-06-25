#include "motor_lift_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "motor_upper.h"
#include "plc_hub_upper.h"

#include "public_def_type.h"
 
/*******************************************************************************
  * @brief   _lift_err_check(void)
  *
  * @param   null
  *
  * @retval  none
  *
  * @note
  *****************************************************************************/
static void _lift_err_check(void)
{
    static u8 input_timer[2] = {0};
    static u16 speed_timer[2] = {0};

    //前举升掉线检测*20ms
    if(++Motor_Lift_Stru.comm_timer[0] > 20)//400ms
    {
        Motor_Lift_Stru.comm_timer[0] = 0;

        if(PLC_TO_HUB_Mesg_Stru.err_id == 0)//
        {
            PLC_TO_HUB_Mesg_Stru.err_id = ERR_ID_MOTOR_LIFT1;
            PLC_TO_HUB_Mesg_Stru.err_data = 0x02;
            setbit(Motor_Lift_Stru.can_state, 0);
        }
    }
    //后举升掉线检测*20ms
    if(++Motor_Lift_Stru.comm_timer[1] > 20)//400ms
    {
        Motor_Lift_Stru.comm_timer[1] = 0;

        if(PLC_TO_HUB_Mesg_Stru.err_id == 0)//
        {
            PLC_TO_HUB_Mesg_Stru.err_id = ERR_ID_MOTOR_LIFT2;
            PLC_TO_HUB_Mesg_Stru.err_data = 0x03;
            setbit(Motor_Lift_Stru.can_state, 1);
        }
    }

    //举升升降时间检测*20ms
    if(Motor_Lift_Stru.lift_action_set >= EM_LIFT_ACT_RUN_UP)//运行中
    {
        if(Motor_Lift_Stru.run_timer < 10000)    Motor_Lift_Stru.run_timer ++;
    }
    else
    {
        Motor_Lift_Stru.run_timer = 0;
    }

    if(Motor_Lift_Stru.run_timer > LIFT_RUN_OVERTIME)//超时设置
    {
        if(PLC_TO_HUB_Mesg_Stru.err_id == 0)
        {
            PLC_TO_HUB_Mesg_Stru.err_id = ERR_ID_MOTOR_LIFT1;
            PLC_TO_HUB_Mesg_Stru.err_data = 0x04;//升降超时
        }
    }

    //前后电机速度超差，即前后高低误差过大
    if(Motor_Lift_Stru.lift_action_set >= EM_LIFT_ACT_RUN_UP&&
       ((Motor_Lift_Stru.real_speed[0]-Motor_Lift_Stru.real_speed[1] > DEFAULT_LIFT_SPEED/2)
        ||(Motor_Lift_Stru.real_speed[0]-Motor_Lift_Stru.real_speed[1] < -DEFAULT_LIFT_SPEED/2)))
    {
        if(Motor_Lift_Stru.speed_timer < 1000)    Motor_Lift_Stru.speed_timer ++;
    }
    else Motor_Lift_Stru.speed_timer = 0;

    if(Motor_Lift_Stru.speed_timer > 250)//5000ms高度未修正完毕报警
    {
        if(PLC_TO_HUB_Mesg_Stru.err_id == 0)
        {
            PLC_TO_HUB_Mesg_Stru.err_id = ERR_ID_MOTOR_LIFT1;
            PLC_TO_HUB_Mesg_Stru.err_data = 0x05;//速度超差
        }
    }


    //前电机速度反馈异常，速度下发后无反馈
    if(Motor_Lift_Stru.lift_action_set >= EM_LIFT_ACT_RUN_UP)
    {
        if((Motor_Lift_Stru.set_speed_front > 0&&Motor_Lift_Stru.real_speed[0] < DEFAULT_LIFT_SPEED/2)
           ||(Motor_Lift_Stru.set_speed_front < 0&&Motor_Lift_Stru.real_speed[0] > -DEFAULT_LIFT_SPEED/2))
        {
            if(speed_timer[0] < 1000)    speed_timer[0] ++;
        }
        else speed_timer[0] = 0;
    }
    else speed_timer[0] = 0;

    if(speed_timer[0] > 100)//2000ms
    {
        if(PLC_TO_HUB_Mesg_Stru.err_id == 0)
        {
            PLC_TO_HUB_Mesg_Stru.err_id = ERR_ID_MOTOR_LIFT1;
            PLC_TO_HUB_Mesg_Stru.err_data = 0x0a;//
        }
    }

    //后电机速度反馈异常，速度下发后无反馈
    if(Motor_Lift_Stru.lift_action_set >= EM_LIFT_ACT_RUN_UP)
    {
        if((Motor_Lift_Stru.set_speed_back > 0&&Motor_Lift_Stru.real_speed[1] < DEFAULT_LIFT_SPEED/2)
           ||(Motor_Lift_Stru.set_speed_back < 0&&Motor_Lift_Stru.real_speed[1] > -DEFAULT_LIFT_SPEED/2))
        {
            if(speed_timer[1] < 1000)    speed_timer[1] ++;
        }
        else speed_timer[1] = 0;
    }
    else speed_timer[1] = 0;

    if(speed_timer[1] > 100)//2000ms
    {
        if(PLC_TO_HUB_Mesg_Stru.err_id == 0)
        {
            PLC_TO_HUB_Mesg_Stru.err_id = ERR_ID_MOTOR_LIFT2;
            PLC_TO_HUB_Mesg_Stru.err_data = 0x0b;//
        }
    }



    //到位信号异常-  前
    if(Input_Lift_Front_Upside == GET_INPUT_TURE&&Input_Lift_Front_Downside == GET_INPUT_TURE)//两个到位-异常
    {
        if(++input_timer[0] > INPUT_UP_TIMESET)//消抖
        {
            input_timer[0] = 0;
            if(PLC_TO_HUB_Mesg_Stru.err_id == 0)
            {
                PLC_TO_HUB_Mesg_Stru.err_id = ERR_ID_MOTOR_LIFT1;
                PLC_TO_HUB_Mesg_Stru.err_data = 0x06;//到位信号异常
            }
        }
    }
    else input_timer[0] = 0;

    //到位信号异常-  后
    if(Input_Lift_Back_Upside == GET_INPUT_TURE&&Input_Lift_Back_Downside == GET_INPUT_TURE)//两个到位-异常
    {
        if(++input_timer[1] > INPUT_UP_TIMESET)//消抖
        {
            input_timer[1] = 0;
            if(PLC_TO_HUB_Mesg_Stru.err_id == 0)
            {
                PLC_TO_HUB_Mesg_Stru.err_id = ERR_ID_MOTOR_LIFT2;
                PLC_TO_HUB_Mesg_Stru.err_data = 0x07;//到位信号异常
            }
        }
    }
    else input_timer[1] = 0;


    //复位
    if(getbit(PLC_TO_HUB_Mesg_Stru.ctrl_cmd, 0) == 1
       &&(PLC_TO_HUB_Mesg_Stru.err_id == ERR_ID_MOTOR_LIFT1
          ||PLC_TO_HUB_Mesg_Stru.err_id == ERR_ID_MOTOR_LIFT2))//有报警时
    {
        PLC_TO_HUB_Mesg_Stru.err_id = 0;
        PLC_TO_HUB_Mesg_Stru.err_data = 0;

        Motor_Lift_Stru.can_state = 0;

        Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_RESET;//复位
    }

}

/*******************************************************************************
  * @brief   _lift_canopen_senddata(u8 id, u8 mode, u16 ctl_word, s32 send_val)
  *
  * @param   null
  *
  * @retval  none
  *
  * @note
  *****************************************************************************/
static void _lift_canopen_senddata(u8 id, u8 mode, u16 ctl_word, s32 send_val)
{
    u8 can_buf[8]= {0};

    can_buf[0] = mode;           // 模式 3-PV速度,1-PP位置

    can_buf[1] = ctl_word;       //
    can_buf[2] = ctl_word>>8;    // 控制字

    can_buf[3] = send_val;           // speed_vx>0前进 speed_vx<0后退
    can_buf[4] = send_val>>8;    // 设置行走轮转速 /   位置
    can_buf[5] = send_val>>16;
    can_buf[6] = send_val>>24;

    can_buf[7] = 0;

    CAN1_Send_One_Frame_Data(0x200+id, can_buf, 8);    // 驱动器-低字节在前

}

/*******************************************************************************
  * @brief   _lift_can_send_step(void)
  *
  * @param   null
  *
  * @retval  none
  *
  * @note 举升CAN  发送数据
  *****************************************************************************/
static void _lift_can_send_step(void)
{
    u8 can_buf[8]= {0};
    static u8 timer_start[6] = {0};
    static u8 timer_reset[8] = {0};
    static u8 timer_stop = 0;
    static u8 timer_run = 0;

    switch(Motor_Lift_Stru.lift_action_set)
    {
        case EM_LIFT_ACT_START://启动1
            Output_Lift_Break = 0;//抱闸抱紧

            can_buf[0] = 0x01;
            can_buf[1] = LIFT_CANID_1;
            CAN1_Send_One_Frame_Data(0x000, can_buf, 2);//启动节点1
            
            if(++timer_start[0] > 3)//*20ms
            {
                timer_start[0] = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;
            
        case EM_LIFT_ACT_START+1://启动2
            _lift_canopen_senddata(LIFT_CANID_1, EM_MOTOR_MODE_PV, 0x06, 0);//shutdown1
            
            if(++timer_start[1] > 3)//*20ms
            {
                timer_start[1] = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;

        case EM_LIFT_ACT_START+2://启动3
            _lift_canopen_senddata(LIFT_CANID_1, EM_MOTOR_MODE_PV, 0x07, 0);//断使能
            
            if(++timer_start[2] > 3)//*20ms
            {
                timer_start[2] = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;

        case EM_LIFT_ACT_START+3://启动4
            can_buf[0] = 0x01;
            can_buf[1] = LIFT_CANID_2;
            CAN1_Send_One_Frame_Data(0x000, can_buf, 2);//启动节点2
            
            if(++timer_start[3] > 3)//*20ms
            {
                timer_start[3] = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;

        case EM_LIFT_ACT_START+4://启动5
            _lift_canopen_senddata(LIFT_CANID_2, EM_MOTOR_MODE_PV, 0x06, 0);//shutdown1
            
            if(++timer_start[4] > 3)//*20ms
            {
                timer_start[4] = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;
            
        case EM_LIFT_ACT_START+5://启动6
            _lift_canopen_senddata(LIFT_CANID_2, EM_MOTOR_MODE_PV, 0x07, 0);//断使能
            
            if(++timer_start[5] > 3)//*20ms
            {
                timer_start[5] = 0;
                Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_STOP;//完成后切换到停止
            }            
            break;

        
        case EM_LIFT_ACT_RESET://复位1
        
            Output_Lift_Break = 0;//抱闸抱紧
            
            can_buf[0] = 0x01;
            can_buf[1] = LIFT_CANID_1;
            CAN1_Send_One_Frame_Data(0x000, can_buf, 2);//启动节点1
            
            if(++timer_reset[0] > 3)//*20ms
            {
                timer_reset[0] = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;

        case EM_LIFT_ACT_RESET+1://复位2
        
            _lift_canopen_senddata(LIFT_CANID_1, EM_MOTOR_MODE_PV, 0x86, 0);//清除报警1

            if(++timer_reset[1] > 3)//*20ms
            {
                timer_reset[1] = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;

        case EM_LIFT_ACT_RESET+2://复位3
        
            _lift_canopen_senddata(LIFT_CANID_1, EM_MOTOR_MODE_PV, 0x06, 0);//shutdown1
            
            if(++timer_reset[2] > 3)//*20ms
            {
                timer_reset[2] = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;

        case EM_LIFT_ACT_RESET+3://复位4
        
            _lift_canopen_senddata(LIFT_CANID_1, EM_MOTOR_MODE_PV, 0x07, 0);//断使能
            
            if(++timer_reset[3] > 3)//*20ms
            {
                timer_reset[3] = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;

        case EM_LIFT_ACT_RESET+4://复位5
        
            can_buf[0] = 0x01;
            can_buf[1] = LIFT_CANID_2;
            CAN1_Send_One_Frame_Data(0x000, can_buf, 2);//启动节点2
            
            if(++timer_reset[4] > 3)//*20ms
            {
                timer_reset[4] = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;

        case EM_LIFT_ACT_RESET+5://复位6
        
            _lift_canopen_senddata(LIFT_CANID_2, EM_MOTOR_MODE_PV, 0x86, 0);//清除报警1
            
            if(++timer_reset[5] > 3)//*20ms
            {
                timer_reset[5] = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;

        case EM_LIFT_ACT_RESET+6://复位7
        
            _lift_canopen_senddata(LIFT_CANID_2, EM_MOTOR_MODE_PV, 0x06, 0);//shutdown1
            
            if(++timer_reset[6] > 3)//*20ms
            {
                timer_reset[6] = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;

        case EM_LIFT_ACT_RESET+7://复位8
        
            _lift_canopen_senddata(LIFT_CANID_2, EM_MOTOR_MODE_PV, 0x07, 0);//断使能
            
            if(++timer_reset[7] > 3)//*20ms
            {
                timer_reset[7] = 0;
                Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_STOP;//复位完成后切换到停止
            }            
            break;

            
        case EM_LIFT_ACT_STOP://停止
            Output_Lift_Break = 0;//抱闸抱紧
            _lift_canopen_senddata(LIFT_CANID_1, EM_MOTOR_MODE_PV, 0x0f, 0);//使能+速度0
            _lift_canopen_senddata(LIFT_CANID_2, EM_MOTOR_MODE_PV, 0x0f, 0);//使能+速度0

            if(++timer_stop >= 20)//*20ms
            {
                timer_stop = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;

        case EM_LIFT_ACT_STOP+1://停止+1
            _lift_canopen_senddata(LIFT_CANID_1, EM_MOTOR_MODE_PV, 0x07, 0);//断使能
            _lift_canopen_senddata(LIFT_CANID_2, EM_MOTOR_MODE_PV, 0x07, 0);//断使能
            break;


        case EM_LIFT_ACT_RUN_UP://上升
        case EM_LIFT_ACT_RUN_DOWN://下降
            Output_Lift_Break = 1;//抱闸松开
            _lift_canopen_senddata(LIFT_CANID_1, EM_MOTOR_MODE_PV, 0x0f, 0);//使能+速度
            _lift_canopen_senddata(LIFT_CANID_2, EM_MOTOR_MODE_PV, 0x0f, 0);//使能+速度

            if(++timer_run >= 20)//*20ms
            {
                timer_run = 0;
                Motor_Lift_Stru.lift_action_set ++;
            }
            break;

        case EM_LIFT_ACT_RUN_UP+1://上升+1
        case EM_LIFT_ACT_RUN_DOWN+1://下降+1
            _lift_canopen_senddata(LIFT_CANID_1, EM_MOTOR_MODE_PV, 0x0f, Motor_Lift_Stru.set_speed_front);//使能+速度
            _lift_canopen_senddata(LIFT_CANID_2, EM_MOTOR_MODE_PV, 0x0f, Motor_Lift_Stru.set_speed_back);//使能+速度

            break;

        default:
            break;

    }
}



/*******************************************************************************
  * @brief   Lift_Task_Deal(void)
  *
  * @param   null
  *
  * @retval  none
  *
  * @note
  *****************************************************************************/
void Lift_Task_Deal(void)
{
    static u8 input_timer[4] = {0};

    //上电延时2秒，等待驱动器准备就绪
    if(Motor_Lift_Stru.power_on_delay < 250)//250*20ms=5000ms=5s
    {
        Motor_Lift_Stru.power_on_delay++;
        return;//延时期间直接返回，不执行任何举升操作
    }

    //上电启动节点
    if(Motor_Lift_Stru.lift_action_set == 0)
        Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_START;

    //启动运行
    if(Motor_Lift_Stru.lift_en == ENABLE&&PLC_TO_HUB_Mesg_Stru.err_id == 0)//举升使能，无报警
    {
        //上升
        if(Motor_Lift_Stru.lift_target_highdata[0] > 0)
        {
            //前后都未到位
            if(Input_Lift_Front_Upside == GET_INPUT_FALSE&&Input_Lift_Back_Upside == GET_INPUT_FALSE)
            {
                input_timer[0] = 0;
                if(Motor_Lift_Stru.lift_action_set == EM_LIFT_ACT_STOP+1)//停止状态下
                {
                    Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_RUN_UP;
                }
                else if(Motor_Lift_Stru.lift_action_set == EM_LIFT_ACT_RUN_UP+1)//上升状态下
                {
                    Motor_Lift_Stru.set_speed_front = DEFAULT_LIFT_SPEED*LIFT_DIRET;//前
                    Motor_Lift_Stru.set_speed_back = DEFAULT_LIFT_SPEED*LIFT_DIRET;//后
                }
                else if(Motor_Lift_Stru.lift_action_set == EM_LIFT_ACT_RUN_DOWN+1)//下降状态下
                {
                    Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_STOP;
                    Motor_Lift_Stru.set_speed_front = 0;//速度清零
                    Motor_Lift_Stru.set_speed_back = 0;
                }
            }

            //前或者后已升到位
            else if(Input_Lift_Front_Upside == GET_INPUT_TURE||Input_Lift_Back_Upside == GET_INPUT_TURE)
            {
                if(++input_timer[0] > INPUT_UP_TIMESET)//到位消抖
                {
                    input_timer[0] = 0;
                    if(Motor_Lift_Stru.lift_action_set >= EM_LIFT_ACT_RUN_UP)//运行状态下
                    {
                        Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_STOP;
                        Motor_Lift_Stru.set_speed_front = 0;//速度清零
                        Motor_Lift_Stru.set_speed_back = 0;
                    }
                }
            }
        }

        //下降
        else if(Motor_Lift_Stru.lift_target_highdata[0] == 0)
        {
            //前后都未到位
            if(Input_Lift_Front_Downside == GET_INPUT_FALSE&&Input_Lift_Back_Downside == GET_INPUT_FALSE)
            {
                input_timer[1] = 0;
                input_timer[2] = 0;
                input_timer[3] = 0;

                if(Motor_Lift_Stru.lift_action_set == EM_LIFT_ACT_STOP+1)//停止状态下
                {
                    Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_RUN_DOWN;
                }
                else if(Motor_Lift_Stru.lift_action_set == EM_LIFT_ACT_RUN_DOWN+1)//下降状态下
                {
                    Motor_Lift_Stru.set_speed_front = -DEFAULT_LIFT_SPEED*LIFT_DIRET;//速度取反
                    Motor_Lift_Stru.set_speed_back = -DEFAULT_LIFT_SPEED*LIFT_DIRET;
                }
                else if(Motor_Lift_Stru.lift_action_set == EM_LIFT_ACT_RUN_UP+1)//上升状态下
                {
                    Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_STOP;
                    Motor_Lift_Stru.set_speed_front = 0;//速度清零
                    Motor_Lift_Stru.set_speed_back = 0;
                }
            }

            //前已降到位，后未降到位
            else if(Input_Lift_Front_Downside == GET_INPUT_TURE&&Input_Lift_Back_Downside == GET_INPUT_FALSE)
            {
                input_timer[2] = 0;
                input_timer[3] = 0;

                if(++input_timer[1] > INPUT_DOWN_TIMESET)//到位消抖
                {
                    input_timer[1] = 0;
                    if(Motor_Lift_Stru.lift_action_set == EM_LIFT_ACT_STOP+1)//停止状态下
                    {
                        Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_RUN_DOWN;
                    }
                    else if(Motor_Lift_Stru.lift_action_set == EM_LIFT_ACT_RUN_DOWN+1)//下降状态下
                    {
                        Motor_Lift_Stru.set_speed_front = 0;
                        Motor_Lift_Stru.set_speed_back = -DEFAULT_LIFT_SPEED*LIFT_DIRET;//速度取反
                    }
                    else if(Motor_Lift_Stru.lift_action_set == EM_LIFT_ACT_RUN_UP+1)//上升状态下
                    {
                        Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_STOP;
                        Motor_Lift_Stru.set_speed_front = 0;//速度清零
                        Motor_Lift_Stru.set_speed_back = 0;
                    }

                }
            }

            //前未降到位，后已降到位
            else if(Input_Lift_Front_Downside == GET_INPUT_FALSE&&Input_Lift_Back_Downside == GET_INPUT_TURE)
            {
                input_timer[1] = 0;
                input_timer[3] = 0;

                if(++input_timer[2] > INPUT_DOWN_TIMESET)//到位消抖
                {
                    input_timer[2] = 0;
                    if(Motor_Lift_Stru.lift_action_set == EM_LIFT_ACT_STOP+1)//停止状态下
                    {
                        Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_RUN_DOWN;
                    }
                    else if(Motor_Lift_Stru.lift_action_set == EM_LIFT_ACT_RUN_DOWN+1)//下降状态下
                    {
                        Motor_Lift_Stru.set_speed_front = -DEFAULT_LIFT_SPEED*LIFT_DIRET;//速度取反
                        Motor_Lift_Stru.set_speed_back = 0;
                    }
                    else if(Motor_Lift_Stru.lift_action_set == EM_LIFT_ACT_RUN_UP+1)//上升状态下
                    {
                        Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_STOP;
                        Motor_Lift_Stru.set_speed_front = 0;//速度清零
                        Motor_Lift_Stru.set_speed_back = 0;
                    }
                }
            }

            //前后都已降到位
            else if(Input_Lift_Front_Downside == GET_INPUT_TURE&&Input_Lift_Back_Downside == GET_INPUT_TURE)
            {
                input_timer[1] = 0;
                input_timer[2] = 0;

                if(++input_timer[3] > INPUT_DOWN_TIMESET)//到位消抖
                {
                    input_timer[3] = 0;
                    if(Motor_Lift_Stru.lift_action_set >= EM_LIFT_ACT_RUN_UP)//运行状态下
                    {
                        Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_STOP;
                        Motor_Lift_Stru.set_speed_front = 0;//速度清零
                        Motor_Lift_Stru.set_speed_back = 0;
                    }
                }
            }
        }

    }
    else//运行时报警//无使能
    {
        //运行时停止运行
        if(Motor_Lift_Stru.lift_action_set >= EM_LIFT_ACT_RUN_UP)
        {
            Motor_Lift_Stru.lift_action_set = EM_LIFT_ACT_STOP;
            Motor_Lift_Stru.set_speed_front = 0;//速度清零
            Motor_Lift_Stru.set_speed_back = 0;
        }
    }

    //升降状态-  总
    if((Input_Lift_Front_Upside == GET_INPUT_TURE&&Input_Lift_Front_Downside == GET_INPUT_FALSE)
       ||(Input_Lift_Back_Upside == GET_INPUT_TURE&&Input_Lift_Back_Downside == GET_INPUT_FALSE))
    {
        Motor_Lift_Stru.all_lift_state = EM_LIFT_STATE_UPSIDE;//升到位
        Motor_Lift_Stru.high_real_data[0] = DEFAULT_LIFT_HIGH_DATA;//高度
        Motor_Lift_Stru.high_real_data[1] = DEFAULT_LIFT_HIGH_DATA;//高度
    }
    else if((Input_Lift_Front_Upside == GET_INPUT_FALSE&&Input_Lift_Front_Downside == GET_INPUT_TURE)
            &&(Input_Lift_Back_Upside == GET_INPUT_FALSE&&Input_Lift_Back_Downside == GET_INPUT_TURE))
    {
        Motor_Lift_Stru.all_lift_state = EM_LIFT_STATE_DOWNSIDE;//降到位
        Motor_Lift_Stru.high_real_data[0] = 0;//高度
        Motor_Lift_Stru.high_real_data[1] = 0;//高度
    }
    else
    {
        if(Motor_Lift_Stru.lift_action_set >= EM_LIFT_ACT_RUN_UP)
            Motor_Lift_Stru.all_lift_state = EM_LIFT_STATE_RUNING;//运行中
        else
            Motor_Lift_Stru.all_lift_state = EM_LIFT_STATE_STOP;//停止中
    }

    _lift_err_check();//异常检测
    _lift_can_send_step();//CAN发送数据

}


/***************************************************************************************
*函    数: void Motor_TY_CAN_RX_Handler(CanRxMsg* rxmessage)
*功    能:
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void Motor_TY_CAN_RX_Handler(CanRxMsg* rxmessage)
{
    switch(rxmessage->StdId)
    {
        case 0x180+LIFT_CANID_1://举升1
            Motor_Lift_Stru.comm_timer[0] = 0;//计数清零

            // 驱动器-电流
            Motor_Lift_Stru.current_data[0] = rxmessage->Data[0]+(u16)(rxmessage->Data[1]<<8);

            // 驱动器故障码
            Motor_Lift_Stru.err_data[0] = rxmessage->Data[2]+(u16)(rxmessage->Data[3]<<8);
            if(Motor_Lift_Stru.err_data[0] > 0)
            {
                if(PLC_TO_HUB_Mesg_Stru.err_id == 0)
                {
                    PLC_TO_HUB_Mesg_Stru.err_id = ERR_ID_MOTOR_LIFT1;
                    PLC_TO_HUB_Mesg_Stru.err_data = 0x08;
                }
            }

            //速度反馈--S32
            Motor_Lift_Stru.real_speed[0] = (s32)(rxmessage->Data[4]+(rxmessage->Data[5]<<8)+
                                                  (rxmessage->Data[6]<<16)+(rxmessage->Data[7]<<24));

            break;

        case 0x180+LIFT_CANID_2://举升2
            Motor_Lift_Stru.comm_timer[1] = 0;//计数清零

            // 驱动器-电流
            Motor_Lift_Stru.current_data[1] = rxmessage->Data[0]+(u16)(rxmessage->Data[1]<<8);

            // 驱动器故障码
            Motor_Lift_Stru.err_data[1] = rxmessage->Data[2]+(u16)(rxmessage->Data[3]<<8);
            if(Motor_Lift_Stru.err_data[1] > 0)
            {
                if(PLC_TO_HUB_Mesg_Stru.err_id == 0)
                {
                    PLC_TO_HUB_Mesg_Stru.err_id = ERR_ID_MOTOR_LIFT2;
                    PLC_TO_HUB_Mesg_Stru.err_data = 0x09;
                }
            }

            //速度反馈--S32
            Motor_Lift_Stru.real_speed[1] = (s32)(rxmessage->Data[4]+(rxmessage->Data[5]<<8)+
                                                  (rxmessage->Data[6]<<16)+(rxmessage->Data[7]<<24));

            break;

        default:
            break;

    }

}



