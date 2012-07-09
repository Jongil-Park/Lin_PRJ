/*
 * file 		: watchdog.c
 * creator   	: jong2ry
 *
 *
 * UDP Server와 messsage queue를 사용해서 data를 주고 받는다. 
 *
 *
 * version :
 *		0.0.1  jong2ry working.
 *
 *
 * code 작성시 유의사항
 *		1. global 변수는 'g_' 접두사를 쓴다.
 *		2. 변수를 선언할 때에는 아래와 같은 규칙으로 선언한다. 
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
 *		3. 변수명의 첫글자는 대분자로 사용한다. 
 *		4. 변수명에서의 각 글자의 구분은 공백없이 대분자로 한다. 
 *			ex > g_nTest, g_bFlag, g_fPointTable
 *		5. 함수명은 소문자로 '_' 기호를 사용해서 생성한다. 
 *		6. 함수와 함수 사이의 간격은 2줄로 한다. 
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

	// watchdog으로 관리하고자 하는 thread의 개수를 설정한다. 
	// 2011.03.21 홍대 GHP
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
