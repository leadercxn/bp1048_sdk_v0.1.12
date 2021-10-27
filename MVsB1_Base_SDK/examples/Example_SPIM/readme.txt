example_spim 工程主要演示了spim的基本操作

1、串口引脚配置
     外接串口小板，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率 256000)；

2、SPIM默认引脚配置
	spis_cs        A[8]  	
	spis_miso      A[7]
	spis_clk       A[6]
	spis_mosi      A[5]

   注意SPI的主机MOSI接从机的MOSI，MISO接从机的MISO，不能有错

3、软件使用说明:
	上电启动之后，可通过串口输入不同指令，进入例程；
	1. 输入‘1’，SPIM发送MCU模式例程；
	2. 输入‘2’，SPIM接收MCU模式例程；
	3. 输入‘3’，SPIM发送DMA模式例程；
	4. 输入‘4’，SPIM接收DMA模式例程；
	5. 输入‘5’，SPIM发送和接收通信例程（与SPIS）；

