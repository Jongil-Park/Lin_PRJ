/****************************************************************************** 
 * file : message_handler.c
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
 * | message_handler.c  | == net32_message_queue ==> | net32.c |
 * |--------------------|                            |---------|
 * 
 * 
 * version :
 * 		0.0.1 - code clean  jong2ry
 * 		0.0.2 - Status 변수를 통한 point_handler()의 동작상태 확인. 2010-07-19  jong2ry
 * 		0.0.3 - command 요청과 report 요청을 분리함. 2010-07-19  jong2ry 
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



#include "define.h"
#include "queue.h"
#include "net32.h"
#include "elba.h"
#include "point_manager.h"
#include "message_handler.h"

extern pthread_mutex_t pointTable_mutex;									// point table mutex
extern float point_table[MAX_NET32_NUMBER][MAX_POINT_NUMBER];				// point table only value
extern CHK_STATUS_T *gp_status;												// status structure
extern int my_pcm;															// my pcm number
extern PTBL_INFO_T *gp_ptbl;												// point-table

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
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = usec;
    select( 0,NULL,NULL,NULL,&tv );
    return;
}


void *message_handler_main(void* arg)
// ----------------------------------------------------------------------------
// MESSAGE_HANDLER_THREAD MAIN LOOP
// Description : This function is called by main().
// Arguments   : none
// Returns     : none
{
	int n_ret = 0;
	int n_pno = 0;
	point_info point;
	struct timeval chk_time_1;
	struct timeval chk_time_2;
	STATUS_THREAD_T *p_msg_handler_t = (STATUS_THREAD_T *) &gp_status->st_msg_handler_t;

	signal(SIGPIPE, SIG_IGN);	// Ignore broken_pipe signal

	while(1)
	{
		// 15분마다 자신의 전 포인트를 전송한다.
		gettimeofday( &chk_time_1, NULL );
		if ( chk_time_1.tv_sec - chk_time_2.tv_sec > 900 ) {

			for ( n_pno = 0; n_pno < 32; n_pno++ ) {
				point.pcm = my_pcm;
				point.pno = n_pno;
				point.value = gp_ptbl[n_pno].f_val;
				point.message_type = NET32_TYPE_REPORT;
		
				net32_put_msgqueue(&point);	
			}
			gettimeofday( &chk_time_2, NULL );
		}

		// net32_queue는 net32.c에서 uart1을 통해서 들어오는 data가 쌓인다.
		n_ret = net32_get_queue( &point );
		
		p_msg_handler_t->n_connetion = 1;		// always 1
		p_msg_handler_t->n_loopcnt++;
	
		if( n_ret != SUCCESS ) {
			msg_sleep( 0, 100 );
			continue;
		}

		if ( g_nViewNet32 ) {
			fprintf ( stderr, "get queue %d %d %f (type %d)\n", 
					  point.pcm, point.pno, point.value, point.message_type  );
			fflush(stderr);
		}


		// Require 요청인 경우에는
		// point-table의 값을 그대로 net32_message_queue로 넣는다.
		if ( point.message_type == NET32_TYPE_REQUIRE && point.pcm == my_pcm ) {
			point.value = point_table[point.pcm][point.pno];
			point.message_type = NET32_TYPE_REPORT;
			net32_put_msgqueue( &point );
			continue;			
		}


		// command 요청과 report 요청을 분리하였다. 
		// 2010-07-19일 이후에 시스템에 문제가 발생하지 않는다면
		// 기존에 작성된 부분을 삭제한다. 
#if 0
		// Report와 Command 요청인 경우에는
		// net32.c에서 이미 필터링이 되어서 
		// net32_queue를 통해 값을 넘겨받았으므로
		// point-table을 업데이트 한다.
		if ( point_table[point.pcm][point.pno] != point.value ) {
			
			pthread_mutex_lock( &pointTable_mutex );
			point_table[point.pcm][point.pno] = point.value;
			pthread_mutex_unlock( &pointTable_mutex );
	
			// 만약 자신의 point-table이 업데이트 되었다면,
			// net32_message_queue로 data를 보낸다.
			if ( point.pcm == my_pcm ) {
				point.message_type = NET32_TYPE_REPORT;
				net32_put_msgqueue( &point );
			}
		}
#else
		// Command 요청인 경우에는
		// pnt_local_adc()를 호출해서 값을 쓴다.
		if ( point.message_type == NET32_TYPE_COMMAND && point.pcm == my_pcm ) {
			pnt_local_pset( &point );											// set local point value
			//pSet(point.pcm, point.pno, point.value);
			elba_put_queue( &point );											// data push elba_queue. 
			continue;			
		}

		// Report 요청인 경우에는
		// point-table을 업데이트 한다.
		// 주의할 점은 자신의 point table을 업데이트 하는 것이 아니라,
		// 다른 PCM의 point-table을 업데이트 하는 것이다. 
		if ( point.message_type == NET32_TYPE_REPORT && point.pcm != my_pcm ) {
			// 다른 pcm의 point table을 update하는 것을 주의한다. -- !!
			pthread_mutex_lock( &pointTable_mutex );
			point_table[point.pcm][point.pno] = point.value;
			pthread_mutex_unlock( &pointTable_mutex );
			continue;			
		}
#endif

	}
}

