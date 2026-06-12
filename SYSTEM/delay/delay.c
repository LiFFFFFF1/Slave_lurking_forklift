#define DELAY_GLOBALS
#include "delay.h"

#define USR_CONFIG_TICK_HZ      (1000)

static u8  fac_us=0;                            //us延时倍乘数

/**
  * 函数功能: 初始化延迟函数
  * SYSCLK:系统时钟
  * 返 回 值: 无
  * 说    明:
  */
void Bsp_Delay_Init(unsigned int sys_Clock)
{

    u32 reload =0;

    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);   //选择外部时钟  HCLK/8
    fac_us=sys_Clock/8000000;                               //为系统时钟的1/8
    reload =SystemCoreClock/8000000;                        //每秒钟的计数次数 单位为M
    reload *=(1000000/USR_CONFIG_TICK_HZ);                  //根据configTICK_RATE_HZ设定溢出时间
    //reload为24位寄存器,最大值:16777216,在72M下,约合0.233s左右
    // fac_ms=1000/USR_CONFIG_TICK_HZ;                           //非OS下,代表每个ms需要的systick时钟数

    SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;                //开启SYSTICK中断
    SysTick->LOAD=reload;                                   //每1/configTICK_RATE_HZ秒中断一次
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;                 //开启SYSTICK
}


/**
  * 函数功能: 延时nus
  * nus为要延时的us数.
  * 返 回 值: 无
  * 说    明:
  */
void Bsp_Delay_US(u32 nus)
{
    u32 ticks;
    u32 told,tnow,tcnt=0;
    u32 reload=SysTick->LOAD;               //LOAD的值
    ticks=nus*fac_us;                       //需要的节拍数
    told=SysTick->VAL;                      //刚进入时的计数器值
    while(1)
    {
        tnow=SysTick->VAL;
        if(tnow!=told)
        {
            if(tnow<told)tcnt+=told-tnow;   //这里注意一下SYSTICK是一个递减的计数器就可以了.
            else tcnt+=reload-tnow+told;
            told=tnow;
            if(tcnt>=ticks)break;           //时间超过/等于要延迟的时间,则退出.
        }
    };
}


/**
  * 函数功能: 延时nms
  * nms为要延时的ms数.
  * 返 回 值: 无
  * 说    明:
    注意nms的范围
    SysTick->LOAD为24位寄存器,所以,最大延时为:
    nms<=0xffffff*8*1000/SYSCLK
    SYSCLK单位为Hz,nms单位为ms
    对72M条件下,nms<=1864
  */
void Bsp_Delay_MS(u16 nms)
{
    Bsp_Delay_US((u32)(nms*1000));              //普通方式延时
}

/**
  * 函数功能: 滴答时钟中断-1ms
  *
  * 返 回 值: 无
  * 说    明:
  */
void SysTick_Handler()
{
    static u16 time_all_cnt=0;
    time_all_cnt++;

    Tick_Time_Stru.rx_deal_ms_bit =1;               // 处理接收

    if((time_all_cnt%5) == 0)
    {
        Tick_Time_Stru.io_deal_ms_bit=1;       //  IO处理时间间隔
    }    
    
    if((time_all_cnt%10) == 0)
    {
        Tick_Time_Stru.motor_send_time_bit =1;      //  发送行走电机周期
    }

    if((time_all_cnt%15) == 0)
    {
        Tick_Time_Stru.magnet_send_time_bit =1;     //  磁导航发送查询时间间隔
    }
    
    if((time_all_cnt%20) == 0)
    {
        Tick_Time_Stru.time_20ms_bit = 1;     //
    }
    
    if((time_all_cnt%30) == 0)
    {
        Tick_Time_Stru.plc_to_hub_ask_bit =1;     //  磁导航发送查询时间间隔
    }
    
    if((time_all_cnt%50) == 0)
    {
        Tick_Time_Stru.rfid_send_time_bit =1;       //  发送RFID时间间隔
    }

    if((time_all_cnt%100) == 0)
    {
        Tick_Time_Stru.obs_send_time_bit =1;        //  壁障器发送查询时间间隔
    }

    if((time_all_cnt%500) == 0)                     //  系统指示灯
    {
        Tick_Time_Stru.time_sys_ms =1;
        time_all_cnt = 0;
    }
    
}






































