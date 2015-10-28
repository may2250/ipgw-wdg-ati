#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#include "public.h"
#include "cmd.h"
#include "alarm.h"


#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

T_UDP_SK_INFO SK_ALARM;
T_UDP_SK_INFO SK_TOSNMP;
st_dbsNetwork g_networkInfo;

#define CMD_HEAD_1 0x77
#define CMD_HEAD_2 0x6C

oid objid_sysuptime[] =	{ 1, 3, 6, 1, 2, 1, 1, 3, 0 };
oid objid_snmptrap[] =	{ 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
oid objid_alarmcode[] =	{ 1, 3, 6, 1, 4, 1, 3445, 8080, 5, 1 };
oid objid_serialflow[] =	{ 1, 3, 6, 1, 4, 1, 3445, 8080, 5, 2 };
oid objid_hostmac[] =	{ 1, 3, 6, 1, 4, 1, 3445, 8080, 5, 3 };
oid objid_alarmtype[] =	{ 1, 3, 6, 1, 4, 1, 3445, 8080, 5, 6 };
oid objid_alarmvalue[] =	{ 1, 3, 6, 1, 4, 1, 3445, 8080, 5, 7 };
oid objid_trapinfo[] =		{ 1, 3, 6, 1, 4, 1, 3445, 8080, 5, 8 };
oid objid_muxhb[] =		{ 1, 3, 6, 1, 4, 1, 3445, 8080, 6, 1 };

char *trapserver = NULL;

int init_socket(T_UDP_SK_INFO *sk)
{
	int len;
	len=sizeof(sk->skaddr);
	bzero(&sk->skaddr, sizeof(sk->skaddr));
	switch(sk->desc)
	{
		case DESC_SOCKET_ALARM:			
			sk->sk= socket(AF_INET, SOCK_DGRAM, 0);
			sk->skaddr.sin_family = AF_INET;
			sk->skaddr.sin_port = htons(ALARM_LISTEN_PORT);
			sk->skaddr.sin_addr.s_addr= htonl(INADDR_ANY);
			
			if(bind(sk->sk ,(struct sockaddr *)&sk->skaddr,sizeof(sk->skaddr))<0)
			{
				perror("bind error");
				exit(0);
  			}
			break;
		case DESC_SOCKET_OTHER:		
			sk->sk= socket(AF_INET, SOCK_DGRAM, 0);
			sk->skaddr.sin_family = AF_INET;
			sk->skaddr.sin_port = htons(SNMP_ALARM_PORT);		
			sk->skaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
			break;
	}
	return CMM_SUCCESS;
}

void SignalProcessHandle(int n)
{
	//close(SK_ALARM.sk);
	close(SK_TOSNMP.sk);
	exit(0);
}

int getNetworkinfo(st_dbsNetwork *g_networkInfo){
    //g_networkInfo->col_ip = ip_search();
    memset(g_networkInfo, 0, sizeof(st_dbsNetwork));
	
	uint8_t cmdGetIp[] =       				 {CMD_HEAD_1, CMD_HEAD_2, 0X02, 0X01, 0X01};
	uint8_t cmdGetMask[] =       		 {CMD_HEAD_1, CMD_HEAD_2, 0X02, 0X02, 0X01};
	uint8_t cmdGetGateway[] =        {CMD_HEAD_1, CMD_HEAD_2, 0X02, 0X03, 0X01};
	uint8_t cmdGetmac[] =       			 {CMD_HEAD_1, CMD_HEAD_2, 0X02, 0X04, 0X01};
	
	uint8_t rdBuf[20];
	

	if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
		return 0;
    if(Cmd_rwParam(cmdGetIp, 6, rdBuf, 20, 5) < 5)/*»ñÈ¡IPµØÖ·*/
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
        sprintf(g_networkInfo->col_ip, "%d.%d.%d.%d", gChn.my_ip[0],  gChn.my_ip[1],  gChn.my_ip[2],  gChn.my_ip[3]);
		 if(Cmd_rwParam(cmdGetMask, 6, rdBuf, 20, 5) < 5)/*»ñÈ¡MASKµØÖ·*/
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
            sprintf(g_networkInfo->col_netmask, "%d.%d.%d.%d", gChn.my_mask[0],  gChn.my_mask[1],  gChn.my_mask[2],  gChn.my_mask[3]);
		 }
		  if(Cmd_rwParam(cmdGetGateway, 6, rdBuf, 20, 5) < 5)/*»ñÈ¡GatewayµØÖ·*/
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
                sprintf(g_networkInfo->col_gw, "%d.%d.%d.%d", gChn.my_gateway[0],  gChn.my_gateway[1],  gChn.my_gateway[2],  gChn.my_gateway[3]);
			 }
			if(Cmd_rwParam(cmdGetmac, 6, rdBuf, 20, 5) < 5)/*»ñÈ¡GatewayµØÖ·*/
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
                sprintf(g_networkInfo->col_mac, "%02X:%02X:%02X:%02X:%02X:%02X", gChn.my_mac[0],  gChn.my_mac[1],  gChn.my_mac[2],  gChn.my_mac[3],  gChn.my_mac[4],  gChn.my_mac[5]);
			 }
	}
	TcpClt_close();
	return 1;
    
}

