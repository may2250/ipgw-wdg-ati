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
	uint16_t usSrcMID;			/*ÔŽÄ£¿éID*/
	uint16_t usDstMID;			/*Ä¿¶ÃÄ£¿éID*/
	uint16_t usMsgType;			/*ÏûÏ¢ÀàÐÍ£¬±êÊŸŸßÌåµÄÏûÏ¢*/
	uint8_t fragment;				/*ÊÇ·ñŽæÔÚžü¶à·ÖÆ¬*/
	uint32_t ulBodyLength;		/*ÏûÏ¢Ìå³€¶È*/
}
T_Msg_Header_CMM;

typedef struct
{
	T_Msg_Header_CMM HEADER;
	uint8_t BUF[0];		/*ÏûÏ¢ÌåÄÚÈÝ*/
}
T_Msg_CMM;

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
	uint32_t	id;
	uint8_t	col_ip[32];
	uint8_t	col_netmask[32];
	uint8_t	col_gw[32];
	uint32_t	col_dhcp;	
	uint8_t	col_dns[32];
	uint8_t	col_mac[32];
}st_dbsNetwork;

typedef struct
{
	uint16_t usSrcMID;			/*源模块ID*/
	uint16_t usDstMID;			/*目的模块ID*/
	uint16_t usMsgType;			/*消息类型*/
	uint32_t ulBodyLength;		/*消息体长度*/
}
T_DB_MSG_HEADER_REQ;

typedef struct
{
	T_DB_MSG_HEADER_REQ HEADER;
	uint8_t BUF[0];				/*Ž«ÈëµÄÏûÏ¢ÌåÄÚÈÝ*/
}
T_DB_MSG_PACKET_REQ;

typedef struct
{
	uint16_t usSrcMID;			
	uint16_t usDstMID;			
	uint16_t usMsgType;			
	uint8_t fragment;				/*是否存在更多分片*/
	uint16_t result;				/*处理状态*/
	uint32_t ulBodyLength;		/*消息体长度*/
}
T_DB_MSG_HEADER_ACK;

typedef struct
{
	T_DB_MSG_HEADER_ACK HEADER;
	uint8_t BUF[0];				
}
T_DB_MSG_PACKET_ACK;


#pragma pack (pop)



#endif
