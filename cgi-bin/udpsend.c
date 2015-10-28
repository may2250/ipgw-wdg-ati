#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h> 
#include <stdlib.h>
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <linux/if.h>
#include "udpsend.h"

int sockfd; 		 // Socket file descriptor

//char remoteip[16]={"127.0.0.1"};


#ifdef PC_OS
char remoteip[16]={"192.168.1.49"};
#endif
#ifdef ARM_OS
char remoteip[16]={"127.0.0.1"};
#endif



struct sockaddr_in addr_remote; // Host address information

long getlocalhostip(){
  int  MAXINTERFACES=16;
	long ip;
	int fd, intrface, retn = 0;
 	struct ifreq buf[MAXINTERFACES]; ///if.h
	struct ifconf ifc; ///if.h
	ip = -1;
	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) //socket.h
	{
 		ifc.ifc_len = sizeof buf;
 		ifc.ifc_buf = (caddr_t) buf;
 		if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc)) //ioctl.h
		{
 			intrface = ifc.ifc_len / sizeof (struct ifreq); 
			while (intrface-- > 0)
 			{
 				if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface])))
 				{
 					ip=inet_addr( inet_ntoa( ((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr) );//types
 					break;
				}
           			
			}
			
		}
 		close (fd);
 	}
	return ip;
}
union ipu{
	long ip;
	unsigned char ipchar[4];
	};

char UDP_SEND_Init()
{
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        return (0);
    }
	 /* Fill the socket address struct */
    addr_remote.sin_family = AF_INET;                   // Protocol Family
    addr_remote.sin_port = htons(PORT);                 // Port number
    
    inet_pton(AF_INET,remoteip, &addr_remote.sin_addr); // Net Address
    bzero(&(addr_remote.sin_zero), 8);                  // Flush the rest of struct
	
}

char UDP_SEND_DATA(unsigned char *data,long length)
{
	long num=0;

	if(length >0)
		{
			num = sendto(sockfd, data, length, 0, (struct sockaddr *)&addr_remote, sizeof(struct sockaddr_in));
		}
	
    if( num < 0 )
    {
    }
    else
    {
     }
	
}

int UDP_RECV_DATA(unsigned char *recvbuf ,unsigned int max_len)
{
	  fd_set rfds;
    struct timeval tv;
    int retval;
    int len=0;
    int recv_len = 0;
		len = sizeof(struct sockaddr_in);
    /* Watch stdin (fd 0) to see when it has input. */

    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);

    /* Wait up to five seconds. */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    retval = select(sockfd + 1, &rfds, NULL, NULL, &tv);
    if (retval == -1)
    	{
    		return 0;
			}
    else if(retval)
    	{
    		recv_len=recvfrom(sockfd, recvbuf, max_len, 0, (struct sockaddr *)&addr_remote,&len);
    		return recv_len;
    	}
    	else 
    		{
    			return 0;
    		}
}

char UDP_SEND_CLOSE()
{
	close (sockfd);
}

int UDP_Clt_writeRead(unsigned char *pInBuf, unsigned short inlen, unsigned char *pOutBuf, unsigned short maxlen)
{
	int rslt = -1;
	   
         UDP_SEND_DATA(pInBuf,inlen);
         rslt=UDP_RECV_DATA(pOutBuf ,maxlen);
		

	return rslt;
}


