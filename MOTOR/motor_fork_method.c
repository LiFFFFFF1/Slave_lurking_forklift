#include "motor_fork_method.h"

// 包含使用模块头文件
#include "motor_upper.h"
#include "plc_hub_upper.h"
#include "encoder_upper.h"
#include "antico_upper.h"

#include "public_def_type.h"
 

#define FORK_DEFAULT_MOTOR_RPM    3000.0f
#define FORK_DEFAULT_DRV_PPR      10000.0f
#define FORK_MASTER_HEART_ID      0x3F
#define FORK_MASTER_HEART_PERIOD  10      // 10ms task * 10 = 100ms
#define FORK_START_NODE_PERIOD    10      // 上电后每100ms启动一次NMT广播帧

#define FORK_SYNC_SPD_PER_MM      1500
#define FORK_SYNC_SPD_MAX         16666
#define FORK_SYNC_DEAD_DIFF_MM     1
#define FORK_SYNC_STOP_DIFF_MM     20
#define FORK_POS_DEAD_MM           1      // 精定位死区，<=1mm给0速并等待稳定
#define FORK_HAND_SPD_MAX_RPM     1000    // 手动速度模式限幅
#define FORK_AUTO_SPD_MAX_RPM     1500    // 位置模式安全最高速度
#define FORK_AUTO_SHORT_TRAVEL_MM 200     // 初始行程小于该值时，限制自动最高速度
#define FORK_AUTO_SHORT_MAX_RPM   1000
#define FORK_AUTO_DECEL_RANGE_MM  100     // 位置模式提前100mm开始减速
#define FORK_AUTO_MID_DECEL_MM    30      // 30mm处降到中段速度
#define FORK_AUTO_FINE_RANGE_MM   5       // 最后5mm低速贴近
#define FORK_AUTO_MID_RPM         300
#define FORK_AUTO_FINE_RPM        100
#define FORK_SPD_RAMP_TIME_MS     500     // 手动/自动共用爬坡时间
#define FORK_TASK_PERIOD_MS       10
#define FORK_POS_STABLE_CNT       50      // 10ms*50=0.5s，稳定后反馈到位
#define FORK_POWER_ON_DELAY_MAX    1500
#define FORK_SOFT_POS_MIN_MM       0
#define FORK_SOFT_POS_MAX_MM       1350
#define FORK_SOFT_SLOW_RANGE_MM    30
#define FORK_SOFT_SLOW_RPM         50
#define FORK_OP_STATUS_MASK       0x006F
#define FORK_OP_STATUS_VALUE      0x0027  // CiA402 Operation Enabled；0x1637按该掩码判断为使能
#define FORK_OP_DISABLE_CONFIRM_CNT 10    // 10ms*10=100ms，防止状态字瞬时抖动误触发

static u16 fork_power_on_delay =0;  // 叉臂独立上电延时，避免和举升共用计数
static u8 fork_power_on_delay_done =0;  // 只允许上电自检等待执行一次
static u8 fork_op_disable_cnt =0;
 
/*

配置 TPDO1    60410010+603F0010+606C0020
配置 RPDO1    60600008+60400010+60FF0020
*/
/*******************************************************************************
  * @brief   _fork_err_check(void)
  *
  * @param   null
  *
  * @retval  none 
  *
  * @note
  *****************************************************************************/
static u8 fork_is_operation_enabled(u16 status_word)
{
    return ((status_word & FORK_OP_STATUS_MASK) ==FORK_OP_STATUS_VALUE);
}

static u8 fork_all_operation_enabled(void)
{
    return (fork_is_operation_enabled(Motor_Lift_Stru.fork_word_status[0]) &&
            fork_is_operation_enabled(Motor_Lift_Stru.fork_word_status[1]));
}

