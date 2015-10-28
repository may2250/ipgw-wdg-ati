
#include <string.h>
#include <stdio.h>

#include "cmd.h"

#include "tcpClient.h"

#include <stdlib.h>

#include <sys/stat.h>





/*StEnc_db_t gWebEncDb;
StEnc_db_outts_t gWebTsOut;
StNetParam_t gWebNet;

StDatabase_dtmb_tx_t gWebDtmbTx;
StNetParam_t gWebDtmbNet;
StStatus_dtmb_t gWebDtmbStatus;*/

#define CMD_HEAD_1 0x77
#define CMD_HEAD_2 0x6C


u32_t Cmd_getIntParam(const u8_t *pParam, int paramLen)
{
	u32_t tmpU32 = 0;
	int i;
	
	for(i = 0; i < paramLen && i < 4; i++)
	{
		tmpU32 <<= 8;
		tmpU32 |= pParam[i];
	}
	return tmpU32;
}


int Cmd_rwParam(u8_t *pParam, int paramLen, u8_t *pOut, int maxLen, int minLen)
{
	int rslt;

	//rslt = TcpClt_writeRead(pParam, paramLen, pOut, maxLen);
	rslt=UDP_Clt_writeRead(pParam, paramLen, pOut, maxLen);
	if(rslt >= minLen)
	{
		return rslt;
	}
	return 0;
}


StChannel_t gChn;

void Cmd_SearchInput(cmd_t chnId)
{
	int k=1;
	memset(&gChn, 0, sizeof(gChn));
	u8_t cmdSearchPrg[] = {CMD_HEAD_1, CMD_HEAD_2, 0x11, 0x1, chnId, 0};
	u8_t cmdRdSearch[] =  {CMD_HEAD_1, CMD_HEAD_2, 0x11, 0x0, chnId, 0};
	u8_t rdBuf[50];
   memset(rdBuf,0,50);
	 
	if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
		return;
    if(Cmd_rwParam(cmdSearchPrg, 6, rdBuf, 50, 4) < 4)/*搜索节目*/
    	{
		TcpClt_close();
		return;
    	}
	else
	{
		if(rdBuf[0]=0x77&&rdBuf[1]==0x6c)
		{
			k=500;
			do{
				   if(Cmd_rwParam(cmdRdSearch, 6, rdBuf, 50, 4) < 4)
				   	{
				   		k=0;
						TcpClt_close();
						return;
				   	}
					 usleep(100000);
					 k--;
				}
			   while((k>0)&&(rdBuf[5]==0x01));
		}
		else
		{
			TcpClt_close();
			return;
		}
	}
	TcpClt_close();
}
void dev_channel_save(char *buf,int len,char *file_name)
{
	FILE *fp;
	fp =fopen(file_name,"wb+");
	if(fp != NULL)
	{
		fwrite((unsigned char *)buf,1,len,fp);
		fflush(fp);
		fclose(fp);
	}
}

unsigned char Cmd_getchannel_pm(cmd_t chnId)
{
	unsigned char rdBuf[10];
	u8_t cmdGetPrg[] = {CMD_HEAD_1, CMD_HEAD_2, 0x11, 0x2, chnId, 0};
	
	if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
		return;
	
	if(Cmd_rwParam(cmdGetPrg, 6, rdBuf, 10, 6) < 6)
		{
			TcpClt_close();
			 return 0;	
		}
	TcpClt_close();
	return  rdBuf[6] < EACH_CHN_PRG_MAXCNT ? rdBuf[6] : EACH_CHN_PRG_MAXCNT;
}
cmd_t Cmd_GetChannel_Rate(cmd_t chnId)
{
	u8_t send_buf[5];
	u8_t read_buf[10];
    cmd_t rate=0;
		
	if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
		return 0 ;

	send_buf[0]=0x77;
	send_buf[1]=0x6c;
	send_buf[2]=0x21;
	send_buf[3]=chnId;
	send_buf[4]=0x03;
	if(Cmd_rwParam(send_buf, 5, read_buf, 10, 9) < 9)
		{
			return 0 ;
			TcpClt_close();
		}
	rate = read_buf[5];
	rate |= (cmd_t)read_buf[6]<<8;
	rate |= (cmd_t)read_buf[7]<<16;
	rate |= (cmd_t)read_buf[8]<<24;
	TcpClt_close();
	return rate;
		
}
u8_t Cmd_GetChannel_Status()
{
	u8_t send_buf[5];
	u8_t read_buf[10];
    u8_t status=0;
		
	if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
		return 0 ;

	send_buf[0]=0x77;
	send_buf[1]=0x6c;
	send_buf[2]=0x11;
	send_buf[3]=0x04;
	if(Cmd_rwParam(send_buf, 4, read_buf, 10, 4) < 4)
		{
			return 0 ;
			TcpClt_close();
		}
	status = read_buf[4];
	TcpClt_close();
	return status;
		
}

