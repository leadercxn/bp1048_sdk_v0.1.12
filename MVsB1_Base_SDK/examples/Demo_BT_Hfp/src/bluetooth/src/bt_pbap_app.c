
#include "type.h"

#include "bt_manager.h"
#include "bt_pbap_api.h"
#include "bt_stack_api.h"
#include "bt_config.h"

#if (BT_PBAP_SUPPORT == ENABLE)
#include "string_convert.h"

enum {
	PHONE = 0,
	SIM1,
	SIM2,
};

uint8_t temp[1024];
char *vcard_begin = "N;CHARSET=UTF-8:";
char *vcard_end = "END:VCARD";
char *pb_name = "PRINTABLE:";

enum {
	PBAP_NONE = 0,
	PBAP_CONNECTED,
};

uint8_t pbapState = 0;

int MyStrToHex(char c)   
{   
    if(c>='0'&&c<='9')   
    {   
        return c-0x30;   
    }   
    else if(c>='a'&&c<='f')   
    {   
        return c-0x57;   
    }   
    else if(c>='A'&&c<='F')   
    {   
        return c-0x37;   
    }   
    return 0xfffff;   
} 

static uint16_t PhoneCnt=0;

void decode_name(uint8_t *buf)
{
	int i;
	int j =0;
	int k = 0;
	uint8_t tempbuf[64];
	int len = strlen(buf);
	for(i=0;i<len;i++)
	{
		if(buf[i] == '=')
		{
			k++;
			if(k==3)
			{
				break;
			}
		}
	}
	for(j=0;j<len;j++)
	{
		if(buf[j] == 0x0D)
		{
			break;
		}
	}
	memset(tempbuf,0,64);
	memcpy(tempbuf,buf+i,j-i);
	//printf("%d %d %d >>1:%s\n",i,k,j,tempbuf);
	i = 0;
	for(k=0;k<64;k++)
	{
		if(tempbuf[k] == '=')
		{
			tempbuf[i] = tempbuf[k];
			i++;
		}
		else if(MyStrToHex(tempbuf[k]) != 0xfffff)
		{
			tempbuf[i] = tempbuf[k];
			i++;
		}
		else
		{
			
		}
	}
	memset(tempbuf+i,0,64-i);
	k = j;
	j = 0;
	for (i = 0; i <strlen(tempbuf); i++)
	{
		int b = tempbuf[i];
		if (b == '=')
		{
			char u = MyStrToHex(tempbuf[++i]);
			char l = MyStrToHex(tempbuf[++i]);
			tempbuf[j] = (char) ((u << 4) + l);
		}
		else
		{
			tempbuf[j]  = b;
		}
		j++;
	}
	/*memset(tempbuf+j,0,64-j);
	{
		int len = StringConvert(tempbuf,64,tempbuf,strlen(tempbuf),UTF8_TO_GBK);
		memset(tempbuf+len,0,64-len);
		printf("C: %s>>>",tempbuf);
		printf("%s\n",buf+k+2);
	}*/
	
	PhoneCnt++;
	printf("%d\n",PhoneCnt);
}