static void _fork_err_check(void)
{
    u16 fork_can_over_time=0;
    
    if(Motor_Lift_Stru.can_run_step ==5)  // 
    {
        fork_can_over_time =200;
    }
    else
    {
        fork_can_over_time =400;
    }
    
    
    if(++Motor_Lift_Stru.fork_can_time[0]>=fork_can_over_time)   // 10ms*
    {
        Motor_Lift_Stru.fork_can_time[0] =0;
        Motor_Lift_Stru.unit_err_code[0] =0xFFFF;
    }
    
    if(++Motor_Lift_Stru.fork_can_time[1]>=fork_can_over_time)   // 10ms*
    {
        Motor_Lift_Stru.fork_can_time[1] =0;
        Motor_Lift_Stru.unit_err_code[1] =0xFFFF;
    }
    

    if(getbit(PLC_TO_HUB_Mesg_Stru.ctrl_cmd, 0) &&
      (Motor_Lift_Stru.unit_err_code[0] || Motor_Lift_Stru.unit_err_code[1] ||
       (Motor_Lift_Stru.can_run_step ==5 && fork_all_operation_enabled() ==0)))
    {
        if(Motor_Lift_Stru.unit_err_code[0]==0xFFFF || Motor_Lift_Stru.unit_err_code[1]==0xFFFF)
        {
            Motor_Lift_Stru.can_run_step =0;   
            fork_power_on_delay =FORK_POWER_ON_DELAY_MAX;
            fork_power_on_delay_done =1;        // 复位后不再重新等待5s
            Motor_Lift_Stru.heart_node_cnt =0;
        }
        else
        {
            Motor_Lift_Stru.can_run_step =1;
        }
        Motor_Lift_Stru.can_send_cnt =0;
        Motor_Lift_Stru.unit_err_code[0] =0;
        Motor_Lift_Stru.unit_err_code[1] =0;
        Motor_Lift_Stru.fork_can_time[0] =0;
        Motor_Lift_Stru.fork_can_time[1] =0;
        fork_op_disable_cnt =0;
    }
    
    
}

/*******************************************************************************
  * @brief   fork_limit_s32
  *
  * @param   val,min_val,max_val
  *
  * @retval  none
  *
  * @note    速度限幅
  *****************************************************************************/
static s32 fork_limit_s32(s32 val,s32 min_val,s32 max_val)
{
    if(val > max_val)
    {
        return max_val;
    }
    else if(val < min_val)
    {
        return min_val;
    }

    return val;
}

static float fork_valid_float(float val,float default_val)
{
    if(val <=0.0f)
    {
        return default_val;
    }

    return val;
}

static s32 fork_float_to_s32(float val)
{
    if(val >=0.0f)
    {
        return (s32)(val+0.5f);
    }

    return (s32)(val-0.5f);
}

static s32 fork_get_max_motor_rpm_s32(void)
{
    float motor_rpm =fork_valid_float(Motor_Lift_Stru.fork_motor_max_rpm,FORK_DEFAULT_MOTOR_RPM);

    return fork_float_to_s32(motor_rpm);
}

static s32 fork_motor_rpm_to_drv_s32(s32 motor_rpm)
{
    float rpm =motor_rpm;
    float max_rpm =fork_valid_float(Motor_Lift_Stru.fork_motor_max_rpm,FORK_DEFAULT_MOTOR_RPM);
    float drv_ppr =fork_valid_float(Motor_Lift_Stru.fork_drv_pulse_per_rev,FORK_DEFAULT_DRV_PPR);
    float drv_speed =0.0f;

    if(rpm > max_rpm)
    {
        rpm =max_rpm;
    }
    else if(rpm < -max_rpm)
    {
        rpm =-max_rpm;
    }

    // motor rpm -> drive velocity(count/s)
    drv_speed =rpm*drv_ppr/60.0f;
    return (s32)drv_speed;
}

static s16 fork_drv_speed_to_motor_rpm_s16(s32 drv_speed)
{
    float drv_ppr =fork_valid_float(Motor_Lift_Stru.fork_drv_pulse_per_rev,FORK_DEFAULT_DRV_PPR);
    float rpm =0.0f;
    s32 rpm_s32 =0;

    rpm =drv_speed*60.0f/drv_ppr;
    rpm_s32 =fork_float_to_s32(rpm);
    rpm_s32 =fork_limit_s32(rpm_s32,-32768,32767);

    return (s16)rpm_s32;
}

static s32 fork_get_drv_speed_limit(void)
{
    return fork_motor_rpm_to_drv_s32(fork_get_max_motor_rpm_s32());
}

static s32 fork_get_drv_speed_step_by_rpm(s32 rpm)
{
    s32 target_drv_speed =fork_motor_rpm_to_drv_s32(rpm);
    s32 step =target_drv_speed/(FORK_SPD_RAMP_TIME_MS/FORK_TASK_PERIOD_MS);

    if(step <0)
    {
        step =-step;
    }
    if(step <1)
    {
        step =1;
    }

    return step;
}