void Cmd_getChannel(cmd_t chnId,cmd_t sp)
{
	int i;
	char pFileName[200];
	FILE * fp;
	struct stat stat_tmp;
	int sLen=0;
	int cnt;
     
	memset(&gChn, 0, sizeof(gChn));
	u8_t cmdGetPrg[] = {CMD_HEAD_1, CMD_HEAD_2, 0x11, 0x2, chnId, 0};
	u8_t cmdNGetPrg[] = {CMD_HEAD_1, CMD_HEAD_2, 0x24, chnId, 1, 1,1};
	u8_t rdBuf[4096];
	int desCnt, streamCnt;
	u8_t *pb;
	u8_t des_len;

	if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
		return;
	
	if(Cmd_rwParam(cmdGetPrg, 6, rdBuf, 1024, 7) < 7)
		goto LINE_ERR_END;
	gChn.serviceCnt = rdBuf[6] < EACH_CHN_PRG_MAXCNT ? rdBuf[6] : EACH_CHN_PRG_MAXCNT;
	for(i = 0; i < gChn.serviceCnt; i++)
	{
		cmdGetPrg[5] = (u8_t)(i+1);
		if(Cmd_rwParam(cmdGetPrg, 6, rdBuf, 1024, 6+18) < (6+18))
		{
			gChn.serviceCnt = 0;
			goto LINE_ERR_END;
		}
		gChn.service[i].chnId = chnId;
		gChn.service[i].isMuxed = FALSE;
		pb = rdBuf + 6;
		gChn.service[i].prgId = B2W(pb);
		pb += 2;
		pb++; // channel_id
		pb+=2; // stream_id
		pb+=2; // org-Network_id
		gChn.service[i].pmtPid = B2W(pb);
		pb += 2;
		pb+=2; // pcr_pid
		desCnt = *pb++;
		if(desCnt>0)
		{
			while(desCnt--)
			{
				pb++; // des_flag
				des_len=*pb++;
				pb += des_len;
				
			}
		}
		streamCnt = *pb++; // av_stream
		if(streamCnt>0)
		{
			while(streamCnt--)
			{
				pb++; //  stream_type
				pb+=2; // stream_pid
				desCnt = *pb++;
				if(desCnt>0)
				{
					while(desCnt--)
					{
						pb++; // des_flag
						des_len=*pb++;
						pb += des_len;
					}
				}
			}
		}
		
		desCnt = *pb < NAME_MAXLEN ? *pb : NAME_MAXLEN; // program_name
		gChn.service[i].name_len=desCnt;
		memcpy(gChn.service[i].name, pb+1, desCnt);
		gChn.service[i].name[desCnt] = '\0';
		pb += *pb + 1;

		desCnt = *pb < NAME_MAXLEN ? *pb : NAME_MAXLEN; // provider_name
		gChn.service[i].provider_len=desCnt;
		memcpy(gChn.service[i].provider, pb+1, desCnt);
		gChn.service[i].provider[desCnt] = '\0';
		pb += *pb + 1;

		gChn.service[i].isCA = *pb > 0;
		pb++;
		// bypass last bytes
	}
	if(sp==0)
	{
		 if( gChn.serviceCnt==0)
			goto LINE_ERR_END;
		//if(Cmd_rwParam(cmdNGetPrg, 7, rdBuf, 4096, 7) < 7)
		if(UDP_Clt_writeRead(cmdNGetPrg, 7, rdBuf, 4096, 7) < 7)	
		goto LINE_ERR_END;
		  sLen=rdBuf[7];
			if(sLen!=0&&(sLen== gChn.serviceCnt))
			{
				cnt=8;
			for(i = 0; i < gChn.serviceCnt; i++)
				{
					gChn.service_o[i].name_len= rdBuf[cnt++];
					memcpy(gChn.service_o[i].name, &rdBuf[cnt], gChn.service_o[i].name_len);cnt+= gChn.service_o[i].name_len;
					gChn.service_o[i].provider_len= rdBuf[cnt++];
					memcpy(gChn.service_o[i].provider,& rdBuf[cnt], gChn.service_o[i].provider_len);cnt+= gChn.service_o[i].provider_len;
				}
			}
			else
			{
				goto LINE_ERR_END;
			}
		}
	else if(sp==1)
		{	
		     if( gChn.serviceCnt==0)
			goto LINE_ERR_END;
			if(UDP_Clt_writeRead(cmdNGetPrg, 7, rdBuf, 4096, 7) < 7)	
		goto LINE_ERR_END;
             sLen=rdBuf[7];
			if(sLen!=0&&(sLen== gChn.serviceCnt))
			{
				cnt=8;
				for(i = 0; i < gChn.serviceCnt; i++)
					{
						gChn.service_o[i].name_len= rdBuf[cnt++];
						memcpy(gChn.service_o[i].name, &rdBuf[cnt], gChn.service_o[i].name_len);cnt+= gChn.service_o[i].name_len;
						gChn.service_o[i].provider_len= rdBuf[cnt++];
						memcpy(gChn.service_o[i].provider,& rdBuf[cnt], gChn.service_o[i].provider_len);cnt+= gChn.service_o[i].provider_len;
					}
				for(i=0;i<gChn.serviceCnt;i++)
				{
					desCnt=0;
					sprintf(pFileName,CHANNEL_PARAM,chnId,i);
					memset(rdBuf,0,1024);
					
					rdBuf[desCnt++]=gChn.service_o[i].name_len;
					for(streamCnt=0;streamCnt<gChn.service_o[i].name_len;streamCnt++)
					{rdBuf[desCnt++]=gChn.service_o[i].name[streamCnt];}
				
					rdBuf[desCnt++]=gChn.service_o[i].provider_len;
					for(streamCnt=0;streamCnt<gChn.service_o[i].provider_len;streamCnt++)
					{rdBuf[desCnt++]=gChn.service_o[i].provider[streamCnt];}
					dev_channel_save(rdBuf,desCnt,pFileName);
				}
			}
			else if(gChn.serviceCnt!=0)
			{
				memset(&gChn.service_o, 0, sizeof(gChn.service_o));
				for(i=0;i<gChn.serviceCnt;i++)
					{
						desCnt=0;
						sprintf(pFileName,CHANNEL_PARAM,chnId,i);
						memset(rdBuf,0,1024);
						rdBuf[desCnt++]=gChn.service_o[i].name_len;
						for(streamCnt=0;streamCnt<gChn.service_o[i].name_len;streamCnt++)
						{rdBuf[desCnt++]=gChn.service_o[i].name[streamCnt];}
					
						rdBuf[desCnt++]=gChn.service_o[i].provider_len;
						for(streamCnt=0;streamCnt<gChn.service_o[i].provider_len;streamCnt++)
						{rdBuf[desCnt++]=gChn.service_o[i].provider[streamCnt];}
						dev_channel_save(rdBuf,desCnt,pFileName);
						}
				goto LINE_ERR_END;
			}
	}
	
LINE_ERR_END:
	TcpClt_close();
//#endif	
	
}

 unsigned char Cmd_setChannel(cmd_t chnId)
{
 int len,i;
 u8_t *send_buf; 
 u8_t rdbuf[10];
 
		 if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
			return 0;
	
			len=8;
			for(i=0;i<gChn.serviceCnt;i++)
			{
			len+=gChn.service_o[i].name_len+gChn.service_o[i].provider_len+2;
			}
			
			send_buf=(char*)calloc(1,len);
			
			send_buf[0]=0x77;send_buf[1]=0x6c;send_buf[2]=0x24;send_buf[3]=chnId;send_buf[4]=0x01;send_buf[5]=0x01;send_buf[6]=0x02;send_buf[7]=gChn.serviceCnt;
			len=8;
			for(i=0;i<gChn.serviceCnt;i++)
			{
				send_buf[len++]=gChn.service_o[i].name_len;
				if(gChn.service_o[i].name_len>0)
				memcpy(&send_buf[len],gChn.service_o[i].name,gChn.service_o[i].name_len);
				len+=gChn.service_o[i].name_len;
				send_buf[len++]=gChn.service_o[i].provider_len;
				if(gChn.service_o[i].provider_len>0)
				memcpy(&send_buf[len],gChn.service_o[i].provider,gChn.service_o[i].provider_len);
				len+=gChn.service_o[i].provider_len;
			}
			if(Cmd_rwParam(send_buf, len, rdbuf, 10, 5) < 5)
			{
				TcpClt_close();
				free(send_buf);
				return 1;
			}
			else{
					TcpClt_close();
					free(send_buf);
						return 2;
				} 

}
void Cmd_getChannelOutput(cmd_t chnId,int Index )
{
	int i;
	char pFileName[200];
	FILE * fp;
	struct stat stat_tmp;
	char buffer[1024];
	int sLen=0;
     
	

    sprintf(pFileName,CHANNEL_PARAM,chnId,Index);
	stat(pFileName,&stat_tmp);
	sLen = stat_tmp.st_size;
		
	fp = fopen(pFileName, "rb+");
	if (fp == NULL)
	{
		sLen=0;
	}
	else
	{
		fread((unsigned char *)buffer,1,sLen,fp);
		fclose(fp);
	}
    if(sLen>0)
    	{
    			gChn.service_o[Index].name_len=buffer[0];
			memcpy(gChn.service_o[Index].name,&buffer[1],gChn.service_o[Index].name_len);

			gChn.service_o[Index].provider_len=buffer[ buffer[0]+1];
			memcpy(gChn.service_o[Index].provider,&buffer[buffer[0]+2],gChn.service_o[Index].provider_len);
    	}


}

