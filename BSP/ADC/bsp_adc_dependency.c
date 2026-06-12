#include "adc.h"


/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
__IO uint16_t ADC_ConvertedValue[ADC_Filter_Count][ADC1_Channel_Count];             //采集数据存放数组，二维数组，16行 3列

uint16_t Current_Num;                                             //当前采集数量

uint32_t ADC_Channel_Sum[ADC1_Channel_Count];                     //每一通道的采样总值
uint16_t ADC_Channel_Max[ADC1_Channel_Count];                     //每一通道的采样最大值
uint16_t ADC_Channel_Min[ADC1_Channel_Count];                     //每一通道的采样最小值
uint16_t ADC_Everage_Value[ADC1_Channel_Count];                   //ADC采样，数字滤波后的最终数据


/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/
/**
  * 函数功能: ADC GPIO 初始化
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
static void ADC1_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE );    //* 打开 ADC端口时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );   //* 打开 GPIOA 端口时钟 */
	
	/* 配置 ADC GPIOA 引脚模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		
	GPIO_Init(GPIOA, &GPIO_InitStructure);		/* 初始化 GPIOA 0,1为模拟量输入模式 */			
}

/**
  * 函数功能: 配置ADC工作模式
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
static void ADC1_Mode_Config(void)
{
	ADC_InitTypeDef ADC_InitStructure;	         //ADC初始化结构体
    DMA_InitTypeDef DMA_InitStructure;           //DMA初始化结构体
	NVIC_InitTypeDef NVIC_InitStructure;         //中断初始化结构体
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE );  //* 打开ADC时钟 */
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);    //* 打开DMA时钟 */
    
	DMA_DeInit(DMA1_Channel1);                            //* 复位DMA控制器 */
	
	/* 配置 DMA 初始化结构体 */	
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t)(&(ADC1->DR));	        //* 外设基址为：ADC1 数据寄存器地址 */	
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_ConvertedValue;	    //* 存储器地址，实际上就是一个内部SRAM的变量 */	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	                        //* 数据源来自外设 */	
	DMA_InitStructure.DMA_BufferSize = ADC1_Channel_Count * ADC_Filter_Count;	//* 数据长度 */	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            //* 外设寄存器只有一个，地址不用递增 */	
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 	                //* 存储器地址递增 */	
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//* 外设数据大小为半字，即两个字节 */	
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	        //* 内存数据大小也为半字，跟外设数据大小相同 */	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	                            //* 循环传输模式 */	
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;                     	//* DMA 传输通道优先级为高，当使用一个DMA通道时，优先级设置不影响 */	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	                            //* 禁止存储器到存储器模式，因为是从外设到存储器 */	
	
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);                                //* 初始化DMA通道1 */		
	DMA_Cmd(DMA1_Channel1 , ENABLE);                                            //* 使能 DMA 通道1 */	
		
	NVIC_InitStructure.NVIC_IRQChannel=DMA1_Channel1_IRQn;                      //* DMA中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;                     //* DMA抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;                            //* DMA相应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                               //* DMA中断使能
	NVIC_Init(&NVIC_InitStructure);                                             //* DMA中断初始化
	
	DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);                               //* DMA传输完成中断使能
 
	/* ADC 模式配置 */
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	                //* 只使用一个ADC，属于单模式 */	
	ADC_InitStructure.ADC_ScanConvMode = ENABLE ;                       //* 使能扫描模式，多通道需要，单通道不需要 */	
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                  //* 连续转换模式使能 */	
//	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                 //* 连续转换模式 失能*/	
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //* 不用外部触发转换，软件开启即可 */	
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	            //* 转换结果右对齐 */	
	ADC_InitStructure.ADC_NbrOfChannel = ADC1_Channel_Count;			//* 转换通道总数量 *	
	
	ADC_Init(ADC1, &ADC_InitStructure);                                 //* 初始化ADC */		
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);                                   //* 配置ADC时钟为PCLK2的8分频，即72M/8=9MMHz 最高14M HZ,超过14M采集数据不准确*/
	
	/* 配置 ADC 通道转换顺序，第一个转换，采样时间为239.5个时钟周期 1/6 us（系统时钟为72Mhz时） * （239.5+12.5）*3*14  = 1/6 * 252 * 3 * 16 约 2ms   */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0,  1, ADC_SampleTime_239Cycles5);  //PA0--通道0  转换顺序为1
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1,  2, ADC_SampleTime_239Cycles5);  //PA1--通道1  转换顺序为2
	
		   
	ADC_DMACmd(ADC1, ENABLE);                   //* 使能ADC DMA 请求 */
  	
	ADC_Cmd(ADC1, ENABLE);                      //* 开启ADC ，并开始转换 */
		
	ADC_ResetCalibration(ADC1);                 //* 初始化ADC 校准寄存器   */
	
	while(ADC_GetResetCalibrationStatus(ADC1));	//* 等待校准寄存器初始化完成 */
	
	ADC_StartCalibration(ADC1);                 //* ADC开始校准*/
	
	while(ADC_GetCalibrationStatus(ADC1));      //*等待校准完成 */
		
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);     //* 由于没有采用外部触发，所以使用软件触发ADC转换  */
}

