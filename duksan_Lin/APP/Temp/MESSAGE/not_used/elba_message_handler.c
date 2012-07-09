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

/*********************************************************/
//Define
#include "define.h"
#include "queue.h"
#include "point_manager.h"
#include "message_handler.h"
#include "elba_message_handler.h"

/*********************************************************/
//Extern mutex
extern pthread_mutex_t elbaQ_mutex;
extern pthread_mutex_t net32Q_mutex;
extern pthread_mutex_t elbaMessageQ_mutex;
extern pthread_mutex_t elbaCmdQ_mutex;
extern pthread_mutex_t plcQ_mutex;			// jong2ry plc.

//extern pthread_cond_t elbaCond;
//extern pthread_cond_t elbaMsgCond;
//extern pthread_cond_t net32Cond; 

/*********************************************************/
//Extern point_queue
extern point_queue elba_queue;				// elba queue.
extern point_queue net32_queue;				// net32 queue
extern point_queue elba_message_queue;		// elba message queue
extern point_queue elba_cmd_queue;			// elba command queue
extern point_queue plc_message_queue;		// jong2ry plc.

/*********************************************************/
//Extern function.
extern int isVirtualPCM(int pcm);		// Check your pcm type
extern int isMyPCM(int pcm);			// Check your pcm type
extern void put_bacnet_message_queue(int pcm, int pno, float value);	// point data put on bacnet message queue

/*********************************************************/
//Extern status variable
extern queue_status status;
extern float point_table[MAX_NET32_NUMBER][MAX_POINT_NUMBER];
extern int multi_ddc;



/*********************************************************/
/*
주어진 시간동안 Thread를 Sleep시킨다. 
Wakeup message가 오게 되면 리턴한다.
*/
void elbaMsgMutexWait (int msec)
/*********************************************************/
{
    struct timeval now;
    struct timespec ts;

    //printf("start ThWaitSig..\n");
    gettimeofday(&now, NULL);
    ts.tv_sec = now.tv_sec + 1;
    ts.tv_nsec = now.tv_usec * 1000 * msec;

    pthread_mutex_lock(&elbaMessageQ_mutex);
    //DEL pthread_cond_timedwait(&elbaMsgCond, &elbaMessageQ_mutex, &ts);
    printf("elbaMsgQueue Wakeup\n");
    pthread_mutex_unlock(&elbaMessageQ_mutex);		
}


