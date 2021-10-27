Example_Flash工程主要演示了Flash的工作流程。

硬件环境要求:
    1. BP10xx系列开发板
    2. 外接串口小板，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率 256000)
    3. 上位机串口调试工具:SSCOM3.2

软件使用说明:	
       上电启动之后，可通过串口输入不同指令，进入5种例程；
   1. 输入‘0’，进入flash erase例程;
   2. 输入‘1’，进入flash read例程;
   3. 输入‘2’，进入flash write例程;
   4. 输入‘3’，进入flash protect例程;
   5. 输入‘4’，进入flash unprotect例程;
	