static s32 fork_linear_rpm_by_dist(u32 cur_mm,u32 low_mm,u32 high_mm,s32 low_rpm,s32 high_rpm)
{
    s32 plan_rpm =low_rpm;

    if(cur_mm <=low_mm || high_mm <=low_mm)
    {
        return low_rpm;
    }
    if(cur_mm >=high_mm)
    {
        return high_rpm;
    }

    plan_rpm =low_rpm +(s32)((high_rpm-low_rpm)*(s32)(cur_mm-low_mm)/(s32)(high_mm-low_mm));

    return fork_limit_s32(plan_rpm,low_rpm,high_rpm);
}

static s32 fork_auto_pos_plan_rpm(u32 abs_err_mm,s32 max_rpm)
{
    s32 plan_rpm =0;
    s32 mid_rpm =fork_limit_s32(FORK_AUTO_MID_RPM,FORK_AUTO_FINE_RPM,max_rpm);
    s32 fine_rpm =fork_limit_s32(FORK_AUTO_FINE_RPM,0,mid_rpm);

    if(abs_err_mm >FORK_AUTO_DECEL_RANGE_MM)
    {
        plan_rpm =max_rpm;
    }
    else if(abs_err_mm >FORK_AUTO_MID_DECEL_MM)
    {
        plan_rpm =fork_linear_rpm_by_dist(abs_err_mm,
                                          FORK_AUTO_MID_DECEL_MM,
                                          FORK_AUTO_DECEL_RANGE_MM,
                                          mid_rpm,
                                          max_rpm);
    }
    else if(abs_err_mm >FORK_AUTO_FINE_RANGE_MM)
    {
        plan_rpm =fork_linear_rpm_by_dist(abs_err_mm,
                                          FORK_AUTO_FINE_RANGE_MM,
                                          FORK_AUTO_MID_DECEL_MM,
                                          fine_rpm,
                                          mid_rpm);
    }
    else if(abs_err_mm >FORK_POS_DEAD_MM)
    {
        plan_rpm =fine_rpm;
    }
    else
    {
        plan_rpm =0;
    }

    return fork_limit_s32(plan_rpm,0,max_rpm);
}

static s32 fork_get_pos_remain_by_dir(s32 target_pos,s32 cur_pos,s32 move_dir)
{
    s32 remain_mm =0;

    if(move_dir >0)
    {
        remain_mm =target_pos-cur_pos;
    }
    else if(move_dir <0)
    {
        remain_mm =cur_pos-target_pos;
    }

    if(remain_mm <0)
    {
        remain_mm =0;
    }

    return remain_mm;
}

static void fork_apply_target_limit_speed(s32 target_pos,s32 move_dir,s32 fork_dst_speed[2])
{
    if(move_dir >0)
    {
        if(Encoder_Mesg_Stru.real_data[0] >=target_pos-FORK_POS_DEAD_MM && fork_dst_speed[0] >0)
        {
            fork_dst_speed[0] =0;
        }
        if(Encoder_Mesg_Stru.real_data[1] >=target_pos-FORK_POS_DEAD_MM && fork_dst_speed[1] >0)
        {
            fork_dst_speed[1] =0;
        }
    }
    else if(move_dir <0)
    {
        if(Encoder_Mesg_Stru.real_data[0] <=target_pos+FORK_POS_DEAD_MM && fork_dst_speed[0] <0)
        {
            fork_dst_speed[0] =0;
        }
        if(Encoder_Mesg_Stru.real_data[1] <=target_pos+FORK_POS_DEAD_MM && fork_dst_speed[1] <0)
        {
            fork_dst_speed[1] =0;
        }
    }
}

/*******************************************************************************
  * @brief   fork_sync_speed
  *
  * @param   base_spd,fork_dst_speed
  *
  * @retval  none
  *
  * @note    叉臂同步补偿，手动/自动模式共用
  *****************************************************************************/
