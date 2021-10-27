Example_Uart 工程主要演示了usart的基本操作：

1、默认打印串口引脚配置
    1. BP10系列开发板；
    2. 使用BP10系列开发板时，外接串口小板，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率 256000)；  

2、UART1默认波特：115200，可自行更改；

3、软件使用说明:
	上电启动之后，可通过打印串口输入不同指令，选择要演示的example (0~5的字符)；
	0:  MCU with no interrupt
	1:  MCU with interrupt
	2:  DMA Circular send with interrupt
	3:  DMA Circular send with no interrupt
	4:  DMA Circular receive with interrupt
	5:  DMA Circular receive with no interrupt

          具体的每个example的配置示例请看代码以及注释。