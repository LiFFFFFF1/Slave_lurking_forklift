#include "sys.h"
#include "delay.h"
#include "iwdg.h"

#include "bsp_loading_dependency.h"
#include "bsp_can_dependency.h"
#include "bsp_gpio_dependency.h"
#include "bsp_time_dependency.h"
#include "bsp_usart_dependency.h"
#include "rfid_upper.h"
#include "plc_hub_upper.h"
#include "music_method.h"
#include "antico_upper.h"
#include "myiic_upper.h"  
#include "encoder_upper.h"
#include "motor_upper.h"

#define  DEF_DEBUG_LOG     1

/*******************************************************************************
  * @brief   main
  *
  * @param
  *
  * @retval
  *
  * @note   主函数入口
  *****************************************************************************/
int main(void)
{
    /* USER CODE BEGIN 1 */
    SCB->VTOR = FLASH_APP1_START_ADDR;//设置APP起始地址
    __enable_irq();
    /******************/
    
    Bsp_Delay_Init(72000000);                                       // 延时函数初始化  72M
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    Bsp_GPIO_Pin_Init();                                            // 初始化IO
    
    Myiic_init_api();//iic存储初始化
    
    //Bsp_Delay_MS(500);
    Bsp_Loading_Config_Parameters();                                // 读取拨码-加载配置

    #if DEF_DEBUG_LOG==0
    IWDG_Init(4, 625);//超时时间约1s
    #endif


    while(1)
    {
        if(Tick_Time_Stru.rx_deal_ms_bit)
        {
            Tick_Time_Stru.rx_deal_ms_bit =0;
            Bsp_Uart_Dma_SendCheck();                       // 检测DMA发送队列
            
            PLC_Hub_UpperRX_Task();
            ANTICO_UpperRX_Task();
        }

        if(Tick_Time_Stru.io_deal_ms_bit)                 // 5ms
        {
            Tick_Time_Stru.io_deal_ms_bit =0;
            Bsp_IO_Deal_Task();                                     // IO处理
        }

        if(Tick_Time_Stru.time_20ms_bit )//20ms
        {
            Tick_Time_Stru.time_20ms_bit  = 0;
            Encoder_UpperTX_Task();
        }
        
         if(Tick_Time_Stru.plc_to_hub_ask_bit)                 // 30ms
        {
            Tick_Time_Stru.plc_to_hub_ask_bit =0;
            PLC_Hub_UpperTX_Mesg_Task();                 // HUB上传数据到主机
        }    
         
        if(Tick_Time_Stru.rfid_send_time_bit)                       // 50ms
        {
            Tick_Time_Stru.rfid_send_time_bit =0;
            Music_Play_Send_API(0x01, 100);
            ANTICO_UpperTX_Task();					//触边模块通信
        }

        if(Tick_Time_Stru.motor_send_time_bit)                 // 10ms
        {
            Tick_Time_Stru.motor_send_time_bit = 0;
            Motor_UpperTX_Task();                                     // 举升电机
        }
        
        if(Tick_Time_Stru.time_sys_ms)                              // 500ms
        {
            Tick_Time_Stru.time_sys_ms =0;
            RUN_SYS =! RUN_SYS;                                     // sys-指示灯
            
            #if DEF_DEBUG_LOG==0
            IWDG_Feed();//喂狗
            #endif
            
        }

    }

}





