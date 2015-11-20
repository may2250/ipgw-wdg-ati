#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "cgic.h"
#include "cJSON.h"
#include "cmd.h"
#include "web.h"
#include "tcpClient.h"
#include "upload.h"
#include "udpsend.h"


#define CgiPrint(X...) fprintf(cgiOut, X)

#define HTML2CGI_EER_OK 0
#define HTML2CGI_EER_NULL_FILE 1

#ifdef PC_OS
#define WEB_STATUS_HTML "/mnt/hgfs/wanlong/devWeb_1502100100/web/status.html"

#define WEB_CHANNEL_HTML "/mnt/hgfs/wanlong/devWeb_1502100100/web/htmlForCgiFiles/channel.html"
#define WEB_PROGRAME_HTML "/mnt/hgfs/wanlong/devWeb_1502100100/web/htmlForCgiFiles/programe.html"


#define WEB_WAITGOTO_HTML "/mnt/hgfs/wanlong/devWeb_1502100100/web/htmlForCgiFiles/waitgoto.html"
#define WEB_NET_HTML "/mnt/hgfs/wanlong/devWeb_1502100100/web/htmlForCgiFiles/net.html"
#define WEB_TP_IP_HTML "/mnt/hgfs/wanlong/devWeb_1502100100/web/htmlForCgiFiles/transmit.html"
#define WEB_BACKUP_HTML "/mnt/hgfs/wanlong/devWeb_1502100100/web/htmlForCgiFiles/backup.html"

#define WEB_SYSTEM_HTML "/mnt/hgfs/wanlong/devWeb_1502100100/web/htmlForCgiFiles/system.html"
#define WEB_TEST_HTML "/mnt/hgfs/wanlong/devWeb_1502100100/web/htmlForCgiFiles/test.html"

#endif

#ifdef ARM_OS

#define WEB_STATUS_HTML "/mnt/Nand2/web/www/status.html"


#define WEB_CHANNEL_HTML "/mnt/Nand2/web/www/htmlForCgiFiles/channel.html"
#define WEB_PROGRAME_HTML "/mnt/Nand2/web/www/htmlForCgiFiles/programe.html"
#define WEB_OUT_PRG_HTML "/mnt/Nand2/web/www/htmlForCgiFiles/output.html"

#define WEB_WAITGOTO_HTML "/mnt/Nand2/web/www/htmlForCgiFiles/waitgoto.html"
#define WEB_NET_HTML "/mnt/Nand2/web/www/htmlForCgiFiles/net.html"
#define WEB_TP_IP_HTML "/mnt/Nand2/web/www/htmlForCgiFiles/transmit.html"
#define WEB_BACKUP_HTML "/mnt/Nand2/web/www/htmlForCgiFiles/backup.html"

#define WEB_SYSTEM_HTML "/mnt/Nand2/web/www/htmlForCgiFiles/system.html"
#define WEB_TEST_HTML "/mnt/Nand2/web/www/htmlForCgiFiles/test.html"
#endif

typedef enum EmValueType_e
{
	EmValueType_null,
	EmValueType_int,
	EmValueType_hex
} EmValueType_m;

typedef struct StHtml2Cgi_item_s
{
	char *pName;
	char *pValue;
	cmd_t *pDataBaseItem;
	EmValueType_m type;
} StHtml2Cgi_item_t;

char *gStr_array_value[] =
{
	"0","1","2","3","4","5","6","7","8","9",
	"10","11","12","13","14","15","16","17","18","19",
	"20","21","22","23","24","25","26","27","28","29",
	"30","31","32","33","34","35","36","37","38","39",
	"40","41","42","43","44","45","46","47","48","49",
};

char *gStr_aBitrate_value[] = 
{
	"64","96","128","192","224"
	,"256","320","384"
};
u16_t aBitrate_value[] = 
{
	64,96,128,192,224
	,256,320,384
};

#define LINUX_GETLINE_FUNC_ENABLE 0
#define LINE_CHAR_MAXCNT 1023

//ssize_t ReadLine(char ** ppLine, size_t *pLen, FILE * fp)
ssize_t ReadLine(char * pLine, size_t *pLen, FILE * fp)
{
#if 1
	int len;
	char *pLineString;

	if(feof(fp))
		return 0;
	pLineString = fgets(pLine, LINE_CHAR_MAXCNT, fp);
	if(pLineString == NULL)
		return 0;
	len = strlen(pLineString);
	return len;
#else
#if LINUX_GETLINE_FUNC_ENABLE
	//return getline(ppLine, pLen, fp);
	return 0;
#else
	ssize_t i = 0;
	//fprintf(stderr, "ReadLine1-fp=%p\n", fp);
	*pLine = fgetc(fp);
	//fprintf(stderr, "ReadLine2\n");
	while(*pLine !='\n' && i < LINE_CHAR_MAXCNT)  
	{
		if(feof(fp))
		{
			if(i == 0)
				return 0;
			else
				break;
		}
		pLine++;
		i++;  
		*pLine = fgetc(fp);  
	}
	//fprintf(stderr, "ReadLine3\n");
	if(i == 0)
		pLine[i++] = '\n';
	pLine[i] = '\0';
	return i;
#endif
#endif
}

int Html2Cgi_parse(char *pFileName, StHtml2Cgi_item_t *pItems, int itemCnt)
{
	FILE * fp;
#if LINUX_GETLINE_FUNC_ENABLE	
	char * line = NULL;
#else
	char line[LINE_CHAR_MAXCNT+1];
#endif
	size_t len = 0;
	ssize_t read;
	char *p, *pStart, *pEnd;
	#define LINE_BUFFER_MAXCNT 1023
	char buffer[LINE_BUFFER_MAXCNT+1];
	int sLen;
	int i;

	cgiHeaderContentType("text/html");

	fp = fopen(pFileName, "r");
	if (fp == NULL)
	{
		printf("Website error!\n");
		return HTML2CGI_EER_NULL_FILE;
	}
	//fprintf(stderr, "Html2Cgi_parse-pFileName[%s]-fp6: %p\n", pFileName, fp);
	while ((read = ReadLine(line, &len, fp)) > 0) {
		//fprintf(stderr, "%s", line);
		p = strstr(line, "@@");
		if(p)
		{
			sLen = strlen(line);
			if(sLen > LINE_BUFFER_MAXCNT)
				sLen = LINE_BUFFER_MAXCNT;
			strncpy(buffer, line, sLen);
			buffer[sLen] = '\0';
			pStart = buffer;
			pEnd = pStart + sLen;

			while(pStart < pEnd)
			{
				for(i = 0; i < itemCnt; i++)
				{
					p = strstr(pStart, pItems[i].pName);
					if(p)
					{
						*p = '\0';
						CgiPrint("%s", pStart);
						if(pItems[i].pValue)
						{
							CgiPrint("%s", pItems[i].pValue);
							//fprintf(stderr, "%s", pItems[i].pValue);
						}
						else if(pItems[i].pDataBaseItem)
						{
							if(pItems[i].type == EmValueType_hex)
								CgiPrint("0x%X", *(pItems[i].pDataBaseItem));	
							else
								CgiPrint("%d", *(pItems[i].pDataBaseItem));	
							//fprintf(stderr, "%d", *(pItems[i].pDataBaseItem));
						}
						pStart = p + strlen(pItems[i].pName);
						break;
					}
				}
				if(i >= itemCnt)
				{
					CgiPrint("%s", pStart);
				//	fprintf(stderr, "%s", pStart);
					break;				
				}
			}
		}	
		else
		{
			CgiPrint("%s", line);
		//	fprintf(stderr, "%s", line);
		}
	}
#if LINUX_GETLINE_FUNC_ENABLE	
	if (line)
		free(line);
#endif	
	fclose(fp);
	return HTML2CGI_EER_OK;
}


int Html2Cgi_waitGoto(char *pHRef, char *pTitle)
{
	StHtml2Cgi_item_t items[]= {
		{"@@formparams['href']", pHRef, NULL},
		{"@@formparams['title']", pTitle, NULL}
	};
	
	return Html2Cgi_parse(WEB_WAITGOTO_HTML
		, items, sizeof(items)/sizeof(StHtml2Cgi_item_t));
}

void Html2Cgi_errWaitGoto(char *pHRef)
{
	Html2Cgi_waitGoto(pHRef, "Save failed! Error is detected parameters!");//"保存失败! 检测到错误的参数!");	
}


cmd_t NetPort_Str2CmdT(char *pStr)
{
	cmd_t rstl = 0;
	rstl= (u16_t)atoi(pStr);
}
cmd_t NetAddr_ipStr2CmdT(char *pStr, int maxLen)
{
#if 0
	pStr[maxLen] = '\0'	
	return htonl(inet_addr(pStr));
#else
	int i;
	char *p;
	cmd_t rstl = 0;

	pStr[maxLen] = '\0'	;
	p = strtok(pStr, ".");
	for(i = 0; i < 4 && p != NULL && *p != '\0'; i++)
	{
		rstl <<= 8;
		rstl |= (u8_t)atoi(p);
		p = strtok(NULL, ".");
	}
	return rstl;
#endif	
}

void NetAddr_macStr2Bytes(char *pStr, int maxLen, u8_t *pMac)
{
	int i;
	char *p;

	pStr[maxLen] = '\0';	
	memset(pMac, 0, 6);
	p = strtok(pStr, ":");
	for(i = 0; i < 6 && p != NULL && *p != '\0'; i++)
	{
		pMac[i] = strtol(p, NULL, 16);
		p = strtok(NULL, ":");
	}
}



