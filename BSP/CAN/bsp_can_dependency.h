#ifndef __BSP_CAN_DEPENDENCY_H__
#define	__BSP_CAN_DEPENDENCY_H__

/* 包含头文件 ----------------------------------------------------------------*/
#include "sys.h"

/*
** Prescaler_Index:  
	001-- 2M    波特率
	002-- 1M    波特率
	004-- 500K  波特率
	008-- 250K  波特率
	010-- 200K  波特率
	016-- 125K  波特率
	020-- 100K  波特率
	040-- 50K   波特率
	080-- 25K   波特率
	100-- 20K   波特率
	200-- 10K   波特率
*/								



/* 函数声明 ------------------------------------------------------------------*/
void CAN1_Init(u8 Prescaler_Index);                         //CAN 基本配置	参数为波特率索引 和匹配远端帧ID 	
u8 CAN1_Send_One_Frame_Data(u32 stdID,u8* msg,u8 len);      //发送一帧数据，数据长度不超过8个字节
u8 CAN1_Start_Node(u8 cmd,u8 stdID);
u8 CAN1_Node_Hearbeat(u8 stdID);                        

#endif /* __CAN_H__ */

