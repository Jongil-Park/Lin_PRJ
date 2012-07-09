/*
 * file 		: net32_mgr.c
 * creator   	: tykim
 *
 *
 * kernel�� TCP ����� ���� Net32 Driver�� data�� ���´�. 
 * �ٸ� Thread�κ��� net32_push_queue ���� point data�� �޴´�.
 * ���� data�� Send-Socket�� ���� kernel�� ������.
 * |---------------|                    |---------|                |--------|
 * | other Thread  | == net32_queue ==> | net32.c |  == Socket ==> | Kernel |
 * |---------------|                    |---------|                |--------|
 *
 *
 * kernel�κ��� Receive-Socket�� ���� ���� point data�� 
 * ���� ó���ϰ� Elba�� ������ �� data�� elba_put_queue�� ���� elba thread�� ������. 
 * |---------|               |---------|                    |------------|
 * | Kernel  | == Socket ==> | net32.c |  == elba_queue ==> | elba_mgr.c |
 * |---------|               |---------|                    |------------|
 *
 *
 * version :
 *		0.0.1  tykim working.
 *		0.0.2  jong2ry code clean.
 *		0.0.3  jong2ry kernel�� TCP�� ����� ���� Driver�� read, write ������� �ٲ��. 
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
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <strings.h>
#include <netdb.h>
#include <sys/time.h>		// gettimeofday()
#include <malloc.h>


////////////////////////////////////////////////////////////////////////////////
//define
#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

#include "net32_mgr.h"															


////////////////////////////////////////////////////////////////////////////////
// extern variable
extern point_queue net32_queue;													// net32 queue
extern pthread_mutex_t net32Q_mutex;											// net32 mutex
extern pthread_mutex_t pointTable_mutex;										// point table mutex

// point and node
extern int g_nMyPcm;															// my pcm number
extern int g_nMultiDdcFlag;														// mode select flag
extern unsigned char g_cNode[MAX_NET32_NUMBER];									// node
extern PTBL_INFO_T *g_pPtbl;													// point table
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];						// extension point table


////////////////////////////////////////////////////////////////////////////////
// global variable
unsigned char dbg_mtr_niq = 0;													// niq display flag
unsigned char dbg_mtr_noq = 0;													// noq display flag
int g_nViewNet32 = 0;															// debug���� ����� �� ����ϴ� flag
NET32_STATUS_T	g_net32_status;													// log net32 status


typedef struct {
	short pcm;
	short pno;
	float value;
	unsigned int type;
	unsigned char msg[12];
} net32_data;


void net32_push_queue(point_info *p)
// ----------------------------------------------------------------------------
// PUSH DATA IN NET32_QUEUE
// Description : net32_queue�� data�� �߰��Ѵ�.
// Arguments   : p					Is a point-value pointer.
// Returns     : none
{
	pthread_mutex_lock( &net32Q_mutex );
	putq( &net32_queue, p );
	pthread_mutex_unlock( &net32Q_mutex );
}


int net32_pop_queue(point_info *p)
// ----------------------------------------------------------------------------
// POP DATA IN NET32_QUEUE
// Description : net32_queue�� data�� �����´�.
// Arguments   : p					Is a point-value pointer.
// Returns     : n_ret				Is a queue status type.
{
	int n_ret;

	memset( p, 0, sizeof(point_info) );
	pthread_mutex_lock( &net32Q_mutex );
	n_ret = getq( &net32_queue, p );
	pthread_mutex_unlock( &net32Q_mutex );
	
	return n_ret;
}


void net32_send_message(NET32_T *p)
// ----------------------------------------------------------------------------
// MESSAGE SEND TO KERNEL
// Description : Data get from Net32_Queue. And message send to kernel.
// Arguments   : nSockFd			Is a client socket 
//				 p					Is a pointer of the NET32_T structure.
// Returns     : always 1
{
	int i = 0;
	int result = 0;
	int nRtnSend = 0;
	point_info point;
	unsigned char *pVal = (unsigned char *)&point.value;
	NET32_T *pNet32 = (NET32_T *)p;	
	NET32_MSG_T *pMsg = (NET32_MSG_T *)pNet32->pTxBuf;

	memset(&point, 0, sizeof(point));
		
	// pop net32-queue
	result = net32_pop_queue( &point );

	// queue�� message�� �ִ��� Ȯ���Ѵ�. 
	if(result != SUCCESS) {
		return;
	}

	// net32 status�� noq���� ������Ų��. 
	g_net32_status.nCountNoq++;

	// net32 queue�� data�� type ���� message�� cmd�� �����Ѵ�. 
	pMsg->cHeader = 0xff;
	if (point.message_type == NET32_TYPE_REPORT) {
		pMsg->cCmd = HANDLER_MSG_REPORT;
	} 
	else if (point.message_type == NET32_TYPE_COMMAND) {
		pMsg->cCmd = HANDLER_MSG_COMMAND;
	} 
	else if (point.message_type == NET32_TYPE_REQUIRE) {
		pMsg->cCmd = HANDLER_MSG_REQUIRE;
	}
	else { 
		pMsg->cCmd = HANDLER_MSG_REQUIRE;
	}		

	// message�� �ϼ��Ѵ�. 
	// reportr, command, require message
	pMsg->cLength = sizeof(NET32_MSG_T);
	pMsg->cDbgNoq = dbg_mtr_noq; 
	pMsg->cDbgNiq = dbg_mtr_niq;
	pMsg->cPcm = point.pcm;
	pMsg->cPno = point.pno;
	pMsg->cVal[0] = pVal[3];
	pMsg->cVal[1] = pVal[2];
	pMsg->cVal[2] = pVal[1];
	pMsg->cVal[3] = pVal[0];
	pMsg->cChkSum = 0;
	for (i = 0; i < pMsg->cLength - 1; i++) 
		pMsg->cChkSum -= pNet32->pTxBuf[i];
	
	// send to driver
	nRtnSend = write( pNet32->nFd, (unsigned char *)pMsg, sizeof(NET32_MSG_T) );

	// debug message
	if ( dbg_mtr_noq ) {
		fprintf(stdout, "\nNOQ: ");
		fflush(stdout);			
		for ( i = 0; i < pMsg->cLength ; i++) {
			fprintf(stdout, "%02X ", pNet32->pTxBuf[i]);
			fflush(stdout);			
		}
		fprintf(stderr, " -> %.2f", point.value);		
		fflush(stdout);			
	}
			
	return ;
}


int net32_recv_message (NET32_T *p)
// ----------------------------------------------------------------------------
// MESSAGE RECEIVE FROM DRIVER
{
	int nRecvBytes = 0;

	nRecvBytes = read( p->nFd, p->pRxBuf, sizeof(net32_data) );

	return nRecvBytes;	
}


void net32_data_handler(void *p) 
// ----------------------------------------------------------------------------
// PARSING A TO RECEIVE MESSAGE 
// Description : Is parsing message.
// Arguments   : p					Is a pointer of the NET32_T structure.
// Returns     : none
{
	int i = 0;
	//int nNodePcm = 0;
	point_info point;
	unsigned char cHandle = 0;
	unsigned char cChksum = 0;
	unsigned char *pVal;
	
	NET32_T *pNet32 = (NET32_T *)p;	
	pVal = (unsigned char *) &point.value;
	
	cHandle = pNet32->pRxBuf[1];

	cChksum = 0;
	for (i = 0; i < (pNet32->pRxBuf[2] + 1); i++)  {
		cChksum += pNet32->pRxBuf[i];
	}

	if ( cChksum == 0 ) {
		switch(cHandle) {
			// progress report process
			// Report = | 0xff | 'R' | 0x0C(Length) | x | x |
			// 			| pcm | pno | fval[3] | fval[2] | fval[1] | fval[0] | ChkSum |
			case 0x20:
				// net32 status�� niq���� ������Ų��. 
				g_net32_status.nCountNiq++;
				
				point.pcm = pNet32->pRxBuf[0];
				point.pno = pNet32->pRxBuf[4];
				pVal[3] = pNet32->pRxBuf[5];
				pVal[2] = pNet32->pRxBuf[6];
				pVal[1] = pNet32->pRxBuf[7];
				pVal[0] = pNet32->pRxBuf[8];

				if ( dbg_mtr_niq ) {						
					fprintf(stderr, " -> %.2f", point.value);
					fflush(stdout);			
				}
				
				// �ڽ��� ���� Report�� ��쿡�� 
				// �̹� point-table�� ������Ʈ�� �Ǿ� �ִ� �����̴�. 
				// �׷��� ������ �ڽ��� ���� �ƴ� ��쿡�� net32 queue�� �ִ´�. 
				if (g_nMyPcm != point.pcm) {
					// �ٸ� pcm�� point table�� update�ϴ� ���� �����Ѵ�. -- !!
					pthread_mutex_lock( &pointTable_mutex );
					g_fExPtbl[point.pcm][point.pno] = point.value;
					pthread_mutex_unlock( &pointTable_mutex );
					point.message_type = NET32_TYPE_REPORT;
					elba_push_queue( &point );											// data push elba_queue. 
					uclient_push_queue(&point);
				}
				else {}
				break;
			
			// progress command process
			// Command = | 0xff | 'C' | 0x0C(Length) | x | x |
			// 			 | pcm | pno | fval[3] | fval[2] | fval[1] | fval[0] | ChkSum |			
			case 0x40:
				// net32 status�� niq���� ������Ų��. 
				g_net32_status.nCountNiq++;
				
				point.pcm = pNet32->pRxBuf[0];
				point.pno = pNet32->pRxBuf[4];
				pVal[3] = pNet32->pRxBuf[5];
				pVal[2] = pNet32->pRxBuf[6];
				pVal[1] = pNet32->pRxBuf[7];
				pVal[0] = pNet32->pRxBuf[8];
				
				if ( dbg_mtr_niq ) {						
					fprintf(stderr, " -> %.2f", point.value);
					fflush(stdout);			
				}		

				// �ڽ��� ���� ��쿡�� net32 queue�� �ִ´�. 
				if (g_nMyPcm == point.pcm) {
					if ( point.value == g_pPtbl[point.pno].f_val ) {
						point.message_type = NET32_TYPE_REPORT;
						net32_push_queue(&point);						
					}
					else {
						pnt_local_pset( &point );
					}
				}
				else {
					point.message_type = NET32_TYPE_REPORT;
					uclient_push_queue(&point);		
				}
				break;
			
			// progress require process
			// | 0xff | 'C' | 0x0C(Length) | x | x |
			case 0x00:
				// net32 status�� niq���� ������Ų��. 
				g_net32_status.nCountNiq++;
						
				point.pcm = pNet32->pRxBuf[0];
				point.pno = pNet32->pRxBuf[4];

				// �ڽ��� ���� ��쿡�� net32 queue�� �ִ´�. 
				if (g_nMyPcm == point.pcm) {
					pthread_mutex_lock( &pointTable_mutex );
					point.value = g_fExPtbl[point.pcm][point.pno];
					pthread_mutex_unlock( &pointTable_mutex );
					point.message_type = NET32_TYPE_REPORT;
					net32_push_queue( &point );					
					uclient_push_queue(&point);				
				}
				else {}			
				break;	
			/*
			// progress node process
			case HANDLER_MSG_NODE:
				// net32 status�� node count���� ������Ų��. 
				g_net32_status.nCountNode++;
				nNodePcm = pNet32->pRxBuf[3];
				g_cNode[nNodePcm] = pNet32->pRxBuf[4];			
				break;
			*/
		}
	}
	else {}
}


