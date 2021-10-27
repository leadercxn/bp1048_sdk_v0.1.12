AudioDMIC Example工程主要演示了AudioADC使用数字mic的软件基本方法。

功能：
1.演示ADC0 DMIC输入，DAC输出过程；

硬件环境要求:
    1. 使用BP10xx系列开发板，外接串口小板，TX/RX/GND 接至A25/A24/GND 引脚 (波特率 256000)；
    5. GPIOB2接DMIC的 data管脚，GPIOB3接DMIC的clock管脚


软件使用说明:
    确认硬件连接OK之后，系统上电运行程序，即可从DAC端听到DMIC的语音输入。
 