void Cmd_getOutput()
{
	int i, chnId;
	char pFileName[200];
	FILE * fp;
	struct stat stat_tmp;
	char buffer[1024];
	int sLen;
	
	#ifdef PC_OS
		sprintf(pFileName,"/mnt/hgfs/wanlong/devWeb_1502100100/cgi-bin/Out_Program");

	
	memset(&gChn, 0, sizeof(gChn));

	
	stat(pFileName,&stat_tmp);
	sLen = stat_tmp.st_size;
		
	fp = fopen(pFileName, "rb");
	if (fp == NULL)
	{
		sLen=99;
	}
	else
	{
		fread((unsigned char *)buffer,1,sizeof(buffer),fp);
		fclose(fp);
	}


	gChn.serviceCnt = 2;

	gChn.service[0].isMuxed = 1;
	chnId =1;
	gChn.service[i].chnId = (u16_t)(chnId);
	gChn.service[i].prgId =sLen;
	gChn.service[i].pmtPid = 0x300;
	sprintf(gChn.service[i].name,buffer);
	sprintf(gChn.service[i].provider,pFileName);
#else
	
#endif	

}


void Cmd_getPrograme(cmd_t chnId, cmd_t prgIndex)
{
	int i;
	
	memset(&gChn, 0, sizeof(gChn));

//#ifdef PC_OS
	for(i = 0; i<EACH_CHN_PRG_MAXCNT; i++)
	{
		if(prgIndex != i+1)
			continue;
		gChn.service[i].isMuxed = i&1;
		gChn.service[i].prgId = (u16_t)(10*chnId + i+1);
		gChn.service[i].pmtPid = (u16_t)(10*chnId + i+0x101);
		sprintf(gChn.service[i].name, "CCTV-%d", 10*chnId + i+1);
		sprintf(gChn.service[i].provider, "China-%d", 10*chnId + i+1);
	}
//#else
	
//#endif	
}

