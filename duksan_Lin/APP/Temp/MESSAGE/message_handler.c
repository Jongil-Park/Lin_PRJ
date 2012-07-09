/****************************************************************************** 
 * file : message_handler.c
 * author : jong2ry
 * 
 * 
 * net32.c�� ���� net32_queue�� ���� point data�� ó���Ѵ�. 
 * point-table�� ������Ʈ �Ѵ�.
 * elba_queue�� ���� elba.c�� point data�� ������. 
 *  
 * |----------|                    |-------------------|                     |--------|
 * | net32.c  | == net32_queue ==> | message_handler.c |  == elba_queue ==>  | elba.c |
 * |----------|                    |-------------------|                     |--------|
 *
 * point-table�� ������Ʈ �� ��, ���࿡ ���� �ٲ�� �Ǹ� 
 * net32_message_queue�� ���� net32.c�� point data�� ������.
 * 
 * !!! -- point-table�� ���� ������Ʈ �� ��쿡�� ������. -- !!!
 *
 * |--------------------|                            |---------|
 * | message_handler.c  | == net32_message_queue ==> | net32.c |
 * |--------------------|                            |---------|
 * 
 * 
 * version :
 * 		0.0.1 - code clean  jong2ry
 * 		0.0.2 - Status ������ ���� point_handler()�� ���ۻ��� Ȯ��. 2010-07-19  jong2ry
 * 		0.0.3 - command ��û�� report ��û�� �и���. 2010-07-19  jong2ry 
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
extern int g_nViewNet32;													// debug���� ����� �� ����ϴ� flag


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
		// 15�и��� �ڽ��� �� ����Ʈ�� �����Ѵ�.
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

		// net32_queue�� net32.c���� uart1�� ���ؼ� ������ data�� ���δ�.
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


		// Require ��û�� ��쿡��
		// point-table�� ���� �״�� net32_message_queue�� �ִ´�.
		if ( point.message_type == NET32_TYPE_REQUIRE && point.pcm == my_pcm ) {
			point.value = point_table[point.pcm][point.pno];
			point.message_type = NET32_TYPE_REPORT;
			net32_put_msgqueue( &point );
			continue;			
		}


		// command ��û�� report ��û�� �и��Ͽ���. 
		// 2010-07-19�� ���Ŀ� �ý��ۿ� ������ �߻����� �ʴ´ٸ�
		// ������ �ۼ��� �κ��� �����Ѵ�. 
#if 0
		// Report�� Command ��û�� ��쿡��
		// net32.c���� �̹� ���͸��� �Ǿ 
		// net32_queue�� ���� ���� �Ѱܹ޾����Ƿ�
		// point-table�� ������Ʈ �Ѵ�.
		if ( point_table[point.pcm][point.pno] != point.value ) {
			
			pthread_mutex_lock( &pointTable_mutex );
			point_table[point.pcm][point.pno] = point.value;
			pthread_mutex_unlock( &pointTable_mutex );
	
			// ���� �ڽ��� point-table�� ������Ʈ �Ǿ��ٸ�,
			// net32_message_queue�� data�� ������.
			if ( point.pcm == my_pcm ) {
				point.message_type = NET32_TYPE_REPORT;
				net32_put_msgqueue( &point );
			}
		}
#else
		// Command ��û�� ��쿡��
		// pnt_local_adc()�� ȣ���ؼ� ���� ����.
		if ( point.message_type == NET32_TYPE_COMMAND && point.pcm == my_pcm ) {
			pnt_local_pset( &point );											// set local point value
			//pSet(point.pcm, point.pno, point.value);
			elba_put_queue( &point );											// data push elba_queue. 
			continue;			
		}

		// Report ��û�� ��쿡��
		// point-table�� ������Ʈ �Ѵ�.
		// ������ ���� �ڽ��� point table�� ������Ʈ �ϴ� ���� �ƴ϶�,
		// �ٸ� PCM�� point-table�� ������Ʈ �ϴ� ���̴�. 
		if ( point.message_type == NET32_TYPE_REPORT && point.pcm != my_pcm ) {
			// �ٸ� pcm�� point table�� update�ϴ� ���� �����Ѵ�. -- !!
			pthread_mutex_lock( &pointTable_mutex );
			point_table[point.pcm][point.pno] = point.value;
			pthread_mutex_unlock( &pointTable_mutex );
			continue;			
		}
#endif

	}
}