void WebSearchChannel()
{
	char prgTable[10240];
	int i;
	char *pc = prgTable;
	cmd_t id;
	char sscchh[128];
	char nsscchh[128];
	char set_ch[128];
    int tmp;
		cgiFormIntegerBounded("SearchChannel", (int *)&id, 1, CHANNEL_MAXCNT, 1);
	
         sprintf(sscchh,"/cgi-bin/index.cgi?channel=%d",id);
		sprintf(nsscchh,"/cgi-bin/index.cgi?SearchChannel=%d",id);
		sprintf(set_ch,"/cgi-bin/index.cgi?Set_Channel=%d",id);
		StHtml2Cgi_item_t items[]= {
		{"@@formparams['id']", NULL, &id, EmValueType_int},
		{"@@formparams['prgTable']", prgTable, NULL, EmValueType_null},	
		{"@@formparams['s_ch']", sscchh, NULL, EmValueType_null},
		{"@@formparams['ns_ch']", nsscchh, NULL, EmValueType_null},
		{"@@formparams['set_ch']", set_ch, NULL, EmValueType_null},
		
			};
	Cmd_SearchInput(id);
	Cmd_getChannel(id,1);
/*
	for(i = 0; i < gChn.serviceCnt; i++)
	{
		sprintf(pc, "<tr%s><td>%d</td><td>0x%X</td><td>0x%X</td><td>%s</td><td>%s</td><td><input name=\"muxed%d\" id=\"muxed%d\" type=\"checkbox\" %s></td></tr>"
			, (i&1)?" bgcolor=\"#EBF3FC\"":"", i+1, gChn.service[i].prgId, gChn.service[i].pmtPid, gChn.service[i].name
			, gChn.service[i].provider, (i+1), (i+1), gChn.service[i].isMuxed?"checked":"");
		pc = prgTable + strlen(prgTable);
	}*/
	for(i = 0; i < gChn.serviceCnt; i++)
	{
		Cmd_getChannelOutput(id,i);
		tmp=id<<8;
		tmp|=i;
		sprintf(pc, "<tr%s><td>%d</td><td>%d</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td><a href=\"/cgi-bin/index.cgi?programe=%d\">Edit</a></tr></td>", (i&1)?" bgcolor=\"#EBF3FC\"":"", i+1, gChn.service[i].prgId, gChn.service[i].name, gChn.service[i].provider
			,gChn.service_o[i].name,gChn.service_o[i].provider,tmp);

		pc = prgTable + strlen(prgTable);
	}
	Html2Cgi_parse(WEB_CHANNEL_HTML, items, sizeof(items)/sizeof(StHtml2Cgi_item_t));

}
void WebGet_channel()
{
	char prgTable[10240];
	int i;
	char *pc = prgTable;
	cmd_t id;
    char sscchh[128];
	char nsscchh[128];
	char set_ch[128];
	int tmp;
	cgiFormIntegerBounded("channel", (int *)&id, 1, CHANNEL_MAXCNT, 1);
	
	sprintf(sscchh,    "/cgi-bin/index.cgi?channel=%d",id);
	sprintf(nsscchh, "/cgi-bin/index.cgi?SearchChannel=%d",id);
	sprintf(set_ch,   "/cgi-bin/index.cgi?Set_Channel=%d",id);
	
	StHtml2Cgi_item_t items[]= {
		{"@@formparams['id']", NULL, &id, EmValueType_int},
		{"@@formparams['prgTable']", prgTable, NULL, EmValueType_null},		
		{"@@formparams['s_ch']", sscchh, NULL, EmValueType_null},
		{"@@formparams['ns_ch']", nsscchh, NULL, EmValueType_null},
		{"@@formparams['set_ch']", set_ch, NULL, EmValueType_null},
	};
	Cmd_getChannel(id,1);

	
	for(i = 0; i < gChn.serviceCnt; i++)
	{
		//Cmd_getChannelOutput(id,i);
		tmp=id<<8;
		tmp|=i;
		sprintf(pc, "<tr%s><td>%d</td><td>%d</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td><a href=\"/cgi-bin/index.cgi?programe=%d\">Edit</a></tr></td>", (i&1)?" bgcolor=\"#EBF3FC\"":"", i+1, gChn.service[i].prgId, gChn.service[i].name, gChn.service[i].provider
			,gChn.service_o[i].name,gChn.service_o[i].provider,tmp);

		pc = prgTable + strlen(prgTable);
	}

	Html2Cgi_parse(WEB_CHANNEL_HTML, items, sizeof(items)/sizeof(StHtml2Cgi_item_t));
}
void WebGet_channel_edit()
{
	char prgTable[10240];
	int i;
	char *pc = prgTable;
	cmd_t id;
    char sscchh[128];
	char nsscchh[128];
	char set_ch[128];
	int tmp;
	cgiFormIntegerBounded("channel_pro_edit", (int *)&id, 1, CHANNEL_MAXCNT, 1);
	
	sprintf(sscchh,    "/cgi-bin/index.cgi?channel=%d",id);
	sprintf(nsscchh, "/cgi-bin/index.cgi?SearchChannel=%d",id);
	sprintf(set_ch,   "/cgi-bin/index.cgi?Set_Channel=%d",id);
	
	StHtml2Cgi_item_t items[]= {
		{"@@formparams['id']", NULL, &id, EmValueType_int},
		{"@@formparams['prgTable']", prgTable, NULL, EmValueType_null},		
		{"@@formparams['s_ch']", sscchh, NULL, EmValueType_null},
		{"@@formparams['ns_ch']", nsscchh, NULL, EmValueType_null},
		{"@@formparams['set_ch']", set_ch, NULL, EmValueType_null},
	};
	
	Cmd_getChannel(id,0);
	memset(&gChn.service_o, 0, sizeof(gChn.service_o));
	for(i = 0; i < gChn.serviceCnt; i++)
	{
		Cmd_getChannelOutput(id,i);
		tmp=id<<8;
		tmp|=i;
		sprintf(pc, "<tr%s><td>%d</td><td>%d</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td><a href=\"/cgi-bin/index.cgi?programe=%d\">Edit</a></tr></td>", (i&1)?" bgcolor=\"#EBF3FC\"":"", i+1, gChn.service[i].prgId, gChn.service[i].name, gChn.service[i].provider
			,gChn.service_o[i].name,gChn.service_o[i].provider,tmp);

		pc = prgTable + strlen(prgTable);
	}

	Html2Cgi_parse(WEB_CHANNEL_HTML, items, sizeof(items)/sizeof(StHtml2Cgi_item_t));
}

void WebGet_output()
{
cmd_t rate[8],i;
char p[50];
char tt[10][10];

/*
	char prgTable[10240];
	int i;
	char *pc = prgTable;
	
	StHtml2Cgi_item_t items[]= {
		{"@@formparams['prgTable']", prgTable, NULL, EmValueType_null},		
	};
	
	Cmd_getOutput();

	for(i = 0; i < gChn.serviceCnt; i++)
	{
		sprintf(pc, "<tr%s><td>%d</td><td>0x%X</td><td>0x%X</td><td>%s</td><td>%s</td><td><a href=\"/cgi-bin/index.cgi?programe=%d\">Edit</a></tr></td>"
			, (i&1)?" bgcolor=\"#EBF3FC\"":"", i+1, gChn.service[i].prgId, gChn.service[i].pmtPid, gChn.service[i].name
			, gChn.service[i].provider, (gChn.service[i].chnId*0x100)+gChn.service[i].prgId);
		pc = prgTable + strlen(prgTable);
	}

	Html2Cgi_parse(WEB_OUT_PRG_HTML, items, sizeof(items)/sizeof(StHtml2Cgi_item_t));*/
	cgiHeaderContentType("text/html");             //不能少

	/* Now show the form */

	
	for(i=0;i<8;i++)
	{
		rate[i]=Cmd_GetChannel_Rate(i);
		CgiPrint("%d,",rate[i]);
	
		
		//sprintf(tt[i],"%s",i);
	}
		

	//sprintf(p,rate);

	//fprintf(cgiOut, rate[0]);
	
}
void Set_Channel()
{
	int i;
	cmd_t id;
	char href[32], title[32];
	sprintf(href, "/cgi-bin/index.cgi?channel=%d", 1);
	  
	if(cgiFormSuccess != cgiFormIntegerBounded("id", (int *)&id, 0, 65535, 1))
	{
		Html2Cgi_waitGoto(href, "Set Channel ERR ,Please Check!");
		return;
	}
	memset(&gChn.service_o, 0, sizeof(gChn.service_o));
	gChn.serviceCnt=Cmd_getchannel_pm(id);
	for(i=0;i<gChn.serviceCnt;i++)
	{
		Cmd_getChannelOutput(id,i);
	}
	if(Cmd_setChannel(id)==2)
	sprintf(title,"Setting Channel OK ......");
	else
	sprintf(title,"Setting Channel ERR ......");
	
	sprintf(href, "/cgi-bin/index.cgi?channel=%d", id);
	Html2Cgi_waitGoto(href, title);
	
}


void WebGet_programe()
{
	cmd_t chnId, prgIndex;
	char href[64];

	cgiFormIntegerBounded("programe", (int *)&prgIndex, 0x0000, 0XFFFF, 0x202);

	chnId = prgIndex >> 8;
	prgIndex &= 0xff;

	StHtml2Cgi_item_t items[]= {
		{"@@formparams['href']", href, NULL},
			
		{"@@formparams['chnId']", NULL, &chnId, EmValueType_int},
		{"@@formparams['index']", NULL, &prgIndex, EmValueType_int},
		//{"@@formparams['prgId']", NULL, &(gChn.service[prgIndex].prgId), EmValueType_hex},
		//{"@@formparams['pmtPid']", NULL, &(gChn.service[prgIndex].pmtPid), EmValueType_hex},
		{"@@formparams['name']", gChn.service_o[prgIndex].name, NULL, EmValueType_null},	
		{"@@formparams['provider']", gChn.service_o[prgIndex].provider, NULL, EmValueType_null},		
	};

	sprintf(href, "/cgi-bin/index.cgi?channel=%d", chnId);
	Cmd_getChannelOutput(chnId,prgIndex);
	//Cmd_getPrograme(chnId, prgIndex);

	Html2Cgi_parse(WEB_PROGRAME_HTML, items, sizeof(items)/sizeof(StHtml2Cgi_item_t));
}

void WebSubmit_programe()
{
	char href[200], title[200];
	cmd_t chnId, prgIndex;
	/*
cgiFormString("name",aa, 50);
cgiFormString("index",bb, 10);
//aa[0]=0x31;*/


	
	if(cgiFormSuccess != cgiFormIntegerBounded("chnId", (int *)&chnId, 1, 10, 1))
	goto LINE_END;
	if(cgiFormSuccess != cgiFormIntegerBounded("index", (int *)&prgIndex, 0, 256, 1))
			goto LINE_END;

	cgiFormString("provider", (char *)gChn.service[prgIndex].provider, NAME_MAXLEN);
	cgiFormString("name", (char *)gChn.service[prgIndex].name, NAME_MAXLEN);
	gChn.service[prgIndex].provider_len=strlen(gChn.service[prgIndex].provider);
	gChn.service[prgIndex].name_len=strlen(gChn.service[prgIndex].name);
	Cmd_programe(chnId, prgIndex);
	sprintf(href, "/cgi-bin/index.cgi?channel_pro_edit=%d", chnId);
	//sprintf(href, "/cgi-bin/index.cgi?channel=%d", 1);
	sprintf(title, "Edit   ...");
	Html2Cgi_waitGoto(href, title);
	return;
	
     LINE_END:
	sprintf(title,"%d",prgIndex);
	sprintf(href, "/cgi-bin/index.cgi?channel=%d", chnId);
	Html2Cgi_waitGoto(href,title);
}


