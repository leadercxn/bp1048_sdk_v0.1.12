I2S_Example工程主要演示了I2S的基本使用方法。

功能：
验证I2S RX, I2S TX, I2S0和I2S1的CLK引脚公用， I2S 24bit位宽使用。


硬件环境要求:
    1. 使用BPxx系列开发板时，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率 256000)；

软件使用说明:
    上电启动之后，会打印如下信息，根据提示信息输入对应的字母，便可进入对应的示例程序。
    /-----------------------------------------------------\
    |                     I2S Example                     |
    |      Mountain View Silicon Technology Co.,Ltd.      |
    \-----------------------------------------------------/
    r: I2S0_Master_RX_Example    
    t: I2S1_Slave_TX_Example     
    s: I2S0_I2S1_Binding_Example 
    h: I2S0_24bit_Example        
