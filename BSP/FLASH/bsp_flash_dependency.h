#ifndef __BSP_FLASH_DEPENDENCY_H__
#define __BSP_FLASH_DEPENDENCY_H__

#include "sys.h"



//根据单片机容量设置
#define STM32_FLASH_SIZE 				256 	 		//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN 				1               //使能FLASH写入(0，不是能;1，使能)


//FLASH起始地址
#define STM32_FLASH_BASE  				FLASH_BASE      //STM32 FLASH的起始地址 0x08000000





//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   

//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址(此地址必须为2的倍数!!)
//pBuffer:数据指针
//NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);	


#endif
