void WebGet_net()
{
//	u8_t ipMaskBuf[4];
	char ipStr[15]="192.168.000.119";
	char maskStr[]="255.255.255.000";
	char gatewayStr[]="192.168.000.1";
	//char macStr[]="C0:A8:01:02:03:06";
	char trapipStr[15]="192.168.001.251";
	StHtml2Cgi_item_t items[]= {
		{"@@formparams['ip']", ipStr, NULL, EmValueType_null},
		{"@@formparams['mask']", maskStr, NULL, EmValueType_null},
		{"@@formparams['gw']", gatewayStr, NULL, EmValueType_null},
		{"@@formparams['trapserverip']", trapipStr, NULL, EmValueType_null},
		//{"@@formparams['mac']", macStr, NULL, EmValueType_null},		
	};
	if(Cmd_getNet())
	{
		sprintf(ipStr, "%d.%d.%d.%d", gChn.my_ip[0],  gChn.my_ip[1],  gChn.my_ip[2],  gChn.my_ip[3]);
		sprintf(maskStr, "%d.%d.%d.%d", gChn.my_mask[0],  gChn.my_mask[1],  gChn.my_mask[2],  gChn.my_mask[3]);
		sprintf(gatewayStr, "%d.%d.%d.%d", gChn.my_gateway[0],  gChn.my_gateway[1],  gChn.my_gateway[2],  gChn.my_gateway[3]);
		//sprintf(macStr, "%X:%X:%X:%X:%X:%X", gChn.my_mac[0],  gChn.my_mac[1],  gChn.my_mac[2],  gChn.my_mac[3],  gChn.my_mac[4],  gChn.my_mac[5]);
	}
	int i = 0, len = 0;
	char trapserver[15] = {0};
    int file = open( "/mnt/Nand2/net-snmp-arm/trapserver.txt", O_RDONLY );
	read(file, trapserver, sizeof(trapserver));
    close( file );
	sprintf(trapipStr, "%s", trapserver);
	//memcpy(trapipStr, trapserver, strlen(trapserver));
	Html2Cgi_parse(WEB_NET_HTML
		, items, sizeof(items)/sizeof(StHtml2Cgi_item_t));
}

void WebGet_Status()
{
	char id[20]="0123456789";
    cmd_t rate[8];
	char prgTable[10240];
	int i;
	char *pc = prgTable;
	u8_t status;


   	cgiHeaderContentType("text/html"); 
	for(i=0;i<8;i++)
	{
		rate[i]=Cmd_GetChannel_Rate(i);
	}
	status=Cmd_GetChannel_Status();

	CgiPrint("sys_state,");
	
	for(i=0;i<8;i++)
	{
		CgiPrint("%d,",status&0x01);
		status>>=1;
	}
	
	for(i=0;i<8;i++)
	{
		CgiPrint("%03d.%03d Mbps,",rate[i]/1000,rate[i]%1000);
	}
}

void parse_tsipnode(xmlNodePtr curr){
	xmlNodePtr tmpnode;
	tmpnode = curr->xmlChildrenNode;
	u32_t tmpU32;
	char *szKey;
	while(tmpnode != NULL){
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"su_ip")))
		{
			szKey = xmlNodeGetContent(tmpnode);
			tmpU32 = NetAddr_ipStr2CmdT(szKey, 23);
			gChn.su_ip[0]=tmpU32>>24;gChn.su_ip[1]=tmpU32>>16;gChn.su_ip[2]=tmpU32>>8;gChn.su_ip[3]=tmpU32>>0;						
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"su_port"))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.su_port = atoi(szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"udp_ttl"))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.udp_ttl = atoi(szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"su_mac"))){
			szKey = xmlNodeGetContent(tmpnode);
			NetAddr_macStr2Bytes(szKey,21,gChn.su_mac);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_ip0"))){
			szKey = xmlNodeGetContent(tmpnode);
			tmpU32 = NetAddr_ipStr2CmdT(szKey, 23);
			gChn.de_ip[0][0]=tmpU32>>24;gChn.de_ip[0][1]=tmpU32>>16;gChn.de_ip[0][2]=tmpU32>>8;gChn.de_ip[0][3]=tmpU32>>0;
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_ip1"))){
			szKey = xmlNodeGetContent(tmpnode);
			tmpU32 = NetAddr_ipStr2CmdT(szKey, 23);
			gChn.de_ip[1][0]=tmpU32>>24;gChn.de_ip[1][1]=tmpU32>>16;gChn.de_ip[1][2]=tmpU32>>8;gChn.de_ip[1][3]=tmpU32>>0;
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_ip2"))){
			szKey = xmlNodeGetContent(tmpnode);
			tmpU32 = NetAddr_ipStr2CmdT(szKey, 23);
			gChn.de_ip[2][0]=tmpU32>>24;gChn.de_ip[2][1]=tmpU32>>16;gChn.de_ip[2][2]=tmpU32>>8;gChn.de_ip[2][3]=tmpU32>>0;
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_ip3"))){
			szKey = xmlNodeGetContent(tmpnode);
			tmpU32 = NetAddr_ipStr2CmdT(szKey, 23);
			gChn.de_ip[3][0]=tmpU32>>24;gChn.de_ip[3][1]=tmpU32>>16;gChn.de_ip[3][2]=tmpU32>>8;gChn.de_ip[3][3]=tmpU32>>0;
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_ip4"))){
			szKey = xmlNodeGetContent(tmpnode);
			tmpU32 = NetAddr_ipStr2CmdT(szKey, 23);
			gChn.de_ip[4][0]=tmpU32>>24;gChn.de_ip[4][1]=tmpU32>>16;gChn.de_ip[4][2]=tmpU32>>8;gChn.de_ip[4][3]=tmpU32>>0;
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_ip5"))){
			szKey = xmlNodeGetContent(tmpnode);
			tmpU32 = NetAddr_ipStr2CmdT(szKey, 23);
			gChn.de_ip[5][0]=tmpU32>>24;gChn.de_ip[5][1]=tmpU32>>16;gChn.de_ip[5][2]=tmpU32>>8;gChn.de_ip[5][3]=tmpU32>>0;
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_ip6"))){
			szKey = xmlNodeGetContent(tmpnode);
			tmpU32 = NetAddr_ipStr2CmdT(szKey, 23);
			gChn.de_ip[6][0]=tmpU32>>24;gChn.de_ip[6][1]=tmpU32>>16;gChn.de_ip[6][2]=tmpU32>>8;gChn.de_ip[6][3]=tmpU32>>0;
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_ip7"))){
			szKey = xmlNodeGetContent(tmpnode);
			tmpU32 = NetAddr_ipStr2CmdT(szKey, 23);
			gChn.de_ip[7][0]=tmpU32>>24;gChn.de_ip[7][1]=tmpU32>>16;gChn.de_ip[7][2]=tmpU32>>8;gChn.de_ip[7][3]=tmpU32>>0;
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_port0"))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.de_port[0] = atoi(szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_port1"))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.de_port[1] = atoi(szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_port2"))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.de_port[2] = atoi(szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_port3"))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.de_port[3] = atoi(szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_port4"))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.de_port[4] = atoi(szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_port5"))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.de_port[5] = atoi(szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_port6"))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.de_port[6] = atoi(szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_port7"))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.de_port[7] = atoi(szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_mac0"))){
			szKey = xmlNodeGetContent(tmpnode);
			NetAddr_macStr2Bytes(szKey,21,gChn.de_mac[0]);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_mac1"))){
			szKey = xmlNodeGetContent(tmpnode);
			NetAddr_macStr2Bytes(szKey,21,gChn.de_mac[1]);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_mac2"))){
			szKey = xmlNodeGetContent(tmpnode);
			NetAddr_macStr2Bytes(szKey,21,gChn.de_mac[2]);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_mac3"))){
			szKey = xmlNodeGetContent(tmpnode);
			NetAddr_macStr2Bytes(szKey,21,gChn.de_mac[3]);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_mac4"))){
			szKey = xmlNodeGetContent(tmpnode);
			NetAddr_macStr2Bytes(szKey,21,gChn.de_mac[4]);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_mac5"))){
			szKey = xmlNodeGetContent(tmpnode);
			NetAddr_macStr2Bytes(szKey,21,gChn.de_mac[5]);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_mac6"))){
			szKey = xmlNodeGetContent(tmpnode);
			NetAddr_macStr2Bytes(szKey,21,gChn.de_mac[6]);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"de_mac7"))){
			szKey = xmlNodeGetContent(tmpnode);
			NetAddr_macStr2Bytes(szKey,21,gChn.de_mac[7]);
			xmlFree(szKey);
		}
		tmpnode = tmpnode->next;
	}
}

void parse_channelnode(xmlNodePtr curr){
	xmlNodePtr tmpnode;
	tmpnode = curr->xmlChildrenNode;
	char *szKey;
	int chnid = 1, cnt = 0;
	char datastr[32]= {0};
	while(tmpnode != NULL){		
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"serviceChnid"))){
			szKey = xmlNodeGetContent(tmpnode);
			chnid = atoi(szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"serviceCnt"))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.serviceCnt = atoi(szKey);
			xmlFree(szKey);
		}
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "service%d_chnId", cnt);
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)datastr))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.service[cnt].chnId = atoi(szKey);
			xmlFree(szKey);
		}
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "service%d_isMuxed", cnt);
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)datastr))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.service[cnt].isMuxed = atoi(szKey);
			xmlFree(szKey);
		}
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "service%d_prgId", cnt);
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)datastr))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.service[cnt].prgId = atoi(szKey);
			xmlFree(szKey);
		}
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "service%d_pmtPid", cnt);
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)datastr))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.service[cnt].pmtPid = atoi(szKey);
			xmlFree(szKey);
		}
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "service%d_name_len", cnt);
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)datastr))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.service[cnt].name_len = atoi(szKey);
			xmlFree(szKey);
		}
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "service%d_name", cnt);
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)datastr))){
			szKey = xmlNodeGetContent(tmpnode);
			memcpy(gChn.service[cnt].name, szKey, strlen(szKey));
			xmlFree(szKey);
		}
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "service%d_provider_len", cnt);
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)datastr))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.service[cnt].provider_len = atoi(szKey);
			xmlFree(szKey);
		}
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "service%d_provider", cnt);
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)datastr))){
			szKey = xmlNodeGetContent(tmpnode);
			memcpy(gChn.service[cnt].provider, szKey, strlen(szKey));
			xmlFree(szKey);
		}
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "service%d_isCA", cnt);
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)datastr))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.service[cnt].isCA = atoi(szKey);
			xmlFree(szKey);
		}		
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "service_o_%d_name_len", cnt);
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)datastr))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.service_o[cnt].name_len = atoi(szKey);
			xmlFree(szKey);
		}
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "service_o_%d_name", cnt);
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)datastr))){
			szKey = xmlNodeGetContent(tmpnode);
			memcpy(gChn.service_o[cnt].name, szKey, strlen(szKey));
			xmlFree(szKey);
		}
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "service_o_%d_provider_len", cnt);
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)datastr))){
			szKey = xmlNodeGetContent(tmpnode);
			gChn.service_o[cnt].provider_len = atoi(szKey);
			xmlFree(szKey);
		}
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "service_o_%d_provider", cnt);
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)datastr))){
			szKey = xmlNodeGetContent(tmpnode);
			memcpy(gChn.service_o[cnt].provider, szKey, strlen(szKey));
			xmlFree(szKey);
			cnt++;
		}
		tmpnode = tmpnode->next;
	}
	if(gChn.serviceCnt > 0){
		Cmd_setChannel(chnid);
	}	
}

