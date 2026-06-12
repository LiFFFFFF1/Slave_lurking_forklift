
2021-1-12：
1、降低PID阀值 0.6改成0.5





/* C语言编程 欢迎补充 */

1、变量 
	1.1 尽量少用全局变量，以指针或函数返回值做逻辑运算 方便移植，全局变量过多，阅读代码不容易理解
	1.2 变量以小写字母开头 尽量以英文或容易辨别的英文缩写  如：index buf_Index max_Val rx_Msg等 
	1.3 全局变量以 g开头，然后英文首字母大写  如：gValue gTime gUsart1_Rx_Buffer[]等 
	1.4 变量定义时尽量加注释，

2、宏定义
	2.1 宏定义可以提高代码阅读性、可理解性
	2.2 宏定义定义 尽量以英文或容易辨别的英文缩写 全部以大写字母表示 中间加“_”便于识别 如：#define INPUT_ON  1   #define OUTPUT_OFF  0

3、枚举类型 
	3.1 枚举类型 以小写字母“e”开头 如下：
			typedef enum
			{
				eX00 =0,  //X00
				eX01   ,  //X01
				eX02   ,  //X02
				eX03   ,  //X03
				eX04   ,  //X04
				eX05   ,  //X05
				eX06   ,  //X06
				eX07   ,  //X07
				eX10   ,  //X10
				eX11   ,  //X11
				eX12   ,  //X12
				eX13   ,  //X13
			}eXPOS;
			
4、结构体
	4.1 结构体定义 如下：
			typedef struct
			{
				u8  rx_Buffer[USART_RX_BUF_LEN];    /* 数据接收缓冲区    */
				u8  rx_Count;   			        /* 接收到的数据数量  */
				u8  rx_Flag;                        /* 接收数据状态      */
				u8  rx_Time_Value;                  /* 接收数据时间      */
				u8  rx_Over_Flag;                   /* 接收数据完成标志  */
			}Usart_RX_TypeDef;

			extern Usart_RX_TypeDef Usart1_RX_Struct; //串口#1 接收数据结构体

	4.2 以大写字母开头、变量最后含有_Struct	

5、指针
   5.1 指针以小写“p”开头+以大写字母开头的变量 如 Usart_RX_TypeDef *pUsart1_RX_Struct  unsign char *pBuffer
      



















			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			