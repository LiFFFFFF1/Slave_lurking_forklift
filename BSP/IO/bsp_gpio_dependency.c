#include "bsp_gpio_dependency.h"

#include "plc_hub_upper.h"
#include "motor_upper.h"
#include "public_def_type.h"
#include "antico_knd_method.h"



/**
  * 函数功能: GPIO输入初始化
  * 输入参数：无
  * 返 回 值: 无
  * 说    明:
  */
static void Input_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    //开启 A-B-C-D-E 的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_GPIOC |
                           RCC_APB2Periph_GPIOD |
                           RCC_APB2Periph_GPIOE,ENABLE);

    /* PA5-6个输入*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //速度
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;          //端口配置为下拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;          //端口配置为上拉输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 //初始化端口

    /* PB0-1-2个输入*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //速度
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;          //端口配置为下拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;          //端口配置为上拉输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);                 //初始化端口

    /* PE7-8-9个输入*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //速度
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;          //端口配置为下拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;          //端口配置为上拉输入
    GPIO_Init(GPIOE, &GPIO_InitStructure);                 //初始化端口


    /* PB9-8  2个输入*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //速度
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;          //端口配置为下拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;          //端口配置为上拉输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);                 //初始化端口

    // 拨码输入IO--外部有上拉

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //速度
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;          //端口配置为下拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   //端口配置为浮空-输入
    GPIO_Init(GPIOC, &GPIO_InitStructure);                 //初始化端口

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //速度
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;          //端口配置为下拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //端口配置为浮空-输入
    GPIO_Init(GPIOD, &GPIO_InitStructure);                 //初始化端口

}

/**
  * 函数功能: GPIO输出初始化
  * 输入参数：无
  * 返 回 值: 无
  * 说    明:
  */
static void Output_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOE, ENABLE);          //使能GPIOX端口时钟

    /* 首先要打开GPIOA的AFIO时钟：用此语句也可使PA15作为普通IO */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);        //关掉JTAG，不关SW

    /* PE2-3-4-5-6-11-12 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 |
                                  GPIO_Pin_5 | GPIO_Pin_6 |GPIO_Pin_11 | GPIO_Pin_12;           //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //IO口速度为50MHz
    GPIO_Init(GPIOE, &GPIO_InitStructure);              //初始化PB端口
    GPIO_SetBits(GPIOE,GPIO_Pin_11);                    // 使能上电
    GPIO_ResetBits(GPIOE,GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 |
                   GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_12);


    /* PA0-3-4个  输出*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_4;//
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);              //初始化PD端口
    GPIO_ResetBits(GPIOA,GPIO_Pin_0 | GPIO_Pin_3);
    
    //MCU_GPIO_OUT1 =1;                                 // 休眠上电
    //MCU_GPIO_OUT2 =1;                                 // 驱动上总电x
    MCU_GPIO_OUT7=0;
}




/**
  * 函数功能: GPIO输入输出配置
  * 输入参数：无
  * 返 回 值: 无
  * 说    明:
  */
void Bsp_GPIO_Pin_Init(void)
{
    Input_Init();               // 输入 引脚初始化
    Output_Init();              // 输出 引脚初始化
}


//  报警IO扫描
// void Bsp_Read_AlarmIO(void)
// {
//     static u8 key_cnt[3]= {0};
//     // u8 ret_key_val=0;


//     // bit3--0-??????1-·???
//     if((PLC_TO_HUB_Mesg_Stru.set_custom_cmdbit &0x08)==0 && INPUT_ESTOP_SKIP)          // ????·???-- GPIO-I4---?¨???±??---??±?
//     {
//         if(key_cnt[2]<50)
//         {
//             key_cnt[2]++;
//         }
//         if(key_cnt[2] == 10)
//         {
//             key_cnt[2] =0;
//             PLC_TO_HUB_Mesg_Stru.upolad_general_alarm |=0X01;  // ?±??
//         }
//     }
//     else
//     {
//         key_cnt[2] =0;
//     }

//     if(INPUT_ESTOP)                                             // GPIO-I1---?±??---??±?
//     {
//         if(key_cnt[0]<50)
//         {
//             key_cnt[0]++;
//         }
//         if(key_cnt[0] == 10)
//         {
//             PLC_TO_HUB_Mesg_Stru.upolad_general_alarm |=0X01;  //?±??
//         }
//     }
//     else
//     {
//         key_cnt[0] =0;
//     }

//     if(INPUT_SAFE == 0)                                         // GPIO-I2-----------????---????
//     {
//         if(key_cnt[1]<50)
//         {
//             key_cnt[1]++;
//         }
//         if(key_cnt[1] == 10)
//         {
//             PLC_TO_HUB_Mesg_Stru.upolad_general_alarm |=0X02;   //????
//         }
//     }
//     else
//     {
//         key_cnt[1] =0;
//     }
//     if(PLC_TO_HUB_Mesg_Stru.set_custom_cmdbit &0x02)                                         //  bit1--??????±?
//     {
//         key_cnt[1] =0;
//     }

