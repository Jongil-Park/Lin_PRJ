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
//

//mutex
pthread_mutex_t plcObserver_mutex = PTHREAD_MUTEX_INITIALIZER;

void plc_main(void)
{
	while(1)
	{
		printf("It's Me... PLC\n");
		sleep(1000);
	}
}


/****************************************************************/
int 
main(void)
/****************************************************************/
{
	pthread_t thread[10];
	pthread_attr_t thread_attr;
	struct sched_param thread_param;
	int thread_policy;	
	int th_id;
	int th_status;
	int rr_min_priority, rr_max_priority;

	
	if(pthread_attr_init(&thread_attr) != 0)
	{
		printf("Thread attr init error\n");
		return ERROR;
	}
	pthread_attr_getschedpolicy(&thread_attr, &thread_policy);
	pthread_attr_getschedparam(&thread_attr, &thread_param);
	//
	printf("Default policy is %s, priority is %d\n", 
			(thread_policy == SCHED_FIFO ? "FIFO"
			 : (thread_policy == SCHED_RR ? "RR"
				: (thread_policy == SCHED_OTHER ? "OTHER"
					: "unknown"))), thread_param.sched_priority);
	
	//
	rr_min_priority = sched_get_priority_min(SCHED_RR);
	rr_max_priority = sched_get_priority_max(SCHED_RR);
	printf("[Round Robin] Min : %d, Max : %d\n", rr_min_priority, rr_max_priority);
	//
	//
	th_id = pthread_create(&thread[0], NULL, (void*(*)(void*)) plc_main, 
			               (void*) &thread[0]);
	printf("Plc_Main thread created\n");
	sleep(3);
	//
	//
	pthread_join(thread[0], (void **)th_status);
	//
	return 1;
}

