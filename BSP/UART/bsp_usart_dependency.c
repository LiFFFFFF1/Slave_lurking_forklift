#define USART_GLOBALS
#include "bsp_usart_dependency.h"	  

 
/* 串口#1 引脚 定义 */
#define USART1_TX_PIN_GPIO           GPIOA
#define USART1_TX_PIN                GPIO_Pin_9

#define USART1_RX_PIN_GPIO           GPIOA
#define USART1_RX_PIN                GPIO_Pin_10

/* 串口#2 引脚 定义 */
#define USART2_TX_PIN_GPIO           GPIOD
#define USART2_TX_PIN                GPIO_Pin_5

#define USART2_RX_PIN_GPIO           GPIOD
#define USART2_RX_PIN                GPIO_Pin_6

/* 串口#3 引脚 定义 */
#define USART3_TX_PIN_GPIO           GPIOD
#define USART3_TX_PIN                GPIO_Pin_8

#define USART3_RX_PIN_GPIO           GPIOD
#define USART3_RX_PIN                GPIO_Pin_9

/* 串口#4 引脚 定义 */
#define UART4_TX_PIN_GPIO            GPIOC
#define UART4_TX_PIN                 GPIO_Pin_10

#define UART4_RX_PIN_GPIO            GPIOC
#define UART4_RX_PIN                 GPIO_Pin_11

/* 串口#5 引脚 定义 */
#define UART5_TX_PIN_GPIO            GPIOC
#define UART5_TX_PIN                 GPIO_Pin_12

#define UART5_RX_PIN_GPIO            GPIOD
#define UART5_RX_PIN                 GPIO_Pin_2

// 串口DMA通道发送-接收优先级
#define  USART1_DMA_PRI_T     5        // 定义DMA通道优先级--发送
#define  USART1_DMA_PRI_R     4        // 定义DMA通道优先级--接收

#define  USART2_DMA_PRI_T     5        // 定义DMA通道优先级--发送
#define  USART2_DMA_PRI_R     4        // 定义DMA通道优先级--接收

#define  USART3_DMA_PRI_T     5        // 定义DMA通道优先级--发送
#define  USART3_DMA_PRI_R     4        // 定义DMA通道优先级--接收

#define  USART4_DMA_PRI_T     5        // 定义DMA通道优先级--发送
#define  USART4_DMA_PRI_R     4        // 定义DMA通道优先级--接收

#define  USART5_ISR_PRI       4        // 定义ISR通道优先级



/**
  * 函数功能: 中断配置选择
  *参    数:  Channel:通道
              PreemptionPriority :抢占优先级
              SubPriority:子优先级

  * 返 回 值: 无
  * 说    明: 
  */
static void Bsp_Usart_Nvic_Config(uint8_t Channel,uint8_t PreemptionPriority,uint8_t SubPriority)
{
	NVIC_InitTypeDef NVIC_InitStructure;
    
    NVIC_InitStructure.NVIC_IRQChannel = Channel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=PreemptionPriority;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = SubPriority;	        //子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		                    //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                                         //根据指定的参数初始化VIC寄存器 
}


/**
  * 函数功能: 串口DMA收发配置
  * USARTx:   串口端口号
  * 返 回 值: 无
  * 说    明: 串口5无DMA
  */
