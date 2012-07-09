/*
 * file 		: point_mgr.c
 * creator   	: tykim
 *
 *
 * point table�� ���� file�� �����Ѵ�. 
 *
 *
 * version :
 *  	0.0.1 - initiailize
 *  	0.0.2 - code clean 
 *
 *
 * code �ۼ��� ���ǻ���
 *		1. global ������ 'g_' ���λ縦 ����.
 *		2. ������ ������ ������ �Ʒ��� ���� ��Ģ���� �����Ѵ�. 
 *			int					n
 *			short				w
 *			long				l
 *			float				f
 *			char				ch
 *			unsigned char       b
 *			unsignnd int		un
 *			unsigned short 		uw 
 *			pointer				p
 *			structure			s or nothing
 *		3. �������� ù���ڴ� ����ڷ� ����Ѵ�. 
 *		4. ���������� �� ������ ������ ������� ����ڷ� �Ѵ�. 
 *			ex > g_nTest, g_bFlag, g_fPointTable
 *		5. �Լ����� �ҹ��ڷ� '_' ��ȣ�� ����ؼ� �����Ѵ�. 
 *		6. �Լ��� �Լ� ������ ������ 2�ٷ� �Ѵ�. 
 */
 
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

////////////////////////////////////////////////////////////////////////////////
//define
#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

#include "point_observer.h"


////////////////////////////////////////////////////////////////////////////////
//extern variable
extern pthread_mutex_t pointTable_mutex;										// point table mutex
extern pthread_mutex_t fileAccess_mutex;										// file access mutex

// point and node
extern int g_nMyPcm;															// my pcm number
extern int g_nMultiDdcFlag;														// mode select flag
extern unsigned char g_cNode[MAX_NET32_NUMBER];									// node
extern PTBL_INFO_T *g_pPtbl;													// point table
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];						// extension point table

// file access flag
extern int g_nFileAccess[MAX_NET32_NUMBER];

//mkdir system api
extern int mkdir(const char *pathname, mode_t mode);


////////////////////////////////////////////////////////////////////////////////
//global variable
int pointObserverDbg = 0;

/*
void file_create_node(void)
// ----------------------------------------------------------------------------
// CREAT NODE.TXT
// Description : create 'duksan/DATA/node.txt'
// Arguments   : none
// Returns     : none
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
	
	if((fp = fopen("/duksan/FILE/node", "w")) == NULL) {
		printf("[ERROR] /duksan/FILE/node not opened\n");
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
*/

int pnt_observer_value_check(void) 
{
	FILE *pFp;
	int nPcm = 0;
	int nPno = 0;
	char chFileName[32];
	float fValue;
	int nRtnVal = POINT_OBSEVER_VALUE_NOT_CHANGE;

	strncpy(chFileName, "/httpd/data.dat\0", sizeof(chFileName));
	
	//if there's no file named "data.dat", exit application.
	if((pFp = fopen(chFileName, "r+")) == NULL) {
		printf("+ %s file open fail\n", chFileName);
		printf("+ poiint observer error 02 \n");

		// make "data.dat" file		
		if((pFp = fopen(chFileName, "w")) != NULL)	{
			printf("+ make %s file", chFileName);
			
			// value initialize
			for (nPcm = 0; nPcm < MAX_NET32_NUMBER;  nPcm++) {
				for (nPno = 0; nPno < MAX_POINT_NUMBER;  nPno++) {
					pthread_mutex_lock( &pointTable_mutex );
					fwrite(&g_fExPtbl[nPcm][nPno], sizeof(float), 1, pFp);
					pthread_mutex_unlock( &pointTable_mutex );
				}
			}

			fclose(pFp);
			nRtnVal = POINT_OBSEVER_VALUE_CHANGE;
			return nRtnVal;
		}
		else {
			printf("+ %s file open fail\n", chFileName);
			printf("+ poiint observer error 03 \n");
			unlink(chFileName);
			nRtnVal = POINT_OBSEVER_FILE_ERROR;			
			return nRtnVal;
		}
	}

	fseek( pFp, 0, SEEK_SET);
		
	// compare value.		
	for ( nPcm = 0; nPcm < MAX_NET32_NUMBER;  nPcm++ ) {
		
		//ERROR ó��
		if ( nRtnVal == POINT_OBSEVER_FILE_ERROR)
			break;

		for ( nPno = 0; nPno < MAX_POINT_NUMBER;  nPno++ ) {
			
			if ( fread(&fValue, sizeof(float), 1, pFp) == 1 ) {
				//if data's been changed, recovery file
				pthread_mutex_lock( &pointTable_mutex );
				if ( g_fExPtbl[nPcm][nPno]  != fValue ) {
					fseek(pFp, -sizeof(float), SEEK_CUR);
					fwrite(&g_fExPtbl[nPcm][nPno], sizeof(float), 1, pFp);	
					nRtnVal = POINT_OBSEVER_VALUE_CHANGE;
				}
				pthread_mutex_unlock( &pointTable_mutex );
			}
			else {
				//ERROR ó��
				unlink(chFileName);
				nRtnVal = POINT_OBSEVER_FILE_ERROR;
				break;
			}
		}
	}

	fclose(pFp);
	return nRtnVal;
}


