#define RFID_UPPER_GLOBALS
#include "rfid_upper.h"

// 包含使用模块 xxx_xxx_method.h
#include "rfid_xs_method.h"
#include "rfid_ck_method.h"
#include "rfid_bjf_method.h"
#include "rfid_xmz_method.h"
#include "rfid_lg_method.h"

/***************************************************************************************
*函    数: void RFID_UpperTX_Mesg_Task(void)
*功    能: 
*参    数:       
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：
*    
*  
****************************************************************************************/
void RFID_UpperTX_Mesg_Task(void)
{
    switch(RFID_Upper_Mesg_Stru.rfid_type)
    {
        /*case RFID_XS:
            RFID_XS_485_Write_Send();//  写入地标
            break;
            
        case RFID_BJF:
            RFID_BJF_485_Write_Send();
            break;
            

            
        case RFID_XMZ:
            
            break;
        */
        
//        case RFID_CK:
//            
//            break; 

        case EM_DEVICE_DEV_RFID_CK:
            RFID_LG_485_Write_Send();//  写入地标
            break;
        case EM_DEVICE_DEV_RFID_LG:
            RFID_LG_485_Write_Send();//  写入地标
            break;

       default:
             break;
           
    }
	
}

/***************************************************************************************
*函    数: void RFID_UpperRX_Task(void)
*功    能: 接收处理
*参    数:       
*作    者: 
*修改时间: 
*返 回 值: 无
*备    注：
*    
*  
****************************************************************************************/
void RFID_UpperRX_Task(void)
{
    switch(RFID_Upper_Mesg_Stru.rfid_type)
    {

        case EM_DEVICE_DEV_RFID_LG:
            RFID_LG_485_RX_Complete_Handler(1);//读取地标 485-3
            break;

        case EM_DEVICE_DEV_RFID_CK:
            RFID_CK_232_RX_Complete_Handler(2);       //232-1    
            break;

        default:
             break;
    }
    
}


























