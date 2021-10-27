Demo BT工程主要演示了蓝牙播放的工作流程，未开启 OS。
由蓝牙stack 接收数据，解码器解码sbc为pcm数据送FIFO（DMA）dac播放。

硬件环境要求:
    1. BP10xx系列开发板。
    2. 外接串口小板，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率 256000)；
    3. 手机搜索蓝牙“BP10_BT PLAY DEMO” 连接后播放音乐。

	