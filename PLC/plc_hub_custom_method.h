#ifndef _PLC_HUB_CUSTOM_METHOD_H
#define _PLC_HUB_CUSTOM_METHOD_H

#include "plc_hub_upper.h"
#include "magnet_sensor_upper.h"
#include "rfid_upper.h"
#include "motor_upper.h"
#include "antico_upper.h"
#include "public_def_type.h"
#include "encoder_upper.h"

/* 接收处理任务 */
extern void PLC_Hub_CustomRX_Task(void);

/* 发送处理任务 */
extern void PLC_Hub_CustomTX_Mesg_Ask(void);

static void Parse_NewProtocol_Down_Frame(u8 *frame);

static void process_function_code(u16 code,u32 value);

#endif

 
