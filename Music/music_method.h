#ifndef _MUSIC_METHOD_H
#define _MUSIC_METHOD_H


// 使用到的底层基础文件
#include "bsp_can_dependency.h"
#include "bsp_usart_dependency.h"	
#include "bsp_modbusrtu.h"
#include "plc_hub_upper.h"



extern void Music_Play_Send_API(u8 music_type,u8 music_volume);

#endif