NET32_T *net32_init(void) 
// ----------------------------------------------------------------------------
// CREATE A NET32_T STRUTURE 
// Description : Is create NET32_T structure.
// Arguments   : none
// Returns     : pNet32				pointer of the NET32_T structure.
{
	NET32_T *pNet32;
	
	pNet32 = (NET32_T *)malloc( sizeof(NET32_T) );
	memset( pNet32, 0x00, sizeof(NET32_T) );

	pNet32->pRxBuf = (unsigned char *)malloc (NET32_BUF_SIZE);
	pNet32->pTxBuf = (unsigned char *)malloc (NET32_BUF_SIZE);

	memset( pNet32->pRxBuf, 0x00, NET32_BUF_SIZE );
	memset( pNet32->pTxBuf, 0x00, NET32_BUF_SIZE );
	
	return pNet32;
}


int net32_open( NET32_T *p ) 
// ----------------------------------------------------------------------------
// OPEN THE NET32_T STRUTURE 
// Description : Is create socket that comunication from KERNEL.
// Arguments   : p					pointer of the NET32_T structure.
// Returns     : fd					socket fd of the NET32_T structure.
//				 -1					socket error or open error
{
	p->nFd = open( "/dev/s3c2410_net32", O_RDWR );

	memset( p->pRxBuf, 0x00, NET32_BUF_SIZE );
	memset( p->pRxBuf, 0x00, NET32_BUF_SIZE );
	
	return p->nFd;
}


