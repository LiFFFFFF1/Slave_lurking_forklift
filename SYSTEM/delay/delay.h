#ifndef __DELAY_H
#define __DELAY_H 			   
#include "sys.h"  


#ifdef DELAY_GLOBALS
   #define DELAY_EXT  
#else
   #define DELAY_EXT  extern
#endif
   

typedef __packed struct
{
   bool rx_deal_ms_bit;
   bool io_deal_ms_bit;
   bool rfid_send_time_bit;
   bool plc_to_hub_ask_bit;
   bool motor_send_time_bit;
   bool magnet_send_time_bit;
   bool obs_send_time_bit;
   bool time_sys_ms;
   bool time_10ms_bit;
   bool time_20ms_bit;    
   
}TICK_TIME_STRU;
DELAY_EXT TICK_TIME_STRU Tick_Time_Stru;

void Bsp_Delay_Init(unsigned int sys_Clock);
void Bsp_Delay_MS(u16 nms);
void Bsp_Delay_US(u32 nus);


#endif





























