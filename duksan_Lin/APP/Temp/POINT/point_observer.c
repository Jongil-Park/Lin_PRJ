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
#include <semaphore.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/poll.h>		// use poll event

#include "define.h"
#include "queue.h"
#include "point_observer.h"
#include "point_manager.h"		// pSet, pGet...


/****************************************************************/
// Extern Variable
extern int my_pcm;
extern int multi_ddc;
extern unsigned char g_cNode[32];

extern int file_access[MAX_NET32_NUMBER];
extern float point_table[MAX_NET32_NUMBER][MAX_POINT_NUMBER];
extern PTBL_INFO_T *gp_ptbl;

extern pthread_mutex_t pointTable_mutex;
extern pthread_mutex_t fileAccess_mutex;

extern int mkdir(const char *pathname, mode_t mode);

int pointObserverDbg = 0;

/*
********************************************************************* 
* START A WAIT_TIMER
* 
* Description	: This function is called by Point_Observer_Thread to create wait-timer.
* 
* 
* Arguments		: sec			Is a second data.
* 
* 				  usec			Is a micro-second data. 
* 
* 
* Returns		: none
* 
********************************************************************* 
*/
void pointObserverSelectSleep(int sec,int usec) 
{
    struct timeval tv;
    tv.tv_sec=sec;
    tv.tv_usec=usec;
    select(0,NULL,NULL,NULL,&tv);
    return;
}


/*
********************************************************************* 
* Point_Observer_Thread MAIN LOOP 
* 
* Description	: This function is called by main().
* 					- call file_check function.
* 
* 
* Arguments		: none.
* 
*  
* Returns		: none.
* 
********************************************************************* 
*/
void *point_observer_main(void* arg)
{
	signal(SIGPIPE, SIG_IGN);	
	usleep(3000);

	while(1) {
		file_check();
	}
}


/*
********************************************************************* 
* CREAT NODE.TXT
* 
* Description	: This function is called by file_check().
* 					- create 'duksan/DATA/node.txt'
* 
* 
* Arguments		: none.
* 
*  
* Returns		: none.
* 
********************************************************************* 
*/
void file_create_node(void)
{
	int i = 0;
	FILE *fp;

	if((fp = fopen("/duksan/DATA/node.txt", "w")) == NULL) {
		printf("[ERROR] /duksan/DATA/node.txt not opened\n");
		return;		
	}
	else {

		for ( i = 0; i < 32; i++ ) {
			if ( g_cNode[i] ) {
				fprintf(fp, "Master : %02d\n", i);
				break;
			}
		}
		
		fprintf(fp, "Node ( 0 ~ 31 ): ");
		for ( i = 0; i < 32; i++ ) {
			if ( !(i & 3) )		
				fprintf(fp, " ");
			if ( g_cNode[i] )	
				fprintf(fp, "O");
			else				
				fprintf(fp, "-");
		}				
		fprintf(fp, "\n\n");

		fclose(fp);
	}		
}


/*
********************************************************************* 
* GET FILE_ACCESS_MUTEX
* 
* Description	: This function is called by fileStatusCheck() to get a fileAccess_mutex.
* 
* 
* Arguments		: index		Is a PCM number(int type) .
* 
*  
* Returns		: SUCCESS	If the call was successful.
* 
* 		 		  ERROR		If not 
* 
********************************************************************* 
*/
int getFileAccess(int index)
{
	pthread_mutex_lock(&fileAccess_mutex);
	if( file_access[index] == 0 ) {
		file_access[index] = 1;
		pthread_mutex_unlock(&fileAccess_mutex);
		return SUCCESS;
	}
	pthread_mutex_unlock(&fileAccess_mutex);
	return ERROR;
}


/*
********************************************************************* 
* RELEASE FILE_ACCESS_MUTEX
* 
* Description	: This function is called by file_check() to release a fileAccess_mutex.
* 
* 
* Arguments		: index		Is a PCM number(int type) .
* 
*  
* Returns		: SUCCESS	If the call was successful.
* 
* 		 		  ERROR		If not 
* 
********************************************************************* 
*/
int releaseFileAccess(int index)
{
    pthread_mutex_lock(&fileAccess_mutex);
	if( file_access[index] == 1 ) {
		file_access[index] = 0;
		pthread_mutex_unlock(&fileAccess_mutex);
		return SUCCESS;
	}
			
	pthread_mutex_unlock(&fileAccess_mutex);
	return ERROR;
}



