Example_CacheDownSize工程主要演示了 应用CacheDownSize的方法。

硬件环境要求:
    1. BP10xx系列开发板 ；
    2. 外接串口小板，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率 256000)；

软件使用说明:
	1. 默认配置, Icache和Dcache都为32KByte，使用I16K_D16K_Set函数downsize Icache和Dcache, downsize之后Icache和Dcache都为16KByte, 
	   downsize之后SRAM增加32KByte，SRAM范围从0x20000000~0x20038000扩展到0x20000000~0x20040000。