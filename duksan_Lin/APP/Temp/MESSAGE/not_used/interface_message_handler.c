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
//Define
#include "define.h"
#include "queue.h"
#include "point_manager.h"
#include "message_handler.h"
#include "interface_message_handler.h"
//Extern mutex
extern pthread_mutex_t elbaQ_mutex;
extern pthread_mutex_t net32Q_mutex;
extern pthread_mutex_t interfaceMessageQ_mutex;

extern pthread_mutex_t queueLock_mutex;
//Extern point_queue
extern point_queue elba_queue;
extern point_queue net32_queue;
extern point_queue interface_message_queue;

//Extern check function.
extern int isVirtualPCM(int pcm);
extern int isMyPCM(int pcm);
//Extern status variable

extern queue_status status;

/****************************************************************/
void ifaceMessageWait(int msec)
/****************************************************************/
{
	int scale = 0;
	int waitClock = 0;
	struct timeval t1, t2;
	unsigned int val1, val2;
	
	scale = 1000000 / 1000;
	waitClock = msec * scale;

	gettimeofday(&t1, NULL);
	while(1)
	{
		sched_yield();	
		gettimeofday(&t2, NULL);

		val1 = (t2.tv_sec - t1.tv_sec) * 1000000;
		val2 = t2.tv_usec + (1000000 - t1.tv_usec);
	
		if ( val1 + val2 > waitClock )
			return;
	}	
}


/*********************************************************/
void*
interface_message_handler_main(void* arg)
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
	usleep(3000);
	
	while(1)
	{
		
#if 1		
		pthread_mutex_lock(&interfaceMessageQ_mutex);
		result = getq(&interface_message_queue, &point);
		pthread_mutex_unlock(&interfaceMessageQ_mutex);
#else
		pthread_mutex_lock(&queueLock_mutex);
		result = getq(&interface_message_queue, &point);
		pthread_mutex_unlock(&queueLock_mutex);
#endif
		if(result != SUCCESS)
		{
			if (result == QUEUE_FULL)
				status.interface_message_full++;			
			//nanosleep(&ts, NULL);
			//sleep(1);
			//sched_yield();
			ifaceMessageWait(100);
			//printf("*");
			continue;
		}

		status.interface_message_out++;
		
		//For cmd_handler
		if(point.message_type == CMD_PSET)
			point.message_type = IF_PSET;

		if(point.message_type == CMD_PGET)
			point.message_type = IF_PGET;

		switch(point.message_type)
		{
			case IF_PSET:
			{
				if(isVirtualPCM(point.pcm) == SUCCESS)
				{
					pSet(point.pcm, point.pno, point.value);
					point.value = pGet(point.pcm, point.pno);
					
					point.message_type = ELBA_REPORT;
#if 1					
					pthread_mutex_lock(&elbaQ_mutex);
					status.elba_in++;
					putq(&elba_queue, point);
					pthread_mutex_unlock(&elbaQ_mutex);
#else
					pthread_mutex_lock(&queueLock_mutex);
					status.elba_in++;
					putq(&elba_queue, point);
					pthread_mutex_unlock(&queueLock_mutex);
#endif					
				}
				else if(isMyPCM(point.pcm) == SUCCESS)
				{
					pSet(point.pcm, point.pno, point.value);
					point.value = pGet(point.pcm, point.pno);
					
					point.message_type = ELBA_REPORT;
#if 1					
					pthread_mutex_lock(&elbaQ_mutex);
					status.elba_in++;
					putq(&elba_queue, point);
					pthread_mutex_unlock(&elbaQ_mutex);
#else
					pthread_mutex_lock(&queueLock_mutex);
					status.elba_in++;
					putq(&elba_queue, point);
					pthread_mutex_unlock(&queueLock_mutex);
#endif					
					
					point.message_type = NET32_BROADCAST;
#if 1					
					pthread_mutex_lock(&net32Q_mutex);
					status.net32_in++;
					putq(&net32_queue, point);
					pthread_mutex_unlock(&net32Q_mutex);
#else
					pthread_mutex_lock(&queueLock_mutex);
					status.net32_in++;
					putq(&net32_queue, point);
					pthread_mutex_unlock(&queueLock_mutex);
#endif					
				}
				else
				{
					point.message_type = NET32_COMMAND;
#if 1					
					pthread_mutex_lock(&net32Q_mutex);
					status.net32_in++;
					putq(&net32_queue, point);
					pthread_mutex_unlock(&net32Q_mutex);
#else
					pthread_mutex_lock(&queueLock_mutex);
					status.net32_in++;
					putq(&net32_queue, point);
					pthread_mutex_unlock(&queueLock_mutex);
#endif					
				}
				break;
			}
			
			case IF_PGET:
			{
				if(isVirtualPCM(point.pcm) == SUCCESS)
				{
					point.value = pGet(point.pcm, point.pno);
				}
				else if(isMyPCM(point.pcm) == SUCCESS)
				{
					point.value = pGet(point.pcm, point.pno);
				}
				else
				{
					point.message_type = NET32_IF_PGET_REQUIRE;
#if 1					
					pthread_mutex_lock(&net32Q_mutex);
					status.net32_in++;
					putq(&net32_queue, point);
					pthread_mutex_unlock(&net32Q_mutex);
#else
					pthread_mutex_lock(&queueLock_mutex);
					status.net32_in++;
					putq(&net32_queue, point);
					pthread_mutex_unlock(&queueLock_mutex);
#endif					
				}
				break;
			}
		}
	}
	//return ;
}


