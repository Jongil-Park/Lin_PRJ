///******************************************************************/
// file : 	ghpSchedSvr.c
// date : 	2010.03.03.
// author : jong2ry
///******************************************************************/
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

#include "define.h"
#include "queue_handler.h"
#include "ghp_app.h"
#include "ghp_ScheduleSvr.h"

unsigned char sched_rx_msg[MAX_BUF_LENGTH];
unsigned char sched_tx_msg[MAX_BUF_LENGTH];

unsigned char message[MAX_BUF_LENGTH];
unsigned char dbg_msg[MAX_BUF_LENGTH];

int g_msgWr = 0;

/*******************************************************************/
//Extern mutex
extern pthread_mutex_t schedule_mutex;


/*
slect를 사용해서 TimeWait를 만들었다. 
*/
/*******************************************************************/
void IfaceGhpSchedSelectSleep(int sec,int msec) 
/*******************************************************************/
{
    struct timeval tv;
    tv.tv_sec=sec;
    tv.tv_usec=msec;                
    select(0,NULL,NULL,NULL,&tv);
    return;
}


/****************************************************************/
int schedule_main(void)
/****************************************************************/
{
	//Variables
	int i, j;
	//
	int fd_max;
	fd_set reads, temps;
	struct timeval tv;
	//
	int server_socket;
	int client_socket;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	//
	int fd;
	int sin_size;
	int server_port;
	int status;

	//Initialize
	i = j = 0;
	//
	fd_max = 0;
	FD_ZERO(&reads);
	FD_ZERO(&temps);
	tv.tv_sec = 0;
	tv.tv_usec = 10000; //0.01sec
	//
	server_socket = 0;
	client_socket = 0;
	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));
	//
	fd = 0;
	sin_size = sizeof(client_addr);
	server_port = SCHED_SERVER_PORT;
	status = INIT_PROCESS;
	
	//Ignore broken_pipe signal
	signal(SIGPIPE, SIG_IGN);
	IfaceGhpSchedSelectSleep(3,0);
	
	while(1)
	{
		IfaceGhpSchedSelectSleep(0,500);
		switch(status)
		{
			case INIT_PROCESS:
			{
				server_socket = socket(PF_INET, SOCK_STREAM, 0);
				printf("Schedule-Server Started, Port [%d]\n", server_port);
				//
				server_addr.sin_family = AF_INET;
				server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
				server_addr.sin_port = htons(server_port);
				//
				setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
				//
				if(bind(server_socket, (struct sockaddr *)&server_addr,
							sizeof(server_addr)) < 0)
				{
					printf("[ERROR] In Bind of Schedule-Server\n");
					close(server_socket);
					status = INIT_PROCESS;
					//sleep(10);
					IfaceGhpSchedSelectSleep(10,0);
					break;
				}
				//
				if(listen(server_socket, 10) < 0)
				{
					printf("[ERROR] In Listen of Schedule-Server\n");
					close(server_socket);
					status = INIT_PROCESS;
					//sleep(10);
					IfaceGhpSchedSelectSleep(10,0);
					break;
				}
				//
				FD_ZERO(&reads);
				FD_SET(server_socket, &reads);
				fd_max = server_socket;
				//
				status = SELECT_PROCESS;
				break;
			}
			//
			case SELECT_PROCESS:
			{
				//
				temps = reads;
				//
				if(select(fd_max+1, &temps, 0, 0, &tv) == -1)
				{
					printf("[ERROR] In Select of Schedule-Server\n");
					status = INIT_PROCESS;
					//sleep(10);
					IfaceGhpSchedSelectSleep(10,0);
					break;
				}
				//
				status = SELECT_PROCESS;
				//
				for(fd = 0; fd < (fd_max + 1); fd++)
				{
					if(FD_ISSET(fd, &temps))
					{	
						//if connection's been requested,
						if(fd == server_socket)
						{
							status = CONNECTION_REQUESTED;
							break;
						}
						else
						{
							status = HANDLE_COMMAND;
							break;
						}
					}
				}
				break;
			}
			//
			case CONNECTION_REQUESTED:
			{
				client_socket = accept(server_socket, (struct sockaddr *)&client_addr,
						               &sin_size);
				//
				if (client_socket == -1)
				{
					printf("[Error] In Accept of Schedule-Server\n");
					status = SELECT_PROCESS;
					break;
				}
				//
				FD_SET(client_socket, &reads);
				if(fd_max < client_socket)
					fd_max = client_socket;
				//
				status = SELECT_PROCESS;
				break;
			}
			//
			case HANDLE_COMMAND:
			{
				schedule_handler(fd, &reads);
				status = SELECT_PROCESS;
				break;
			}
		}			
	}
	//
	//return;
}

