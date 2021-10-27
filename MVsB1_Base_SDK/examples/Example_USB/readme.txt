Usb_Example工程主要演示了BP10usb host和device的基本使用方法。

功能：
1.usb host 读写U盘扇区；
2.usb device audio。


硬件环境要求:
    1. BP10系列开发板
    2. 外接串口小板，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率 256000)

软件使用说明:
    1.打开代码usb_host_example();注释代码usb_device_example();编译下载后，插上U盘，上电启动之后，
	通过打印串口会看到U盘的各种信息，最后会“读写U盘数据OK”表示成功。
    2.打开代码usb_device_example();注释代码usb_host_example();编译下载后，使用USB线插到电脑上，
	电脑上回显示出声卡，电脑可以播放音乐，也可以录音；选择不同参数可以出现不同的device设备。
	
注意：
为了解决和Audio_APP工程 API 兼容性问题，该工程默认增加了一个系统宏 CFG_APP_USB_AUDIO_MODE_EN，系统宏默认开启。

	
	