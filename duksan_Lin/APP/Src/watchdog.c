/*
 * file 		: watchdog.c
 * creator   	: jong2ry
 *
 *
 * UDP Server�� messsage queue�� ����ؼ� data�� �ְ� �޴´�. 
 *
 *
 * version :
 *		0.0.1  jong2ry working.
 *
 *
 * code �ۼ��� ���ǻ���
 *		1. global ������ 'g_' ���λ縦 ����.
 *		2. ������ ������ ������ �Ʒ��� ���� ��Ģ���� �����Ѵ�. 
 *			int					n
 *			short				w
 *			long				l
 *			float				f
 *			char				ch
 *			unsigned char       b
 *			unsignnd int		un
 *			unsigned short 		uw 
 *			pointer				p
 *			structure			s or nothing
 *		3. �������� ù���ڴ� ����ڷ� ����Ѵ�. 
 *		4. ���������� �� ������ ������ ������� ����ڷ� �Ѵ�. 
 *			ex > g_nTest, g_bFlag, g_fPointTable
 *		5. �Լ����� �ҹ��ڷ� '_' ��ȣ�� ����ؼ� �����Ѵ�. 
 *		6. �Լ��� �Լ� ������ ������ 2�ٷ� �Ѵ�. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/un.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/poll.h>															// use poll event
 
 

////////////////////////////////////////////////////////////////////////////////
//define
#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"


int g_nWatchdogCount;
WATCHDOG_LIST *g_pList;
WATCHDOG_LIST *g_pPrevList;
	
//void watchdog_debug(char *mess, ...) { fprintf(stdout, mess); fflush(stdout); }
	

void *watchdog_main(void* arg)
{
	int i = 0;
	
	g_pList = (WATCHDOG_LIST *)malloc(sizeof(WATCHDOG_LIST));
	g_pPrevList = (WATCHDOG_LIST *)malloc(sizeof(WATCHDOG_LIST));
	
	memset ( g_pList, 0x00, sizeof(WATCHDOG_LIST) );
	memset ( g_pPrevList, 0x00, sizeof(WATCHDOG_LIST) );
	
	signal(SIGPIPE, SIG_IGN);								// Ignore broken_pipe signal.

	// watchdog���� �����ϰ��� �ϴ� thread�� ������ �����Ѵ�. 
	// 2011.03.21 ȫ�� GHP
	g_nWatchdogCount = 1;
	
	sleep(3);

	while (1) {
		sched_yield();
		fprintf(stdout, "Wait 30 Sec.. check Watchdog list %02d\n", g_nWatchdogCount);
		fflush(stdout);		
		sleep(30);
		
		if ( g_nWatchdogCount == 0 ) 
			continue;
	
		for ( i = 0; i < g_nWatchdogCount; i++ ) {
			
			fprintf(stdout, ">Watchdog %d %d", g_pPrevList->nLoogCnt[i], g_pList->nLoogCnt[i]);
			fflush(stdout);
			
			if ( g_pPrevList->nLoogCnt[i] == g_pList->nLoogCnt[i] ) {
			
				fprintf(stdout, "Warnning watchdog error %d\n", i);
				fflush(stdout);
				
				syslog_record(SYSLOG_DESTROY_IFACE_GHP);
						
				system("killall duksan");
			}
			else {
				g_pPrevList->nLoogCnt[i] = g_pList->nLoogCnt[i];
				continue;
			}
		}
	}
		
	system("killall duksan");		
}	