static void Bsp_Usart_DMA_Config(USART_TypeDef* USARTx)
{
	DMA_InitTypeDef DMA_InitStructure;
    
    if(USARTx ==USART1)
    {
        //-----------USART1_DMA接收配置-------------------- 
        
        /* 使能DMA1时钟 */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        /* 将DMA1通道5重设为缺省值 */
        DMA_DeInit(DMA1_Channel5);
        /* 外设基地址 */
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);
        /* 缓存基地址 */
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)COM1_Serial.fifo_bufferA;
        /* 外设作为数据传输的来源 */
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
        /* 缓存大小 */
        DMA_InitStructure.DMA_BufferSize = QUEUE_LENGTH;
        /* 外设地址寄存器不变 */
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        /* 内存地址寄存器递增 */
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        /* DMA数据宽度8位 */
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        /* 外设数据宽度8位 */
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        /* 工作在正常缓存模式 */
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        /* DMA通道拥有-优先级 */
        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;  
        /* DMA通道没有设置为内存到内存传输 */
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
 
        /* 初始化该DMA通道 */
        DMA_Init(DMA1_Channel5, &DMA_InitStructure);
        
        /* 使能串口1 接收DMA */
        USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
        /* 使能传输完成中断*/
        DMA_ITConfig(DMA1_Channel5, DMA_IT_TC,ENABLE);
        
        Bsp_Usart_Nvic_Config(DMA1_Channel5_IRQn,USART1_DMA_PRI_R,0);// 串口1DMA-通道
        
        /* 使能该DMA通道 */
        DMA_Cmd(DMA1_Channel5, ENABLE);		
        
        
        //-----------USART1_DMA发送配置--------------------     
        
        /* 使能DMA1时钟 */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        /* 将DMA1通道4重设为缺省值 */
        DMA_DeInit(DMA1_Channel4);
        /* 外设基地址 */
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);
        /* 缓存基地址 */
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Serialusart1.fifo_bufferA;
        /* 内存作为数据传输的来源 */
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
        /* 缓存大小 */
        DMA_InitStructure.DMA_BufferSize = 0;
        /* 外设地址寄存器不变 */
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        /* 内存地址寄存器递增 */
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        /* DMA数据宽度8位 */
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        /* 外设数据宽度8位 */
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        /* 工作在正常缓存模式 */
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        /* DMA通道拥有 中 优先级 */
        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
        /* DMA通道没有设置为内存到内存传输 */
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        /* 初始化该DMA通道 */
        DMA_Init(DMA1_Channel4, &DMA_InitStructure);
        
        
        /* 使能串口1 发送 DMA */
        USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
        /* 使能传输完成中断*/
        DMA_ITConfig(DMA1_Channel4, DMA_IT_TC,ENABLE);	       

        Bsp_Usart_Nvic_Config(DMA1_Channel4_IRQn,USART1_DMA_PRI_T,0);// 串口1DMA-通道
    }
    else if(USARTx ==USART2)
    {
        //-----------USART2_DMA接收配置-------------------- 
        
        /* 使能DMA1时钟 */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        /* 将DMA1通道6重设为缺省值 */
        DMA_DeInit(DMA1_Channel6);
        /* 外设基地址 */
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART2->DR);
        /* 缓存基地址 */
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)COM2_Serial.fifo_bufferA;
        /* 外设作为数据传输的来源 */
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
        /* 缓存大小 */
        DMA_InitStructure.DMA_BufferSize = QUEUE_LENGTH;
        /* 外设地址寄存器不变 */
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        /* 内存地址寄存器递增 */
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        /* DMA数据宽度8位 */
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        /* 外设数据宽度8位 */
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        /* 工作在正常缓存模式 */
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        /* DMA通道拥有-优先级 */
        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
        /* DMA通道没有设置为内存到内存传输 */
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        /* 初始化该DMA通道 */
        DMA_Init(DMA1_Channel6, &DMA_InitStructure);
        
        /* 使能串口1 接收DMA */
        USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
        /* 使能传输完成中断*/
        DMA_ITConfig(DMA1_Channel6, DMA_IT_TC,ENABLE);
        
        Bsp_Usart_Nvic_Config(DMA1_Channel6_IRQn,USART2_DMA_PRI_R,0);// 串口2DMA-通道
        
        /* 使能该DMA通道 */
        DMA_Cmd(DMA1_Channel6, ENABLE);
        
        
        //-----------USART2_DMA发送配置--------------------     
        
        /* 使能串口2 接收DMA */
        USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
        
        /* 使能DMA1时钟 */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        /* 将DMA1通道7重设为缺省值 */
        DMA_DeInit(DMA1_Channel7);
        /* 外设基地址 */
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART2->DR);
        /* 缓存基地址 */
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Serialusart2.fifo_bufferA;
        /* 内存作为数据传输的来源 */
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
        /* 缓存大小 */
        DMA_InitStructure.DMA_BufferSize = 0;
        /* 外设地址寄存器不变 */
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        /* 内存地址寄存器递增 */
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        /* DMA数据宽度8位 */
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        /* 外设数据宽度8位 */
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        /* 工作在正常缓存模式 */
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        /* DMA通道拥有 中 优先级 */
        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
        /* DMA通道没有设置为内存到内存传输 */
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        /* 初始化该DMA通道 */
        DMA_Init(DMA1_Channel7, &DMA_InitStructure);
        

        /* 使能串口2 发送 DMA */
        USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
        /* 使能传输完成中断*/
        DMA_ITConfig(DMA1_Channel7, DMA_IT_TC,ENABLE);
        
        Bsp_Usart_Nvic_Config(DMA1_Channel7_IRQn,USART2_DMA_PRI_T,0);// 串口2DMA-通道
    }
    else if(USARTx ==USART3)
    {
        //-----------USART3_DMA接收配置-------------------- 
        
        /* 使能DMA1时钟 */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        /* 将DMA1通道3重设为缺省值 */
        DMA_DeInit(DMA1_Channel3);
        /* 外设基地址 */
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART3->DR);
        /* 缓存基地址 */
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)COM3_Serial.fifo_bufferA;
        /* 外设作为数据传输的来源 */
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
        /* 缓存大小 */
        DMA_InitStructure.DMA_BufferSize = QUEUE_LENGTH;
        /* 外设地址寄存器不变 */
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        /* 内存地址寄存器递增 */
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        /* DMA数据宽度8位 */
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        /* 外设数据宽度8位 */
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        /* 工作在正常缓存模式 */
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        /* DMA通道拥有--优先级 */
        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
        /* DMA通道没有设置为内存到内存传输 */
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        /* 初始化该DMA通道 */
        DMA_Init(DMA1_Channel3, &DMA_InitStructure);
        
        /* 使能串口3 接收DMA */
        USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
        /* 使能传输完成中断*/
        DMA_ITConfig(DMA1_Channel3, DMA_IT_TC,ENABLE);
        
        Bsp_Usart_Nvic_Config(DMA1_Channel3_IRQn,USART4_DMA_PRI_R,0);// 串口3-DMA-通道
        
        /* 使能该DMA通道 */
        DMA_Cmd(DMA1_Channel3, ENABLE);
        
        
        
        //-----------USART3_DMA发送配置-------------------- 
        
        /* 使能DMA1时钟 */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        /* 将DMA1通道2重设为缺省值 */
        DMA_DeInit(DMA1_Channel2);
        /* 外设基地址 */
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART3->DR);
        /* 缓存基地址 */
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Serialusart3.fifo_bufferA;
        /* 内存作为数据传输的来源 */
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
        /* 缓存大小 */
        DMA_InitStructure.DMA_BufferSize = 0;
        /* 外设地址寄存器不变 */
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        /* 内存地址寄存器递增 */
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        /* DMA数据宽度8位 */
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        /* 外设数据宽度8位 */
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        /* 工作在正常缓存模式 */
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        /* DMA通道拥有 中 优先级 */
        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
        /* DMA通道没有设置为内存到内存传输 */
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        /* 初始化该DMA通道 */
        DMA_Init(DMA1_Channel2, &DMA_InitStructure);
        
        /* 使能串口3 发送 DMA */
        USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
        
        /* 使能传输完成中断*/
        DMA_ITConfig(DMA1_Channel2, DMA_IT_TC,ENABLE);
        /* 串口3-DMA1_Channel2_IRQn-通道 */
        Bsp_Usart_Nvic_Config(DMA1_Channel2_IRQn,USART3_DMA_PRI_T,0);
    }
    else if(USARTx ==UART4)
    {
        //-----------UART4_DMA接收配置-------------------- 
        /* 使能DMA2时钟 */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
        
        /* 将DMA2通道3重设为缺省值 */
        DMA_DeInit(DMA2_Channel3);       
        /* 外设基地址 */
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&UART4->DR);
        /* 缓存基地址 */
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)COM4_Serial.fifo_bufferA;
        /* 外设作为数据传输的来源 */
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
        /* 缓存大小 */
        DMA_InitStructure.DMA_BufferSize = QUEUE_LENGTH;
        /* 外设地址寄存器不变 */
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        /* 内存地址寄存器递增 */
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        /* DMA数据宽度8位 */
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        /* 外设数据宽度8位 */
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        /* 工作在正常缓存模式 */
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        /* DMA通道拥有-优先级 */
        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
        /* DMA通道没有设置为内存到内存传输 */
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        /* 初始化该DMA通道 */
        DMA_Init(DMA2_Channel3, &DMA_InitStructure);
        
        /* 使能串口4 接收DMA */
        USART_DMACmd(UART4, USART_DMAReq_Rx, ENABLE);
        /* 使能传输完成中断*/
        DMA_ITConfig(DMA2_Channel3, DMA_IT_TC,ENABLE);
        
        /* 串口4DMA-通道 */
        Bsp_Usart_Nvic_Config(DMA2_Channel3_IRQn,USART4_DMA_PRI_R,0);
        
        /* 使能该DMA通道 */
        DMA_Cmd(DMA2_Channel3, ENABLE);
        
        
        
        //-----------UART4_DMA发送配置-------------------- 
        
        /* 使能DMA2时钟 */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

        /* 将DMA2通道5重设为缺省值 */
        DMA_DeInit(DMA2_Channel5);
        /* 外设基地址 */
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&UART4->DR);
        /* 缓存基地址 */
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Serialusart4.fifo_bufferA;
        /* 内存作为数据传输的来源 */
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
        /* 缓存大小 */
        DMA_InitStructure.DMA_BufferSize = 0;
        /* 外设地址寄存器不变 */
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        /* 内存地址寄存器递增 */
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        /* DMA数据宽度8位 */
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        /* 外设数据宽度8位 */
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        /* 工作在正常缓存模式 */
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        /* DMA通道拥有 中 优先级 */
        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
        /* DMA通道没有设置为内存到内存传输 */
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        /* 初始化该DMA通道 */
        DMA_Init(DMA2_Channel5, &DMA_InitStructure);

        /* 使能串口4 发送 DMA */
        USART_DMACmd(UART4, USART_DMAReq_Tx, ENABLE);
        /* 使能传输完成中断   */
        DMA_ITConfig(DMA2_Channel5, DMA_IT_TC,ENABLE);
        
        /* 串口4 DMA2_Channel4_5_IRQn 通道*/
        Bsp_Usart_Nvic_Config(DMA2_Channel4_5_IRQn,USART4_DMA_PRI_T,0);
        
    }
    else if(USARTx ==UART5)
    {
        /* 串口5   UART5_IRQn 通道  */
        Bsp_Usart_Nvic_Config(UART5_IRQn,USART5_ISR_PRI,0);    
        /* 使能串口5  USART_IT_RXNE */
        USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
    }
}


