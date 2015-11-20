#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <net/if.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "public.h"
#include "cmd.h"
#include "alarm.h"


#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

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
oid objid_ch[] =		{ 1, 3, 6, 1, 4, 1, 3445, 8080, 7, 1 };

char *trapserver = NULL;
char hostmac[30] = {0};

void SignalProcessHandle(int n)
{
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

 int get_mac(char* mac)
{
    struct ifreq tmp;
    int sock_mac;
    char mac_addr[30];
    sock_mac = socket(AF_INET, SOCK_STREAM, 0);
    if( sock_mac == -1){
        perror("create socket fail\n");
        return -1;
    }
    memset(&tmp,0,sizeof(tmp));
    strncpy(tmp.ifr_name,"eth0",sizeof(tmp.ifr_name)-1 );
    if( (ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0 ){
        printf("mac ioctl error\n");
        return -1;
    }
    sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
            (unsigned char)tmp.ifr_hwaddr.sa_data[0],
            (unsigned char)tmp.ifr_hwaddr.sa_data[1],
            (unsigned char)tmp.ifr_hwaddr.sa_data[2],
            (unsigned char)tmp.ifr_hwaddr.sa_data[3],
            (unsigned char)tmp.ifr_hwaddr.sa_data[4],
            (unsigned char)tmp.ifr_hwaddr.sa_data[5]
            );
    printf("local mac:%s\n", mac_addr);
    close(sock_mac);
    memcpy(mac,mac_addr,strlen(mac_addr));
    return 1;
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
    
	//printf("===alarm trapserver==%s!\n", trapserver);
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
	/*  绑定设备MAC  */
	snmp_add_var(pdu, objid_hostmac, sizeof(objid_hostmac) / sizeof(oid), 's', hostmac);
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
	free(svalue);
}

void send_alarm_notification_to_snmp(st_alarminfo *alarminfo)
{
	netsnmp_session session, *ss;	
	netsnmp_pdu    *pdu;
	int             status;
	long            sysuptime;
	char            csysuptime[20];
	char           *trap = NULL;
	uint8_t speername[32] = {0};
	uint8_t svalue[16] = {0};
	uint8_t trapinfo[64] = {0};
	
	snmp_sess_init(&session);
	
	session.version = SNMP_VERSION_2c;
	sprintf(speername, "%s:%d", trapserver, 162);
	session.peername = speername;
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
	       return;
	}
    
	pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
    sysuptime = get_uptime();
	sprintf(csysuptime, "%ld", sysuptime);
	trap = csysuptime;
	snmp_add_var(pdu, objid_sysuptime, sizeof(objid_sysuptime)/sizeof(oid), 't', trap);
	/*  绑定告警OID  */
	snmp_add_var(pdu, objid_snmptrap, sizeof(objid_snmptrap) / sizeof(oid), 'o', objid_snmptrap);
	/*  绑定设备MAC  */
	snmp_add_var(pdu, objid_hostmac, sizeof(objid_hostmac) / sizeof(oid), 's', hostmac);
	
	switch(alarminfo->type){
		case 1:		/* AIS INPUT POTR LOCK LOSS */
			/*  绑定告警通道*/
			sprintf(svalue, "%d", alarminfo->channeltid);
			snmp_add_var(pdu, objid_ch, sizeof(objid_ch) / sizeof(oid), 'c', svalue);
			/*  绑定告警码*/
			sprintf(svalue, "%d", 20901);
			snmp_add_var(pdu, objid_alarmcode, sizeof(objid_alarmcode) / sizeof(oid), 'i', svalue);
			/*  绑定告警信息*/
			sprintf(trapinfo, "ASI INPUT PORT LOCK LOSS[%d|%d|%d|%d|%d|%d|%d|%d].", alarminfo->port_in_status[0], alarminfo->port_in_status[1],
					alarminfo->port_in_status[2], alarminfo->port_in_status[3],alarminfo->port_in_status[4], alarminfo->port_in_status[5],
					alarminfo->port_in_status[6], alarminfo->port_in_status[7]);
			snmp_add_var(pdu, objid_trapinfo, sizeof(objid_trapinfo) / sizeof(oid), 's', trapinfo);
			break;
		case 2:     /* TS OUTPUT LOSS*/
			/*  绑定告警码*/
			sprintf(svalue, "%d", 20902);
			snmp_add_var(pdu, objid_alarmcode, sizeof(objid_alarmcode) / sizeof(oid), 'i', svalue);
			/*  绑定告警信息*/
			snmp_add_var(pdu, objid_trapinfo, sizeof(objid_trapinfo) / sizeof(oid), 's', "TS OUTPUT LOSS.");
			break;
		case 3:    /* TS PORT LINK LOSS */
			/*  绑定告警通道*/
			sprintf(svalue, "%d", alarminfo->channeltid);
			snmp_add_var(pdu, objid_ch, sizeof(objid_ch) / sizeof(oid), 'c', svalue);
			/*  绑定告警码*/
			sprintf(svalue, "%d", 20903);
			snmp_add_var(pdu, objid_alarmcode, sizeof(objid_alarmcode) / sizeof(oid), 'i', svalue);
			/*  绑定告警信息*/
			sprintf(trapinfo, "TS PORT LINK LOSS[%d|%d|%d|%d|%d|%d|%d|%d].", alarminfo->ts_port_linkloss[0], alarminfo->ts_port_linkloss[1],
					alarminfo->ts_port_linkloss[2], alarminfo->ts_port_linkloss[3],alarminfo->ts_port_linkloss[4], alarminfo->ts_port_linkloss[5],
					alarminfo->ts_port_linkloss[6], alarminfo->ts_port_linkloss[7]);
			snmp_add_var(pdu, objid_trapinfo, sizeof(objid_trapinfo) / sizeof(oid), 's', trapinfo);
			break;
	}
	
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
	st_alarminfo alarminfo;
    int i = 0, status = 0;
	char uptime[64] = {0};	
	get_mac(hostmac);  
    while(1)
	{
		/*获取告警信息*/        
		//AIS INPUT POTR LOCK LOSS
		status=Cmd_GetChannel_Status();
		for(i=0;i<8;i++)
		{
			alarminfo.port_in_status[i] = status&0x01;   						
			status>>=1;
		}
		for(i=0;i<8;i++)
		{
			if(alarminfo.port_in_status[i] == 0){
				/* send alarm */ 
				alarminfo.channeltid = i + 1;
				alarminfo.type = 1;
				send_alarm_notification_to_snmp(&alarminfo);
				break;
			}
		}
		
		//ts output loss
        u8_t send_buf[5];
		u8_t read_buf[16] = {0};
        if(TcpClt_connect(TCP_SVC_IP, TCP_SVC_PORT) < 0){
			sleep(20);
            continue ;
        }
    
        send_buf[0]=0x77;
        send_buf[1]=0x6c;
        send_buf[2]=0x21;
        send_buf[3]=0x01;
        send_buf[4]=0x04;
	
		if(Cmd_rwParam(send_buf, 5, read_buf, 10, 5) < 5)
		{
			TcpClt_close();
			sleep(20);
			continue ;
		}	
        alarminfo.ts_out_loss = read_buf[5];
		if(alarminfo.ts_out_loss == 0){
			/* send alarm */
			alarminfo.channeltid = 0;
			alarminfo.type = 2;
			send_alarm_notification_to_snmp(&alarminfo);		
		}
        TcpClt_close();	
        
        /*获取心跳信息*/
        /* 获取系统运行时间*/        
        int upsec = 0;
        //getSysuptime("/proc/uptime", &len , &uptime);
		int fd=open("/proc/uptime", O_RDONLY);
		memset(uptime, 0, sizeof(uptime));
		read(fd, uptime, sizeof(uptime));
		close(fd);
        upsec = atol(strtok(uptime, "."));
        heartbeatinfo.uptime = upsec;
        for(i=0;i<8;i++)
        {
            heartbeatinfo.linkstatus[i]=Cmd_GetChannel_Rate(i);
			alarminfo.ts_port_linkloss[i] = heartbeatinfo.linkstatus[i]==0?0:1;
        }
        send_top_heartbeat(&heartbeatinfo);
		
		//ts port link loss
		for(i=0;i<8;i++)
        {
			if(alarminfo.ts_port_linkloss[i] == 0){
				/* send alarm */
				alarminfo.channeltid = i + 1;
				alarminfo.type = 3;
				send_alarm_notification_to_snmp(&alarminfo);
				break;
			}
        }
        sleep(20);
	}
}

int main(void)
{
    printf("===alarm server started!\n");   
    /* 获取本机network信息 */
    getNetworkinfo(&g_networkInfo);
       
    signal(SIGTERM, SignalProcessHandle);    
    
    int i = 0, len = 0;;
    int file = open( "/mnt/Nand2/net-snmp-arm/trapserver.txt", O_RDONLY );
    if ( file == -1 )
        return 0;
    while ( 1 )
    {
        trapserver = realloc( trapserver, 64);
        if ( trapserver == NULL )
        {
            close( file );
            return 0;
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