//     if(PLC_TO_HUB_Mesg_Stru.upolad_general_alarm && PLC_TO_HUB_Mesg_Stru.set_cmd == 0x02)   // ????????
//     {
//         /*if(INPUT_ESTOP == 0 && (PLC_TO_HUB_Mesg_Stru.upolad_general_alarm & 0x61))        // ?±??±¨??----??????·?
//         {
//             Motor_Walk_Stru.power_init_step =20;
//             Motor_Walk_Stru.agv_pid_val.outval =0;
//             Motor_Walk_Stru.agv_pid_val.previous_err =0;
//             Motor_Walk_Stru.agv_pid_val.last_err =0;

//             Motor_Walk_Stru.set_clear_alarm  =0x00;
//             Motor_Walk_Stru.com_timer[0] =0;
//             Motor_Walk_Stru.com_timer[1] =0;
//             PLC_TO_HUB_Mesg_Stru.upload_motor1_alarm =0;
//             PLC_TO_HUB_Mesg_Stru.upload_motor2_alarm =0;
//         }*/
//         PLC_TO_HUB_Mesg_Stru.upolad_general_alarm =0;
//         key_cnt[0] =0;
//         key_cnt[1] =0;

//     }

// }        


//IO状态更新
static void Get_IO_State(void)
{
    if(MCU_GPIO_IN1 == GET_INPUT_TURE)    setbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 0);
    else clrbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 0);
    if(MCU_GPIO_IN2 == GET_INPUT_TURE)    setbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 1);
    else clrbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 1);
    if(MCU_GPIO_IN3 == GET_INPUT_TURE)    setbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 2);
    else clrbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 2);
    if(MCU_GPIO_IN4 == GET_INPUT_TURE)    setbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 3);
    else clrbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 3);
    if(MCU_GPIO_IN5 == GET_INPUT_TURE)    setbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 4);
    else clrbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 4);
    if(MCU_GPIO_IN6 == GET_INPUT_TURE)    setbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 5);
    else clrbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 5);
    if(MCU_GPIO_IN7 == GET_INPUT_TURE)    setbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 6);
    else clrbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 6);
    if(MCU_GPIO_IN8 == GET_INPUT_TURE)    setbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 7);
    else clrbit(PLC_TO_HUB_Mesg_Stru.inputL_state, 7);
    if(MCU_GPIO_IN9 == GET_INPUT_TURE)    setbit(PLC_TO_HUB_Mesg_Stru.inputH_state, 0);
    else clrbit(PLC_TO_HUB_Mesg_Stru.inputH_state, 0);
    if(MCU_GPIO_IN10 == GET_INPUT_TURE)    setbit(PLC_TO_HUB_Mesg_Stru.inputH_state, 1);
    else clrbit(PLC_TO_HUB_Mesg_Stru.inputH_state, 1);

    if(MCU_GPIO_OUT1 == 1)    setbit(PLC_TO_HUB_Mesg_Stru.output_state, 0);
    else clrbit(PLC_TO_HUB_Mesg_Stru.output_state, 0);
    if(MCU_GPIO_OUT2 == 1)    setbit(PLC_TO_HUB_Mesg_Stru.output_state, 1);
    else clrbit(PLC_TO_HUB_Mesg_Stru.output_state, 1);
    if(MCU_GPIO_OUT3 == 1)    setbit(PLC_TO_HUB_Mesg_Stru.output_state, 2);
    else clrbit(PLC_TO_HUB_Mesg_Stru.output_state, 2);
    if(MCU_GPIO_OUT4 == 1)    setbit(PLC_TO_HUB_Mesg_Stru.output_state, 3);
    else clrbit(PLC_TO_HUB_Mesg_Stru.output_state, 3);
    if(MCU_GPIO_OUT5 == 1)    setbit(PLC_TO_HUB_Mesg_Stru.output_state, 4);
    else clrbit(PLC_TO_HUB_Mesg_Stru.output_state, 4);
    if(MCU_GPIO_OUT6 == 1)    setbit(PLC_TO_HUB_Mesg_Stru.output_state, 5);
    else clrbit(PLC_TO_HUB_Mesg_Stru.output_state, 5);
    if(MCU_GPIO_OUT7 == 1)    setbit(PLC_TO_HUB_Mesg_Stru.output_state, 6);
    else clrbit(PLC_TO_HUB_Mesg_Stru.output_state, 6);
    if(MCU_GPIO_OUT8 == 1)    setbit(PLC_TO_HUB_Mesg_Stru.output_state, 7);
    else clrbit(PLC_TO_HUB_Mesg_Stru.output_state, 7);


}