void net32_save_file(void) 
// ----------------------------------------------------------------------------
// SAVE NET32 STATUS FILE
// Description : Is save file of net32 status
// Arguments   : none
// Returns     : none
{
	FILE *fp = NULL;
	int nFront = 0;
	int nRear = 0;
	
	fp = fopen("/duksan/FILE/net32", "w");
	if( fp == NULL ) {
		return;		
	}	

	fprintf(fp,	"Net32 Count\n" );
	fprintf(fp,	"+ Node : %d,    Niq : %d,    Noq : %d\n", 
		g_net32_status.nCountNode,
		g_net32_status.nCountNiq,
		g_net32_status.nCountNoq );

	pthread_mutex_lock( &net32Q_mutex );
	nRear = net32_queue.rear;	
	nFront = net32_queue.front;	
	pthread_mutex_unlock( &net32Q_mutex );

	fprintf(fp,	"+ Net32 Message Queue :  %d / %d \n", 
		nFront,nRear);		

	fprintf(fp,	"\n" );
	fclose(fp);
	
	g_net32_status.nCountNiq = 0;
	g_net32_status.nCountNoq  = 0;	
}


void *net32_main(void *arg)
// ----------------------------------------------------------------------------
// NET32 THREAD MANAGER
// Description	: This function is called by main().
// 					- create NET32_T structure
//					- data that getting Net32_Queue send to KERNEL.
// 					- data receive from KERNEL and insert Net32_Queue.
// Arguments   : arg				Thread Argument
{
	int i = 0;
	unsigned char *pchTemp;
	NET32_T *pNet32;
	struct timespec ts;


	// net32 initialize 
	pNet32 = net32_init();
	
	pchTemp = (unsigned char *) malloc (2048);
	if ( pchTemp == NULL )  {
		system("sync");
		system("sync");
		system("sync");
		system("killall duksan");			
	}
	
	// Initialize NET32-Status variable
	g_net32_status.nCountNode = 0;
	g_net32_status.nCountNiq = 0;
	g_net32_status.nCountNoq = 0;
	net32_save_file();
	
	// net32_socket open
	if ( net32_open(pNet32) < 0 ) {
		fprintf( stdout, "[NET32] Open Error.\n");
		fflush( stdout );
		
		// write error log
		syslog_record(SYSLOG_NET32_OPEN_ERROR);
		
		// free buffer
		free(pchTemp);
		free(pNet32->pRxBuf);
		free(pNet32->pTxBuf);
		
		// kill application
		sleep(2);
		system("sync");
		system("sync");
		system("sync");
		system("killall duksan");			
	}		
	
	// While Loop
	while(1) { 
		// ���� �޽����� �ִٸ� Net32 Driver�� �����մϴ�. 
		net32_send_message(pNet32);		
		
		// wait delay
		// net32���� token�� data�� �ְ� �޴� �ð��� �� 6msec �����̴�. 
		// �׷��Ƿ� �� 4msec���� net32 driver�� ����Ͽ�,
		// ���ʿ��� ��ŷ��� ���̵��� �Ѵ�.
		ts.tv_sec = 0;
		ts.tv_nsec = 3 * M_SEC;				
		nanosleep(&ts, NULL);	

		// receive message
		// �޾ƿ��� data�� net32�� message�� �״�� �޾ƿ��� �ȴ�. 
		pNet32->nRecvByte = net32_recv_message(pNet32);		

		if ( pNet32->nRecvByte > 0 ) {
			
			// debug message
			if ( dbg_mtr_niq ) {
				fprintf(stdout, "\nNIQ: ");
				fflush(stdout);			
				for ( i = 0; i < pNet32->nRecvByte; i++) {
					fprintf(stdout, "%02X ", pNet32->pRxBuf[i]);
					fflush(stdout);			
				}
			}
					
			// message handler
			net32_data_handler(pNet32);
			
			// debug message
			if ( dbg_mtr_niq ) {
				fflush(stdout);			
			}
		}

		continue;
	} 
	
	free(pNet32);
	free(pchTemp);
	free(pNet32->pRxBuf);
	free(pNet32->pTxBuf);

	syslog_record(SYSLOG_DESTROY_NET32);
}