/**
  * 函数功能: dma发送通道-设置地址和数据长度（DMA发送重新配置）
  * DMA_CHx:   DMA通道
  * cmar:  存储地址
  * len:   数据长度
  * 说    明: 串口5无DMA
  */
static void Bsp_Usart_Dma_Send_Config(DMA_Channel_TypeDef *DMA_CHx ,u32 cmar, u16 len)
{ 
	DMA_Cmd(DMA_CHx, DISABLE );  //关闭USARTTX DMA所指示的通道 
  // while (DMA_GetCmdStatus(DMA_CHx) != DISABLE){}	//确保DMA可以被设置  
	DMA_CHx->CMAR=(u32)cmar;     //DMA,存储器0地址	
    DMA_CHx->CNDTR= len;          //DMA传输项个数
  
	DMA_Cmd(DMA_CHx, ENABLE);   //使能USARTTX DMA所指示的通道 
}
/***************************************************************************************
*函数功能: dma接收通道-设置地址和数据长度（DMA接收重新配置）
*功    能: DMA接收重新配置
*参    数: DMA_CHx:DMA通道CHx
           cmar:内存地址
****************************************************************************************/
static void Bsp_Usart_Dma_Recv_Config(DMA_Channel_TypeDef *DMA_CHx ,u32 cmar)
{ 
	DMA_Cmd(DMA_CHx, DISABLE);                              //关闭USARTTX DMA所指示的通道 

	DMA_CHx->CMAR=(u32)cmar;                                //DMA, 存储器地址	
    DMA_CHx->CNDTR=USART_REC_LEN;                           //DMA, 接收字节数
	DMA_Cmd(DMA_CHx, ENABLE);                               //使能USARTTX DMA所指示的通道 
}