/*
********************************************************************* 
* CREAT NODE.TXT
* 
* Description	: This function is called by point_observer_main().
* 					- if 'duksan/DATA/pointxx.dat' file not found, create dat file.
* 					- search value that to change. and wrie dat file.
* 
* 
* Arguments		: none.
* 
*  
* Returns		: none.
* 
********************************************************************* 
*/
int file_check(void)
{
	//Variables	   
	int i, j;
	//int s = 0;
	float value;
	int file_check_status;
	//
	FILE *fp;
	DIR  *dp;
	char filename[32];
	point_info point;
	char index[2];
	
	//Initialize
	i = j		   		= 0;
	value   			= 0;
	file_check_status 	= POINT_OBSERVER_GET_FILE_NAME;
	//
	fp = NULL;
	dp  = NULL;
	memset(filename, 0, sizeof(filename));
	strncpy(filename, "/duksan/DATA/point00.dat\0", sizeof(filename));
	memset(&point, 0, sizeof(point));
	memset(index, 0, sizeof(index));
	
	while(1)
	{
		if (pointObserverDbg)
			printf("file_check_status = %d, PCM = %d\n", file_check_status, i);
		
		switch(file_check_status)
		{
			case POINT_OBSERVER_GET_FILE_NAME:
			{
				index[0] = (i / 10) + 48;
				index[1] = (i % 10) + 48;
				strncpy(&filename[18], index, sizeof(index));
				if (pointObserverDbg)
					printf("filename = %s \n", filename);
				//
				file_check_status = POINT_OBSERVER_FILE_STATUS_CHECK;
				break;
			}

			case POINT_OBSERVER_FILE_STATUS_CHECK:
			{
				file_check_status = fileStatusCheck(filename, i);
				break;
			}

			case POINT_OBSERVER_FILE_CREATE:
			{
				//if there's no diredtory named "DATA", make one.
				if((dp = opendir("../DATA")) == NULL)
					mkdir("./../DATA", 00777);
				
				if((fp = fopen(filename, "w")) == NULL)
				{
					if (pointObserverDbg)
						printf("[ERROR] File Open with Option 'w', File = [%s]\n", filename);
				}
				//
				else
				{
					if (pointObserverDbg)
						printf("[SUCCESS] File %s has been Created\n", filename);
					//set content of file with value in point_table
					for(j = 0; j < MAX_POINT_NUMBER; j++)
					{
						fwrite(&point_table[i][j], sizeof(float), 1, fp);
						if (pointObserverDbg)
							printf("pno = %d, value = %f\n", j, point_table[i][j]);
					}
					//
					fclose(fp);
				}
				file_check_status = POINT_OBSERVER_INCREASE_INDEX;
				break;
			}

			case POINT_OBSERVER_FILE_READ:
			{
				if((fp = fopen(filename, "r+")) == NULL)
				{
					file_check_status = POINT_OBSERVER_FILE_CREATE;
					break;
				}
				//
				for(j = 0; j < MAX_POINT_NUMBER; j++)
				{
					if(fread(&value, sizeof(float), 1, fp) == 1)
					{
						//if data's been changed, recovery file
						if(point_table[i][j] != value)
						{
							if (pointObserverDbg)
								printf("DEBUG : Detected changes in file %s, pno = %d, current = %f, new = %f\n", filename, j, value, point_table[i][j]);

							fseek(fp, -sizeof(float), SEEK_CUR);
							fwrite(&point_table[i][j], sizeof(float), 1, fp);
							if ( i == my_pcm ) {
								pSet( i, j, point_table[i][j] );
							}
						}
					}
					//if file has been corrupted,
					else
					{
						//remove file
						if(unlink(filename) == 0)
						{
							printf("File %s has been Erased (Because of Corruption)\n", filename);
						}
					}
				}
				//
				fclose(fp);
				releaseFileAccess(i);
				//
				file_check_status = POINT_OBSERVER_INCREASE_INDEX;
				break;
			}
			
			case POINT_OBSERVER_FILE_ACCESS_FAILED:
			{
				file_check_status = POINT_OBSERVER_INCREASE_INDEX;
				break;
			}
			
			case POINT_OBSERVER_INCREASE_INDEX:
			{
				pointObserverSelectSleep(0, 100000);
				//
				i = i + 1;
				//
				if(i < MAX_NET32_NUMBER)
					file_check_status = POINT_OBSERVER_GET_FILE_NAME;
				else
					file_check_status = POINT_OBSERVER_FINISH_FILE_CHECK_PROCESS;
				break;
			}
			
			case POINT_OBSERVER_FINISH_FILE_CHECK_PROCESS:
			{
				file_create_node();
				return SUCCESS;
			}
		}
	}
	//
	return ERROR;
}



/*
********************************************************************* 
* CALL getFileAccess FUNCTION
* 
* Description	: This function is called by file_check().
* 					- call getFileAccess()
* 
* 
* Arguments		: filename	Is a filename pointer.
* 
* 				  index		Is a PCM number(int type) 
*  
* Returns		: POINT_OBSERVER_FILE_READ				If the call was successful
* 
* 				  POINT_OBSERVER_FILE_ACCESS_FAILED		If not
* 
********************************************************************* 
*/
int fileStatusCheck(char* filename, int index)
{

	FILE* fp = NULL;

	//if there's no file such name, 
	if((fp = fopen(filename, "r")) == NULL)
	{
		return POINT_OBSERVER_FILE_CREATE;
	}

	fclose(fp);			
	if(getFileAccess(index) == SUCCESS)
		return POINT_OBSERVER_FILE_READ;
	else
		return POINT_OBSERVER_FILE_ACCESS_FAILED;
}
