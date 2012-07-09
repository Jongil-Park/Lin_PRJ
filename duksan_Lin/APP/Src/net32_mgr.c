/*
 * file 		: net32_mgr.c
 * creator   	: tykim
 *
 *
 * kernel과 TCP 통신을 통해 Net32 Driver의 data를 얻어온다. 
 * 다른 Thread로부터 net32_push_queue 통해 point data를 받는다.
 * 받은 data는 Send-Socket을 통해 kernel로 보낸다.
 * |---------------|                    |---------|                |--------|
 * | other Thread  | == net32_queue ==> | net32.c |  == Socket ==> | Kernel |
 * |---------------|                    |---------|                |--------|
 *
 *
 * kernel로부터 Receive-Socket을 통해 받은 point data를 
 * 직접 처리하고 Elba로 보내야 할 data는 elba_put_queue를 통해 elba thread로 보내다. 
 * |---------|               |---------|                    |------------|
 * | Kernel  | == Socket ==> | net32.c |  == elba_queue ==> | elba_mgr.c |
 * |---------|               |---------|                    |------------|
 *
 *
 * version :
 *		0.0.1  tykim working.
 *		0.0.2  jong2ry code clean.
 *		0.0.3  jong2ry kernel과 TCP로 통신한 것을 Driver의 read, write 방식으로 바꿨다. 
 *
 *
 * code 작성시 유의사항
 *		1. global 변수는 'g_' 접두사를 쓴다.
 *		2. 변수를 선언할 때에는 아래와 같은 규칙으로 선언한다. 
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
 *		3. 변수명의 첫글자는 대분자로 사용한다. 
 *		4. 변수명에서의 각 글자의 구분은 공백없이 대분자로 한다. 
 *			ex > g_nTest, g_bFlag, g_fPointTable
 *		5. 함수명은 소문자로 '_' 기호를 사용해서 생성한다. 
 *		6. 함수와 함수 사이의 간격은 2줄로 한다. 
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
int g_nViewNet32 = 0;															// debug문을 출력할 때 사용하는 flag
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
// Description : net32_queue에 data를 추가한다.
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
// Description : net32_queue의 data를 꺼내온다.
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

	// queue에 message가 있는지 확인한다. 
	if(result != SUCCESS) {
		return;
	}

	// net32 status의 noq값을 증가시킨다. 
	g_net32_status.nCountNoq++;

	// net32 queue의 data의 type 따라 message의 cmd를 설정한다. 
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

	// message를 완성한다. 
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
				// net32 status의 niq값을 증가시킨다. 
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
				
				// 자신의 값이 Report된 경우에는 
				// 이미 point-table이 업데이트가 되어 있는 상태이다. 
				// 그렇기 때문에 자신의 값이 아닌 경우에만 net32 queue에 넣는다. 
				if (g_nMyPcm != point.pcm) {
					// 다른 pcm의 point table을 update하는 것을 주의한다. -- !!
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
				// net32 status의 niq값을 증가시킨다. 
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

				// 자신의 값인 경우에만 net32 queue에 넣는다. 
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
				// net32 status의 niq값을 증가시킨다. 
				g_net32_status.nCountNiq++;
						
				point.pcm = pNet32->pRxBuf[0];
				point.pno = pNet32->pRxBuf[4];

				// 자신의 값인 경우에만 net32 queue에 넣는다. 
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
				// net32 status의 node count값을 증가시킨다. 
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
		// 보낼 메시지가 있다면 Net32 Driver로 전송합니다. 
		net32_send_message(pNet32);		
		
		// wait delay
		// net32에서 token과 data를 주고 받는 시간은 약 6msec 정도이다. 
		// 그러므로 약 4msec마다 net32 driver와 통신하여,
		// 불필요한 통신량을 줄이도록 한다.
		ts.tv_sec = 0;
		ts.tv_nsec = 3 * M_SEC;				
		nanosleep(&ts, NULL);	

		// receive message
		// 받아오는 data는 net32의 message를 그대로 받아오게 된다. 
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