/**
  * 函数功能: 写数据到FIFO队列
  * QUEUE_com:  发送结构体
  * buffer:    写入数据地址
  * len:        写入数据长度
  * 返 回 值: 无
  * 说    明: 
  */
static void Bsp_Usart_WriteFIFO_Buffer(QUEUE *QUEUE_com,u8 *buffer,u16 len)
{
    u16 i=0;

	for(i=0;i<len;i++)
	  FIFO_ComQueueIn(QUEUE_com,buffer[i]);
}









/**
  * 函数功能: 串口端口硬件初始化
  * USARTx: 指定串口号
  * baud_rate：波特率
  * wordlength：数据宽度
  * parity：校验位
  * 返 回 值: 无
  * 说    明: 
  */
void Bsp_Usart_Config_Init(USART_TypeDef* USARTx,u32 baud_rate,u16 wordlength,u16 parity)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

    
    /* 使能串口GPIOx时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	   
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);       // 使能映射IO时钟	
    /*  串口-GPIO配置---*/
    if(USARTx == USART1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);  // 使能串口时钟	
        
        //-----------------USART1 GIPD 配置--------------------------- 
        
        /* 设置RX为复用推挽输出 */
        GPIO_InitStructure.GPIO_Pin = USART1_TX_PIN;   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
        GPIO_Init(USART1_TX_PIN_GPIO, &GPIO_InitStructure);

        /* 设置RX为浮空输入 */
        GPIO_InitStructure.GPIO_Pin = USART1_RX_PIN;   
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
        GPIO_Init(USART1_RX_PIN_GPIO, &GPIO_InitStructure);
        
        FIFO_BuffInit(&COM1_Serial);                            // 初始化循环队列-接收
        FIFO_BuffInit(&Serialusart1);                           // 初始化循环队列-发送
    }
    else if(USARTx == USART2)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); 
        //-----------------USART2 GIPD 配置--------------------------- 
		
		GPIO_PinRemapConfig(GPIO_Remap_USART2,ENABLE);		    // 配置部分映射
		
        /* 设置RX为复用推挽输出 */
        GPIO_InitStructure.GPIO_Pin = USART2_TX_PIN;   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
        GPIO_Init(USART2_TX_PIN_GPIO, &GPIO_InitStructure);

        /* 设置RX为浮空输入 */
        GPIO_InitStructure.GPIO_Pin = USART2_RX_PIN;   
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
        GPIO_Init(USART2_RX_PIN_GPIO, &GPIO_InitStructure);
        
        FIFO_BuffInit(&COM2_Serial);                            // 初始化循环队列-接收
        FIFO_BuffInit(&Serialusart2);                           // 初始化循环队列-发送
    }
    else if(USARTx == USART3)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);	    
        //-----------------USART3 GIPD 配置--------------------------- 
        GPIO_PinRemapConfig(GPIO_FullRemap_USART3,ENABLE);		// 配置完全映射
		
        /* 设置RX为复用推挽输出 */
        GPIO_InitStructure.GPIO_Pin = USART3_TX_PIN;   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
        GPIO_Init(USART3_TX_PIN_GPIO, &GPIO_InitStructure);

        /* 设置RX为浮空输入 */
        GPIO_InitStructure.GPIO_Pin = USART3_RX_PIN;   
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
        GPIO_Init(USART3_RX_PIN_GPIO, &GPIO_InitStructure);
        
        FIFO_BuffInit(&COM3_Serial);                            // 初始化循环队列-接收
        FIFO_BuffInit(&Serialusart3);                           // 初始化循环队列-发送
    }
    else if(USARTx == UART4)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);      
    
        //-----------------UART4 GIOPD 配置--------------------------- 
        
        /* 设置RX为复用推挽输出 */
        GPIO_InitStructure.GPIO_Pin = UART4_TX_PIN;   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
        GPIO_Init(UART4_TX_PIN_GPIO, &GPIO_InitStructure);

        /* 设置RX为浮空输入 */
        GPIO_InitStructure.GPIO_Pin = UART4_RX_PIN;   
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
        GPIO_Init(UART4_RX_PIN_GPIO, &GPIO_InitStructure);
        
        FIFO_BuffInit(&COM4_Serial);                            // 初始化循环队列-接收
        FIFO_BuffInit(&Serialusart4);                           // 初始化循环队列-发送
    }
    else if(USARTx == UART5)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
        
        //-----------------UART5 GIPD 配置--------------------------- 
        /* 设置RX为复用推挽输出 */
        GPIO_InitStructure.GPIO_Pin = UART5_TX_PIN;   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
        GPIO_Init(UART5_TX_PIN_GPIO, &GPIO_InitStructure);

        /* 设置RX为浮空输入 */
        GPIO_InitStructure.GPIO_Pin = UART5_RX_PIN;   
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
        GPIO_Init(UART5_RX_PIN_GPIO, &GPIO_InitStructure);
        
        FIFO_BuffInit(&COM5_Serial);                        // 初始化循环队列-接收
        FIFO_BuffInit(&Serialusart5);                       // 初始化循环队列-发送
    }
    else
    {
        return;
    }
    
    /*  USART 初始化设置  */
	USART_InitStructure.USART_BaudRate = baud_rate;         //串口波特率
	USART_InitStructure.USART_WordLength = wordlength;      //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;  //一个停止位
	USART_InitStructure.USART_Parity = parity;              //无奇偶校验位
                                                            //无硬件数据流控制
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
                                                            //收发模式
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
	
	USART_Init(USARTx, &USART_InitStructure);               //初始化串口x
    
	
	/* 清除空闲中断挂起 */
	USART_ClearITPendingBit(USARTx, USART_IT_RXNE | USART_IT_IDLE | USART_IT_TC);
    
    Bsp_Usart_DMA_Config(USARTx);                           // DMA配置
    
    
    /* 清除 中断  */
    USART_ClearITPendingBit(USARTx, USART_IT_TC);
    USART_ClearFlag(USARTx, USART_FLAG_TC);
    
    /* 禁止串口发送完成中断 */
    USART_ITConfig(USARTx,USART_IT_TC, DISABLE);            // ENABLE,DISABLE
    
    USART_Cmd(USARTx, ENABLE);                              //使能串口x
} 

