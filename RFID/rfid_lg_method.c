#define RFID_LG_GLOBALS
#include "rfid_lg_method.h"

// 包含使用模块 xxx_xxx_upper.h
#include "rfid_upper.h"

// 私有定义
static u8 LG_Write_SendBuf[33] = {0};

/***************************************************************************************
*函    数: void RFID_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
*功    能: 读取缓存数据一帧
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
static void RFID_LG_ReadbufData(USART_STRU *Serial,QUEUE *QUEUE_com)
{
    u8 dat=0,readstatus=0;

    if((Serial->recv_complete_bit & 0x8000) == 0)
    {
        while(1)
        {
            readstatus = FIFO_ComQueueOut(QUEUE_com,&dat);

            if(readstatus == FIFO_OK)
            {
                Serial->the_over_time =0;

                if(Serial->step==0)//
                {
                    if(dat == 0x55)
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[0] = dat;
                        Serial->count = 1;
                    }

                }

                else if(Serial->step== 1)//
                {
                    if(dat == 0x02)
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[1] = dat;
                        Serial->count = 2;

                    }
                    else
                    {
                        Serial->step =0;
                    }
                }

                else if(Serial->step== 2)//
                {
                    if(dat == 0x84||dat == 0x85)
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[2] = dat;
                        Serial->count = 3;
                    }
                    else
                    {
                        Serial->step =0;
                    }
                }

                else if(Serial->step== 3)//
                {
                    if(dat == 0x00)
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[3] = dat;
                        Serial->count = 4;
                    }
                    else
                    {
                        Serial->step =0;
                    }
                }

                else if(Serial->step== 4)//
                {
                    if(dat == 0x12||dat == 0x1A)
                    {
                        Serial->step++;
                        Serial->recv_pbuffer[4] = dat;
                        Serial->count = 5;
                    }
                    else
                    {
                        Serial->step =0;
                    }
                }

                else if(Serial->step== 5)//
                {
                    Serial->recv_pbuffer[Serial->count++] = dat;
                    if(Serial->count >= (Serial->recv_pbuffer[4] + 7))///25、33
                    {

                        Serial->recv_complete_bit=Serial->count;
                        Serial->recv_complete_bit |= 0x8000;

                        Serial->step = 0;
                        Serial->count = 0;
                        return;
                    }
                }

            }

            else if(readstatus == FIFO_ERROR_EMPTY)
            {
                Serial->the_over_time++;
                if(Serial->the_over_time>=100)
                {
                    Serial->the_over_time =0;
                    Serial->step = 0;
                    Serial->count = 0;
                }
                return ;
            }

        }

    }

}


/***************************************************************************************
*函    数: void RFID_485_RX_Complete_Handler(void)
*功    能: 接收处理
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void RFID_LG_485_RX_Complete_Handler(u8 port_com)
{
    u8 crc8_val=0;
    USART_STRU *Serial;
    QUEUE *QUEUE_com;
    u16 recv_len=0;

    switch(port_com)
    {
        default:
            return;
        case 1:
            Serial    = &Usart1_Serial_Stru;
            QUEUE_com = &COM1_Serial;
            break;
        case 2:
            Serial    = &Usart2_Serial_Stru;
            QUEUE_com = &COM2_Serial;
            break;
        case 3:
            Serial    = &Usart3_Serial_Stru;
            QUEUE_com = &COM3_Serial;
            break;
        case 4:
            Serial    = &Usart4_Serial_Stru;
            QUEUE_com = &COM4_Serial;
            break;
        case 5:
            Serial    = &Usart5_Serial_Stru;
            QUEUE_com = &COM5_Serial;
            break;
    }


    RFID_LG_ReadbufData(Serial,QUEUE_com);                   // 获取对应串口对接数据

    if(Serial->recv_complete_bit & 0x8000)
    {

        recv_len =Serial->recv_complete_bit&0x7FFF;
        Serial->recv_complete_bit =0;
        //校验
        crc8_val = Bsp_CRC8_Check(&Serial->recv_pbuffer[5], Serial->recv_pbuffer[4]);
        // 判断校验和针尾
        if(crc8_val != Serial->recv_pbuffer[recv_len-2] || Serial->recv_pbuffer[recv_len-1] != 0xAA)
        {
            return;
        }
        
        switch(Serial->recv_pbuffer[2])     // 命令码
        {
            default:
                break;
            case 0x84:  //心跳返回+读卡的数据
                RFID_Upper_Mesg_Stru.commun_timer = 0;  
                PLC_TO_HUB_Mesg_Stru.upload_run_mileage = 0;//里程清零
                // 数据长度,08--读到卡上报，00-心跳数据
                if(Serial->recv_pbuffer[20]==0x00 && Serial->recv_pbuffer[21] ==0x08)
                {
                    //兼容兴颂RFID                
                    RFID_Upper_Mesg_Stru.read_number  = (Serial->recv_pbuffer[23]&0x0f)*1000;   
                    RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[24]&0x0f)*100;   
                    RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[25]&0x0f)*10;   
                    RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[26]&0x0f)*1;            
                    
                    //上传
                    if(RFID_Upper_Mesg_Stru.read_number)
                    PLC_TO_HUB_Mesg_Stru.upload_rfid_site = RFID_Upper_Mesg_Stru.read_number;
                }
                
                break;
            case 0x85:   // 写读卡返回
                RFID_Upper_Mesg_Stru.commun_timer = 0;
            
                //兼容兴颂RFID                
                RFID_Upper_Mesg_Stru.read_number  = (Serial->recv_pbuffer[23]&0x0f)*1000;   
                RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[24]&0x0f)*100;   
                RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[25]&0x0f)*10;   
                RFID_Upper_Mesg_Stru.read_number += (Serial->recv_pbuffer[26]&0x0f)*1;            
                
                //上传
                if(RFID_Upper_Mesg_Stru.read_number)
                PLC_TO_HUB_Mesg_Stru.upload_rfid_site = RFID_Upper_Mesg_Stru.read_number;

                PLC_TO_HUB_Mesg_Stru.upload_run_mileage = 0;//里程清零
                
                break;
        }
        

    }

}

/***************************************************************************************
*函    数: void RFID_LG_485_Write_Send(void)
*功    能:
*参    数:
*作    者:
*修改时间:
*返 回 值: 无
****************************************************************************************/
void RFID_LG_485_Write_Send(void)//写卡时发送数据
{
    u8 crc8temp = 0;

    // 在线判断 
    if(RFID_Upper_Mesg_Stru.commun_timer < 1000)    RFID_Upper_Mesg_Stru.commun_timer ++;

    if(RFID_Upper_Mesg_Stru.commun_timer > 20)			// 离线 *50ms
    {
        RFID_Upper_Mesg_Stru.offline_state = 1;
    }
    else//在线
    {
        RFID_Upper_Mesg_Stru.offline_state = 0;
    }

    //故障检测
    if(RFID_Upper_Mesg_Stru.offline_state)				//	离线
	{
		PLC_TO_HUB_Mesg_Stru.upload_rfid_alarm = 0xff;
	}
	
    if(PLC_TO_HUB_Mesg_Stru.set_cmd == 0x02 && PLC_TO_HUB_Mesg_Stru.upload_rfid_alarm)			//	复位
    {
		PLC_TO_HUB_Mesg_Stru.upload_rfid_alarm = 0;
		RFID_Upper_Mesg_Stru.offline_state = 0;
        RFID_Upper_Mesg_Stru.commun_timer = 0;
		PLC_TO_HUB_Mesg_Stru.upload_run_mileage =0;
    }

    //写卡
    if(RFID_Upper_Mesg_Stru.rfid_mode == RFID_WRITE_ONLY_MODE)
    {
        if(RFID_Upper_Mesg_Stru.write_number > 0 && RFID_Upper_Mesg_Stru.write_number < 0xffff)
        {
            LG_Write_SendBuf[0] = 0x55;
            LG_Write_SendBuf[1] = 0x02;
            LG_Write_SendBuf[2] = 0x05;
            LG_Write_SendBuf[3] = 0x00;
            LG_Write_SendBuf[4] = 0x1A;
            LG_Write_SendBuf[5] = 0xFF;
            LG_Write_SendBuf[6] = 0xFF;
            LG_Write_SendBuf[7] = 0xFF;
            LG_Write_SendBuf[8] = 0xFF;
            LG_Write_SendBuf[9] = 0xFF;
            LG_Write_SendBuf[10] = 0xFF;
            LG_Write_SendBuf[11] = 0xFF;
            LG_Write_SendBuf[12] = 0xFF;
            LG_Write_SendBuf[13] = 0xFF;
            LG_Write_SendBuf[14] = 0xFF;
            LG_Write_SendBuf[15] = 0xFF;
            LG_Write_SendBuf[16] = 0xFF;
            LG_Write_SendBuf[17] = 0x00;
            LG_Write_SendBuf[18] = 0x00;
            LG_Write_SendBuf[19] = 0x00;
            LG_Write_SendBuf[20] = 0x00;
            LG_Write_SendBuf[21] = 0x08;
            LG_Write_SendBuf[22] = 0x00;

            //LG_Write_SendBuf[23] = (u8)RFID_Upper_Mesg_Stru.write_number&0xff;
            //LG_Write_SendBuf[24] = (u8)((RFID_Upper_Mesg_Stru.write_number>>8)&0xff);
            //LG_Write_SendBuf[25] = 0x00;
            //LG_Write_SendBuf[26] = 0x00;
			LG_Write_SendBuf[23] = RFID_Upper_Mesg_Stru.write_number / 1000;//千位
            LG_Write_SendBuf[24] = (RFID_Upper_Mesg_Stru.write_number % 1000) / 100;//百位
            LG_Write_SendBuf[25] = (RFID_Upper_Mesg_Stru.write_number % 100) / 10;//十位
            LG_Write_SendBuf[26] = (RFID_Upper_Mesg_Stru.write_number % 10) / 1;//个位
			
            LG_Write_SendBuf[27] = 0x00;
            LG_Write_SendBuf[28] = 0x00;
            LG_Write_SendBuf[29] = 0x00;
            LG_Write_SendBuf[30] = 0x00;

            crc8temp = Bsp_CRC8_Check(&LG_Write_SendBuf[5], LG_Write_SendBuf[4]);
            LG_Write_SendBuf[31] = crc8temp;
            LG_Write_SendBuf[32] = 0xAA;

            Bsp_Usart_Usr_SendArray(RFID_Upper_Mesg_Stru.commun_port, &LG_Write_SendBuf[0], 33);//发送数据
           

            RFID_Upper_Mesg_Stru.write_number = 0;//清零

        }

        RFID_Upper_Mesg_Stru.rfid_mode = RFID_READ_ONLY_MODE;

    }

}