static void fork_sync_speed(s32 base_spd,s32 fork_dst_speed[2])
{
    s32 fork_diff =0;
    s32 abs_fork_diff =0;
    s32 sync_spd =0;
    s32 drv_speed_limit =fork_get_drv_speed_limit();

    fork_dst_speed[0] =base_spd;
    fork_dst_speed[1] =base_spd;

    if(base_spd ==0)
    {
        return;
    }

    fork_diff =Encoder_Mesg_Stru.real_data[0]-Encoder_Mesg_Stru.real_data[1];
    abs_fork_diff =fork_diff;
    if(abs_fork_diff <0)
    {
        abs_fork_diff =-abs_fork_diff;
    }

    if(abs_fork_diff <=FORK_SYNC_DEAD_DIFF_MM)
    {
        return;
    }

    sync_spd =abs_fork_diff*FORK_SYNC_SPD_PER_MM;
    sync_spd =fork_limit_s32(sync_spd,0,FORK_SYNC_SPD_MAX);

    if(base_spd >0)
    {
        if(fork_diff >0)    // 左臂比右臂伸出多
        {
            if(abs_fork_diff >=FORK_SYNC_STOP_DIFF_MM)
            {
                fork_dst_speed[0] =0;
                fork_dst_speed[1] =sync_spd;
            }
            else
            {
                fork_dst_speed[0] =base_spd;
                fork_dst_speed[1] =base_spd+sync_spd;
            }
        }
        else                // 右臂比左臂伸出多
        {
            if(abs_fork_diff >=FORK_SYNC_STOP_DIFF_MM)
            {
                fork_dst_speed[0] =sync_spd;
                fork_dst_speed[1] =0;
            }
            else
            {
                fork_dst_speed[0] =base_spd;
                fork_dst_speed[1] =base_spd-sync_spd;
                if(fork_dst_speed[1] <0)
                {
                    fork_dst_speed[1] =0;
                }
            }
        }
    }
    else
    {
        if(fork_diff >0)    // 左臂比右臂伸出多
        {
            if(abs_fork_diff >=FORK_SYNC_STOP_DIFF_MM)
            {
                fork_dst_speed[0] =-sync_spd;
                fork_dst_speed[1] =0;
            }
            else
            {
                fork_dst_speed[0] =base_spd;
                fork_dst_speed[1] =base_spd+sync_spd;
                if(fork_dst_speed[1] >0)
                {
                    fork_dst_speed[1] =0;
                }
            }
        }
        else                // 右臂比左臂伸出多
        {
            if(abs_fork_diff >=FORK_SYNC_STOP_DIFF_MM)
            {
                fork_dst_speed[0] =0;
                fork_dst_speed[1] =-sync_spd;
            }
            else
            {
                fork_dst_speed[0] =base_spd;
                fork_dst_speed[1] =base_spd-sync_spd;
            }
        }
    }

    fork_dst_speed[0] =fork_limit_s32(fork_dst_speed[0],-drv_speed_limit,drv_speed_limit);
    fork_dst_speed[1] =fork_limit_s32(fork_dst_speed[1],-drv_speed_limit,drv_speed_limit);
}

static void fork_apply_soft_limit_speed(s32 fork_dst_speed[2])
{
    s32 slow_drv_speed =fork_motor_rpm_to_drv_s32(FORK_SOFT_SLOW_RPM);
    s32 left_pos =Encoder_Mesg_Stru.real_data[0];
    s32 right_pos =Encoder_Mesg_Stru.real_data[1];
    s32 move_dir =0;

    if(fork_dst_speed[0] >0 || fork_dst_speed[1] >0)
    {
        move_dir =1;
    }
    else if(fork_dst_speed[0] <0 || fork_dst_speed[1] <0)
    {
        move_dir =-1;
    }

    if(move_dir >0)
    {
        if(left_pos >=FORK_SOFT_POS_MAX_MM || right_pos >=FORK_SOFT_POS_MAX_MM)
        {
            fork_dst_speed[0] =0;
            fork_dst_speed[1] =0;
            return;
        }

        if(left_pos >=(FORK_SOFT_POS_MAX_MM -FORK_SOFT_SLOW_RANGE_MM) ||
           right_pos >=(FORK_SOFT_POS_MAX_MM -FORK_SOFT_SLOW_RANGE_MM))
        {
            fork_dst_speed[0] =fork_limit_s32(fork_dst_speed[0],0,slow_drv_speed);
            fork_dst_speed[1] =fork_limit_s32(fork_dst_speed[1],0,slow_drv_speed);
        }
    }
    else if(move_dir <0)
    {
        if(left_pos <=FORK_SOFT_POS_MIN_MM || right_pos <=FORK_SOFT_POS_MIN_MM)
        {
            fork_dst_speed[0] =0;
            fork_dst_speed[1] =0;
            return;
        }

        if(left_pos <=(FORK_SOFT_POS_MIN_MM +FORK_SOFT_SLOW_RANGE_MM) ||
           right_pos <=(FORK_SOFT_POS_MIN_MM +FORK_SOFT_SLOW_RANGE_MM))
        {
            fork_dst_speed[0] =fork_limit_s32(fork_dst_speed[0],-slow_drv_speed,0);
            fork_dst_speed[1] =fork_limit_s32(fork_dst_speed[1],-slow_drv_speed,0);
        }
    }
}