/**
  * 函数功能: 串口自动发送加载
  * 返 回 值: 无
  * 说    明: 循环队列-双缓存发送
  */
void Bsp_Uart_Dma_SendCheck(void)
{
	if (Serialusart1.send_lock == 0)
	{
		if (Serialusart1.RecLen > 0)
		{
			Serialusart1.send_lock = 1;
			if (Serialusart1.FIFO_buffer_status == 0)
			{
                Bsp_Usart_Dma_Send_Config(DMA1_Channel4,(u32)Serialusart1.fifo_bufferA,Serialusart1.RecLen);//重新启动DMA				
				Serialusart1.FIFO_buffer_status = 1;
			}
			else if (Serialusart1.FIFO_buffer_status == 1)
			{
				Bsp_Usart_Dma_Send_Config(DMA1_Channel4,(u32)Serialusart1.fifo_bufferB,Serialusart1.RecLen);//重新启动DMA		
				Serialusart1.FIFO_buffer_status = 0;
			}
			
			Serialusart1.RecLen = 0;
		}
	}

	if (Serialusart2.send_lock == 0)
	{
		if (Serialusart2.RecLen > 0)
		{
			Serialusart2.send_lock = 1;
			if (Serialusart2.FIFO_buffer_status == 0)
			{
				Bsp_Usart_Dma_Send_Config(DMA1_Channel7,(u32)Serialusart2.fifo_bufferA,Serialusart2.RecLen);//重新启动DMA
				Serialusart2.FIFO_buffer_status = 1;
			}
			else if (Serialusart2.FIFO_buffer_status == 1)
			{
				Bsp_Usart_Dma_Send_Config(DMA1_Channel7,(u32)Serialusart2.fifo_bufferB,Serialusart2.RecLen);//重新启动DMA	
				Serialusart2.FIFO_buffer_status = 0;
			}
			
			Serialusart2.RecLen = 0;
		}
	}

	if (Serialusart3.send_lock == 0)
	{
		if (Serialusart3.RecLen > 0)
		{
			Serialusart3.send_lock = 1;
			if (Serialusart3.FIFO_buffer_status == 0)
			{
				Bsp_Usart_Dma_Send_Config(DMA1_Channel2,(u32)Serialusart3.fifo_bufferA,Serialusart3.RecLen);//重新启动DMA
				Serialusart3.FIFO_buffer_status = 1;
			}
			else if (Serialusart3.FIFO_buffer_status == 1)
			{
				Bsp_Usart_Dma_Send_Config(DMA1_Channel2,(u32)Serialusart3.fifo_bufferB,Serialusart3.RecLen);//重新启动DMA
				Serialusart3.FIFO_buffer_status = 0;
			}
			
			Serialusart3.RecLen = 0;
		}
	}

	if (Serialusart4.send_lock == 0)
	{
		if (Serialusart4.RecLen > 0)
		{
			Serialusart4.send_lock = 1;
			if (Serialusart4.FIFO_buffer_status == 0)
			{
				Bsp_Usart_Dma_Send_Config(DMA2_Channel5,(u32)Serialusart4.fifo_bufferA,Serialusart4.RecLen);//重新启动DMA
				Serialusart4.FIFO_buffer_status = 1;
			}
			else if (Serialusart4.FIFO_buffer_status == 1)
			{
				Bsp_Usart_Dma_Send_Config(DMA2_Channel5,(u32)Serialusart4.fifo_bufferB,Serialusart4.RecLen);//重新启动DMA
				Serialusart4.FIFO_buffer_status = 0;
			}
			
			Serialusart4.RecLen = 0;
		}
	}
}