void Data_Proess(uint8_t event,uint8_t *Buffer,uint32_t Length)
{
	int i =0;
	int j = 0;
	int k = 0;
	int start = 0;
	if(temp[0] != 0)
	{
		for(j=0;j<1024;j++)
		{
			if(temp[j] == 0)
			{
				break;
			}
		}
		i = 0;
		for(i=0;i<Length;i++)
		{
			if(memcmp(vcard_end,(uint8_t*)&Buffer[i],sizeof(vcard_end)) == 0)
			{
				memcpy(temp+j,Buffer,i);
				break;
			}
		}
		if(i==Length)
		{
			//printf("error %s %d\n",__FILE__,__LINE__);
		}
		/*{
			int len = StringConvert(temp,1024,temp,strlen(temp),UTF8_TO_GBK);
			memset(temp+len,0,1024-len);
			for(i=0;i<len-sizeof(pb_name);i++)
			{
				if(memcmp(pb_name,temp+i,strlen(pb_name)) == 0)
				{
					i = 1024;
					break;
				}
			}
			if(i==1024)
			{
				//printf(">>>>>>>>%s\n",temp);
				decode_name(temp);
			}
			else
			{
				printf("A: %s\n",temp);
				PhoneCnt++;
				printf("%d\n",PhoneCnt);
			}
		}*/
		
		//PhoneCnt++;
		//printf("%d\n",PhoneCnt);
	}
	memset(temp,0,1024);
	start = 0;
	i = 0;
	j =0;
	k = 0;
	while(1)
	{
		if(memcmp(vcard_begin,(uint8_t*)&Buffer[i],sizeof(vcard_begin)) == 0)
		{
			if(i)
			{
				if(Buffer[i-1] != 'F')
				{
					start = 1;
					memset(temp,0,1024);
				}
			}
		}
		else if(memcmp(vcard_end,(uint8_t*)&Buffer[i],sizeof(vcard_end)) == 0)
		{
			if(start)
			{
				/*int len = StringConvert(temp,1024,temp,strlen(temp),UTF8_TO_GBK);
				memset(temp+len,0,1024-len);
				for(k=0;k<len-sizeof(pb_name);k++)
				{
					if(memcmp(pb_name,temp+k,strlen(pb_name)) == 0)
					{
						k = 1024;
						break;
					}
				}
				if(k==1024)
				{
					decode_name(temp);
				}
				else
				{
					printf("B: %s\n",temp);
					
					PhoneCnt++;
					printf("%d\n",PhoneCnt);
				}*/
				
				//PhoneCnt++;
				//printf("%d\n",PhoneCnt);
			}
			start = 0;
			j = 0;
			memset(temp,0,1024);
		}
		if(start)
		{
			temp[j] = Buffer[i];
			j++;
		}
		i++;
		if(i>=Length)
		{
			break;
		}
	}
}
/*
void BtPbapEvent(uint32_t event, BT_STACK_CALLBACK_PARAMS * param)
{
	switch(event)
	{
		case BT_STACK_EVENT_PBAP_CONNECTED:
			printf("PBAP EVENT:connected\n");
		break;
		
		case BT_STACK_EVENT_PBAP_DISCONNECTED:
			printf("PBAP EVENT:disconnect\n");
		break;
		
		default:
		break;
	}
}
*/
void PbapCbFunc(const PbapCallbackInfo *Info)
{
	switch(Info->event)
	{
		case PBAP_CONNECT_START:
			DBG("rfcomm open ok.pbap start connect\n");
			break;

		case PBAP_CONNECT_OK:
			DBG("pbap connect ok\n");
			memset(temp,0,1024);

			pbapState = PBAP_CONNECTED;
			break;

		case PBAP_CONNECT_ERROR:
			DBG("pbap connect error\n");
			break;

		case PBAP_CLOSE_OK:
			DBG("pbap close ok\n");
			
			pbapState = PBAP_NONE;
			break;

		case PBAP_CLOSE_START:
			DBG("pbap close start\n");
			break;

		case PBAP_DATA_START:
			Data_Proess(Info->event,Info->buffer,Info->length);
			//DBG("pbap data start\nsize:%d\n%s\n",Length,Buffer);
			DBG("pbap data start size:%d, [%x]\n",Info->length,Info->buffer[0]);
			break;

		case PBAP_DATA:
			Data_Proess(Info->event,Info->buffer,Info->length);
			//DBG("size:%d\n%s\n",Length,Buffer);
			DBG("pbap data size:%d, [%x]\n",Info->length,Info->buffer[0]);
			
			//PhoneCnt++;
			//printf("%d\n",PhoneCnt);
			break;

		case PBAP_DATA_SINGLE:
			Data_Proess(Info->event,Info->buffer,Info->length);
			//DBG("size:%d\n%s\n",Length,Buffer);
			DBG("pbap data single size:%d, [%x]\n",Info->length,Info->buffer[0]);
			
			//PhoneCnt++;
			//printf("%d\n",PhoneCnt);
			break;

		case PBAP_DATA_END:
			Data_Proess(Info->event,Info->buffer,Info->length);
			//memset(temp,0,1024);
			//DBG("size:%d\n%s\n",Length,Buffer);
			DBG("pbap data end size:%d, [%x]\n",Info->length,Info->buffer[0]);
			break;

		case PBAP_DATA_END1:
			Data_Proess(Info->event,Info->buffer,Info->length);
			//memset(temp,0,1024);
			//DBG("size:%d\n%s\n",Length,Buffer);
			DBG("pbap data end1 size:%d\n",Info->length);
			break;

		case PBAP_NOT_ACCEPTABLE:
			DBG("PBAP: not acceptable\n");
			break;

		case PBAP_NOT_FOUND:
			DBG("PBAP: not found\n");
			break;

 		case PBAP_PACKET_END:
			DBG("PBAP Received ok\n");
 			break;
	}
}

void BtConnectPbap(void)
{
	PBAPConnect();
}

