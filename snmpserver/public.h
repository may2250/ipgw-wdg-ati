#ifndef __MID_PUBLIC_H__
#define __MID_PUBLIC_H__

#define MAX_UDP_SIZE    			1472

#define SNMP_ALARM_PORT 			1225 /* snmp模块监听端口 */
#define ALARM_LISTEN_PORT 			2345 /* 告警模块监听端口 */

//描述sokect作用的枚举变量
enum
{
	DESC_SOCKET_ALARM = 0,		/* 与ALARM模块通信的socket描述符 */
	DESC_SOCKET_OTHER
};

enum
{
	CMM_SUCCESS = 0,
	CMM_FAILED,
	
	CMM_UNKNOWN_ERR = 0xFF
};

typedef struct
{
	uint8_t desc;		/* ÃèÊöSOCKET×÷ÓÃµÄÃ¶ŸÙ±äÁ¿*/
	int sk;
	struct sockaddr_in skaddr;
}T_UDP_SK_INFO;

typedef struct
{
	T_UDP_SK_INFO sk;
	uint32_t blen;
	uint8_t b[MAX_UDP_SIZE];
}BBLOCK_QUEUE;

/* DBS */
typedef struct
{
	T_UDP_SK_INFO channel;
	uint16_t srcmod;
	uint32_t blen;
	uint8_t buf[MAX_UDP_SIZE];
}T_DBS_DEV_INFO;

#pragma pack (push, 1)
typedef struct
{
	uint32_t AlarmCode;
	uint32_t ChannelIndex;
	uint32_t AlarmStatus;
	uint32_t AlarmValue;
	uint8_t AlarmInfo[64];
}T_ALARM_DESC;

typedef struct
{
	uint32_t channeltid;
    uint32_t serviceindex;
    uint32_t svc_id;
    uint8_t servicename[64];
    uint8_t providername[64];
    uint8_t out_servicename[64];
    uint8_t out_providername[64];	
}st_channelMux;

typedef struct
{
	long uptime;
    uint32_t linkstatus[8];
}st_heartbeatinfo;

typedef struct
{
	uint32_t channeltid;
	uint32_t type;
    uint8_t port_in_status[8];
	uint8_t ts_out_loss;
	uint8_t ts_port_linkloss[8];
}st_alarminfo; 

typedef struct
{
	uint32_t	id;
	uint8_t	col_ip[32];
	uint8_t	col_netmask[32];
	uint8_t	col_gw[32];
	uint32_t	col_dhcp;	
	uint8_t	col_dns[32];
	uint8_t	col_mac[32];
}st_dbsNetwork;

#pragma pack (pop)



#endif