/**
  * 函数功能: 用户发送串口数据
  * USARTx:   指定串口
  * data:     写入数据地址
  * len:      写入数据长度
  * 返 回 值: 无
  * 说    明: 循环队列-双缓存发送
  */
void Bsp_Usart_Usr_SendArray(USART_TypeDef* USARTx, u8 *data, u16 len)
{	
    
      if(USARTx == USART1)
      {
         Bsp_Usart_WriteFIFO_Buffer(&Serialusart1,data,len);
	  }
	  else if(USARTx == USART2)
      {
         Bsp_Usart_WriteFIFO_Buffer(&Serialusart2,data,len);
	  }
	  else if(USARTx == USART3)
      {
         Bsp_Usart_WriteFIFO_Buffer(&Serialusart3,data,len);
	  }
	  else if(USARTx == UART4)
      {
         Bsp_Usart_WriteFIFO_Buffer(&Serialusart4,data,len);
	  }
	  else if(USARTx == UART5)
      {         
         Bsp_Usart_WriteFIFO_Buffer(&Serialusart5,data,len);    
         USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
	  }
      else
      {
         return;
      }
}


/**
  * 函数功能: 串口1-DMA接收中断
  * 返 回 值: 无
  * 说    明: DMA1_Channel5
  */
void DMA1_Channel5_IRQHandler(void)
{
      if(DMA_GetFlagStatus(DMA1_FLAG_TC5))     
	  { 	
          DMA_ClearFlag(DMA1_FLAG_TC5);                     //清除完成标志
          
          COM1_Serial.FIFO_Status_FULL = 1;                 //数据已填满
          
		
		  	
		  if(COM1_Serial.FIFO_buffer_status == 0) 
		  {
                COM1_Serial.FIFO_buffer_status = 1;         //重新启动DMA
				Bsp_Usart_Dma_Recv_Config(DMA1_Channel5,(u32)COM1_Serial.fifo_bufferB);
		  }
		  else
		  {
                COM1_Serial.FIFO_buffer_status = 0;         //重新启动DMA	
				Bsp_Usart_Dma_Recv_Config(DMA1_Channel5,(u32)COM1_Serial.fifo_bufferA);				
		  }
	 }
}
/**
  * 函数功能: 串口1-DMA发送中断
  * 返 回 值: 无
  * 说    明: DMA1_Channel4
  */