void Cmd_programe(cmd_t chnId, cmd_t prgIndex)
{
cmd_t desCnt,cnt;
char pFileName[100];
unsigned char buf[1024];

		   desCnt=0;	
		    sprintf(pFileName,CHANNEL_PARAM,chnId,prgIndex);
			memset(buf,0,1024);
			
			buf[desCnt++]=gChn.service[prgIndex].name_len;
			for(cnt=0;cnt<gChn.service[prgIndex].name_len;cnt++)
			{buf[desCnt++]=gChn.service[prgIndex].name[cnt];}
			

			
			buf[desCnt++]=gChn.service[prgIndex].provider_len;
			for(cnt=0;cnt<gChn.service[prgIndex].provider_len;cnt++)
			{buf[desCnt++]=gChn.service[prgIndex].provider[cnt];}

			dev_channel_save(buf,desCnt,pFileName);
}



u8_t Cmd_getNet()
{
	memset(&gChn, 0, sizeof(gChn));
	
	u8_t cmdGetIp[] =       				 {CMD_HEAD_1, CMD_HEAD_2, 0X02, 0X01, 0X01};
	u8_t cmdGetMask[] =       		 {CMD_HEAD_1, CMD_HEAD_2, 0X02, 0X02, 0X01};
	u8_t cmdGetGateway[] =        {CMD_HEAD_1, CMD_HEAD_2, 0X02, 0X03, 0X01};
	u8_t cmdGetmac[] =       			 {CMD_HEAD_1, CMD_HEAD_2, 0X02, 0X04, 0X01};
	
	u8_t rdBuf[20];
	

	if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
		return 0;
    if(Cmd_rwParam(cmdGetIp, 6, rdBuf, 20, 5) < 5)/*获取IP地址*/
    	{
		TcpClt_close();
		return 0;
    	}
	else
	{
		gChn.my_ip[0]=rdBuf[5];
		gChn.my_ip[1]=rdBuf[6];
		gChn.my_ip[2]=rdBuf[7];
		gChn.my_ip[3]=rdBuf[8];
		 if(Cmd_rwParam(cmdGetMask, 6, rdBuf, 20, 5) < 5)/*获取MASK地址*/
	    	{
			TcpClt_close();
			return 0;
	    	}
		 else
		 {
		 	gChn.my_mask[0]=rdBuf[5];
			gChn.my_mask[1]=rdBuf[6];
			gChn.my_mask[2]=rdBuf[7];
			gChn.my_mask[3]=rdBuf[8];
		 }
		  if(Cmd_rwParam(cmdGetGateway, 6, rdBuf, 20, 5) < 5)/*获取Gateway地址*/
		    	{
				TcpClt_close();
				return 0;
		    	}
			 else
			 {
			 	gChn.my_gateway[0]=rdBuf[5];
				gChn.my_gateway[1]=rdBuf[6];
				gChn.my_gateway[2]=rdBuf[7];
				gChn.my_gateway[3]=rdBuf[8];
			 }
			if(Cmd_rwParam(cmdGetmac, 6, rdBuf, 20, 5) < 5)/*获取Gateway地址*/
		    	{
				TcpClt_close();
				return 0;
		    	}
			 else
			 {
			 	gChn.my_mac[0]=rdBuf[5];
				gChn.my_mac[1]=rdBuf[6];
				gChn.my_mac[2]=rdBuf[7];
				gChn.my_mac[3]=rdBuf[8];
				gChn.my_mac[4]=rdBuf[9];
				gChn.my_mac[5]=rdBuf[10];
			 }
	}
	TcpClt_close();
	return 1;
}