/*******************************************************************************
  * @brief   _fork_canopen_senddata(u8 id, u8 mode, u16 ctl_word, s32 send_val)
  *
  * @param   null
  *
  * @retval  none
  *
  * @note
  *****************************************************************************/
static void _fork_canopen_senddata(u8 id, u8 mode, u16 ctl_word, s32 send_val)
{
    u8 can_buf[8]= {0};

    can_buf[0] = mode;           // 模式：3-PV速度，1-PP位置

    can_buf[1] = ctl_word;       //
    can_buf[2] = ctl_word>>8;    // 控制字

    can_buf[3] = send_val;           // speed_vx>0前进，speed_vx<0后退
    can_buf[4] = send_val>>8;        // 设置转速/位置值
    can_buf[5] = send_val>>16;
    can_buf[6] = send_val>>24;

    CAN1_Send_One_Frame_Data(0x200+id, can_buf, 7);    // 驱动器采用低字节在前

}

/*******************************************************************************
  * @brief   _fork_action_task
  *
  * @param   null
  *
  * @retval  none
  *
  * @note
  *****************************************************************************/
static void _fork_action_task(void)
{
    static s32 fork_base_spd=0;
    static u8 fork_auto_target_valid=0;
    static s32 fork_auto_last_target_pos=0;
    static s32 fork_auto_task_max_rpm=FORK_AUTO_SPD_MAX_RPM;
    static s32 fork_auto_move_dir=0;
    u32 u32_temp_val=0;
    s32 fork_pos_remain[2]={0};
    s32 s32_temp_val=0;
    s32 s32_spd_step=0;
    s32 fork_pos_err[2]={0};
    s32 abs_fork_pos_err[2]={0};
    s32 fork_sync_dst_speed[2]={0};
    u8 fork_pos_main_idx=0;
    
    if((PLC_TO_HUB_Mesg_Stru.inputL_state&0x03) !=0x03 ||                       // 左右插臂触边，常闭
       (ANTICO_Mesg_Stru.err_state | Encoder_Mesg_Stru.err_state) ||            // 常规故障
       Motor_Lift_Stru.unit_err_code[0] || Motor_Lift_Stru.unit_err_code[1]     // 插臂故障
      )
    {
        fork_base_spd =0;
        fork_auto_target_valid =0;
        fork_auto_move_dir =0;
        Motor_Lift_Stru.fork_dst_spd[0] =0;
        Motor_Lift_Stru.fork_dst_spd[1] =0;
        Motor_Lift_Stru.unit_act_state =0;          // 0-无任务，1-执行中，2-到位
        Motor_Lift_Stru.fork_location_cnt =0;
        return;
    }

    // 插臂动作与同步控制
    if(Motor_Lift_Stru.unit_type==1 && Motor_Lift_Stru.unit_act_id==0) 
    {
        if(Motor_Lift_Stru.unit_ctrl_mode==1)       // 速度模式
        {
            fork_auto_target_valid =0;
            s32_temp_val =fork_limit_s32(Motor_Lift_Stru.unit_ctrl_data,-FORK_HAND_SPD_MAX_RPM,FORK_HAND_SPD_MAX_RPM);
            s32_temp_val =fork_motor_rpm_to_drv_s32(s32_temp_val);
            s32_spd_step =fork_get_drv_speed_step_by_rpm(FORK_HAND_SPD_MAX_RPM);
            
            if(s32_temp_val ==0)
            {
                fork_base_spd =0;
            }
            else if(fork_base_spd < s32_temp_val)
            {
                fork_base_spd +=s32_spd_step;
                if(fork_base_spd > s32_temp_val)
                {
                    fork_base_spd =s32_temp_val;
                }
            }
            else if(fork_base_spd > s32_temp_val)
            {
                fork_base_spd -=s32_spd_step;
                if(fork_base_spd < s32_temp_val)
                {
                    fork_base_spd =s32_temp_val;
                }
            }
            fork_sync_speed(fork_base_spd,fork_sync_dst_speed);
            fork_apply_soft_limit_speed(fork_sync_dst_speed);
            Motor_Lift_Stru.fork_dst_spd[0] =fork_sync_dst_speed[0];
            Motor_Lift_Stru.fork_dst_spd[1] =fork_sync_dst_speed[1];
            
            Motor_Lift_Stru.unit_act_state =1;      // 1-执行中，2-到位
            Motor_Lift_Stru.fork_location_cnt =0;
        }
        else if(Motor_Lift_Stru.unit_ctrl_mode==2)  // 位置模式
        {
            s32 s32_fork_target_pos =fork_limit_s32((s32)Motor_Lift_Stru.unit_ctrl_data,
                                                    FORK_SOFT_POS_MIN_MM,
                                                    FORK_SOFT_POS_MAX_MM);

            fork_pos_err[0] =s32_fork_target_pos-Encoder_Mesg_Stru.real_data[0];
            fork_pos_err[1] =s32_fork_target_pos-Encoder_Mesg_Stru.real_data[1];

            abs_fork_pos_err[0] =fork_pos_err[0];
            abs_fork_pos_err[1] =fork_pos_err[1];
            if(abs_fork_pos_err[0] <0)
            {
                abs_fork_pos_err[0] =-abs_fork_pos_err[0];
            }
            if(abs_fork_pos_err[1] <0)
            {
                abs_fork_pos_err[1] =-abs_fork_pos_err[1];
            }

            if(fork_auto_target_valid ==0 || fork_auto_last_target_pos !=s32_fork_target_pos)
            {
                fork_pos_main_idx =(abs_fork_pos_err[1] > abs_fork_pos_err[0]) ? 1 : 0;
                u32_temp_val =abs_fork_pos_err[fork_pos_main_idx];
                if(fork_pos_err[fork_pos_main_idx] >0)
                {
                    fork_auto_move_dir =1;
                }
                else if(fork_pos_err[fork_pos_main_idx] <0)
                {
                    fork_auto_move_dir =-1;
                }
                else
                {
                    fork_auto_move_dir =0;
                }

                if(u32_temp_val <FORK_AUTO_SHORT_TRAVEL_MM)
                {
                    fork_auto_task_max_rpm =FORK_AUTO_SHORT_MAX_RPM;
                }
                else
                {
                    fork_auto_task_max_rpm =FORK_AUTO_SPD_MAX_RPM;
                }

                fork_auto_last_target_pos =s32_fork_target_pos;
                fork_auto_target_valid =1;
            }

            //--------------------基础速度--------------------------
            fork_pos_remain[0] =fork_get_pos_remain_by_dir(s32_fork_target_pos,
                                                            Encoder_Mesg_Stru.real_data[0],
                                                            fork_auto_move_dir);
            fork_pos_remain[1] =fork_get_pos_remain_by_dir(s32_fork_target_pos,
                                                            Encoder_Mesg_Stru.real_data[1],
                                                            fork_auto_move_dir);
            fork_pos_main_idx =(fork_pos_remain[1] > fork_pos_remain[0]) ? 1 : 0;
            u32_temp_val =(u32)fork_pos_remain[fork_pos_main_idx];
            if(u32_temp_val > FORK_POS_DEAD_MM && fork_auto_move_dir !=0)
            {
                s32_temp_val =fork_motor_rpm_to_drv_s32(fork_auto_pos_plan_rpm(u32_temp_val,
                                                                               fork_auto_task_max_rpm));
                if(fork_auto_move_dir <0)
                {
                    s32_temp_val =-s32_temp_val;
                }

                s32_spd_step =fork_get_drv_speed_step_by_rpm(fork_auto_task_max_rpm);

                if(fork_base_spd < s32_temp_val)
                {
                    fork_base_spd +=s32_spd_step;
                    if(fork_base_spd > s32_temp_val)
                    {
                        fork_base_spd =s32_temp_val;
                    }
                }
                else if(fork_base_spd > s32_temp_val)
                {
                    fork_base_spd -=s32_spd_step;
                    if(fork_base_spd < s32_temp_val)
                    {
                        fork_base_spd =s32_temp_val;
                    }
                }
                
                Motor_Lift_Stru.fork_location_cnt =0;
            }
            else
            {
                fork_base_spd =0;
            }
            //--------------------基础速度 end----------------------

            fork_sync_speed(fork_base_spd,fork_sync_dst_speed);
            fork_apply_target_limit_speed(s32_fork_target_pos,
                                          fork_auto_move_dir,
                                          fork_sync_dst_speed);
            fork_apply_soft_limit_speed(fork_sync_dst_speed);
            Motor_Lift_Stru.fork_dst_spd[0] =fork_sync_dst_speed[0];
            Motor_Lift_Stru.fork_dst_spd[1] =fork_sync_dst_speed[1];

            if(Motor_Lift_Stru.fork_dst_spd[0] !=0
               ||Motor_Lift_Stru.fork_dst_spd[1] !=0)
            {
                Motor_Lift_Stru.fork_location_cnt =0;
            }

            if(Motor_Lift_Stru.fork_location_cnt < FORK_POS_STABLE_CNT)
            {
                Motor_Lift_Stru.fork_location_cnt++;
                Motor_Lift_Stru.unit_act_state =1;   // 1-执行中，2-到位
            }
            else
            {
                Motor_Lift_Stru.unit_act_state =2;   // 1-执行中，2-到位
            }
            
        }
        else
        {
            fork_base_spd =0;
            fork_auto_target_valid =0;
            fork_auto_move_dir =0;
            Motor_Lift_Stru.fork_dst_spd[0] =0;
            Motor_Lift_Stru.fork_dst_spd[1] =0;
            Motor_Lift_Stru.unit_act_state =0;          // 0-无任务，1-执行中，2-到位
            Motor_Lift_Stru.fork_location_cnt =0;
        }
    }
    else
    {
        fork_base_spd =0;
        fork_auto_target_valid =0;
        fork_auto_move_dir =0;
        Motor_Lift_Stru.fork_dst_spd[0] =0;
        Motor_Lift_Stru.fork_dst_spd[1] =0;
    }
}