void parse_trapsevernode(xmlNodePtr curr){
	xmlNodePtr tmpnode;
	tmpnode = curr->xmlChildrenNode;
	char *szKey;
	while(tmpnode != NULL){
		if((!xmlStrcmp(tmpnode->name, (const xmlChar*)"trapserverip"))){
			szKey = xmlNodeGetContent(tmpnode);			
			int fd;
			fd=open("/mnt/Nand2/net-snmp-arm/trapserver.txt",O_WRONLY|O_CREAT);
			write(fd, szKey, strlen(szKey));
			close(fd);
			xmlFree(szKey);
		}
		tmpnode = tmpnode->next;
	}	
}

void WebGet_Import()
{
	char updatas[20480] = {0};
	char datastr[32]= {0};
    xmlNodePtr root, curr;	
	
	if(cgiFormSuccess != cgiFormString("updatas", updatas, sizeof(updatas)))
	{
		Html2Cgi_waitGoto("/cgi-bin/index.cgi?transmit=1", "Import datas Err,Please Check!");
		return;
	}
	Html2Cgi_waitGoto("/cgi-bin/index.cgi?transmit=1", "Importing......");
	xmlDocPtr pdoc = xmlParseMemory(updatas, sizeof(updatas));
	root = xmlDocGetRootElement(pdoc);
	curr = root->xmlChildrenNode;
	while(curr != NULL){  
		if((!xmlStrcmp(curr->name, (const xmlChar*)"_tsip"))){
			parse_tsipnode(curr);			
			Cmd_SetTS2IP();
		}else if((!xmlStrcmp(curr->name, (const xmlChar*)"_channel1"))){
			parse_channelnode(curr);			
		}else if((!xmlStrcmp(curr->name, (const xmlChar*)"_channel2"))){
			parse_channelnode(curr);			
		}else if((!xmlStrcmp(curr->name, (const xmlChar*)"_channel3"))){
			parse_channelnode(curr);			
		}else if((!xmlStrcmp(curr->name, (const xmlChar*)"_channel4"))){
			parse_channelnode(curr);			
		}else if((!xmlStrcmp(curr->name, (const xmlChar*)"_channel5"))){
			parse_channelnode(curr);			
		}else if((!xmlStrcmp(curr->name, (const xmlChar*)"_channel6"))){
			parse_channelnode(curr);			
		}else if((!xmlStrcmp(curr->name, (const xmlChar*)"_channel7"))){
			parse_channelnode(curr);			
		}else if((!xmlStrcmp(curr->name, (const xmlChar*)"_channel8"))){
			parse_channelnode(curr);			
		}else if((!xmlStrcmp(curr->name, (const xmlChar*)"_trapserverip"))){
			parse_trapsevernode(curr);		
		}
		curr = curr->next;
	}
	
	xmlFreeDoc(pdoc);
	/*cJSON *backupdatas = cJSON_Parse(updatas);
	cJSON *tsip = cJSON_GetObjectItem(backupdatas, "_tsip");
	char *su_ip = cJSON_GetObjectItem(tsip, "su_ip")->valuestring;
	tmpU32 = NetAddr_ipStr2CmdT(su_ip, 23);
	gChn.su_ip[0]=tmpU32>>24;gChn.su_ip[1]=tmpU32>>16;gChn.su_ip[2]=tmpU32>>8;gChn.su_ip[3]=tmpU32>>0;
	gChn.su_port = cJSON_GetObjectItem(tsip, "su_port")->valueint;
	gChn.udp_ttl = cJSON_GetObjectItem(tsip, "udp_ttl")->valueint;
	char *su_mac = cJSON_GetObjectItem(tsip, "su_mac")->valuestring;
	//tmpU32 = NetPort_Str2CmdT(su_port);gChn.su_port=tmpU32;
	NetAddr_macStr2Bytes(su_mac,21,gChn.su_mac);
	Html2Cgi_waitGoto("/cgi-bin/index.cgi?transmit=1", "Importing......");
	tmpU32 = NetAddr_ipStr2CmdT(cJSON_GetObjectItem(tsip, "de_ip0")->valuestring, 23);
	gChn.de_ip[0][0]=tmpU32>>24;gChn.de_ip[0][1]=tmpU32>>16;gChn.de_ip[0][2]=tmpU32>>8;gChn.de_ip[0][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(cJSON_GetObjectItem(tsip, "de_ip1")->valuestring, 23);
	gChn.de_ip[1][0]=tmpU32>>24;gChn.de_ip[1][1]=tmpU32>>16;gChn.de_ip[1][2]=tmpU32>>8;gChn.de_ip[1][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(cJSON_GetObjectItem(tsip, "de_ip2")->valuestring, 23);
	gChn.de_ip[2][0]=tmpU32>>24;gChn.de_ip[2][1]=tmpU32>>16;gChn.de_ip[2][2]=tmpU32>>8;gChn.de_ip[2][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(cJSON_GetObjectItem(tsip, "de_ip3")->valuestring, 23);
	gChn.de_ip[3][0]=tmpU32>>24;gChn.de_ip[3][1]=tmpU32>>16;gChn.de_ip[3][2]=tmpU32>>8;gChn.de_ip[3][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(cJSON_GetObjectItem(tsip, "de_ip4")->valuestring, 23);
	gChn.de_ip[4][0]=tmpU32>>24;gChn.de_ip[4][1]=tmpU32>>16;gChn.de_ip[4][2]=tmpU32>>8;gChn.de_ip[4][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(cJSON_GetObjectItem(tsip, "de_ip5")->valuestring, 23);
	gChn.de_ip[5][0]=tmpU32>>24;gChn.de_ip[5][1]=tmpU32>>16;gChn.de_ip[5][2]=tmpU32>>8;gChn.de_ip[5][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(cJSON_GetObjectItem(tsip, "de_ip6")->valuestring, 23);
	gChn.de_ip[6][0]=tmpU32>>24;gChn.de_ip[6][1]=tmpU32>>16;gChn.de_ip[6][2]=tmpU32>>8;gChn.de_ip[6][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(cJSON_GetObjectItem(tsip, "de_ip7")->valuestring, 23);
	gChn.de_ip[7][0]=tmpU32>>24;gChn.de_ip[7][1]=tmpU32>>16;gChn.de_ip[7][2]=tmpU32>>8;gChn.de_ip[7][3]=tmpU32>>0;
	gChn.de_port[0] = cJSON_GetObjectItem(tsip, "de_port0")->valueint;
	gChn.de_port[1] = cJSON_GetObjectItem(tsip, "de_port1")->valueint;
	gChn.de_port[2] = cJSON_GetObjectItem(tsip, "de_port2")->valueint;
	gChn.de_port[3] = cJSON_GetObjectItem(tsip, "de_port3")->valueint;
	gChn.de_port[4] = cJSON_GetObjectItem(tsip, "de_port4")->valueint;
	gChn.de_port[5] = cJSON_GetObjectItem(tsip, "de_port5")->valueint;
	gChn.de_port[6] = cJSON_GetObjectItem(tsip, "de_port6")->valueint;
	gChn.de_port[7] = cJSON_GetObjectItem(tsip, "de_port7")->valueint;
	NetAddr_macStr2Bytes(cJSON_GetObjectItem(tsip, "de_mac0")->valuestring,21,gChn.de_mac[0]);
	NetAddr_macStr2Bytes(cJSON_GetObjectItem(tsip, "de_mac1")->valuestring,21,gChn.de_mac[1]);
	NetAddr_macStr2Bytes(cJSON_GetObjectItem(tsip, "de_mac2")->valuestring,21,gChn.de_mac[2]);
	NetAddr_macStr2Bytes(cJSON_GetObjectItem(tsip, "de_mac3")->valuestring,21,gChn.de_mac[3]);
	NetAddr_macStr2Bytes(cJSON_GetObjectItem(tsip, "de_mac4")->valuestring,21,gChn.de_mac[4]);
	NetAddr_macStr2Bytes(cJSON_GetObjectItem(tsip, "de_mac5")->valuestring,21,gChn.de_mac[5]);
	NetAddr_macStr2Bytes(cJSON_GetObjectItem(tsip, "de_mac6")->valuestring,21,gChn.de_mac[6]);
	NetAddr_macStr2Bytes(cJSON_GetObjectItem(tsip, "de_mac7")->valuestring,21,gChn.de_mac[7]);
	Cmd_SetTS2IP();
	
	for(i = 1; i <9; i++){
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "_channel%d", i);
		cJSON *_channel = cJSON_GetObjectItem(backupdatas, datastr);
		gChn.serviceCnt = cJSON_GetObjectItem(_channel, "serviceCnt")->valueint;
		for(j = 0; j < gChn.serviceCnt; j++)
		{
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "service%d_chnId", j);
			gChn.service[j].chnId = cJSON_GetObjectItem(_channel, datastr)->valueint;
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "service%d_isMuxed", j);
			gChn.service[j].isMuxed = cJSON_GetObjectItem(_channel, datastr)->valueint;
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "service%d_prgId", j);
			gChn.service[j].prgId = cJSON_GetObjectItem(_channel, datastr)->valueint;
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "service%d_pmtPid", j);
			gChn.service[j].pmtPid = cJSON_GetObjectItem(_channel, datastr)->valueint;
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "service%d_name_len", j);
			gChn.service[j].name_len = cJSON_GetObjectItem(_channel, datastr)->valueint;
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "service%d_name", j);
			memcpy(gChn.service[j].name, cJSON_GetObjectItem(_channel, datastr)->valuestring, gChn.service[j].name_len);
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "service%d_provider_len", j);
			gChn.service[j].provider_len = cJSON_GetObjectItem(_channel, datastr)->valueint;
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "service%d_provider", j);
			memcpy(gChn.service[j].provider, cJSON_GetObjectItem(_channel, datastr)->valuestring, gChn.service[j].provider_len);
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "service%d_isCA", j);
			gChn.service[j].isCA = cJSON_GetObjectItem(_channel, datastr)->valueint;
				
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "service_o_%d_name_len", j);
			gChn.service_o[j].name_len = cJSON_GetObjectItem(_channel, datastr)->valueint;
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "service_o_%d_name", j);
			memcpy(gChn.service_o[j].name, cJSON_GetObjectItem(_channel, datastr)->valuestring, gChn.service_o[j].name_len);
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "service_o_%d_provider_len", j);
			gChn.service_o[j].provider_len = cJSON_GetObjectItem(_channel, datastr)->valueint;
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "service_o_%d_provider", j);
			memcpy(gChn.service_o[j].provider, cJSON_GetObjectItem(_channel, datastr)->valuestring, gChn.service_o[j].provider_len);
			memset(datastr, 0, sizeof(datastr));			
		}
		Cmd_setChannel(i);
	}
	cJSON *trapsip = cJSON_GetObjectItem(backupdatas, "_trapserverip");
	char *trapip = cJSON_GetObjectItem(trapsip, "trapserverip")->valuestring;
	int fd;
	fd=open("/mnt/Nand2/net-snmp-arm/trapserver.txt",O_WRONLY|O_CREAT);
	write(fd, trapip, strlen(trapip));
	close(fd);
	*/
}