/*********************************************************/
void *elba_message_handler_main(void *arg)
/*********************************************************/
{
	//Variables
	int i, j;
	int result;
	float value;
	point_info point;
	struct timespec ts;
	
	//Initialize
	i = j = 0;
	result = ERROR;
	value = 0;
	memset(&point, 0, sizeof(point));
	
	
	//ts.tv_sec = 0;
	//ts.tv_nsec = 1000000; 
	
	//Ignore broken_pipe signal
	signal(SIGPIPE, SIG_IGN);
	elbaMsgMutexWait(100);
	
	while(1)
	{
		
		/* 
		elba thread가 elba_message_queue에 data를 넣는다.
		만약 elba_message_queue에 data가 있다면, getq는 SUCCESS를 리턴한다.
		*/
		pthread_mutex_lock(&elbaMessageQ_mutex);
		result = getq(&elba_message_queue, &point);
		elbaMsgMutexWait(1);
		pthread_mutex_unlock(&elbaMessageQ_mutex);

		/*
		getq의 리턴값이 SUCCESS가 아닌 경우에 Queue full을 확인한다.
		그리고 500mSec동안 thread를 sleep 한다.
		elbaMsgMutexWait()를 호출하여 elba thread로부터 오는 Wakeup message를 500mSec동안 기다린다.
		*/
		if(result != SUCCESS)
		{
			if (result == QUEUE_FULL)
				status.elba_message_full++;
			elbaMsgMutexWait(500);
			continue;
		}
		status.elba_message_out++;
		
		/*
		if(point.message_type == ELBA_COMMAND)
		{
			//put to elbaCmdQ for interface
			pthread_mutex_lock(&elbaCmdQ_mutex);
			status.elba_cmd_in++;
			putq(&elba_cmd_queue, point);
			pthread_mutex_unlock(&elbaCmdQ_mutex);
		}
		*/

		//printf("DEBUG : getQ from elba_message_queue, pcm = %d pno = %d\n", point.pcm, point.pno);
		
		switch(point.message_type)
		{
			case ELBA_REQUIRE:
			{
				/*
				point.message_type = NET32_REQUIRE;
			
				pthread_mutex_lock(&net32Q_mutex);
				status.net32_in++;
				putq(&net32_queue, point);
				//DEL pthread_cond_signal(&net32Cond);
				pthread_mutex_unlock(&net32Q_mutex);	
				*/
				if (multi_ddc)
				{
					point.value = point_table[point.pcm][point.pno];
					point.message_type = ELBA_REPORT;
		
					pthread_mutex_lock(&elbaQ_mutex);
					status.elba_in++;
					putq(&elba_queue, point);
					//DEL pthread_cond_signal(&elbaCond);
					pthread_mutex_unlock(&elbaQ_mutex);				
				}
				else
				{
				}
						
				break;
				/*
				if(isVirtualPCM(point.pcm) == SUCCESS)
				{
					point.value = pGet(point.pcm, point.pno);
					point.message_type = ELBA_REPORT;
					//
					pthread_mutex_lock(&elbaQ_mutex);
					status.elba_in++;
					putq(&elba_queue, point);
					pthread_mutex_unlock(&elbaQ_mutex);
				}
				else if(isMyPCM(point.pcm) == SUCCESS)
				{
					point.value = pGet(point.pcm, point.pno);
					point.message_type = ELBA_REPORT;
					//
					pthread_mutex_lock(&elbaQ_mutex);
					status.elba_in++;
					putq(&elba_queue, point);
					pthread_mutex_unlock(&elbaQ_mutex);
				}
				else
				{
					point.message_type = NET32_REQUIRE;
					pthread_mutex_lock(&net32Q_mutex);
					status.net32_in++;
					putq(&net32_queue, point);
					pthread_mutex_unlock(&net32Q_mutex);
				}
				break;
				*/
			}
			//
			case ELBA_COMMAND:
			{
				pSet(point.pcm, point.pno, point.value);
/*
				put_bacnet_message_queue(point.pcm, point.pno, point.value);

#if 1				
				pthread_mutex_lock(&plcQ_mutex);
				putq(&plc_message_queue, point);	// jong2ry plc.
				pthread_mutex_unlock(&plcQ_mutex);
#else
				pthread_mutex_lock(&queueLock_mutex);
				putq(&plc_message_queue, point);	// jong2ry plc.
				pthread_mutex_unlock(&queueLock_mutex);
#endif			
*/	
				
				/*
				if(isVirtualPCM(point.pcm) == SUCCESS)
				{
					pSet(point.pcm, point.pno, point.value);
					point.value = pGet(point.pcm, point.pno);
					//
					point.message_type = ELBA_REPORT;
					//printf("DEBUG : putQ to elba_queue(ELBA_COMMAND)\n");
					pthread_mutex_lock(&elbaQ_mutex);
					status.elba_in++;
					putq(&elba_queue, point);
					pthread_mutex_unlock(&elbaQ_mutex);
				}
				//
				else if(isMyPCM(point.pcm) == SUCCESS)
				{
					pSet(point.pcm, point.pno, point.value);
					point.value = pGet(point.pcm, point.pno);
					//printf("DEBUG : putQ to elba_queue(ELBA_COMMAND)\n");
					//
					point.message_type = ELBA_REPORT;
					pthread_mutex_lock(&elbaQ_mutex);
					status.elba_in++;
					putq(&elba_queue, point);
					pthread_mutex_unlock(&elbaQ_mutex);				
					//
					point.message_type = NET32_BROADCAST;
					pthread_mutex_lock(&net32Q_mutex);
					status.net32_in++;
					putq(&net32_queue, point);
					pthread_mutex_unlock(&net32Q_mutex);
				}
				//
				else
				{
					point.message_type = NET32_COMMAND;
					pthread_mutex_lock(&net32Q_mutex);
					status.net32_in++;
					putq(&net32_queue, point);
					pthread_mutex_unlock(&net32Q_mutex);
				}
				//
				break;
				*/
			}
		}
	}
	//return;
}