#if 0
void BtConnectPbap(uint8_t *addr)
{
/*	uint8_t bdAddr[6];
//PTS DONGLE
//00:1B:DC:08:E3:E6
	bdAddr[5] = 0x00;
	bdAddr[4] = 0x1B;
	bdAddr[3] = 0xDC;
	bdAddr[2] = 0x08;
	bdAddr[1] = 0xE3;
	bdAddr[0] = 0xE6;
*/
	//do_pbap_open(1, bdAddr);
	do_pbap_open(1, addr);
}
#endif

//only BV-04-01
void BtConnectPbapBV0401(void)
{
	uint8_t bdAddr[6];
//PTS DONGLE
//00:1B:DC:08:E3:E6
	bdAddr[5] = 0x00;
	bdAddr[4] = 0x1B;
	bdAddr[3] = 0xDC;
	bdAddr[2] = 0x08;
	bdAddr[1] = 0xE3;
	bdAddr[0] = 0xE6;

	do_pbap_open(0, bdAddr);
}

//only BV-09-C
void BtConnectPbapBV09C(void)
{
	uint8_t bdAddr[6];
//PTS DONGLE
//00:1B:DC:08:E3:E6
	bdAddr[5] = 0x00;
	bdAddr[4] = 0x1B;
	bdAddr[3] = 0xDC;
	bdAddr[2] = 0x08;
	bdAddr[1] = 0xE3;
	bdAddr[0] = 0xE6;

	do_pbap_open(0x10, bdAddr);
}

void BtDisconnectPbap(void)
{
	PBAPDisconnect();
}

//获取SIM卡上电话簿信息 
void GetSim1CardPhoneBook(void)
{
	/*if(pbapState != PBAP_CONNECTED)
	{
		DBG("PBAP Disconnect\n");
		return;
	}*/
		
	DBG("sim1 card:\n");
	PhoneCnt = 0;
	PBAP_PullPhoneBook(SIM1, "pb.vcf", 0x83, 1);
}

void GetSim2CardPhoneBook(void)
{
	if(pbapState != PBAP_CONNECTED)
	{
		DBG("PBAP Disconnect\n");
		return;
	}

	DBG("sim2 card:\n");
	PhoneCnt = 0;
	PBAP_PullPhoneBook(SIM2, "pb.vcf", 0x83, 1);
}

//获取手机自身电话簿信息 
void GetMobilePhoneBook(void)
{
	/*if(pbapState != PBAP_CONNECTED)
	{
		DBG("PBAP Disconnect\n");
		return;
	}*/
	
	//DBG("phone book:\n");
	PhoneCnt = 0;
	PBAP_PullPhoneBook(PHONE, "pb.vcf", 0x83, 1);
}

void GetMobilePhoneBook_BV06C(void)
{
	DBG("phone book:\n");
	PhoneCnt = 0;
	PBAP_PullPhoneBook(PHONE, "pb.vcf", 0x83, 0x21);
}

//BI-01-C
void GetMobilePhoneBook_BI01C(void)
{
	//GET(0x83),no need continue
	DBG("phone book:\n");
	PhoneCnt = 0;
	PBAP_PullPhoneBook(PHONE, "pb.vcf", 0x83, 0);
}

//获取呼入电话信息 
void GetIncomingCallBook(void)
{
	if(pbapState != PBAP_CONNECTED)
	{
		DBG("PBAP Disconnect\n");
		return;
	}
		
	DBG("Incoming Call:\n");
	PBAP_PullPhoneBook(PHONE, "ich.vcf", 0x83, 1);
}

//获取呼出电话簿信息 
void GetOutgoingCallBook(void)
{
	if(pbapState != PBAP_CONNECTED)
	{
		DBG("PBAP Disconnect\n");
		return;
	}
		
	DBG("Outgoing Call:\n");
	PBAP_PullPhoneBook(PHONE, "och.vcf", 0x83, 1);
}

//获取未接电话簿信息 
void GetMissedCallBook(void)
{
	if(pbapState != PBAP_CONNECTED)
	{
		DBG("PBAP Disconnect\n");
		return;
	}
		
	DBG("Missed Call:\n");
	PBAP_PullPhoneBook(PHONE, "mch.vcf", 0x83, 1);
}

void GetCombinedCallBook(void)
{
	if(pbapState != PBAP_CONNECTED)
	{
		DBG("PBAP Disconnect\n");
		return;
	}
		
	DBG("Combined Call:\n");
	PBAP_PullPhoneBook(PHONE, "cch.vcf", 0x83, 1);
}


//BV-02-I
/*void PullvCardEntry(void)
{
	PBAP_SendSetPath();
}
*/

//BTStackInitPbap(PbapCbFunc);

#endif