void WebGet_Export()
{
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL, node = NULL;
	xmlChar *xmlbuff;
	int buffersize;
	char datastr[32]= {0};
	if(Cmd_getTS2IP())
	{
		cgiHeaderContentType("application/xml");
		// Creates a new document, a node and set it as a root node
		doc = xmlNewDoc(BAD_CAST "1.0");
		root_node = xmlNewNode(NULL, BAD_CAST "root");
		xmlDocSetRootElement(doc, root_node);
		//Here goes another way to create nodes.
		node = xmlNewNode(NULL, BAD_CAST "_tsip");
		sprintf(datastr, "%d.%d.%d.%d", gChn.su_ip[0],  gChn.su_ip[1],  gChn.su_ip[2],  gChn.su_ip[3]);
		xmlNewTextChild(node, NULL, BAD_CAST "su_ip", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d", gChn.su_port);
		xmlNewTextChild(node, NULL, BAD_CAST "su_port", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d", gChn.udp_ttl);
		xmlNewTextChild(node, NULL, BAD_CAST "udp_ttl", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.su_mac[0],  gChn.su_mac[1],  gChn.su_mac[2],  gChn.su_mac[3],  gChn.su_mac[4],  gChn.su_mac[5]);
		xmlNewTextChild(node, NULL, BAD_CAST "su_mac", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[0][0],  gChn.de_ip[0][1],  gChn.de_ip[0][2],  gChn.de_ip[0][3]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_ip0", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[1][0],  gChn.de_ip[1][1],  gChn.de_ip[1][2],  gChn.de_ip[1][3]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_ip1", datastr);
		memset(datastr, 0, sizeof(datastr));	
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[2][0],  gChn.de_ip[2][1],  gChn.de_ip[2][2],  gChn.de_ip[2][3]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_ip2", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[3][0],  gChn.de_ip[3][1],  gChn.de_ip[3][2],  gChn.de_ip[3][3]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_ip3", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[4][0],  gChn.de_ip[4][1],  gChn.de_ip[4][2],  gChn.de_ip[4][3]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_ip4", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[5][0],  gChn.de_ip[5][1],  gChn.de_ip[5][2],  gChn.de_ip[5][3]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_ip5", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[6][0],  gChn.de_ip[6][1],  gChn.de_ip[6][2],  gChn.de_ip[6][3]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_ip6", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[7][0],  gChn.de_ip[7][1],  gChn.de_ip[7][2],  gChn.de_ip[7][3]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_ip7", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d", gChn.de_port[0]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_port0", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d", gChn.de_port[1]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_port1", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d", gChn.de_port[2]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_port2", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d", gChn.de_port[3]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_port3", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d", gChn.de_port[4]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_port4", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d", gChn.de_port[5]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_port5", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d", gChn.de_port[6]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_port6", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d", gChn.de_port[7]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_port7", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[0][0],  gChn.de_mac[0][1],  gChn.de_mac[0][2],  gChn.de_mac[0][3],  gChn.de_mac[0][4],  gChn.de_mac[0][5]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_mac0", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[1][0],  gChn.de_mac[1][1],  gChn.de_mac[1][2],  gChn.de_mac[1][3],  gChn.de_mac[1][4],  gChn.de_mac[1][5]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_mac1", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[2][0],  gChn.de_mac[2][1],  gChn.de_mac[2][2],  gChn.de_mac[2][3],  gChn.de_mac[2][4],  gChn.de_mac[2][5]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_mac2", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[3][0],  gChn.de_mac[3][1],  gChn.de_mac[3][2],  gChn.de_mac[3][3],  gChn.de_mac[3][4],  gChn.de_mac[3][5]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_mac3", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[4][0],  gChn.de_mac[4][1],  gChn.de_mac[4][2],  gChn.de_mac[4][3],  gChn.de_mac[4][4],  gChn.de_mac[4][5]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_mac4", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[5][0],  gChn.de_mac[5][1],  gChn.de_mac[5][2],  gChn.de_mac[5][3],  gChn.de_mac[5][4],  gChn.de_mac[5][5]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_mac5", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[6][0],  gChn.de_mac[6][1],  gChn.de_mac[6][2],  gChn.de_mac[6][3],  gChn.de_mac[6][4],  gChn.de_mac[6][5]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_mac6", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[7][0],  gChn.de_mac[7][1],  gChn.de_mac[7][2],  gChn.de_mac[7][3],  gChn.de_mac[7][4],  gChn.de_mac[7][5]);
		xmlNewTextChild(node, NULL, BAD_CAST "de_mac7", datastr);		
		xmlAddChild(root_node, node);
		int i = 0, j = 0;
		char tmpstr[32] = {0};
		for(i = 1; i <9; i++){
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "_channel%d", i);
			node = xmlNewNode(NULL, BAD_CAST datastr);
			Cmd_getChannel(i,1);
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "%d", i);
			xmlNewTextChild(node, NULL, BAD_CAST "serviceChnid", datastr);
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "%d", gChn.serviceCnt);
			xmlNewTextChild(node, NULL, BAD_CAST "serviceCnt", datastr);			
			for(j = 0; j < gChn.serviceCnt; j++)
			{
				memset(tmpstr, 0, sizeof(tmpstr));
				sprintf(tmpstr, "service%d_chnId", j);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "%d", gChn.service[j].chnId);
				xmlNewTextChild(node, NULL, tmpstr, datastr);
				memset(tmpstr, 0, sizeof(tmpstr));
				sprintf(tmpstr, "service%d_isMuxed", j);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "%d", gChn.service[j].isMuxed);
				xmlNewTextChild(node, NULL, tmpstr, datastr);
				memset(tmpstr, 0, sizeof(tmpstr));
				sprintf(tmpstr, "service%d_prgId", j);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "%d", gChn.service[j].prgId);
				xmlNewTextChild(node, NULL, tmpstr, datastr);
				memset(tmpstr, 0, sizeof(tmpstr));
				sprintf(tmpstr, "service%d_pmtPid", j);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "%d", gChn.service[j].pmtPid);
				xmlNewTextChild(node, NULL, tmpstr, datastr);
				memset(tmpstr, 0, sizeof(tmpstr));
				sprintf(tmpstr, "service%d_name_len", j);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "%d", gChn.service[j].name_len);
				xmlNewTextChild(node, NULL, tmpstr, datastr);
				memset(tmpstr, 0, sizeof(tmpstr));
				sprintf(tmpstr, "service%d_name", j);
				xmlNewTextChild(node, NULL, tmpstr, gChn.service[j].name);		
				memset(tmpstr, 0, sizeof(tmpstr));
				sprintf(tmpstr, "service%d_provider_len", j);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "%d", gChn.service[j].provider_len);
				xmlNewTextChild(node, NULL, tmpstr, datastr);
				memset(tmpstr, 0, sizeof(tmpstr));
				sprintf(tmpstr, "service%d_provider", j);
				xmlNewTextChild(node, NULL, tmpstr, gChn.service[j].provider);
				memset(tmpstr, 0, sizeof(tmpstr));
				sprintf(tmpstr, "service%d_isCA", j);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "%d", gChn.service[j].isCA);
				xmlNewTextChild(node, NULL, tmpstr, datastr);	
				memset(tmpstr, 0, sizeof(tmpstr));
				sprintf(tmpstr, "service_o_%d_name_len", j);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "%d", gChn.service_o[j].name_len);
				xmlNewTextChild(node, NULL, tmpstr, datastr);
				memset(tmpstr, 0, sizeof(tmpstr));
				sprintf(tmpstr, "service_o_%d_name", j);
				xmlNewTextChild(node, NULL, tmpstr, gChn.service_o[j].name);
				memset(tmpstr, 0, sizeof(tmpstr));
				sprintf(tmpstr, "service_o_%d_provider_len", j);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "%d", gChn.service_o[j].provider_len);
				xmlNewTextChild(node, NULL, tmpstr, datastr);
				memset(tmpstr, 0, sizeof(tmpstr));
				sprintf(tmpstr, "service_o_%d_provider", j);
				xmlNewTextChild(node, NULL, tmpstr, gChn.service_o[j].provider);
			}		
			
			xmlAddChild(root_node, node);			
		}
		node = xmlNewNode(NULL, BAD_CAST "_trapserverip");
		int fd=open("/mnt/Nand2/net-snmp-arm/trapserver.txt", O_RDONLY);
		memset(datastr, 0, sizeof(datastr));
		read(fd, datastr, sizeof(datastr));
		close(fd);
		xmlNewTextChild(node, NULL, BAD_CAST "trapserverip", datastr);
		xmlAddChild(root_node, node);
			
		xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, 1);
		CgiPrint((char *)xmlbuff);
		xmlFreeDoc(doc);
		xmlCleanupParser();
		xmlMemoryDump();//debug memory for regression tests 
	
	}
	
	
	/*int i = 0, j = 0;
	if(Cmd_getTS2IP())
	{
		cJSON *rootjson, *basejson,*itemjson, *itemarry, *subjson;
		cgiHeaderContentType("application/json");
		rootjson= cJSON_CreateObject();
		cJSON_AddItemToObject(rootjson, "root", basejson= cJSON_CreateObject());
		cJSON_AddItemToObject(basejson, "_tsip", itemjson= cJSON_CreateObject());
		char datastr[32]= {0};
		sprintf(datastr, "%d.%d.%d.%d", gChn.su_ip[0],  gChn.su_ip[1],  gChn.su_ip[2],  gChn.su_ip[3]);
		cJSON_AddStringToObject(itemjson,"su_ip", datastr);
		cJSON_AddNumberToObject(itemjson,"su_port", gChn.su_port);
		cJSON_AddNumberToObject(itemjson,"udp_ttl", gChn.udp_ttl);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.su_mac[0],  gChn.su_mac[1],  gChn.su_mac[2],  gChn.su_mac[3],  gChn.su_mac[4],  gChn.su_mac[5]);
		cJSON_AddStringToObject(itemjson,"su_mac", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[0][0],  gChn.de_ip[0][1],  gChn.de_ip[0][2],  gChn.de_ip[0][3]);
		cJSON_AddStringToObject(itemjson,"de_ip0", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[1][0],  gChn.de_ip[1][1],  gChn.de_ip[1][2],  gChn.de_ip[1][3]);
		cJSON_AddStringToObject(itemjson,"de_ip1", datastr);
		memset(datastr, 0, sizeof(datastr));	
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[2][0],  gChn.de_ip[2][1],  gChn.de_ip[2][2],  gChn.de_ip[2][3]);
		cJSON_AddStringToObject(itemjson,"de_ip2", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[3][0],  gChn.de_ip[3][1],  gChn.de_ip[3][2],  gChn.de_ip[3][3]);
		cJSON_AddStringToObject(itemjson,"de_ip3", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[4][0],  gChn.de_ip[4][1],  gChn.de_ip[4][2],  gChn.de_ip[4][3]);
		cJSON_AddStringToObject(itemjson,"de_ip4", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[5][0],  gChn.de_ip[5][1],  gChn.de_ip[5][2],  gChn.de_ip[5][3]);
		cJSON_AddStringToObject(itemjson,"de_ip5", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[6][0],  gChn.de_ip[6][1],  gChn.de_ip[6][2],  gChn.de_ip[6][3]);
		cJSON_AddStringToObject(itemjson,"de_ip6", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%d.%d.%d.%d", gChn.de_ip[7][0],  gChn.de_ip[7][1],  gChn.de_ip[7][2],  gChn.de_ip[7][3]);
		cJSON_AddStringToObject(itemjson,"de_ip7", datastr);
		cJSON_AddNumberToObject(itemjson,"de_port0", gChn.de_port[0]);
		cJSON_AddNumberToObject(itemjson,"de_port1", gChn.de_port[1]);
		cJSON_AddNumberToObject(itemjson,"de_port2", gChn.de_port[2]);
		cJSON_AddNumberToObject(itemjson,"de_port3", gChn.de_port[3]);
		cJSON_AddNumberToObject(itemjson,"de_port4", gChn.de_port[4]);
		cJSON_AddNumberToObject(itemjson,"de_port5", gChn.de_port[5]);
		cJSON_AddNumberToObject(itemjson,"de_port6", gChn.de_port[6]);
		cJSON_AddNumberToObject(itemjson,"de_port7", gChn.de_port[7]);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[0][0],  gChn.de_mac[0][1],  gChn.de_mac[0][2],  gChn.de_mac[0][3],  gChn.de_mac[0][4],  gChn.de_mac[0][5]);
		cJSON_AddStringToObject(itemjson,"de_mac0", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[1][0],  gChn.de_mac[1][1],  gChn.de_mac[1][2],  gChn.de_mac[1][3],  gChn.de_mac[1][4],  gChn.de_mac[1][5]);
		cJSON_AddStringToObject(itemjson,"de_mac1", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[2][0],  gChn.de_mac[2][1],  gChn.de_mac[2][2],  gChn.de_mac[2][3],  gChn.de_mac[2][4],  gChn.de_mac[2][5]);
		cJSON_AddStringToObject(itemjson,"de_mac2", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[3][0],  gChn.de_mac[3][1],  gChn.de_mac[3][2],  gChn.de_mac[3][3],  gChn.de_mac[3][4],  gChn.de_mac[3][5]);
		cJSON_AddStringToObject(itemjson,"de_mac3", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[4][0],  gChn.de_mac[4][1],  gChn.de_mac[4][2],  gChn.de_mac[4][3],  gChn.de_mac[4][4],  gChn.de_mac[4][5]);
		cJSON_AddStringToObject(itemjson,"de_mac4", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[5][0],  gChn.de_mac[5][1],  gChn.de_mac[5][2],  gChn.de_mac[5][3],  gChn.de_mac[5][4],  gChn.de_mac[5][5]);
		cJSON_AddStringToObject(itemjson,"de_mac5", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[6][0],  gChn.de_mac[6][1],  gChn.de_mac[6][2],  gChn.de_mac[6][3],  gChn.de_mac[6][4],  gChn.de_mac[6][5]);
		cJSON_AddStringToObject(itemjson,"de_mac6", datastr);
		memset(datastr, 0, sizeof(datastr));
		sprintf(datastr, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[7][0],  gChn.de_mac[7][1],  gChn.de_mac[7][2],  gChn.de_mac[7][3],  gChn.de_mac[7][4],  gChn.de_mac[7][5]);
		cJSON_AddStringToObject(itemjson,"de_mac7", datastr);		
		
		for(i = 1; i <9; i++){
			memset(datastr, 0, sizeof(datastr));
			sprintf(datastr, "_channel%d", i);
			cJSON_AddItemToObject(basejson, datastr, itemjson = cJSON_CreateObject());
			Cmd_getChannel(i,1);
			cJSON_AddNumberToObject(itemjson,"serviceCnt", gChn.serviceCnt);
			for(j = 0; j < gChn.serviceCnt; j++)
			{
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "service%d_chnId", j);
				cJSON_AddNumberToObject(itemjson, datastr, gChn.service[j].chnId);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "service%d_isMuxed", j);
				cJSON_AddNumberToObject(itemjson, datastr, gChn.service[j].isMuxed);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "service%d_prgId", j);
				cJSON_AddNumberToObject(itemjson, datastr, gChn.service[j].prgId);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "service%d_pmtPid", j);
				cJSON_AddNumberToObject(itemjson, datastr, gChn.service[j].pmtPid);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "service%d_name_len", j);
				cJSON_AddNumberToObject(itemjson, datastr, gChn.service[j].name_len);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "service%d_name", j);
				cJSON_AddStringToObject(itemjson, datastr, gChn.service[j].name);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "service%d_provider_len", j);
				cJSON_AddNumberToObject(itemjson, datastr, gChn.service[j].provider_len);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "service%d_provider", j);
				cJSON_AddStringToObject(itemjson, datastr, gChn.service[j].provider);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "service%d_isCA", j);
				cJSON_AddNumberToObject(itemjson, datastr, gChn.service[j].isCA);
				
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "service_o_%d_name_len", j);
				cJSON_AddNumberToObject(itemjson, datastr, gChn.service_o[j].name_len);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "service_o_%d_name", j);
				cJSON_AddStringToObject(itemjson, datastr, gChn.service_o[j].name);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "service_o_%d_provider_len", j);
				cJSON_AddNumberToObject(itemjson, datastr, gChn.service_o[j].provider_len);
				memset(datastr, 0, sizeof(datastr));
				sprintf(datastr, "service_o_%d_provider", j);
				cJSON_AddStringToObject(itemjson, datastr, gChn.service_o[j].provider);
			}
		
		}
		cJSON_AddItemToObject(basejson, "_trapserverip", itemjson= cJSON_CreateObject());
		int fd=open("/mnt/Nand2/net-snmp-arm/trapserver.txt", O_RDONLY);
		memset(datastr, 0, sizeof(datastr));
		read(fd, datastr, sizeof(datastr));
		close(fd);
		cJSON_AddStringToObject(itemjson, "trapserverip", datastr);
		
		
		CgiPrint(cJSON_PrintUnformatted(rootjson));		
		cJSON_Delete(rootjson);
		
	}
	*/
}