/************************************************************/
void schedule_handler(int fd, fd_set* reads)
/************************************************************/
{
	//int i = 0;
	//int status = 0;
	int rxmsg_length = 0;
	SEND_DATA *pRx;

	memset(sched_rx_msg, 0, sizeof(sched_rx_msg));
	rxmsg_length = recv(fd, sched_rx_msg, sizeof(sched_rx_msg), 0);

	if(rxmsg_length == 0)
	{
		g_msgWr = 0;
		memset(message, 0, sizeof(message));
		
		FD_CLR(fd, reads);
		close(fd);
		printf("Schedule-Server Close\n");
		return;
	}
	
	if ( (rxmsg_length + g_msgWr) > sizeof(message) - 2 )
	{
		g_msgWr = 0;
		memset(message, 0, sizeof(message));
		
		FD_CLR(fd, reads);
		close(fd);
		printf("Buffer Pull Schedule-Server Close\n");
		return;			
	}
		

	//printf("Handler RX msg[%d] : \n", rxmsg_length);
	memcpy(&message[g_msgWr], &sched_rx_msg, rxmsg_length);
	g_msgWr += rxmsg_length;
	pRx = (SEND_DATA *)&message;

	if(g_msgWr == pRx->length && Check_Message(pRx))
	{
		Parsing_Data(pRx);
		SetDate(pRx);
	}
}

/****************************************************************/
void SetDate(SEND_DATA *pRx)
/****************************************************************/
{
    struct tm set_time;
    time_t m_time;

	printf("%d-%d-%d %d:%d:%d\n", pRx->year, pRx->month, pRx->day, pRx->hour,	pRx->min, pRx->sec);			
	set_time.tm_year = pRx->year;  // 2010 - 1900 
	set_time.tm_mon  = pRx->month;    // 3 - 1 
	set_time.tm_mday = pRx->day; 
	set_time.tm_hour = pRx->hour; 
	set_time.tm_min  = pRx->min; 
	set_time.tm_sec  = pRx->sec; 
	
	m_time = mktime(&set_time);
	printf("m_time = %d\n", (int)m_time);
	printf("stime = %d\n", stime(&m_time));
}


/****************************************************************/
int Check_Message(SEND_DATA *pRx)
/****************************************************************/
{
	unsigned int i = 0;
	unsigned char *p;
	unsigned short chksum = 0;
	
	p = (unsigned char *)pRx;
	for ( i=0; i<pRx->length-2; i++)
		chksum += p[i];
	
	printf(">> length = %d, chksum = 0x%x, Cal = 0x%x\n", pRx->length, pRx->chksum, chksum);
	if(chksum == pRx->chksum)
		return SUCCESS;	
	else		
		return FAIL;	
}

/****************************************************************/
void Parsing_Data(SEND_DATA *pRx)
/****************************************************************/
{
	int i = 0;
	//int j = 0;
	//int k = 0;
	FILE *fp = NULL;
	unsigned char *p;
	int filesize = 0;
	
	pthread_mutex_lock(&schedule_mutex);
	if((fp = fopen("SchedGHP.dat", "w")) == NULL)
	{
		if (fp != NULL)  fclose(fp);
		printf("[ERROR] File Open with Option 'w'\n");		
		pthread_mutex_unlock(&schedule_mutex);
		return;
	}	
	printf("'SchedGHP.dat' File Open with Option 'w'\n");		
	
	p = (unsigned char *)pRx;
	for ( i=0; i<pRx->length; i++)
		fwrite(&p[i], sizeof(unsigned char), 1, fp);
	
	fclose(fp);
	pthread_mutex_unlock(&schedule_mutex);

////////////////////////////////////////////////////////////////////
//Debug..
#if 1
	if((fp = fopen("SchedGHP.dat", "r")) == NULL)
	{
		if (fp != NULL)  fclose(fp);
		printf("[ERROR] File Open with Option 'r'\n");		
		return;
	}	
	
	fseek(fp, 0L, SEEK_END); 
	filesize = ftell( fp );
	memset(&dbg_msg, 0x00, sizeof(dbg_msg));
	fseek(fp, 0L, SEEK_SET); 

	for ( i=0; i<filesize; i++)
		fread (&dbg_msg[i], 1, sizeof(unsigned char), fp);
	
	pRx = (SEND_DATA *)&dbg_msg;
	
	if(Check_Message(pRx) != SUCCESS)
		return;

	for ( i=0; i<200; i++ ) {
		printf("Class = %d, %d, %d, %d\n", 
			pRx->classData[i].index,
			pRx->classData[i].classNum,	
			pRx->classData[i].startT,
			pRx->classData[i].stopT);	
	}

/*	
	for ( i=0; i<MAX_GROUP_COUNT; i++ )
	{	
		printf("pRx->group[%d].group = %d\n", i, pRx->group[i].group); 	
		printf("pRx->group[%d].method = %d\n", i, pRx->group[i].method); 	
		printf("pRx->group[%d].mode = %d\n", i, pRx->group[i].mode); 	
		printf("pRx->group[%d].limitT = %d\n", i, pRx->group[i].limitT); 	
	
		for ( j=0; j<DAY_COUNT; j++ )
		{
			for ( k=0; k<MAX_SCHEDULE_COUNT; k++ )
			{
			printf("pRx->group[%d].startT[%d][%d] = %d\n", i, j, k
				,pRx->group[i].startT[j][k]); 	
			printf("pRx->group[%d].stopT[%d][%d] = %d\n", i, j, k
				,pRx->group[i].stopT[j][k]); 	
			}
		}	
	}

	for ( i=0; i<MAX_TIMETABLE_COUNT; i++ )
	{
		printf("pRx->time.startT[%d] = %d\n", i ,pRx->time.startT[i]); 	
		printf("pRx->time.stopT[%d] = %d\n", i ,pRx->time.stopT[i]); 	

	}	
*/
#endif
}