void httpd_file_check(void)
{
	int nRtnCheck = 0;

	nRtnCheck = pnt_observer_value_check();
	switch(nRtnCheck) {
	// �ٲ���� ��.
	case POINT_OBSEVER_VALUE_CHANGE: 
		break;
	
	// ���� �ٲ��� ���� ��
	case POINT_OBSEVER_VALUE_NOT_CHANGE: 
		break;

	// file error
	case POINT_OBSEVER_FILE_ERROR: 
		printf("File Erase\n");
		system("rm /httpd/data.dat");
		break;
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
	if( g_nFileAccess[index] == 0 ) {
		g_nFileAccess[index] = 1;
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
	if( g_nFileAccess[index] == 1 ) {
		g_nFileAccess[index] = 0;
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
	float value;
	int file_check_status;
	FILE *fp;
	DIR  *dp;
	char filename[32];
	point_info point;
	char index[2];
	struct timespec ts;
	
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

	//pnt_observer_check();
	
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
					//set content of file with value in g_fExPtbl
					for(j = 0; j < MAX_POINT_NUMBER; j++)
					{	
						pthread_mutex_lock( &pointTable_mutex );
						fwrite(&g_fExPtbl[i][j], sizeof(float), 1, fp);
						pthread_mutex_unlock( &pointTable_mutex );
						if (pointObserverDbg)
							printf("pno = %d, value = %f\n", j, g_fExPtbl[i][j]);

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
						pthread_mutex_lock( &pointTable_mutex );
						if(g_fExPtbl[i][j] != value)
						{
							if (pointObserverDbg)
								printf("DEBUG : Detected changes in file %s, pno = %d, current = %f, new = %f\n", filename, j, value, g_fExPtbl[i][j]);

							fseek(fp, -sizeof(float), SEEK_CUR);
							fwrite(&g_fExPtbl[i][j], sizeof(float), 1, fp);
							// jong2ry 2011_0309
							// elba�� 2���� Data�� ���۵Ǳ� ������ �ּ�ó���մϴ�. 
							/*
							if ( i == g_nMyPcm ) {
								//pSet( i, j, g_fExPtbl[i][j] );
							}
							*/
							// jong2ry 2011_0322
							// ���� �ڵ�� ���߱� ���Ͽ� �߰��մϴ�. 
							if ( i == g_nMyPcm ) {
								g_pPtbl[j].f_val = g_fExPtbl[i][j]; 
							}							
							
						}
						pthread_mutex_unlock( &pointTable_mutex );
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
				//pointObserverSelectSleep(0, 30);
				ts.tv_sec = 0;
				ts.tv_nsec = 30 * M_SEC;				
				nanosleep(&ts, NULL);
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
				//file_create_node();
				// jogn2ry 2011_0311 
				// ���� ������� �ʱ� ������ �����ϴ�. 
				//httpd_file_check();		
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
	sleep(1);

	while(1) {
		file_check();
		sched_yield();
	}
	syslog_record(SYSLOG_DESTROY_POINT_OBSERVER);	
}