void DMA1_Channel4_IRQHandler(void)
{
      if(DMA_GetFlagStatus(DMA1_FLAG_TC4))    
	  { 			
          DMA_ClearFlag(DMA1_FLAG_TC4);                     //清除完成标志
          
          DMA_Cmd(DMA1_Channel4, DISABLE);
		  Serialusart1.send_lock = 0;
          USART_ITConfig(USART1,USART_IT_TC,ENABLE);
	 }
}

/**
  * 函数功能: 串口2-DMA接收中断
  * 返 回 值: 无
  * 说    明: DMA1_Channel6
  */
void DMA1_Channel6_IRQHandler(void)
{
      if(DMA_GetFlagStatus(DMA1_FLAG_TC6))     
	  { 	
		
          DMA_ClearFlag(DMA1_FLAG_TC6);                     //清除完成标志
          
          COM2_Serial.FIFO_Status_FULL = 1;                 //数据已填满
          
		
		  	
		  if(COM2_Serial.FIFO_buffer_status == 0) 
		  {
                COM2_Serial.FIFO_buffer_status = 1;         //重新启动DMA
				Bsp_Usart_Dma_Recv_Config(DMA1_Channel6,(u32)COM2_Serial.fifo_bufferB);
		  }
		  else
		  {
                COM2_Serial.FIFO_buffer_status = 0;         //重新启动DMA	
				Bsp_Usart_Dma_Recv_Config(DMA1_Channel6,(u32)COM2_Serial.fifo_bufferA);				
		  }
	 }
}
/**
  * 函数功能: 串口2-DMA发送中断
  * 返 回 值: 无
  * 说    明: DMA1_Channel7
  */
void DMA1_Channel7_IRQHandler(void)
{
      if(DMA_GetFlagStatus(DMA1_FLAG_TC7))     
	  { 	
          DMA_ClearFlag(DMA1_FLAG_TC7);                     //清除完成标志
          
          DMA_Cmd(DMA1_Channel7, DISABLE);
		  Serialusart2.send_lock = 0;
          USART_ITConfig(USART2,USART_IT_TC,ENABLE);
	 }
}


/**
  * 函数功能: 串口3-DMA接收中断
  * 返 回 值: 无
  * 说    明: DMA1_Channel3
  */