/**
  * 函数功能: ADC初始化
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
*/
void ADC1_Init(void)
{
	ADC1_GPIO_Config();
	ADC1_Mode_Config();	
}

/**
  * 函数功能: 滤波算法
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
*/
void Filter_Handler(void)  //滤波算法，中位 + 平均滤波法
{
	int i,j;

	for(i=0;i<ADC1_Channel_Count;i++)  //得到最小最大初始值
	{
		ADC_Channel_Max[i]=ADC_ConvertedValue[0][i] & 0x0fff;
		ADC_Channel_Min[i]=ADC_ConvertedValue[0][i] & 0x0fff;
	    ADC_Channel_Sum[i]=0;
	}
	
	for(i=1;i<ADC_Filter_Count;i++)  //逐行比较，得到最大值和最小值
	{
		for(j=0;j<ADC1_Channel_Count;j++)
		{
			if(ADC_Channel_Max[j] < (ADC_ConvertedValue[i][j] & 0x0fff))
			{
				ADC_Channel_Max[j] = ADC_ConvertedValue[i][j] & 0x0fff;
			}
			if(ADC_Channel_Min[j] > (ADC_ConvertedValue[i][j] & 0x0fff))
			{
				ADC_Channel_Min[j] = ADC_ConvertedValue[i][j] & 0x0fff;
			}
		}
	}
	for(i=0;i<ADC1_Channel_Count;i++) //得到各个通道采样值总和
	{
		for(j=0;j<ADC_Filter_Count;j++)
		{
    		ADC_Channel_Sum[i] += ADC_ConvertedValue[j][i] & 0x0fff;  //得到各个通道采样值总和
		}
	}
	for(i=0;i<ADC1_Channel_Count;i++)//得到各个通道的平均值
	{
		ADC_Everage_Value[i] = (ADC_Channel_Sum[i]-ADC_Channel_Max[i]-ADC_Channel_Min[i]) / (ADC_Filter_Count-2);
	}

}

//ADC 采集 DAM传输完成中断回调函数
void (*ADC_DMA_Trans_Complete_CallBack)(void);

/**
  * 函数功能: DMA中断处理函数
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：中断时间 约 1/6 us * 通道数（3）* 采样个数（16）* 采样周期（239.5+12.5）= 2016 us 约2ms
*/
void DMA1_Channel1_IRQHandler()
{
	if(DMA_GetITStatus(DMA1_IT_TC1)!=RESET)
	{
		DMA_ClearITPendingBit(DMA1_IT_TC1);
		
		Filter_Handler();  //滤波算法，中位 + 平均滤波法
		
		if(ADC_DMA_Trans_Complete_CallBack != NULL)
			ADC_DMA_Trans_Complete_CallBack();//ADC 采集 DAM传输完成中断回调函数
	}
}














