#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "dataType.h"


#define TCP_CLIENT_READ_SUPPORT
//#define TCP_RW_MUTEX_SUPPORT



extern int TcpClt_connect(u32_t ip32_LittleMostEndian, u16_t port);
extern int TcpClt_send(u8_t *pInBuf, u16_t len);
extern void TcpClt_close(void);
extern bool_t TcpClt_isConnected();

#ifdef TCP_RW_MUTEX_SUPPORT
extern void TcpClt_init();
#endif

#ifdef TCP_CLIENT_READ_SUPPORT
extern int TcpClt_read(u8_t *pOutBuf, u16_t maxlen);
#endif

extern int TcpClt_writeRead(u8_t *pInBuf, u16_t inlen, u8_t *pOutBuf, u16_t maxlen);


#endif
