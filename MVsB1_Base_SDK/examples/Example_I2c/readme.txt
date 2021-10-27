Example_I2c 工程主要演示了I2C的基本操作


1、串口引脚配置
	外接串口小板，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率256000)；
    
2、I2C默认引脚配置，注意将引脚配置上拉，或者硬件设计时加上拉电阻（10K）
	SDA 	GPIOB4
	SCL		GPIOB5

试验中是主从机相互传了255个数据：00-FE，传输成功后会有相应的打印信息显示成功

3、软件使用说明:
	上电启动之后，直接进入例程，在串口打印出观察打印信息；
	根据提示串口可知当前测试模式为主机或从机：
	主机与从机的测试模式有例程开头宏定义MASTER_SLAVE决定，从机是否使用中断收发由宏定义SLAVE_INT决定
	
特殊说明：
		客户在使用时，需要将使用的I2C相关引脚软件上配置上拉，或者硬件上将IO接上拉电阻
		在使用收发函数时需要注意：
		I2C_MasterSendBuffer(uint8_t SlaveAddr, void* SendBuf, uint32_t BufLen, uint32_t timeout)
		I2C_MasterReceiveBuffer(uint8_t SlaveAddr, void* RecvBuf, uint32_t BufLen, uint32_t timeout)
		此两个函数是不需要额外寄存器的主机收发方式
		
		I2C_MasterSendData(uint8_t SlaveAddr, uint8_t RegAddr, void* SendBuf, uint32_t BufLen, uint32_t timeout)
		I2C_MasterReceiveData(uint8_t SlaveAddr, uint8_t RegAddr, void* RecvBuf, uint32_t BufLen, uint32_t timeout)
		此两个函数是需要额外寄存器的主机收发方式
		
		注意i2c_interface.h里相关API函数说明。

		注意地址的更改：B1X_ADDR为本机地址（无论主从），PCI_ADDR为外部对象地址（无论主从），在实际应用中请根据实际应用更改，将地址对应好   
