PPWM_Example工程主要演示了 应用PPWM的流程。

硬件环境要求:
    1. BP10xx系列开发板 ；
    2. 外接串口小板，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率 256000)；

软件使用说明:
	1. 上电启动之后，初始化的相关配置；
	2. 音频数据流向：LineIn->AudioADC->PPWM；