//三色灯控制
void Light_Ctrl(void)
{
    static u16 timer = 0;

    switch(PLC_TO_HUB_Mesg_Stru.light_cmd)
    {
        case EM_LIGHT_INIT:

            Output_Light_Red = 0;
            Output_Light_Yellow_L = 0;
            Output_Light_Yellow_R = 0;
            Output_Light_Green = 0;

            break;

        
        case EM_LIGHT_STOP:

            Output_Light_Red = 0;
            Output_Light_Yellow_L = 0;
            Output_Light_Yellow_R = 0;
            if(++timer > T_FLASH_SLOW)
            {
                timer = 0;
                Output_Light_Green = !Output_Light_Green;							  
            }

            break;

        case EM_LIGHT_ERROR:

            Output_Light_Red = 1;
            Output_Light_Yellow_L = 0;
            Output_Light_Yellow_R = 0;
            Output_Light_Green = 0;

            break;

        case EM_LIGHT_BZ_OFF:

            Output_Light_Red = 0;
            Output_Light_Green = 0;
            if(++timer > T_FLASH_FAST)
            {
                timer = 0;
                Output_Light_Yellow_L = !Output_Light_Yellow_L;
                Output_Light_Yellow_R = !Output_Light_Yellow_R;
            }

            break;

        case EM_LIGHT_BZ_NEAR:

            Output_Light_Red = 0;
            if(++timer > (T_FLASH_SLOW*2))    timer = 0;
            if(timer < T_FLASH_SLOW)
            {
                Output_Light_Green = 0;
                Output_Light_Yellow_L = 1;
                Output_Light_Yellow_R = 1;
            }
            else
            {
                Output_Light_Green = 1;
                Output_Light_Yellow_L = 0;
                Output_Light_Yellow_R = 0;
            }

            break;

        case EM_LIGHT_LOWBAT:

            Output_Light_Green = 0;
            Output_Light_Yellow_L = 0;
            Output_Light_Yellow_R = 0;
            if(++timer > T_FLASH_SLOW)
            {
                timer = 0;
                Output_Light_Red = !Output_Light_Red;
            }

            break;

        case EM_LIGHT_IPC_NOTE:

            Output_Light_Green = 0;
            if(++timer > (T_FLASH_FAST*2))    timer = 0;
            if(timer < T_FLASH_FAST)
            {
                Output_Light_Red = 0;
                Output_Light_Yellow_L = 1;
                Output_Light_Yellow_R = 1;
            }
            else
            {
                Output_Light_Red = 1;
                Output_Light_Yellow_L = 0;
                Output_Light_Yellow_R = 0;
            }

            break;




        case EM_LIGHT_HAND:

            Output_Light_Red = 0;
            Output_Light_Green = 0;
            if(++timer > T_FLASH_SLOW)
            {
                timer = 0;
                Output_Light_Yellow_L = !Output_Light_Yellow_L;
                Output_Light_Yellow_R = !Output_Light_Yellow_R;
            }

            break;

        case EM_LIGHT_TURN_L:

            Output_Light_Red = 0;
            Output_Light_Green = 0;
            Output_Light_Yellow_R = 0;
            if(++timer > T_FLASH_SLOW)
            {
                timer = 0;
                Output_Light_Yellow_L = !Output_Light_Yellow_L;
            }

            break;

        case EM_LIGHT_TURN_R:

            Output_Light_Red = 0;
            Output_Light_Green = 0;
            Output_Light_Yellow_L = 0;
            if(++timer > T_FLASH_SLOW)
            {
                timer = 0;
                Output_Light_Yellow_R = !Output_Light_Yellow_R;
            }

            break;

        case EM_LIGHT_RUN:

            Output_Light_Red = 0;
            Output_Light_Yellow_L = 0;
            Output_Light_Yellow_R = 0;
            Output_Light_Green = 1;

            break;

        case EM_LIGHT_CCW:

            Output_Light_Red = 0;
            Output_Light_Green = 0;
            Output_Light_Yellow_R = 0;
            if(++timer > T_FLASH_FAST)
            {
                timer = 0;
                Output_Light_Yellow_L = !Output_Light_Yellow_L;
            }

            break;

        case EM_LIGHT_CW:

            Output_Light_Red = 0;
            Output_Light_Green = 0;
            Output_Light_Yellow_L = 0;
            if(++timer > T_FLASH_FAST)
            {
                timer = 0;
                Output_Light_Yellow_R = !Output_Light_Yellow_R;
            }

            break;

        case EM_LIGHT_IPC_OFFLINE:

            Output_Light_Red = 0;
            Output_Light_Green = 0;
            if(++timer > T_FLASH_BRUST)//爆闪
            {
                timer = 0;
                Output_Light_Yellow_L = !Output_Light_Yellow_L;
                Output_Light_Yellow_R = !Output_Light_Yellow_R;
            }

            break;

        case EM_LIGHT_RCS_OFFLINE:

            Output_Light_Red = 0;
            Output_Light_Yellow_L = 0;
            Output_Light_Yellow_R = 0;
            if(++timer > T_FLASH_BRUST)//爆闪
            {
                timer = 0;
                Output_Light_Green = !Output_Light_Green;
            }

            break;


        case EM_LIGHT_IPC_1:

            Output_Light_Red = 1;
            Output_Light_Yellow_L = 0;
            Output_Light_Yellow_R = 0;
            Output_Light_Green = 0;

            break;

        case EM_LIGHT_IPC_2:

            Output_Light_Green = 0;
            Output_Light_Yellow_L = 0;
            Output_Light_Yellow_R = 0;
            if(++timer > T_FLASH_SLOW)
            {
                timer = 0;
                Output_Light_Red = !Output_Light_Red;
            }

            break;

        case EM_LIGHT_IPC_3:

            Output_Light_Green = 0;
            Output_Light_Yellow_L = 0;
            Output_Light_Yellow_R = 0;
            if(++timer > T_FLASH_FAST)
            {
                timer = 0;
                Output_Light_Red = !Output_Light_Red;
            }

            break;

        case EM_LIGHT_IPC_4:

            Output_Light_Red = 0;
            Output_Light_Yellow_L = 1;
            Output_Light_Yellow_R = 1;
            Output_Light_Green = 0;

            break;

        case EM_LIGHT_IPC_5:

            Output_Light_Red = 0;
            Output_Light_Green = 0;
            if(++timer > T_FLASH_SLOW)
            {
                timer = 0;
                Output_Light_Yellow_L = !Output_Light_Yellow_L;
                Output_Light_Yellow_R = !Output_Light_Yellow_R;
            }

            break;

        case EM_LIGHT_IPC_6:

            Output_Light_Red = 0;
            Output_Light_Green = 0;
            if(++timer > T_FLASH_FAST)
            {
                timer = 0;
                Output_Light_Yellow_L = !Output_Light_Yellow_L;
                Output_Light_Yellow_R = !Output_Light_Yellow_R;
            }

            break;

        case EM_LIGHT_IPC_7:

            Output_Light_Red = 0;
            Output_Light_Yellow_L = 0;
            Output_Light_Yellow_R = 0;
            Output_Light_Green = 1;

            break;

        case EM_LIGHT_IPC_8:

            Output_Light_Red = 0;
            Output_Light_Yellow_L = 0;
            Output_Light_Yellow_R = 0;
            if(++timer > T_FLASH_SLOW)
            {
                timer = 0;
                Output_Light_Green = !Output_Light_Green;
            }

            break;

        case EM_LIGHT_IPC_9:

            Output_Light_Red = 0;
            Output_Light_Yellow_L = 0;
            Output_Light_Yellow_R = 0;
            if(++timer > T_FLASH_FAST)
            {
                timer = 0;
                Output_Light_Green = !Output_Light_Green;
            }

            break;

        default:
            break;

    }

}


