#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "tcpClient.h"

#define TCP_CLIENT_INVALID_FD -1

static int m_tcpClientSockFd = TCP_CLIENT_INVALID_FD;

#ifdef TCP_RW_MUTEX_SUPPORT
static pthread_mutex_t m_cltTcpWriteMutex;
#endif

#define TCPCLT_PRINT(X...)// printf("[TCP_CLT:]"X)


int TcpClt_connect(u32_t ip32_LittleMostEndian, u16_t port)
{
	 UDP_SEND_Init();
     return 1;
	struct sockaddr_in addr;
	int rslt;
	struct timeval ltimeout = {1, 0}; // Second - Microseconds

	TcpClt_close();

	if((m_tcpClientSockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		TCPCLT_PRINT("New socket failed!\n");
		return -1;
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip32_LittleMostEndian;//inet_addr(pIpStr); //0x6501a8c0;//
	addr.sin_port = htons(port);
	rslt = connect(m_tcpClientSockFd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
	if(rslt == -1){
		TcpClt_close();
	}
	setsockopt(m_tcpClientSockFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&ltimeout, sizeof(ltimeout));
	return rslt;
}

int TcpClt_send(u8_t *pInBuf, u16_t len){
	int rslt = -1;
#ifdef TCP_RW_MUTEX_SUPPORT	
	pthread_mutex_lock(&m_cltTcpWriteMutex);
#endif
	if(m_tcpClientSockFd != TCP_CLIENT_INVALID_FD)
	{
		rslt = write(m_tcpClientSockFd, pInBuf, len);
	}
#ifdef TCP_RW_MUTEX_SUPPORT	
	pthread_mutex_unlock(&m_cltTcpWriteMutex);
#endif
	return rslt;
}

#ifdef TCP_CLIENT_READ_SUPPORT
int TcpClt_read(u8_t *pOutBuf, u16_t maxlen)
{
	int rslt = -1;
	if(m_tcpClientSockFd != TCP_CLIENT_INVALID_FD)
	{
		rslt = read(m_tcpClientSockFd, pOutBuf, maxlen);
	}
	return rslt;
}
#endif

void TcpClt_close(void){

	UDP_SEND_CLOSE();
	return;
	if(m_tcpClientSockFd != TCP_CLIENT_INVALID_FD)
	{
		close(m_tcpClientSockFd);
		m_tcpClientSockFd = TCP_CLIENT_INVALID_FD;
	}
}

bool_t TcpClt_isConnected()
{
	return (m_tcpClientSockFd != TCP_CLIENT_INVALID_FD);
}

int TcpClt_writeRead(u8_t *pInBuf, u16_t inlen, u8_t *pOutBuf, u16_t maxlen)
{
	int rslt = -1;
	if(m_tcpClientSockFd != TCP_CLIENT_INVALID_FD)
	{
		rslt = write(m_tcpClientSockFd, pInBuf, inlen);
		//usleep(50000);
		rslt = read(m_tcpClientSockFd, pOutBuf, maxlen);
	}
	return rslt;
}

#ifdef TCP_RW_MUTEX_SUPPORT	
void TcpClt_init()
{
	int res = pthread_mutex_init(&(m_cltTcpWriteMutex), NULL);
	if(res != 0)
	{
		printf(" TcpClt_init error in Muxtex init!\n");
	}
}
#endif	


