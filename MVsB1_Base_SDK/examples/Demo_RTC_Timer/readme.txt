Example_RTC工程主要演示了RTC的基本操作

1、默认打印串口引脚配置
    RXD为 GPIOB6
    TXD为 GPIOB7
   波特率115200，数据位8bit，停止位1bit

2、UART1默认波特	115200 可自行更改
//Uart1_0 TX-GPIOB7
//Uart1_1 TX-GPIOB0
//Uart1_2 TX-GPIOB2
//Uart1_0 RX-GPIOB6
//Uart1_1 RX-GPIOB1
//Uart1_2 RX-GPIOB3

   注意usart的TX与RX的连接

3、软件使用说明:
	A、RTC可以从三个时钟源中选择，分别是OSC 24MHz、OSC 32.768KHz、HRC 32KHz；前两者精度依赖于实际晶振的精度，后者是RC时钟精度差一些；
	B、选择RTC时钟时，一定要和硬件电路保持一致，和系统的时钟保持是一致；即）“NoAlarmIDFlag（）”的时钟参数要和“Clock_Config（）”的时钟参数保持一致
	C、RTC闹钟使用时，可以选择轮询方式或中断方式；
	D、RTC时间显示有阳历、农历、以及闹钟，颗根据需要选择
	       使用RTC时间显示时要注意：	
	       a、先校准当前时间，参见Demo里调用“System_TimerReSet(&CurrTime)”
	       b、闹钟设置参见Demo里的结构体数组“ALARM_TIME_INFO AlarmList[gMAX_ALARM_NUM] = {};”
	          AlarmHour表示设置的闹钟小时（24小时制）；
	          AlarmMin表示设置的闹钟分钟；
	          RTC_ALARM_STATUS表示的闹钟状态：开、关；
	          RTC_ALARM_MODE表示设置的闹钟模式： 单次闹钟、连续闹钟   
	          AlarmWdayData表示闹钟的具体日期： [bit0~bit6]分别表示周日~周六   
	          
	                                   注意：AlarmWdayData参数只对连续闹钟模式有效；
	                                                单次闹钟模式：若所设置的闹钟时间相对于当前时间已经过期，那么该闹铃将系统自动调到下一天；  
	                                                连续闹钟 模式：[bit0~bit6]分别表示周日~周六 ，对应日期的bit位被置1则有效，置0则无效；若置1的闹钟过期则自动调到一周后；
	                                                每一个闹钟到来会打印出相应的闹钟编号，闹钟编号即闹钟数组的下标+1；
	                                                最多支持8组闹钟，超过8组以前8组为准 ；                                  
	                                                                                                                                
	   