/*******************************************************************************
  * @brief   _fork_can_send_step(void)
  *
  * @param   null
  *
  * @retval  none
  *
  * @note 叉齿CAN发送数据
  *****************************************************************************/
static void _fork_can_send_step(void)
{
   
    u8 u8_temp_val=0;

    if(++Motor_Lift_Stru.heart_node_cnt>=10)
    {
        Motor_Lift_Stru.heart_node_cnt =0;
        CAN1_Node_Hearbeat(0x3F);
    }
    
    switch(Motor_Lift_Stru.can_run_step)
    {
        case 0:
            
            Motor_Lift_Stru.can_send_cnt++;            
            if(Motor_Lift_Stru.can_send_cnt == 1 || (Motor_Lift_Stru.can_send_cnt%50 ==0))
            {
                CAN1_Start_Node(0x01,0x00);
            }
            
            if((Motor_Lift_Stru.fork_word_status[0] &0x200) && (Motor_Lift_Stru.fork_word_status[1] &0x200))
            {
               Motor_Lift_Stru.can_run_step++;
            }
            break;
        case 1:
            _fork_canopen_senddata(1,0x03,0x86,0);
            _fork_canopen_senddata(2,0x03,0x86,0);
            Motor_Lift_Stru.can_run_step++;
            Motor_Lift_Stru.can_send_cnt =0;
            break;
        case 2:
            _fork_canopen_senddata(1,0x03,0x06,0);
            _fork_canopen_senddata(2,0x03,0x06,0);
            Motor_Lift_Stru.can_run_step++;
            break;
        case 3:
            _fork_canopen_senddata(1,0x03,0x07,0);
            _fork_canopen_senddata(2,0x03,0x07,0);
            Motor_Lift_Stru.can_run_step++;
            break;
        case 4:
            _fork_canopen_senddata(1,0x03,0x0F,0);
            _fork_canopen_senddata(2,0x03,0x0F,0);
            Motor_Lift_Stru.can_run_step++;
            Motor_Lift_Stru.heart_node_cnt =0;
            break;
        case 5:
            if(fork_all_operation_enabled() ==0)
            {
                if(++fork_op_disable_cnt >=FORK_OP_DISABLE_CONFIRM_CNT)
                {
                    Motor_Lift_Stru.fork_dst_spd[0] =0;
                    Motor_Lift_Stru.fork_dst_spd[1] =0;
                    Motor_Lift_Stru.can_send_cnt =0;
                    Motor_Lift_Stru.can_run_step =1;   // PDO仍在线但掉使能时，重新下发使能序列
                    fork_op_disable_cnt =0;
                }
                else
                {
                    _fork_canopen_senddata(1,0x03,0x0F,0);
                    _fork_canopen_senddata(2,0x03,0x0F,0);
                }
                break;
            }
            fork_op_disable_cnt =0;
            _fork_canopen_senddata(1,0x03,0x0F,Motor_Lift_Stru.fork_dst_spd[0]);
            _fork_canopen_senddata(2,0x03,0x0F,Motor_Lift_Stru.fork_dst_spd[1]);
            break;
            
        case 20:        // 复位节点
            
            if(Motor_Lift_Stru.can_send_cnt<200)
            {
                u8_temp_val =10;
            }
            else 
            {
                u8_temp_val =100;
            }
            
            if((Motor_Lift_Stru.can_send_cnt%u8_temp_val) ==0)
            {
                CAN1_Start_Node(0x81,0x00);
            }
            
            Motor_Lift_Stru.fork_can_time[0] =0;
            Motor_Lift_Stru.fork_can_time[1] =0;
            if(++Motor_Lift_Stru.can_send_cnt>=400)
            {
               Motor_Lift_Stru.can_send_cnt =0;
               Motor_Lift_Stru.can_run_step =0;
            }
            break;
        
    }

}