int recieve_alarm_notification(st_channelMux *alarminfo)
{
	assert( NULL != alarminfo );
	
	uint32_t len = 0;
	int FromAddrSize = 0;	
	uint8_t buffer[MAX_UDP_SIZE];
	time_t b_time;
	T_Msg_CMM *msg = (T_Msg_CMM *)buffer;
	T_ALARM_DESC *alarm_desc = NULL;
	
	len = recvfrom(SK_ALARM.sk, buffer, MAX_UDP_SIZE, 0, (struct sockaddr *)&(SK_ALARM.skaddr), &FromAddrSize);

	if ( -1 == len )
	{
		return CMM_FAILED;
	}
	else
	{
		alarm_desc = (T_ALARM_DESC *)(msg->BUF);
		time(&b_time);

		
		/*strcpy(alarminfo->DevMac, g_networkInfo.col_mac);

		switch(alarminfo->AlarmCode)
		{
			case 200901:
			{
				
                break;
			}
			case 200001:
			{
				break;
			}
			default:
			{
				strcpy(alarminfo->trap_info,"Unkonwn alarm type");
				break;
			}
		}
		*/
		return CMM_SUCCESS;
	}
}

int snmp_input(int operation, netsnmp_session * session, int reqid, netsnmp_pdu *pdu, void *magic)
{
	return 1;
}

void send_top_heartbeat(st_heartbeatinfo *heartbeatinfo)
{
	netsnmp_session session, *ss;	
	netsnmp_pdu    *pdu;
	long            sysuptime;
    char           *trap = NULL;
    int status = 0;
    uint8_t tmp[128] = {0};
	uint8_t *svalue = NULL;    
    svalue = (uint8_t *)malloc(768);
	if( NULL == svalue) return;
    
	snmp_sess_init(&session);
	session.version = SNMP_VERSION_2c;
	sprintf(tmp, "%s:%d", trapserver, 162);
	session.peername = tmp;
	session.community = "public";
	session.community_len = strlen("public");
	snmp_enable_stderrlog();
	init_snmp("snmpapp");
	
	session.callback = snmp_input;
	session.callback_magic = NULL;

	ss = snmp_add(&session, netsnmp_transport_open_client("snmptrap", session.peername), NULL, NULL);
	if (ss == NULL)
	{
        	snmp_sess_perror("snmptrap", &session);
		free(svalue);
	       return;
	}
    pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
    	sysuptime = get_uptime();
    	sprintf(tmp, "%ld", sysuptime);
    	trap = tmp; 
	/*  绑定告警时间*/
    snmp_add_var(pdu, objid_sysuptime, sizeof(objid_sysuptime)/sizeof(oid), 't', trap);
	/*  绑定告警OID  */
	snmp_add_var(pdu, objid_snmptrap, sizeof(objid_snmptrap) / sizeof(oid), 'o', objid_snmptrap);
	/*  绑定告警码*/
	sprintf(svalue, "%d", 20001);
	snmp_add_var(pdu, objid_alarmcode, sizeof(objid_alarmcode) / sizeof(oid), 'i', svalue);
	/*  绑定告警信息*/
	snmp_add_var(pdu, objid_trapinfo, sizeof(objid_trapinfo) / sizeof(oid), 's', "heartbeat info");
	sprintf(svalue, "[%ld|%03d.%03d Mbps|%03d.%03d Mbps|%03d.%03d Mbps|%03d.%03d Mbps|%03d.%03d Mbps|%03d.%03d Mbps|%03d.%03d Mbps|%03d.%03d Mbps]",
            heartbeatinfo->uptime, heartbeatinfo->linkstatus[0], heartbeatinfo->linkstatus[0], heartbeatinfo->linkstatus[1], heartbeatinfo->linkstatus[1]
            , heartbeatinfo->linkstatus[2], heartbeatinfo->linkstatus[2], heartbeatinfo->linkstatus[3], heartbeatinfo->linkstatus[3]
            , heartbeatinfo->linkstatus[4], heartbeatinfo->linkstatus[4], heartbeatinfo->linkstatus[5], heartbeatinfo->linkstatus[5]
            , heartbeatinfo->linkstatus[6], heartbeatinfo->linkstatus[6], heartbeatinfo->linkstatus[7], heartbeatinfo->linkstatus[7]);
	snmp_add_var(pdu, objid_muxhb, sizeof(objid_muxhb) / sizeof(oid), 's', svalue);
	/*  °ó¶šÖÕ¶Ë»ù±ŸÐÅÏ¢·ÖÆ¬1  */
	bzero(svalue, 768); 
    status = (snmp_send(ss, pdu) == 0);

	if (status)
	{
		snmp_sess_perror("snmptrap", ss);
		snmp_free_pdu(pdu);
	}
	snmp_close(ss);
}

