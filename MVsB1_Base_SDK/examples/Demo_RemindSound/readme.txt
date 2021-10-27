Demo_RemindSound工程主要演示了flash区域的提示音播放工作流程。
由三部分组成：flash const data 提示音数据区，decoder，dac

硬件环境要求:
	1. 使用BP10xx系列开发板，外接串口小板，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率 256000)；

软件使用说明:	
	1、提示音bin文件生成和下载说明参见MVsB1_Base_SDK\Tools\remind_script 工具和文档。默认MP3，（同时支持flac，wav，m4a、SBC等常用格式，但注意修改播放类型）--由运行MergeAudio2BinNew.exe生成，变更地址使用命令行
	2、程序启动前必须下载提示音：MVsB1_Base_SDK\Tools\remind_script\all.bin flashburner 地址：0x100000。小心下载地址设置。
	3、测试程序会根据预定义的提示音ITEM名/字符串，查找flash中提示音条目，播放
	4、查看代码可以获知DAC的配置，及解码器decoder的使用流程，解码器整合了FS文件系统播放功能，所以Demo编译需相关驱动支持。
	