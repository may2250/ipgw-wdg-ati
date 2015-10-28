#ifndef INC_WEB_H
#define INC_WEB_H

#include "dataType.h"

/*extern void WebGet_encoder();
extern void WebSubmit_encoderFst86391();
extern void WebSubmit_encoderMb86h46();
extern void WebGet_tsOut();
extern void WebSubmit_tsOut();
extern void WebGet_transmit();
extern void WebSubmit_transmit();*/

extern void WebGet_Status();
extern void WebGet_Import();
extern void WebGet_Export();
extern void WebGet_Backup();

extern void WebSearchChannel();

extern void WebGet_channel();
extern void WebGet_channel_edit();

extern void Set_Channel();
extern void WebSearchInput();

extern void WebGet_output();
extern void Cmd_getChannelOutput(cmd_t chnId,cmd_t Index );


extern void WebGet_programe();
extern void WebSubmit_programe();

extern void WebGet_net();
extern void WebSubmit_net();
extern void WebGet_system();

extern void WebGet_Tsip();
extern void WebSet_Tsip();

extern void WebCmd();
extern void WebSubmit_upload();
extern void WebSubmit_factory();
extern void WebSubmit_reboot();
extern void WebGet_test();
extern void  WebEditP_Test();

extern int Html2Cgi_waitGoto(char *pHRef, char *pTitle);



#endif
