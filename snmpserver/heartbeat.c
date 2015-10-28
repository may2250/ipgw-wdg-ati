#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#include "public.h"
#include "cmd.h"
#include "heartbeat.h"

#define CMD_HEAD_1 0x77
#define CMD_HEAD_2 0x6C

void process_heartbeat(void)
{
	st_channelMux alarminfo;
	struct timeval start, end;
    while(1)
	{
					
        gettimeofday( &start, NULL );
        /* ŽŠÀíÐÄÌøÏûÏ¢*/
        send_top_heartbeat();
        gettimeofday( &end, NULL );
        if( (int)(end.tv_sec - start.tv_sec) > 10 )
        {
            /* œèÓÃalarminfo.trap_info ×÷ÎªÁÙÊ±ŽæŽ¢*/
            sprintf(alarminfo.trap_info, "heartbeat time is too long [%ds]", (int)(end.tv_sec - start.tv_sec));
        }	
		
	}
}


int main(void)
{
    printf("===heartbeat server started!\n");
	SK_ALARM.desc = DESC_SOCKET_ALARM;	
	if( CMM_SUCCESS != init_socket(&SK_ALARM) )
	{
		fprintf(stderr, "ERROR: alarm->init_socket[SK_ALARM], exit !\n");
		return -1;
	}
    /* 循环处理信息 */
	process_heartbeat();
	return 0;
}
