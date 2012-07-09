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
#include "define.h"
#include "queue.h"
#include "point_manager.h"
#include "message_handler.h"
#include "net32_message_handler.h"

/**********************************************************/
//extern mutex
extern pthread_mutex_t elbaQ_mutex;
extern pthread_mutex_t net32MessageQ_mutex;
extern pthread_mutex_t pointTable_mutex;
extern pthread_mutex_t fileAccess_mutex;

//extern pthread_cond_t net32MsgCond;

/**********************************************************/
//extern queue
extern point_queue elba_queue;
extern point_queue net32_message_queue;
//
extern queue_status status;

/**********************************************************/
//extern variable
extern float point_table[MAX_NET32_NUMBER][MAX_POINT_NUMBER];
extern int multi_ddc;
extern int file_access[MAX_NET32_NUMBER];

/**********************************************************/
//extern function
extern int getFileAccess(int index);
extern int releaseFileAccess(int index);



/*********************************************************/
void net32MsgMutexWait (int msec)
/*********************************************************/
{
    struct timeval now;
    struct timespec ts;

    //printf("start ThWaitSig..\n");
    gettimeofday(&now, NULL);
    ts.tv_sec = now.tv_sec + 1;
    ts.tv_nsec = now.tv_usec * 1000 * msec;

    pthread_mutex_lock(&net32MessageQ_mutex);
    //DEL pthread_cond_timedwait(&net32MsgCond, &net32MessageQ_mutex, &ts);
    pthread_mutex_unlock(&net32MessageQ_mutex);		
}



/*********************************************************/
void *net32_message_handler_main(void* arg)
/*********************************************************/
{
	int i = 0;
	int result = ERROR;
	//float value = 0;
	point_info point;
	struct timespec ts;
	FILE* fp;
	char filename[32];
	char index[2];
	float pre_value; 		// jong2ry 091126
	int pcm = 0;
	int pno = 0;
	float value = 0;
	
	
	//ts.tv_sec = 0;
	//ts.tv_nsec = 1000000; 
	//Ignore broken_pipe signal
	signal(SIGPIPE, SIG_IGN);	
	usleep(3000);
	
	while(1)
	{
		
		memset(&point, 0, sizeof(point));

		pthread_mutex_lock(&net32MessageQ_mutex);
		result = getq(&net32_message_queue, &point);
		pthread_mutex_unlock(&net32MessageQ_mutex);
	
		if(result != SUCCESS)
		{
			if (result == QUEUE_FULL)
				status.net32_message_full++;

			net32MsgMutexWait(500);
			//printf("$");
			continue;
		}
		status.net32_message_out++;
		
		if (multi_ddc)
		{
// file-access time을 줄이기 위하여 아래와 같이 코딩합니다. 
// 2010.03.18 jong2ry
#if 0	
			if (point.message_type == NET32_COMMAND)
			{
				point.value = point_table[point.pcm][point.pno];
				point.message_type = ELBA_REPORT;
				pthread_mutex_lock(&elbaQ_mutex);
				status.elba_in++;
				putq(&elba_queue, point);
				pthread_mutex_unlock(&elbaQ_mutex);				
			}
#else
			if (point.message_type == NET32_COMMAND)
			{
				fp = NULL;
				pcm = point.pcm;
				pno = point.pno;
				value = point.value;
				strncpy(filename, "/duksan/DATA/point00.dat\0", sizeof(filename));
				memset(index, 0, sizeof(index));
				
				index[0] = (pcm / 10) + 48;
				index[1] = (pcm % 10) + 48;
				strncpy(&filename[18], index, sizeof(index));
				
				//Get file access right
				while(getFileAccess(pcm) == ERROR);
				
				if((fp = fopen(filename, "r+")) == NULL)
				{
					releaseFileAccess(pcm);
					//return ERROR;
					continue;
				}
				
				// jong2ry 091126
				fseek(fp, sizeof(float) * pno, SEEK_SET);
				if(fread(&pre_value, sizeof(float), 1, fp) != 1)
					printf("[ERROR] pGet to File has been Failed\n");
			
				if (pre_value == value)
				{
					fclose(fp);
					releaseFileAccess(pcm);		//Release file access right		
					//return SUCCESS;
					continue;
				}
			
				rewind(fp);
				fseek(fp, sizeof(float) * pno, SEEK_SET);
				if(fwrite(&value, sizeof(float), 1, fp) != 1)
					printf("[ERROR] pSet to File has been Failed\n");
			
				fclose(fp);
				releaseFileAccess(pcm);		//Release file access right	
				point_table[pcm][pno] = value;
			}
#endif
		}
		else
		{
		}
		
	}
}









