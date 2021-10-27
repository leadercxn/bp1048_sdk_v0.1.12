
#include "type.h"
#include "debug.h"
#include "bt_config.h"
#include "bt_stack_api.h"
#include "string_convert.h"
#include "bt_pbap_api.h"
#include "bt_manager.h"

#if (BT_PBAP_SUPPORT == ENABLE)

//显示PBAP接收的数据
//#define PBAP_INFO_DEBUG

#ifdef PBAP_INFO_DEBUG
uint8_t temp[1024];
char *vcard_begin = "N;CHARSET=UTF-8:";
char *vcard_end = "END:VCARD";
char *pb_name = "PRINTABLE:";
#endif
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static void SetPbapState(BT_PBAP_STATE state)
{
	GetBtManager()->pbapState = state;
}

BT_PBAP_STATE GetPbapState(void)
{
	return GetBtManager()->pbapState;
}

/////////////////////////////////////////////////////////////////////////////////
#ifdef PBAP_INFO_DEBUG
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
#endif


void PbapDataProcess(uint8_t *Buffer,uint32_t Length)
{
#ifdef PBAP_INFO_DEBUG
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
#endif
}

/////////////////////////////////////////////////////////////////////////////////
//callback
void BtPbapCallback(BT_PBAP_CALLBACK_EVENT event, BT_PBAP_CALLBACK_PARAMS * param)
{
	switch(event)
	{
		case BT_STACK_EVENT_PBAP_CONNECTING:
			//APP_DBG("rfcomm open ok.pbap start connect\n");
			break;

		case BT_STACK_EVENT_PBAP_CONNECTED:
			APP_DBG("PBAP EVENT:connected\n");
			SetPbapState(BT_PBAP_STATE_CONNECTED);
		#ifdef PBAP_INFO_DEBUG
			memset(temp,0,1024);
		#endif
			break;

		case BT_STACK_EVENT_PBAP_CONNECT_ERROR:
			APP_DBG("PBAP EVENT:connect error\n");
			break;

		case BT_STACK_EVENT_PBAP_DISCONNECT:
			APP_DBG("PBAP EVENT:disconnect \n");
			SetPbapState(BT_PBAP_STATE_NONE);
			break;

		case BT_STACK_EVENT_PBAP_DISCONNECTING:
			APP_DBG("PBAP EVENT:diconnecting \n");
			break;

		case BT_STACK_EVENT_PBAP_DATA_START:
			PbapDataProcess(param->buffer,param->length);
			APP_DBG("pbap data start size:%d, [%x]\n",param->length,param->buffer[0]);
			break;

		case BT_STACK_EVENT_PBAP_DATA:
			PbapDataProcess(param->buffer,param->length);
			APP_DBG("pbap data size:%d, [%x]\n",param->length,param->buffer[0]);
			break;

		case BT_STACK_EVENT_PBAP_DATA_SINGLE:
			PbapDataProcess(param->buffer,param->length);
			APP_DBG("pbap data single size:%d, [%x]\n",param->length,param->buffer[0]);
			break;

		case BT_STACK_EVENT_PBAP_DATA_END:
			PbapDataProcess(param->buffer,param->length);
			APP_DBG("pbap data end size:%d, [%x]\n",param->length,param->buffer[0]);
			break;

		case BT_STACK_EVENT_PBAP_DATA_END1:
			PbapDataProcess(param->buffer,param->length);
			APP_DBG("pbap data end1 size:%d\n",param->length);
			break;

		case BT_STACK_EVENT_PBAP_NOT_ACCEPTABLE:
			APP_DBG("PBAP: not acceptable\n");
			break;

		case BT_STACK_EVENT_PBAP_NOT_FOUND:
			APP_DBG("PBAP: not found\n");
			break;

 		case BT_STACK_EVENT_PBAP_PACKET_END:
			APP_DBG("PBAP Received ok\n");
 			break;
	}
}


/////////////////////////////////////////////////////////////////////////////////
//获取SIM卡上电话簿信息 
void GetSim1CardPhoneBook(void)
{
	if(GetPbapState() != BT_PBAP_STATE_CONNECTED)
		return;
	
	APP_DBG("sim1 card:\n");
	//PhoneCnt = 0;
	PBAP_PullPhoneBook(SIM1, "pb.vcf");
}

void GetSim2CardPhoneBook(void)
{
	if(GetPbapState() != BT_PBAP_STATE_CONNECTED)
		return;
	
	APP_DBG("sim2 card:\n");
	//PhoneCnt = 0;
	PBAP_PullPhoneBook(SIM2, "pb.vcf");
}

//获取手机自身电话簿信息 
void GetMobilePhoneBook(void)
{
	if(GetPbapState() != BT_PBAP_STATE_CONNECTED)
		return;
	
	APP_DBG("phone book:\n");
	//PhoneCnt = 0;
	PBAP_PullPhoneBook(PHONE, "pb.vcf");
}

//获取呼入电话信息 
void GetIncomingCallBook(void)
{
	if(GetPbapState() != BT_PBAP_STATE_CONNECTED)
		return;
	
	APP_DBG("Incoming Call:\n");
	PBAP_PullPhoneBook(PHONE, "ich.vcf");
}

//获取呼出电话簿信息 
void GetOutgoingCallBook(void)
{
	if(GetPbapState() != BT_PBAP_STATE_CONNECTED)
		return;
	
	APP_DBG("Outgoing Call:\n");
	PBAP_PullPhoneBook(PHONE, "och.vcf");
}

//获取未接电话簿信息 
void GetMissedCallBook(void)
{
	if(GetPbapState() != BT_PBAP_STATE_CONNECTED)
		return;
	
	APP_DBG("Missed Call:\n");
	PBAP_PullPhoneBook(PHONE, "mch.vcf");
}

void GetCombinedCallBook(void)
{
	if(GetPbapState() != BT_PBAP_STATE_CONNECTED)
		return;
	
	APP_DBG("Combined Call:\n");
	PBAP_PullPhoneBook(PHONE, "cch.vcf");
}

#endif

