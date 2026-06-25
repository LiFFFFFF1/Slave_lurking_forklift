/**
  ******************************************************************************
  * @file    dev_tm1650_iic.c
  * @author  zsq Team
  * @version V1.1.0
  * @date    14-04-2014
  * @brief   驱动tm1650-4个数码管
  *          
 @verbatim
  ******************************************************************************
  */
  
#include "myiic_tm1650_method.h"


#define TM1650_SDA_IN()  {GPIOF->MODER &=~(3<<(0*2));GPIOF->MODER |=0<<0*2;}	    //PF0输入模式
#define TM1650_SDA_OUT() {GPIOF->MODER &=~(3<<(0*2));GPIOF->MODER |=1<<0*2;\
                          GPIOF->OTYPER &=~(1<<0);   GPIOF->OTYPER|=1<<0;}          //PF0输出模式,


                          
#define TM1650_SDA  PFout(0) 
#define TM1650_SCL  PFout(1)                                                        // PF1
                          
#define TM1650_READ_SDA   PFin(0) 
                          
       
// TM1650驱动数码管显示     
const u8 dis_segtab[]={
0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,
0x77,0x7C,0x39,0x5E,0x79,0x71,  // 0-9，A-F
0x40,                           // 16- -
0x80,                           // 17- .
0x00,                           // 18- 不显
};
                          
/********************局部使用函数*******************/
/**
  * @brief  开始信号
  * @param  
  *                      
  *                     
  * @param
  * @retval 
  */
void _tm1650_start(void)     
{
	TM1650_SDA_OUT();	//set SDA output
	TM1650_SCL =1;
	TM1650_SDA =1;
	delay_us(5);
	TM1650_SDA =0;
}

/**************************************************************************************************************************
*
*@brief:   产生IIC停止信号
*@param:   
*@retval   None
**************************************************************************************************************************/
void _tm1650_stop(void)
{
	TM1650_SDA_OUT();      //sda线输出
	TM1650_SCL=1;
    delay_us(5);
	TM1650_SDA=0;          //STOP:when CLK is high DATA change form low to high
    delay_us(5);
	TM1650_SDA=1;         //发送I2C总线结束信号 
}

/**************************************************************************************************************************
*
*@brief:   tm1650读取应答
*@param:   
*@retval   None
**************************************************************************************************************************/
void _tm1650_ack(void)
{
    u8 timeout=1;
    
    TM1650_SCL=1;
    TM1650_SDA_IN();       // SDA-SET-INPUT
    delay_us(5);
    TM1650_SCL=0;
    while(TM1650_READ_SDA && (timeout<100))
    {
        timeout++;
    }
    delay_us(5);
	TM1650_SCL=0;         //发送I2C总线结束信号 
    
}

/*******************************************************************************
 * @brief   
 *
 * @param  
 *
 * @retval 
 *
 * @note    向tm1650写一个byte数据
 *****************************************************************************/
void _tm1650_write(u8 wd_data)
{
    u8 i=0;
    TM1650_SDA_OUT();      //sda线输出
    
    TM1650_SCL =0;
    delay_us(5);
    for(i=0;i<8;i++)
    {
        if(wd_data&0x80)
        {
           TM1650_SDA=1;  
        }
        else
        {
            TM1650_SDA=0; 
        }
        TM1650_SCL =0;
        delay_us(5);
        TM1650_SCL =1;
        delay_us(5);
        TM1650_SCL =0;
        delay_us(5);
        wd_data <<=1;
    }
    delay_us(5);
}

/*******************************************************************************
 * @brief   
 *
 * @param  add:地址,wd_data:数据
 *
 * @retval 
 *
 * @note    向tm1650指定地址写入一个byte数据
 *****************************************************************************/
void _tm1650_iic_write_cmd(u8 add,u8 wd_data)
{
     _tm1650_start();
     _tm1650_write(add);
     _tm1650_ack();
     _tm1650_write(wd_data);
     _tm1650_stop();
}

/*******************************************************************************
 * @brief   
 *
 * @param  dig:指定数码段位,set_data:数据
 *
 * @retval 
 *
 * @note    
 *****************************************************************************/
void tm1650_iic_write_display(u8 dig,u8 set_data)
{
    u8 point_flag=0;
    u8 dis_num =0;
    
    point_flag =set_data&0x80;
    dis_num  =set_data&0x7F;
    
    _tm1650_iic_write_cmd(0x68+dig*2,dis_segtab[dis_num]|point_flag);
}

/*******************************************************************************
 * @brief   
 *
 * @param  
 *
 * @retval 
 *
 * @note    tm1650显示初始化
 *****************************************************************************/
void tm1650_iic_init_display(void)
{
   _tm1650_iic_write_cmd(0x48,TM1650_BRIGHT6);          // 6级亮度，8段显示
}

   