//控制指令
void Ctrl_Cmd_Deal(void)
{
    //static u8 iap_start_flag = 0;
    //static u8 iap_start_cnt = 0;
    //static u8 input_timer = 0;
    static u16 timer = 0;

    //关机
    if(getbit(PLC_TO_HUB_Mesg_Stru.ctrl_cmd, 1) == 1)    
    {
	if(timer < 1000)//*5ms
        {
            timer ++;
            Output_Ctrl_Shutdown = 1;
        }
        else Output_Ctrl_Shutdown = 0;
    }
    else 
    {
        Output_Ctrl_Shutdown = 0;
        timer = 0;//清零
    }


    //充电控制
    if(getbit(PLC_TO_HUB_Mesg_Stru.ctrl_cmd, 4) == 1)
    {
    	Output_Ctrl_Charge = 0;//开启充电
	PLC_TO_HUB_Mesg_Stru.charge_state = EM_CHARGING_ON;//充电中状态
    }
    else 
    {
        Output_Ctrl_Charge = 1;//关闭充电
	PLC_TO_HUB_Mesg_Stru.charge_state = EM_CHARGING_OFF;//充电停止状态
    }


}


// IO处理
void Bsp_IO_Deal_Task(void)
{
    //IO状态更新
    Get_IO_State();

    //三色灯控制
    Light_Ctrl();

    //指令控制
    Ctrl_Cmd_Deal();

}

























