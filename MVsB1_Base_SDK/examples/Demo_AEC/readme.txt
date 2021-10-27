Demo_AEC工程主要演示了BlueAEC库的基本使用方法。

功能：
1.演示LineIn1(LR)输入参考信号，MIC1,MIC2输入唤醒词，DAC0(L)输出MIC原声，DAC0(R)输出AEC处理完的数据，左右声道供比对。


硬件环境要求:
    1. BP10xx系列开发板
    2. 外接串口小板，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率 256000)；

软件使用说明:
    1. 修改monitor_mode可以切换监听模式。
    2. 串口输入n：减少MIC原声对参考信号的delay
    3. 串口输入m：增加MIC原声对参考信号的delay
    4. 串口输入a：打开/关闭 AEC处理

 