void WebGet_Backup()
{
	char strdata[1024] = {0}; 
	StHtml2Cgi_item_t items[]= {
		{"@@formparams['updatas']", strdata, NULL, EmValueType_null},
	};
	
	Html2Cgi_parse(WEB_BACKUP_HTML
		, items, sizeof(items)/sizeof(StHtml2Cgi_item_t));
}


void WebSubmit_net()
{
#define IP_MASK_BUF_MAXLEN 23
	char ipBuf[IP_MASK_BUF_MAXLEN+1];
	char ipMaskBuf[IP_MASK_BUF_MAXLEN+1];
	char ipGatewayBuf[IP_MASK_BUF_MAXLEN+1];
	char trapipBuf[IP_MASK_BUF_MAXLEN+1];
	
	char href[32], title[32];
	u32_t tmpU32;

	sprintf(href, "/cgi-bin/index.cgi?net=1");

	if(cgiFormSuccess != cgiFormString("ip", ipBuf, 23))
	{
		Html2Cgi_waitGoto(href, "Save IP Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("mask", ipMaskBuf, 23))
	{
		Html2Cgi_waitGoto(href, "Save Mask Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("gw", ipGatewayBuf, 23))
	{
		Html2Cgi_waitGoto(href, "Save IP Gateway Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("trapserverip", trapipBuf, 23))
	{
		Html2Cgi_waitGoto(href, "Save TrapServerIP Addr Err,Please Check!");
		return;
	}

	tmpU32 = NetAddr_ipStr2CmdT(ipBuf, 23);
     gChn.my_ip[0]=tmpU32>>24;
	gChn.my_ip[1]=tmpU32>>16;
	gChn.my_ip[2]=tmpU32>>8;
	gChn.my_ip[3]=tmpU32>>0;

	tmpU32 = NetAddr_ipStr2CmdT(ipMaskBuf, 23);
     gChn.my_mask[0]=tmpU32>>24;
	gChn.my_mask[1]=tmpU32>>16;
	gChn.my_mask[2]=tmpU32>>8;
	gChn.my_mask[3]=tmpU32>>0;

	tmpU32 = NetAddr_ipStr2CmdT(ipGatewayBuf, 23);
     gChn.my_gateway[0]=tmpU32>>24;
	gChn.my_gateway[1]=tmpU32>>16;
	gChn.my_gateway[2]=tmpU32>>8;
	gChn.my_gateway[3]=tmpU32>>0;	
	Cmd_submitNet();
	//change trap ip
	int fd;
	char trapstr[15] = {0};
	fd=open("/mnt/Nand2/net-snmp-arm/trapserver.txt",O_WRONLY|O_CREAT);
	read(fd, trapstr, sizeof(trapstr));
	
	if(strcmp(trapstr, trapipBuf)){
		write(fd, trapipBuf, 15);
	}
	sprintf(title, "Saving Network ...");
	Html2Cgi_waitGoto(href, title);
	close(fd);
}

void WebGet_system()
{
	u8_t verBytes[10];
	u8_t *pb;
	char verHw[8];
	char verSw[8];
	char verFw[16];
	StHtml2Cgi_item_t items[]= {
		{"@@formparams['verHw']", verHw, NULL, EmValueType_null},
		{"@@formparams['verSw']", verSw, NULL, EmValueType_null},
		{"@@formparams['verFw']", verFw, NULL, EmValueType_null},
	};
	
	memset(verBytes, 0, sizeof(verBytes));
	Cmd_getVersion(verBytes);

	pb = verBytes;
	sprintf(verHw, "%d.%02d", pb[0], pb[1]);
	pb += 2;
	sprintf(verSw, "%d.%02d", pb[0], pb[1]);
	pb += 2;
	sprintf(verFw, "%03d", pb[0], pb[1], pb[2], pb[3]);
	pb += 2;

	Html2Cgi_parse(WEB_SYSTEM_HTML
		, items, sizeof(items)/sizeof(StHtml2Cgi_item_t));

}
void WebGet_Tsip()
{

	char su_ip[]=	"192.168.000.138";
	char su_port[]="66666";
	char su_mac[]="00:1A:A0:08:06:00";
	char udp_ttl[]="255255";
	
     char de_ip0[]="192.168.000.130";
	char de_ip1[]="192.168.000.131";
	char de_ip2[]="192.168.000.132";
	char de_ip3[]="192.168.000.133";
	char de_ip4[]="192.168.000.134";
	char de_ip5[]="192.168.000.135";
	char de_ip6[]="192.168.000.136";
	char de_ip7[]="192.168.000.137";

     char de_port0[]="100000";
	char de_port1[]="100001";
	char de_port2[]="100002";
	char de_port3[]="100003";
	char de_port4[]="100004";
	char de_port5[]="100005";
	char de_port6[]="100006";
	char de_port7[]="100007";

	char de_mac0[]="00:1A:A0:08:01:00";
	char de_mac1[]="00:1A:A0:08:02:00";
	char de_mac2[]="00:1A:A0:08:03:00";
	char de_mac3[]="00:1A:A0:08:04:00";
	char de_mac4[]="00:1A:A0:08:05:00";
	char de_mac5[]="00:1A:A0:08:06:00";
	char de_mac6[]="00:1A:A0:08:07:00";
	char de_mac7[]="00:1A:A0:08:08:00";


	char href[32], title[32];

	sprintf(href, "/cgi-bin/index.cgi?transmit=1");	

	StHtml2Cgi_item_t items[]= {
		{"@@formparams['su_ip']", su_ip, NULL, EmValueType_null},
		{"@@formparams['su_port']", su_port, NULL, EmValueType_null},
		{"@@formparams['su_mac']", su_mac, NULL, EmValueType_null},
		{"@@formparams['udp_ttl']", udp_ttl, NULL, EmValueType_null},

		{"@@formparams['de_ip0']", de_ip0, NULL, EmValueType_null},
		{"@@formparams['de_ip1']", de_ip1, NULL, EmValueType_null},
		{"@@formparams['de_ip2']", de_ip2, NULL, EmValueType_null},
		{"@@formparams['de_ip3']", de_ip3, NULL, EmValueType_null},
		{"@@formparams['de_ip4']", de_ip4, NULL, EmValueType_null},
		{"@@formparams['de_ip5']", de_ip5, NULL, EmValueType_null},
		{"@@formparams['de_ip6']", de_ip6, NULL, EmValueType_null},
		{"@@formparams['de_ip7']", de_ip7, NULL, EmValueType_null},

		{"@@formparams['de_port0']", de_port0, NULL, EmValueType_null},
		{"@@formparams['de_port1']", de_port1, NULL, EmValueType_null},
		{"@@formparams['de_port2']", de_port2, NULL, EmValueType_null},
		{"@@formparams['de_port3']", de_port3, NULL, EmValueType_null},
		{"@@formparams['de_port4']", de_port4, NULL, EmValueType_null},
		{"@@formparams['de_port5']", de_port5, NULL, EmValueType_null},
		{"@@formparams['de_port6']", de_port6, NULL, EmValueType_null},
		{"@@formparams['de_port7']", de_port7, NULL, EmValueType_null},

		{"@@formparams['de_mac0']", de_mac0, NULL, EmValueType_null},
		{"@@formparams['de_mac1']", de_mac1, NULL, EmValueType_null},
		{"@@formparams['de_mac2']", de_mac2, NULL, EmValueType_null},
		{"@@formparams['de_mac3']", de_mac3, NULL, EmValueType_null},
		{"@@formparams['de_mac4']", de_mac4, NULL, EmValueType_null},
		{"@@formparams['de_mac5']", de_mac5, NULL, EmValueType_null},
		{"@@formparams['de_mac6']", de_mac6, NULL, EmValueType_null},
		{"@@formparams['de_mac7']", de_mac7, NULL, EmValueType_null},
	};


	if(Cmd_getTS2IP())
	{
	//	gChn.udp_ttl=64;
		sprintf(su_ip, "%d.%d.%d.%d", gChn.su_ip[0],  gChn.su_ip[1],  gChn.su_ip[2],  gChn.su_ip[3]);
		sprintf(su_port, "%d", gChn.su_port);
		sprintf(udp_ttl, "%d", gChn.udp_ttl);
		sprintf(su_mac, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.su_mac[0],  gChn.su_mac[1],  gChn.su_mac[2],  gChn.su_mac[3],  gChn.su_mac[4],  gChn.su_mac[5]);
		
		sprintf(de_ip0, "%d.%d.%d.%d", gChn.de_ip[0][0],  gChn.de_ip[0][1],  gChn.de_ip[0][2],  gChn.de_ip[0][3]);
		sprintf(de_ip1, "%d.%d.%d.%d", gChn.de_ip[1][0],  gChn.de_ip[1][1],  gChn.de_ip[1][2],  gChn.de_ip[1][3]);
		sprintf(de_ip2, "%d.%d.%d.%d", gChn.de_ip[2][0],  gChn.de_ip[2][1],  gChn.de_ip[2][2],  gChn.de_ip[2][3]);
		sprintf(de_ip3, "%d.%d.%d.%d", gChn.de_ip[3][0],  gChn.de_ip[3][1],  gChn.de_ip[3][2],  gChn.de_ip[3][3]);
		sprintf(de_ip4, "%d.%d.%d.%d", gChn.de_ip[4][0],  gChn.de_ip[4][1],  gChn.de_ip[4][2],  gChn.de_ip[4][3]);
		sprintf(de_ip5, "%d.%d.%d.%d", gChn.de_ip[5][0],  gChn.de_ip[5][1],  gChn.de_ip[5][2],  gChn.de_ip[5][3]);
		sprintf(de_ip6, "%d.%d.%d.%d", gChn.de_ip[6][0],  gChn.de_ip[6][1],  gChn.de_ip[6][2],  gChn.de_ip[6][3]);
		sprintf(de_ip7, "%d.%d.%d.%d", gChn.de_ip[7][0],  gChn.de_ip[7][1],  gChn.de_ip[7][2],  gChn.de_ip[7][3]);

		sprintf(de_port0, "%d", gChn.de_port[0]);
		sprintf(de_port1, "%d", gChn.de_port[1]);
		sprintf(de_port2, "%d", gChn.de_port[2]);
		sprintf(de_port3, "%d", gChn.de_port[3]);
		sprintf(de_port4, "%d", gChn.de_port[4]);
		sprintf(de_port5, "%d", gChn.de_port[5]);
		sprintf(de_port6, "%d", gChn.de_port[6]);
		sprintf(de_port7, "%d", gChn.de_port[7]);

		sprintf(de_mac0, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[0][0],  gChn.de_mac[0][1],  gChn.de_mac[0][2],  gChn.de_mac[0][3],  gChn.de_mac[0][4],  gChn.de_mac[0][5]);
		sprintf(de_mac1, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[1][0],  gChn.de_mac[1][1],  gChn.de_mac[1][2],  gChn.de_mac[1][3],  gChn.de_mac[1][4],  gChn.de_mac[1][5]);
		sprintf(de_mac2, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[2][0],  gChn.de_mac[2][1],  gChn.de_mac[2][2],  gChn.de_mac[2][3],  gChn.de_mac[2][4],  gChn.de_mac[2][5]);
		sprintf(de_mac3, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[3][0],  gChn.de_mac[3][1],  gChn.de_mac[3][2],  gChn.de_mac[3][3],  gChn.de_mac[3][4],  gChn.de_mac[3][5]);
		sprintf(de_mac4, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[4][0],  gChn.de_mac[4][1],  gChn.de_mac[4][2],  gChn.de_mac[4][3],  gChn.de_mac[4][4],  gChn.de_mac[4][5]);
		sprintf(de_mac5, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[5][0],  gChn.de_mac[5][1],  gChn.de_mac[5][2],  gChn.de_mac[5][3],  gChn.de_mac[5][4],  gChn.de_mac[5][5]);
		sprintf(de_mac6, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[6][0],  gChn.de_mac[6][1],  gChn.de_mac[6][2],  gChn.de_mac[6][3],  gChn.de_mac[6][4],  gChn.de_mac[6][5]);
		sprintf(de_mac7, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.de_mac[7][0],  gChn.de_mac[7][1],  gChn.de_mac[7][2],  gChn.de_mac[7][3],  gChn.de_mac[7][4],  gChn.de_mac[7][5]);
		//sprintf(title, "%d", gChn.udp_ttl); 
	//sprintf(title, "Saving Network ...");//"正在保存 [网络参数] ...");
	//Html2Cgi_waitGoto(href, udp_ttl);
		
	}
	
	Html2Cgi_parse(WEB_TP_IP_HTML
		, items, sizeof(items)/sizeof(StHtml2Cgi_item_t));
}
void WebSet_Tsip()
{
	#define IP_MASK_BUF_MAXLEN 30
	
	char su_ip[IP_MASK_BUF_MAXLEN+1];
	char su_port[IP_MASK_BUF_MAXLEN+1];
	char su_mac[IP_MASK_BUF_MAXLEN+1];
	char udp_ttl[IP_MASK_BUF_MAXLEN+1];
	
	char de_ip0[IP_MASK_BUF_MAXLEN+1];
	char de_ip1[IP_MASK_BUF_MAXLEN+1];
	char de_ip2[IP_MASK_BUF_MAXLEN+1];
	char de_ip3[IP_MASK_BUF_MAXLEN+1];
	char de_ip4[IP_MASK_BUF_MAXLEN+1];
	char de_ip5[IP_MASK_BUF_MAXLEN+1];
	char de_ip6[IP_MASK_BUF_MAXLEN+1];
	char de_ip7[IP_MASK_BUF_MAXLEN+1];

	char de_port0[IP_MASK_BUF_MAXLEN+1];
	char de_port1[IP_MASK_BUF_MAXLEN+1];
	char de_port2[IP_MASK_BUF_MAXLEN+1];
	char de_port3[IP_MASK_BUF_MAXLEN+1];
	char de_port4[IP_MASK_BUF_MAXLEN+1];
	char de_port5[IP_MASK_BUF_MAXLEN+1];
	char de_port6[IP_MASK_BUF_MAXLEN+1];
	char de_port7[IP_MASK_BUF_MAXLEN+1];

	char de_mac0[IP_MASK_BUF_MAXLEN+1];
	char de_mac1[IP_MASK_BUF_MAXLEN+1];
	char de_mac2[IP_MASK_BUF_MAXLEN+1];
	char de_mac3[IP_MASK_BUF_MAXLEN+1];
	char de_mac4[IP_MASK_BUF_MAXLEN+1];
	char de_mac5[IP_MASK_BUF_MAXLEN+1];
	char de_mac6[IP_MASK_BUF_MAXLEN+1];
	char de_mac7[IP_MASK_BUF_MAXLEN+1];
	
	char href[32], title[32];
	
	u32_t tmpU32;

	sprintf(href, "/cgi-bin/index.cgi?transmit=1");

	if(cgiFormSuccess != cgiFormString("su_ip", su_ip, 23))
	{
		Html2Cgi_waitGoto(href, "Save Source IP Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("su_port", su_port, 23))
	{
		Html2Cgi_waitGoto(href, "Save Source Port  Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("udp_ttl", udp_ttl, 23))
	{
		Html2Cgi_waitGoto(href, "Save UDP TTL Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("su_mac", su_mac, 23))
	{
		Html2Cgi_waitGoto(href, "Save Source  MAC Addr Err,Please Check!");
		return;
	}
	/****************************************************************************/

	if(cgiFormSuccess != cgiFormString("de_ip0", de_ip0, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-1 IP Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_ip1", de_ip1, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-2 IP Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_ip2", de_ip2, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-3 IP Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_ip3", de_ip3, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-4 IP Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_ip4", de_ip4, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-5 IP Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_ip5", de_ip5, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-6 IP Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_ip6", de_ip6, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-7 IP Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_ip7", de_ip7, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-8 IP Addr Err,Please Check!");
		return;
	}
	/****************************************************************************/

	if(cgiFormSuccess != cgiFormString("de_port0", de_port0, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-1 Port Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_port1", de_port1, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-2 Port Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_port2", de_port2, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-3 Port Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_port3", de_port3, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-4 Port Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_port4", de_port4, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-5 Port Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_port5", de_port5, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-6 Port Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_port6", de_port6, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-7 Port Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_port7", de_port7, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-8 Port Err,Please Check!");
		return;
	}
	/****************************************************************************/
	if(cgiFormSuccess != cgiFormString("de_mac0", de_mac0, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-1 MAC Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_mac1", de_mac1, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-2 MAC Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_mac2", de_mac2, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-3 MAC Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_mac3", de_mac3, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-4 MAC Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_mac4", de_mac4, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-5 MAC Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_mac5", de_mac5, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-6 MAC Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_mac6", de_mac6, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-7 MAC Addr Err,Please Check!");
		return;
	}
	if(cgiFormSuccess != cgiFormString("de_mac7", de_mac7, 23))
	{
		Html2Cgi_waitGoto(href, "Save Target CH-8 MAC Addr Err,Please Check!");
		return;
	}
	tmpU32 = NetAddr_ipStr2CmdT(su_ip, 23);gChn.su_ip[0]=tmpU32>>24;gChn.su_ip[1]=tmpU32>>16;gChn.su_ip[2]=tmpU32>>8;gChn.su_ip[3]=tmpU32>>0;
	tmpU32 = NetPort_Str2CmdT(su_port);gChn.su_port=tmpU32;
	NetAddr_macStr2Bytes(su_mac,21,gChn.su_mac);
	tmpU32 = NetPort_Str2CmdT(udp_ttl);gChn.udp_ttl=tmpU32;

	tmpU32 = NetAddr_ipStr2CmdT(de_ip0, 23);gChn.de_ip[0][0]=tmpU32>>24;gChn.de_ip[0][1]=tmpU32>>16;gChn.de_ip[0][2]=tmpU32>>8;gChn.de_ip[0][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(de_ip1, 23);gChn.de_ip[1][0]=tmpU32>>24;gChn.de_ip[1][1]=tmpU32>>16;gChn.de_ip[1][2]=tmpU32>>8;gChn.de_ip[1][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(de_ip2, 23);gChn.de_ip[2][0]=tmpU32>>24;gChn.de_ip[2][1]=tmpU32>>16;gChn.de_ip[2][2]=tmpU32>>8;gChn.de_ip[2][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(de_ip3, 23);gChn.de_ip[3][0]=tmpU32>>24;gChn.de_ip[3][1]=tmpU32>>16;gChn.de_ip[3][2]=tmpU32>>8;gChn.de_ip[3][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(de_ip4, 23);gChn.de_ip[4][0]=tmpU32>>24;gChn.de_ip[4][1]=tmpU32>>16;gChn.de_ip[4][2]=tmpU32>>8;gChn.de_ip[4][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(de_ip5, 23);gChn.de_ip[5][0]=tmpU32>>24;gChn.de_ip[5][1]=tmpU32>>16;gChn.de_ip[5][2]=tmpU32>>8;gChn.de_ip[5][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(de_ip6, 23);gChn.de_ip[6][0]=tmpU32>>24;gChn.de_ip[6][1]=tmpU32>>16;gChn.de_ip[6][2]=tmpU32>>8;gChn.de_ip[6][3]=tmpU32>>0;
	tmpU32 = NetAddr_ipStr2CmdT(de_ip7, 23);gChn.de_ip[7][0]=tmpU32>>24;gChn.de_ip[7][1]=tmpU32>>16;gChn.de_ip[7][2]=tmpU32>>8;gChn.de_ip[7][3]=tmpU32>>0;
	tmpU32 = NetPort_Str2CmdT(de_port0);gChn.de_port[0]=tmpU32;
	tmpU32 = NetPort_Str2CmdT(de_port1);gChn.de_port[1]=tmpU32;
	tmpU32 = NetPort_Str2CmdT(de_port2);gChn.de_port[2]=tmpU32;
	tmpU32 = NetPort_Str2CmdT(de_port3);gChn.de_port[3]=tmpU32;
	tmpU32 = NetPort_Str2CmdT(de_port4);gChn.de_port[4]=tmpU32;
	tmpU32 = NetPort_Str2CmdT(de_port5);gChn.de_port[5]=tmpU32;
	tmpU32 = NetPort_Str2CmdT(de_port6);gChn.de_port[6]=tmpU32;
	tmpU32 = NetPort_Str2CmdT(de_port7);gChn.de_port[7]=tmpU32;
	NetAddr_macStr2Bytes(de_mac0,21,gChn.de_mac[0]);
	NetAddr_macStr2Bytes(de_mac1,21,gChn.de_mac[1]);
	NetAddr_macStr2Bytes(de_mac2,21,gChn.de_mac[2]);
	NetAddr_macStr2Bytes(de_mac3,21,gChn.de_mac[3]);
	NetAddr_macStr2Bytes(de_mac4,21,gChn.de_mac[4]);
	NetAddr_macStr2Bytes(de_mac5,21,gChn.de_mac[5]);
	NetAddr_macStr2Bytes(de_mac6,21,gChn.de_mac[6]);
	NetAddr_macStr2Bytes(de_mac7,21,gChn.de_mac[7]);
	//sprintf(title, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.su_mac[0],  gChn.su_mac[1],  gChn.su_mac[2],  gChn.su_mac[3],  gChn.su_mac[4],  gChn.su_mac[5]); 
	sprintf(title, "Saving Network ...");//"正在保存 [网络参数] ...");
	Html2Cgi_waitGoto(href, title);
	Cmd_SetTS2IP();
}

void WebSubmit_upload()
{
	//upload_main();
	WebGet_system();
}

void WebSubmit_factory()
{
	Cmd_submitFactory();
	
	Html2Cgi_waitGoto( "/cgi-bin/index.cgi?system=1", "Restore the device to factory settings ...");//"正在 [恢复出厂设置] ...");
}

void WebSubmit_reboot()
{
	Cmd_submitReboot();
	Html2Cgi_waitGoto( "/cgi-bin/index.cgi?system=1", "Rebooting ...");//"正在 [重启设备] ...");

}

void WebGet_test()
{
#if 1
	Html2Cgi_parse(WEB_TEST_HTML, NULL, 0);
#else
	cgiHeaderContentType("text/html");

	fprintf(cgiOut, "<HTML><HEAD><TITLE>My First CGI</TITLE></HEAD><BODY><H1>Hello CGIC2</H1></BODY></HTML>\n");
//	printf("<HTML><HEAD><TITLE>My First CGI</TITLE></HEAD><BODY><H1>Hello CGIC</H1></BODY></HTML>\n");
#endif
}



void WebEditP_Test()
{
		char href[32], title[32];
		sprintf(href, "/cgi-bin/index.cgi?tsOut=1");
		sprintf(title, "Save Program");//"正在保存 [网络参数] ...");
		Html2Cgi_waitGoto(href, title);
}

