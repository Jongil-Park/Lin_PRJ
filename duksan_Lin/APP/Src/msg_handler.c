/****************************************************************************** 
 * file : msg_handler.c
 * author : jong2ry
 * 
 * 
 * net32.c로 부터 net32_queue를 받은 point data를 처리한다. 
 * point-table을 업데이트 한다.
 * elba_queue를 통해 elba.c로 point data를 보낸다. 
 *  
 * |----------|                    |-------------------|                     |--------|
 * | net32.c  | == net32_queue ==> | message_handler.c |  == elba_queue ==>  | elba.c |
 * |----------|                    |-------------------|                     |--------|
 *
 * point-table을 업데이트 할 때, 만약에 값이 바뀌게 되면 
 * net32_message_queue를 통해 net32.c로 point data를 보낸다.
 * 
 * !!! -- point-table의 값이 업데이트 된 경우에만 동작함. -- !!!
 *
 * |--------------------|                            |---------|
 * | message_handler.c  | == net32_queue ==> | net32.c |
 * |--------------------|                            |---------|
 * 
 * 
 * version :
 * 		0.0.1 - code clean  jong2ry
 * 		0.0.2 - Status 변수를 통한 point_handler()의 동작상태 확인. 2010-07-19  jong2ry
 * 		0.0.3 - command 요청과 report 요청을 분리함. 2010-07-19  jong2ry
 * 		0.0.4 - file name change (message_hadler.c -> msg_handler.c)
 * 
 * 
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/poll.h>		// use poll event
#include <sys/time.h>		// gettimeofday()



#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"
#include "queue_handler.h"														// queue handler

#include "net32_mgr.h"										// net32 manager
#include "elba_mgr.h"										// elba manager
#include "message_handler.h"


extern pthread_mutex_t pointTable_mutex;									// point table mutex
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];				// point table only value
//extern CHK_STATUS_T *g_pStatus;												// status structure
extern int g_nMyPcm;															// my pcm number
extern PTBL_INFO_T *g_pPtbl;												// point-table

// from net32.c
extern int g_nViewNet32;													// debug문을 출력할 때 사용하는 flag


static void msg_sleep(int sec,int usec)
// ----------------------------------------------------------------------------
// WAIT TIMER
// Description : use select function for timer.
// Arguments   : sec		Is a second value.
//				 usec		Is a micro-second value. 
// Returns     : none
{
    fd_set reads, temps;
    struct timeval tv;
    
    FD_ZERO(&reads);
    FD_SET(0, &reads);
    
    tv.tv_sec = sec;
    tv.tv_usec = usec;
    temps = reads;
    
    select( 0, &temps, 0, 0, &tv );
    
	return;
}


void *message_handler_main(void* arg)
// ----------------------------------------------------------------------------
// MESSAGE_HANDLER_THREAD MAIN LOOP
// Description : This function is called by main().
// Arguments   : none
// Returns     : none
{
	//int n_ret = 0;
	int n_pno = 0;
	point_info point;
	struct timeval chk_time_1;
	struct timeval chk_time_2;

	signal(SIGPIPE, SIG_IGN);	// Ignore broken_pipe signal

	while(1)
	{
		// 15분마다 자신의 전 포인트를 전송한다.
		gettimeofday( &chk_time_1, NULL );
		if ( chk_time_1.tv_sec - chk_time_2.tv_sec > 900 ) {

			for ( n_pno = 0; n_pno < 32; n_pno++ ) {
				point.pcm = g_nMyPcm;
				point.pno = n_pno;
				point.value = g_pPtbl[n_pno].f_val;
				point.message_type = NET32_TYPE_REPORT;
		
				net32_push_queue(&point);	
			}
			gettimeofday( &chk_time_2, NULL );
		}
		
		msg_sleep( 1, 0 );
		continue;			
	}
	system_log(SYSLOG_DESTROY_MSG_HANDLER);
}

