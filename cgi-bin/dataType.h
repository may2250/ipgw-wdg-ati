/*********************************************************************
* ChengDu YiMei
+-------------------------------------------------------------------
* File: data type
* Note:
* Date: 2013.03.27
+--------------------------------------------------------------------
* Author: DuansShaoNan
* Date: 2013.03.27
* Change: New file
*********************************************************************/

#ifndef INC_DATA_TYPE_H
#define INC_DATA_TYPE_H

#include <stddef.h>
#include <string.h>


#define CHANNEL_MAXCNT 8
#define EACH_CHN_PRG_MAXCNT 128
#define NAME_MAXLEN 64


typedef unsigned char   u8_t;
typedef unsigned short  u16_t;
typedef unsigned int    u32_t;
typedef unsigned long long   u64_t; // int64_t
typedef unsigned char   bool_t;

typedef char   s8_t;
typedef short  s16_t;
typedef int    s32_t;
typedef long long   s64_t;


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
//#define NULL (void *)0
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef IO
#define IO
#endif




typedef unsigned int cmd_t;

#define B2W(B) ((((u16_t)*((B+1)+0))<<8)|(((u16_t)*((B)))<<0))
#define B4D(B) ((((u32_t)*((B)+1))<<24)|(((u32_t)*((B)+0))<<16)|\
	(((u32_t)*((B)+2))<<8)|(((u32_t)*((B)+3))<<0))
#define W2B(W,B) {*((B)+0)=(u8_t)((W)>>8);*((B)+1)=(u8_t)((W)>>0);}
#define D4B(D,B) {*((B)+0)=(u8_t)((D)>>24);*((B)+1)=(u8_t)((D)>>16);\
	*((B)+2)=(u8_t)((D)>>8);*((B)+3)=(u8_t)((D)>>0);}



#define BYTE u8_t
#define PBYTE u8_t *
#define WORD u16_t
#define DWORD u32_t
#define INT int
#define INT64 long long
#define UINT u32_t
#define UINT32 u32_t
#define UINT64 u64_t
#define ULONG u64_t
#define BOOL bool_t

#define VOID void
#define PVOID void *
#define HANDLE PVOID

#define MIN(a, b) ( ((a) < (b)) ? (a) : (b) )



//////////////////////////////////////////////////////////////////////


typedef struct StChn_service_s
{
	BOOL isMuxed;
	cmd_t chnId;/*通道号*/
	cmd_t prgId;/*节目号*/
	cmd_t pmtPid;
	u8_t name_len;
	char name[64];
	u8_t provider_len;
	char provider[64];
	BOOL isCA;
} StChn_service_t;

typedef struct service_o
{
	cmd_t chnId;/*通道号*/
	u8_t name_len;
	char name[256];
	u8_t provider_len;
	char provider[256];
} StChn_service_o;


typedef struct StChannel_s
{
	int serviceCnt;/*节目总数*/
	StChn_service_t service[EACH_CHN_PRG_MAXCNT];
	StChn_service_o service_o[EACH_CHN_PRG_MAXCNT];
	u8_t my_ip[4];
	u8_t my_mask[4];
	u8_t my_gateway[4];
	u8_t my_mac[6];

	u8_t su_ip[4];
	u16_t su_port;
	u8_t su_mac[6];
	u8_t udp_ttl;
	
	u8_t de_ip[8][4];
	u16_t de_port[8];
	u8_t de_mac[8][6];
	
} StChannel_t;


#endif


