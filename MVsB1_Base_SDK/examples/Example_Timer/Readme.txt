Timer_Example工程主要演示了 Timer的基本使用方法。

硬件环境要求:
    1. BP10系列开发板；
    2. 外接串口小板，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率256000)；


软件使用说明:
	上电启动之后，可通过串口输入不同指令，进入6种例程；
	1. 输入‘a’，进入Timerx的中断模式例程，会定时翻转GPIO和打印debug信息；
	2. 输入‘b’，进入Timerx的查询模式例程，会定时翻转GPIO和打印debug信息；
	3. 输入‘c’，进入Timer3主机TIMER4~6从机的模式例程，会定时翻转GPIO和打印debug信息。