/*******************************************************************************
  * @brief   Fork_Task_Deal(void)
  *
  * @param   null
  *
  * @retval  none
  *
  * @note
  *****************************************************************************/
void Fork_Task_Deal(void)
{
    // 上报给主站的反馈参数使用左右叉当前位置，单位按当前控制逻辑保持为 mm。
    Motor_Lift_Stru.unit_act_data[0] = (s16)Encoder_Mesg_Stru.real_data[0];
    Motor_Lift_Stru.unit_act_data[1] = (s16)Encoder_Mesg_Stru.real_data[1];

    // 仅上电后等待一次驱动器自检，后续急停/复位/CAN重启不再进入5s等待。
    if(fork_power_on_delay_done ==0)
    {
        if(fork_power_on_delay < FORK_POWER_ON_DELAY_MAX)    // 1500 * 10ms = 15s 
        {
            fork_power_on_delay++;
            _fork_can_send_step();
            return;
        }

        fork_power_on_delay =FORK_POWER_ON_DELAY_MAX;
        fork_power_on_delay_done =1;
    }

    _fork_err_check();      // 异常检测
    _fork_action_task();    // 插臂动作执行任务
    _fork_can_send_step();  // CAN发送数据
}


/*******************************************************************************
  * @brief   Motor_WC_CAN_RX_Handler(CanRxMsg* rxmessage)
  *
  * @param   rxmessage
  *
  * @retval  none
  *
  * @note    纬创伸缩驱动器CAN接收处理
  *****************************************************************************/
