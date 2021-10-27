example_spis 工程主要演示了spis的基本操作

1、串口引脚配置
    1. 使用BP10系列开发板时，外接串口小板，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率256000)；
    
2、SPIS默认引脚配置
	spis_cs(i)     A[23]  	
	spis_miso      A[22]
	spis_clk(i)    A[21]
	spis_mosi(i)   A[20]

   注意SPI的主机MOSI接从机的MOSI，MISO接从机的MISO，不能有错

3、软件使用说明:
	上电启动之后，可通过串口输入不同指令，进入例程；
	1. 输入‘1’，SPIS发送通信(DMA查询模式)例程；
	2. 输入‘2’，SPIS接收通信(DMA查询模式)例程；
注意：先将从机模式准备好，再由主机发送收发步骤
               测试直接采用SDK中SPIM例程。先通过串口配置好SPIS，然后串口配置SPIM

