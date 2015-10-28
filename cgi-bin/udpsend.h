#ifndef _UDPSEND_H_
#define _UDPSEND_H_

#include <stdio.h> 
#include <stdlib.h>
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 

#define PORT 7132
extern int sockfd; 		 // Socket file descriptor	
extern char remoteip[16];  //eg. 192.168.100.100
extern struct sockaddr_in addr_remote; // Host address information

extern char UDP_SEND_Init();
extern char UDP_SEND_DATA(unsigned char *data,long length);
extern int  UDP_RECV_DATA(unsigned char *recvbuf ,unsigned int max_len);
extern char UDP_SEND_CLOSE();
extern int UDP_Clt_writeRead(unsigned char *pInBuf, unsigned short inlen, unsigned char *pOutBuf, unsigned short maxlen);


#endif