void Motor_WC_CAN_RX_Handler(CanRxMsg* rxmessage)
{
    s32 s32_temp_val=0;
    u16 u16_temp_val=0;
    u8 code_id=0;
    
    switch(rxmessage->StdId)
    {
        case 0x180+FORK_CANID:  // 叉齿1
        case 0x180+FORK_CANID+1:// 叉齿2
            code_id =rxmessage->StdId-(0x180+FORK_CANID);
        
            Motor_Lift_Stru.fork_can_time[code_id] =0;  // 通信计数
            // 状态字-0x6041
            Motor_Lift_Stru.fork_word_status[code_id] =rxmessage->Data[0]+(u16)(rxmessage->Data[1]<<8);
            
            
            // 故障码
            u16_temp_val = (u16)(rxmessage->Data[3]<<8)+rxmessage->Data[2];
            Motor_Lift_Stru.unit_err_code[code_id] =u16_temp_val;
            // 606C-速度反馈，单位 count/s
            s32_temp_val =(s32)(rxmessage->Data[7]<<24)+(s32)(rxmessage->Data[6]<<16)+ \
                          (s32)(rxmessage->Data[5]<<8)+rxmessage->Data[4];
            Motor_Lift_Stru.fork_real_speed[code_id] =fork_drv_speed_to_motor_rpm_s16(s32_temp_val);    // 单位 rpm
            break;

        default:
            break;

    }

}