void getSysuptime(char *filename, int *len, char **data){
    int step = 64;

    int file = open( filename, O_RDONLY );
    if ( file == -1 )
        return -1;

    int i = 0;
    while ( 1 )
    {
        *data = realloc( *data, step*(i+1));
        if ( data == NULL )
        {
            close( file );
            return -1;
        }

        int cur_len = read( file, *data+(step*i), step );
        if ( cur_len == 0 )
            break;
        else
            *len += cur_len;

        i++;
    }

    close( file );

    return 0;
}

void process_alarm(void)
{
	st_heartbeatinfo heartbeatinfo;
    int i = 0;
    while(1)
	{
		/*获取告警信息*/
        //ts port status
        /*u8_t send_buf[5];
        u8_t read_buf[10];
            
        if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0)
            return 0 ;
    
        send_buf[0]=0x77;
        send_buf[1]=0x6c;
        send_buf[2]=0x21;
        send_buf[4]=0x04;
        for(i=1;i<9;i++){
            send_buf[3]=i;
            memset(read_buf, 0, sizeof(read_buf));
            printf("---sendbuf--->>>%02x:%02x:%02x:%02x:%02x\n", send_buf[0],send_buf[1],send_buf[2],send_buf[3],send_buf[4]);
        
            if(Cmd_rwParam(send_buf, 5, read_buf, 10, 5) < 5)
            {
                TcpClt_close();
                return 0 ;
            }
            printf("---readbuf--->>>%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", read_buf[0],read_buf[1],read_buf[2],read_buf[3],read_buf[4],read_buf[5],read_buf[6],read_buf[7]);
        
            heartbeatinfo.linkstatus[i - 1] = read_buf[5];
        }
        TcpClt_close();
        */
        /*获取心跳信息*/
        /* 获取系统运行时间*/
        char *uptime = NULL;
        int upsec = 0;
        int len = 0;
        getSysuptime("/proc/uptime", &len , &uptime);
        upsec = atol(strtok(uptime, "."));
        heartbeatinfo.uptime = upsec;
        free(uptime);
        
        for(i=0;i<8;i++)
        {
            heartbeatinfo.linkstatus[i]=Cmd_GetChannel_Rate(i);
        }
        send_top_heartbeat(&heartbeatinfo);
        sleep(20);
	}
}

int main(void)
{
    printf("===alarm server started!\n");
	/*SK_ALARM.desc = DESC_SOCKET_ALARM;	
	if( CMM_SUCCESS != init_socket(&SK_ALARM) )
	{
		fprintf(stderr, "ERROR: alarm->init_socket[SK_ALARM], exit !\n");
		return -1;
	}*/
    SK_TOSNMP.desc = DESC_SOCKET_OTHER;
	if( CMM_SUCCESS != init_socket(&SK_TOSNMP) )
	{
		fprintf(stderr, "ERROR: alarm->init_socket[SK_TOSNMP], exit !\n");
		return -1;
	}    
    /* 获取本机network信息 */
    getNetworkinfo(&g_networkInfo);
       
    signal(SIGTERM, SignalProcessHandle);    
    
    int i = 0, len = 0;;
    int file = open( "/mnt/Nand2/net-snmp-arm/trapserver.txt", O_RDONLY );
    if ( file == -1 )
        return ;
    while ( 1 )
    {
        trapserver = realloc( trapserver, 64);
        if ( trapserver == NULL )
        {
            close( file );
            return ;
        }

        int cur_len = read( file, trapserver+(64*i), 64);
        if ( cur_len == 0 )
            break;
        else
            len += cur_len;

        i++;
    }
    close( file );
    /* 循环处理信息 */
	process_alarm();
	return 0;
}