void DMA1_Channel3_IRQHandler(void)
{
      if(DMA_GetFlagStatus(DMA1_FLAG_TC3))     
	  { 	
		
          DMA_ClearFlag(DMA1_FLAG_TC3);                     //清除完成标志
          
          COM3_Serial.FIFO_Status_FULL = 1;                 //数据已填满
          
		 
		  	
		  if(COM3_Serial.FIFO_buffer_status == 0) 
		  {
                COM3_Serial.FIFO_buffer_status = 1;         //重新启动DMA
				Bsp_Usart_Dma_Recv_Config(DMA1_Channel3,(u32)COM3_Serial.fifo_bufferB);
		  }
		  else
		  {
                COM3_Serial.FIFO_buffer_status = 0;         //重新启动DMA		
				Bsp_Usart_Dma_Recv_Config(DMA1_Channel3,(u32)COM3_Serial.fifo_bufferA);			
		  }
	 }
}
/**
  * 函数功能: 串口3-DMA发送中断
  * 返 回 值: 无
  * 说    明: DMA1_Channel2
  */
void DMA1_Channel2_IRQHandler(void)
{
      if(DMA_GetFlagStatus(DMA1_FLAG_TC2))     
	  { 	
          DMA_ClearFlag(DMA1_FLAG_TC2);                     //清除完成标志      
          
          DMA_Cmd(DMA1_Channel2, DISABLE);
		  Serialusart3.send_lock = 0;

          USART_ITConfig(USART3,USART_IT_TC,ENABLE);
	 }
}



/**
  * 函数功能: 串口4-DMA接收中断
  * 返 回 值: 无
  * 说    明: DMA2_Channel3
  */
void DMA2_Channel3_IRQHandler(void)
{
      if(DMA_GetFlagStatus(DMA2_FLAG_TC3))     
	  { 	
		
          DMA_ClearFlag(DMA2_FLAG_TC3);                     // 清除完成标志
          
          COM4_Serial.FIFO_Status_FULL = 1;                 // 数据已填满
          
		  	
		  if(COM4_Serial.FIFO_buffer_status == 0) 
		  {
                COM4_Serial.FIFO_buffer_status = 1;         // 重新启动DMA
				Bsp_Usart_Dma_Recv_Config(DMA2_Channel3,(u32)COM4_Serial.fifo_bufferB);
		  }
		  else
		  {
                COM4_Serial.FIFO_buffer_status = 0;         // 重新启动DMA	
				Bsp_Usart_Dma_Recv_Config(DMA2_Channel3,(u32)COM4_Serial.fifo_bufferA);				
		  }
	 }
}
/**
  * 函数功能: 串口4-DMA发送中断
  * 返 回 值: 无
  * 说    明: DMA2_Channel5
  */
void DMA2_Channel4_5_IRQHandler(void)
{
     if(DMA_GetFlagStatus(DMA2_FLAG_TC5))     
	 { 	
          DMA_ClearFlag(DMA2_FLAG_TC5);                     //清除完成标志
          
          DMA_Cmd(DMA2_Channel5, DISABLE);
		  Serialusart4.send_lock = 0;

          USART_ITConfig(UART4,USART_IT_TC, ENABLE);
	 }
}

/**
  * 函数功能: 串口5-收发中断
  * 返 回 值: 无
  * 说    明: 
  */
void UART5_IRQHandler(void)   
{
  u8 res_data;
  u8 readstatus;
        
  if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
  {
	res_data =USART_ReceiveData(UART5);                 //读取接收到的数据
	FIFO_ComQueueIn(&COM5_Serial,res_data);
  } 
  if(USART_GetITStatus(UART5, USART_IT_TXE) != RESET)   //串口发送中断
  {   
     readstatus = FIFO_ComQueueOut(&Serialusart5,&res_data);
     if(readstatus == FIFO_OK)
     {
        USART_SendData(UART5,res_data);                 //发送数据
     }
     else
     {
        USART_ITConfig(UART5, USART_IT_TXE, DISABLE);   //关闭发送中断
     }
  }
}












/***************************************************************************************
*函	 数:  int fputc(int ch, FILE *f)
*功	 能: 重定向c库函数printf到USART1
*参	 数: 无
*作	 者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
/*int fputc(int ch, FILE *f)
{
		 
	 USART_SendData(USART1, (uint8_t) ch);// 发送一个字节数据到USART1 
	 while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	 //等待发送完毕  
	 return (ch);
}*/
/***************************************************************************************
*函	 数: int fgetc(FILE *f)
*功	 能: 重定向c库函数scanf到USART1
*参	 数: 无
*作	 者: 
*修改时间: 
*返 回 值: 无
****************************************************************************************/
int fgetc(FILE *f)
{
		
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);//等待串口1输入数据 
	return (int)USART_ReceiveData(USART1);
}













