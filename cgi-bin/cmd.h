#ifndef INC_CMD_H
#define INC_CMD_H

#include "dataType.h"



#ifdef PC_OS
#define TCP_SVC_IP 0x2a00a8c0/*192.168.0.42*/
//#define TCP_SVC_IP 0x8300a8c0/*192.168.0.131*/
//#define TCP_SVC_IP 0x0100007f


#endif
#ifdef ARM_OS
#define TCP_SVC_IP 0x0100007f
#endif

#ifdef PC_OS 
#define CHANNEL_PARAM  "/mnt/hgfs/wanlong/devWeb_1502100100/cgi-bin/Out_Channel_%d_Index_%d"
#endif

#ifdef ARM_OS
//#define CHANNEL_PARAM	"/mnt/Nand2/web/param/Out_Channel_%d_Index_%d"
#define CHANNEL_PARAM "/tmp/Out_Channel_%d_Index_%d"

#endif



//#define TCP_SVC_IP 0x2a00a8c0/*192.168.0.42*/
//#define TCP_SVC_IP 0x8000a8c0/*192.168.0.128*/
//#define TCP_SVC_IP 0x8300a8c0/*192.168.0.131*/
//#define TCP_SVC_IP 0x2e00a8c0/*192.168.0.46*/


#define TCP_SVC_PORT 161 // 10020



extern StChannel_t gChn;

extern cmd_t Cmd_GetChannel_Rate(cmd_t chnId);
extern u8_t Cmd_GetChannel_Status();

extern void Cmd_getChannel(cmd_t chnId,cmd_t sp);
extern void Cmd_SearchInput(cmd_t chnId);
extern unsigned char Cmd_getchannel_pm(cmd_t chnId);
extern unsigned char Cmd_setChannel(cmd_t chnId);

extern void Cmd_getOutput();
extern void Cmd_getPrograme(cmd_t chnId, cmd_t prgIndex);
extern void Cmd_programe(cmd_t chnId, cmd_t prgIndex);

extern u8_t Cmd_submitNet();
extern u8_t Cmd_getNet();
extern void Cmd_getVersion(u8_t *pVerBytes);

extern void Cmd_submitFactory();
extern void Cmd_submitReboot();
extern u8_t Cmd_getTS2IP();
extern u8_t Cmd_SetTS2IP();

extern int Cmd_rwParamInt(u8_t p1, u8_t p2, u8_t p3, u8_t rwFlag, cmd_t *pIn, cmd_t *pOut);

#endif

