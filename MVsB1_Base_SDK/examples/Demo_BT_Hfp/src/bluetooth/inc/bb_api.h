
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
 * get rssi value
 **************************************************/
signed char GetRssiValue(void);

/***************************************************
 * get the company ID of the remote device 
 **************************************************/
unsigned short GetRemoteCompanyID(void);

/***************************************************
 * set rf tx power max level(max: 23 -- 8db)
 **************************************************/
void SetRfTxPwrMaxLevel(unsigned char level);

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

#endif //_BB_API_H_
