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
#include "global.h"
#include "struct.h"
#include "plc_observer.h"
#include "dnp_observer.h"
#include "cmd_server.h"
//
#define	MAX_ENV_NUMBER	2
//
extern unsigned char chPlcIp[2][20];
//

/****************************************************************/
int set_config_plc()
/****************************************************************/
{
	//Variables
	int i;
	int flag;
	int index;
	FILE* fp;
	char name[20];
	char value[MAX_ENV_NUMBER][100];

	//Initailze
	i  = 0;
	flag = 0;
	index = 0;
	fp = NULL;
	memset(name, 0, sizeof(name));
	memset(value, 0, sizeof(value));
	memset(chPlcIp, 0x00, sizeof(chPlcIp));
	
	if((fp = fopen(".configPLC", "r")) == NULL)
	{
		printf("Can't open file .configPLC for setting PLC IP Address\n");
		return ERROR;
	}
	//
	for(i = 0; i < MAX_ENV_NUMBER; i++)
	{
		fscanf(fp, "%s %s\n", name, value[i]);
		if(strncmp(name, "plc1", 10) == 0)
		{
			memcpy(&chPlcIp[0], &value[0], sizeof(chPlcIp[0]));
			printf("PLC 1 IP Address:: %s\n", chPlcIp[0]);
		}
		else if(strncmp(name, "plc2", 10) == 0)
		{
			memcpy(&chPlcIp[1], &value[1], sizeof(chPlcIp[1]));
			printf("PLC 2 IP Address:: %s\n", chPlcIp[1]);
		}
	}
	return SUCCESS;
}

/****************************************************************/
int main(void)
/****************************************************************/
{
	pthread_t thread[10];
	pthread_attr_t thread_attr;
	struct sched_param thread_param;
	int thread_policy;	
	int th_id;
	int th_status;
	int rr_min_priority, rr_max_priority;
	int nRtnVal = 0;
	//
	//	
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
	nRtnVal = set_config_plc();
	if (nRtnVal == ERROR)
	{
		printf("PLC Config Fail. Check .configPLC\n");
		return 1;
	}
	//
	//
	th_id = pthread_create(&thread[0], NULL, (void*(*)(void*)) plc_observer_main, 
			               (void*) &thread[0]);
	printf("Plc_Observer_Main thread created\n");
	sleep(3);
	//
	th_id = pthread_create(&thread[1], NULL, (void*(*)(void*)) dnp_observer_main, 
			               (void*) &thread[1]);
	printf("Dnp_Observer_Main thread created\n");
	sleep(3);
	//
	th_id = pthread_create(&thread[2], NULL, (void*(*)(void*)) command_handler_main, 
			               (void*) &thread[2]);
	printf("Command_Server_Main thread created\n");
	sleep(3);
	//
	//
	pthread_join(thread[0], (void **)th_status);
	//
	return 1;
}