u8_t Cmd_submitNet()
{
	
	u8_t cmdSetIp[] ={CMD_HEAD_1, CMD_HEAD_2, 0x02,0x01,0x02,gChn.my_ip[0], gChn.my_ip[1], gChn.my_ip[2], gChn.my_ip[3]};
	u8_t cmdSetMask[] ={CMD_HEAD_1, CMD_HEAD_2, 0x02,0x02,0x02,gChn.my_mask[0], gChn.my_mask[1], gChn.my_mask[2], gChn.my_mask[3]};
	u8_t cmdSetGateway[] ={CMD_HEAD_1, CMD_HEAD_2, 0x02,0x03,0x02,gChn.my_gateway[0], gChn.my_gateway[1], gChn.my_gateway[2], gChn.my_gateway[3]};
	u8_t rdBuf[20];
	
	if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
	return 0;
	if(Cmd_rwParam(cmdSetGateway,9, rdBuf, 20, 5) < 5)/*设置Gateway地址*/
	{
		TcpClt_close();
		return 1;
	}
	if(Cmd_rwParam(cmdSetMask,9, rdBuf, 20, 5) < 5)/*设置Mask地址*/
	{
		TcpClt_close();
		return 2;
	}
	if(Cmd_rwParam(cmdSetIp,9, rdBuf, 20, 5) < 5)/*设置IP地址*/
	{
		TcpClt_close();
		return 3;
	}
	return 3;

}
u8_t Cmd_getTS2IP()
{
	u8_t ch ,i;
	memset(&gChn, 0, sizeof(gChn));
	
	u8_t rdBuf[20];
	u8_t wrBuf[20];
	
	if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
		return 0;
	wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf1;wrBuf[3]=0x01;wrBuf[4]=0x01;wrBuf[5]=0x01;
    if(Cmd_rwParam(wrBuf, 6, rdBuf, 20, 5) < 5)/*获取SU_IP地址*/
    	{
		TcpClt_close();
		return 0;
    	}
	else
	{
		gChn.su_ip[0]=rdBuf[6];
		gChn.su_ip[1]=rdBuf[7];
		gChn.su_ip[2]=rdBuf[8];
		gChn.su_ip[3]=rdBuf[9];
		wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf1;wrBuf[3]=0x01;wrBuf[4]=0x02;wrBuf[5]=0x01;
		 if(Cmd_rwParam(wrBuf, 6, rdBuf, 20, 5) < 5)/*获取SU_MAC*/
	    	{
			TcpClt_close();
			return 0;
	    	}
		 else
		 {
		 	gChn.su_mac[0]=rdBuf[6];
			gChn.su_mac[1]=rdBuf[7];
			gChn.su_mac[2]=rdBuf[8];
			gChn.su_mac[3]=rdBuf[9];
			gChn.su_mac[4]=rdBuf[10];
			gChn.su_mac[5]=rdBuf[11];
		 }
		 wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf1;wrBuf[3]=0x01;wrBuf[4]=0x03;wrBuf[5]=0x01;
		  if(Cmd_rwParam(wrBuf, 6, rdBuf, 20, 5) < 5)/*获取SU_PORT*/
		    	{
				TcpClt_close();
				return 0;
		    	}
			 else
			 {
			 	gChn.su_port=rdBuf[6];
				gChn.su_port|=(u16_t)rdBuf[7]<<8;
			 }
			 wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf2;wrBuf[3]=0x00;wrBuf[4]=0x02;wrBuf[5]=0x01;
			if(Cmd_rwParam(wrBuf, 6, rdBuf, 20, 5) < 5)/*获取UDP_TTL*/
		    	{
				TcpClt_close();
				return 0;
		    	}
			 else
			 {
			 	gChn.udp_ttl=rdBuf[6];

			 }
			 i=1;
			 for(ch=1;ch<9;ch++)/*获取目的参数，共8个通道*/
			 {
			 
			 	 wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf2;wrBuf[3]=ch;wrBuf[4]=0x01;wrBuf[5]=0x01;
				 if(Cmd_rwParam(wrBuf, 6, rdBuf, 20, 5) < 5)/*获取de_ip*/
			    	 {
			    		i=0;
					break;
			    	 }
				else
				 {
					gChn.de_ip[ch-1][0]=rdBuf[6];
					gChn.de_ip[ch-1][1]=rdBuf[7];
					gChn.de_ip[ch-1][2]=rdBuf[8];
					gChn.de_ip[ch-1][3]=rdBuf[9];
				 }
				wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf2;wrBuf[3]=ch;wrBuf[4]=0x02;wrBuf[5]=0x01;
				 if(Cmd_rwParam(wrBuf, 6, rdBuf, 20, 5) < 5)/*获取de_mac*/
			    	 {
			    		i=0;
					break;
			    	 }
				else
				 {
					gChn.de_mac[ch-1][0]=rdBuf[6];
					gChn.de_mac[ch-1][1]=rdBuf[7];
					gChn.de_mac[ch-1][2]=rdBuf[8];
					gChn.de_mac[ch-1][3]=rdBuf[9];
					gChn.de_mac[ch-1][4]=rdBuf[10];
					gChn.de_mac[ch-1][5]=rdBuf[11];
				 }
				wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf2;wrBuf[3]=ch;wrBuf[4]=0x03;wrBuf[5]=0x01;
				 if(Cmd_rwParam(wrBuf, 6, rdBuf, 20, 5) < 5)/*获取de_mac*/
			    	 {
			    		i=0;
					break;
			    	 }
				else
				 {
					gChn.de_port[ch-1]=rdBuf[6];
					gChn.de_port[ch-1]|=(u16_t)rdBuf[7]<<8;
				 }
			 }
			 if(i=0)
			 {
			 	TcpClt_close();
				return 0;
			 }
	}
	TcpClt_close();
	return 1;
}
u8_t Cmd_SetTS2IP()
{
	u8_t rdBuf[20];
	u8_t wrBuf[20];
    u8_t ch,i=1;
	
	if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
	return 0;

	/*设置TTL*/
	wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf2;wrBuf[3]=0x00;wrBuf[4]=0x02;wrBuf[5]=0x02;
	wrBuf[6]=gChn.udp_ttl;
	if(Cmd_rwParam(wrBuf,7, rdBuf, 20, 5) < 5)
	{
		TcpClt_close();
		return 0;
	}
	
	wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf1;wrBuf[3]=0x01;wrBuf[4]=0x01;wrBuf[5]=0x02;
	wrBuf[6]=gChn.su_ip[0];wrBuf[7]=gChn.su_ip[1];wrBuf[8]=gChn.su_ip[2];wrBuf[9]=gChn.su_ip[3];
	if(Cmd_rwParam(wrBuf,10, rdBuf, 20, 5) < 5)/*设置源IP地址*/
	{
		TcpClt_close();
		return 0;
	}
	wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf1;wrBuf[3]=0x01;wrBuf[4]=0x02;wrBuf[5]=0x02;
	wrBuf[6]=gChn.su_mac[0];wrBuf[7]=gChn.su_mac[1];wrBuf[8]=gChn.su_mac[2];
	wrBuf[9]=gChn.su_mac[3];wrBuf[10]=gChn.su_mac[4];wrBuf[11]=gChn.su_mac[5];
	if(Cmd_rwParam(wrBuf,12, rdBuf, 20, 5) < 5)/*设置源MAC地址*/
	{
		TcpClt_close();
		return 0;
	}
	wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf1;wrBuf[3]=0x01;wrBuf[4]=0x03;wrBuf[5]=0x02;
	wrBuf[6]=gChn.su_port;wrBuf[7]=gChn.su_port>>8;
	if(Cmd_rwParam(wrBuf,8, rdBuf, 20, 5) < 5)/*设置源端口号*/
	{
		TcpClt_close();
		return 0;
	}
	usleep(50000);
	for(ch=1;ch<9;ch++)
	{
		wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf2;wrBuf[3]=ch;wrBuf[4]=0x01;wrBuf[5]=0x02;
		wrBuf[6]=gChn.de_ip[ch-1][0];wrBuf[7]=gChn.de_ip[ch-1][1];wrBuf[8]=gChn.de_ip[ch-1][2];wrBuf[9]=gChn.de_ip[ch-1][3];
		if(Cmd_rwParam(wrBuf,10, rdBuf, 20, 5) < 5)/*设置目的IP地址*/
		{
			TcpClt_close();
			i=0;
			break;
		}
		wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf2;wrBuf[3]=ch;wrBuf[4]=0x02;wrBuf[5]=0x02;
		wrBuf[6]=gChn.de_mac[ch-1][0];wrBuf[7]=gChn.de_mac[ch-1][1];wrBuf[8]=gChn.de_mac[ch-1][2];
		wrBuf[9]=gChn.de_mac[ch-1][3];wrBuf[10]=gChn.de_mac[ch-1][4];wrBuf[11]=gChn.de_mac[ch-1][5];
		if(Cmd_rwParam(wrBuf,12, rdBuf, 20, 5) < 5)/*设置目的MAC地址*/
		{
			i=0;
			break;
		}
		wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf2;wrBuf[3]=ch;wrBuf[4]=0x03;wrBuf[5]=0x02;
		wrBuf[6]=gChn.de_port[ch-1];wrBuf[7]=gChn.de_port[ch-1]>>8;
		if(Cmd_rwParam(wrBuf,8, rdBuf, 20, 5) < 5)/*设置目的口号*/
		{
			i=0;
			break;
		}
	}
	 if(i=0)
		 {
		 	TcpClt_close();
			return 0;
		 }

	 
wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0xf2;wrBuf[3]=0x08;wrBuf[4]=0xf0;wrBuf[5]=0x02;
 if(Cmd_rwParam(wrBuf,6, rdBuf, 20, 5) < 5)
	{
		TcpClt_close();
		return 0;
	}
	 TcpClt_close();
	return 3;
}

