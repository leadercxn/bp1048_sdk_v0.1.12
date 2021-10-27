AudioADC_Example工程主要演示了AudioADC的基本使用方法。

功能：
1.演示LineIn1输入，DAC输出过程；
2.演示MIC1，MIC2，DAC输出过程。
3.LineIn信号和Mic双声道混音之后输出到DAC的过程。

硬件环境要求:
    1. 使用BP10xx系列开发板，外接串口小板，TX/RX/GND 接至A25/A24/GND 引脚 (波特率 256000)；

软件使用说明:
    上电启动之后，可通过串口输入不同指令，进入3种例程；
    1. 输入‘m’或‘M’，进入MIC模式；
    2. 输入‘l’或‘L’，进入LineIn模式；
    3. 输入‘x’或‘X’，进入混音模式，LineIn信号和Mic双声道混音之后输出到DAC；
 