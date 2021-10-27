
#ifndef _BB_API_H_
#define _BB_API_H_


/***************************************************
 *
 **************************************************/
void Bt_init(void* params);

/***************************************************
 *
 **************************************************/
void rw_main(void);
void rwip_reset(void);

/***************************************************
 * get frequence offset value
 **************************************************/
signed char GetFreqOffsetValue(void);

/***************************************************
 * set bt frequence trim value
 * input: value (0x00~0x1d)
 **************************************************/
void BtSetFreqTrim(unsigned char value);

/***************************************************
 * set bt link supervision timeout
 * input: timeout: 0x0001~0xffff  (default:0x7D00 -- 20s)
 * link supervision timeout = time * 0.625ms (Range: 0.625ms - 40.9s)
 **************************************************/
void BtSetLinkSupervisionTimeout(unsigned int timeout);

/***************************************************
 * get rssi value
 **************************************************/
signed char GetRssiValue(void);

/***************************************************
 * get the company ID of the remote device 
 **************************************************/
unsigned short GetRemoteCompanyID(void);

/***************************************************
 * set rf tx power max level(max: 23 -- 8dbm)
 * set rf page tx power level(max: 23 -- 8dbm)
 **************************************************/
void SetRfTxPwrMaxLevel(unsigned char level, unsigned char pageLevel);

/***************************************************
 * get the rf chip id
 **************************************************/
unsigned short GetRfChipId(void);


/***************************************************
 * get the rf pll lock state
 * return: -1 = pll unlock(error)
 * 0 = ok
 **************************************************/
signed char GetRfPllLockState(void);

/***************************************************
 * rf power Up/Down
 **************************************************/
void RF_PowerDownByHw(void);
void RF_PowerUpBySw(void);
void RF_PowerDownBySw(void);

/***************************************************
 * sniff use Hosc/RC
 **************************************************/
void sniff_rc_init_set();//Rc初始化设置
void sniff_cntclk_set(int8_t set);//0:32K 1:32768Hz 蓝牙计数器的时钟设置

//需要用户自己实现的接口
void BtCntClkSet(void);

#endif //_BB_API_H_