void Cmd_getVersion(u8_t *pVerBytes)
{
		u8_t rdBuf[20];
		u8_t wrBuf[20];
	    u8_t ch,i=1;
		
		if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
		return 0;
		wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0x01;wrBuf[3]=0x02;
		if(Cmd_rwParam(wrBuf,4, rdBuf, 20, 1) < 1)
		{
			TcpClt_close();
			return ;
		}
         pVerBytes[1]=rdBuf[4];
	    pVerBytes[0]=rdBuf[5];
		wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0x01;wrBuf[3]=0x03;	
		if(Cmd_rwParam(wrBuf,4, rdBuf, 20, 1) < 1)
		{
			TcpClt_close();
			return ;
		}	
	    pVerBytes[3]=rdBuf[4];
	    pVerBytes[2]=rdBuf[5];
        wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0x01;wrBuf[3]=0x04;	
		if(Cmd_rwParam(wrBuf,4, rdBuf, 20, 1) < 1)
		{
			TcpClt_close();
			return ;
		}	
	    pVerBytes[5]=rdBuf[4];
	    pVerBytes[4]=rdBuf[5];






	TcpClt_close();
}

void Cmd_submitReboot()
{

	u8_t rdBuf[20];
	u8_t wrBuf[20];
    u8_t ch,i=1;
	
	if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
	return 0;
	wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0x01;wrBuf[3]=0xf1;
	if(Cmd_rwParam(wrBuf,4, rdBuf, 20, 1) < 1)
	{
		TcpClt_close();
		return ;
	}
	TcpClt_close();
	return ;
}

void Cmd_submitFactory()
{
	u8_t rdBuf[20];
	u8_t wrBuf[20];
    u8_t ch,i=1;
	
	if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
	return 0;
	wrBuf[0]=0x77;wrBuf[1]=0x6c;wrBuf[2]=0x01;wrBuf[3]=0xf2;
	if(Cmd_rwParam(wrBuf,4, rdBuf, 20, 1) < 1)
	{
		TcpClt_close();
		return ;
	}
	TcpClt_close();
	return ;

}




