Example_I2c ������Ҫ��ʾ��I2C�Ļ�������


1��������������
	��Ӵ���С�壬TX/RX/GND ���� A25/A24/GND ���� (������256000)��
    
2��I2CĬ���������ã�ע�⽫������������������Ӳ�����ʱ���������裨10K��
	SDA 	GPIOB4
	SCL		GPIOB5

�����������ӻ��໥����255�����ݣ�00-FE������ɹ��������Ӧ�Ĵ�ӡ��Ϣ��ʾ�ɹ�

3�����ʹ��˵��:
	�ϵ�����֮��ֱ�ӽ������̣��ڴ��ڴ�ӡ���۲��ӡ��Ϣ��
	������ʾ���ڿ�֪��ǰ����ģʽΪ������ӻ���
	������ӻ��Ĳ���ģʽ�����̿�ͷ�궨��MASTER_SLAVE�������ӻ��Ƿ�ʹ���ж��շ��ɺ궨��SLAVE_INT����
	
����˵����
		�ͻ���ʹ��ʱ����Ҫ��ʹ�õ�I2C��������������������������Ӳ���Ͻ�IO����������
		��ʹ���շ�����ʱ��Ҫע�⣺
		I2C_MasterSendBuffer(uint8_t SlaveAddr, void* SendBuf, uint32_t BufLen, uint32_t timeout)
		I2C_MasterReceiveBuffer(uint8_t SlaveAddr, void* RecvBuf, uint32_t BufLen, uint32_t timeout)
		�����������ǲ���Ҫ����Ĵ����������շ���ʽ
		
		I2C_MasterSendData(uint8_t SlaveAddr, uint8_t RegAddr, void* SendBuf, uint32_t BufLen, uint32_t timeout)
		I2C_MasterReceiveData(uint8_t SlaveAddr, uint8_t RegAddr, void* RecvBuf, uint32_t BufLen, uint32_t timeout)
		��������������Ҫ����Ĵ����������շ���ʽ
		
		ע��i2c_interface.h�����API����˵����

		ע���ַ�ĸ��ģ�B1X_ADDRΪ������ַ���������ӣ���PCI_ADDRΪ�ⲿ�����ַ���������ӣ�����ʵ��Ӧ���������ʵ��Ӧ�ø��ģ�����ַ��Ӧ��   
