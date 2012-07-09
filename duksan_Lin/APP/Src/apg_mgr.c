/* file : apg_mgr.c
 * author : jong2ry@imecasys.com
 * 
 * 
 * User가 작성한 APG Application과 Data를 주고 받는다. 
 * APG_handler는 Select를 사용해서 구현하였다. 
 * Message Format은 다음과 같다.  
 *  
 *  
 *  
 *  startup request format 
 *  
 *  | length (2byte) | cmd('I') | chksum (1byte) |
 *  
 *  
 *  
 *  startup response format 
 *  
 *  | length (2byte) | my_id | chksum (1byte) |
 *  
 *	  
 *  
 *  pset request / response format
 *  pget request / response format
 *  
 *  | length (2byte) | cmd('S' or 'G') | pcm (1byte) | pno (2byte) | value (4byte) | .... | 0x00 (1byte) | chksum (1byte) |
 *  
 *  <-- header --->   <--------------------------- data -------------------------->        <-------- chksum-------------->
 *  
 *  data 구간이 APG Application에서 요청하는 것 만큼 반복된다.
 * 
 *  
 *  
 *  pdef request format 
 *  
 *  | length (2byte) |
 *  
 * 	<-- header --->
 *  
 * 	|cmd('D') | pcm (1byte) | pno (2byte) | hist (4byte) | scale (4byte) | offset (4byte) | min (4byte) | max (4byte) |
 *  
 *   <------------------------------------------------ data ------------------------------------------------------> 
 *  
 *  | 0x00 (1byte) | chksum (1byte) |
 *  
 *  <-------- chksum-------------->
 *  
 *  
 *  
 *  pdef response format
 *  
 *  | length (2byte) | cmd('G') | pcm (1byte) | pno (2byte) | value (4byte) | 0x00 (1byte) | chksum (1byte) |
 *  
 *  <-- header --->   <--------------------------- data --------------------------> <-------- chksum-------------->
 *  
 *  
 * 
 *  
 *  
 * version :
 *  	0.0.1 - initiailize
 *  
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/un.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/poll.h>															// use poll event

#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

#include "queue_handler.h"														// queue handler
#include "apg_mgr.h"															// apg handler.	
							

////////////////////////////////////////////////////////////////////////////////
// extern variable
extern point_queue apg_queue;													// apg queue
extern pthread_mutex_t pointTable_mutex;										// point table mutex

// point and node
extern int g_nMyPcm;															// my pcm number
extern int g_nMultiDdcFlag;														// mode select flag
extern unsigned char g_cNode[MAX_NET32_NUMBER];									// node
extern PTBL_INFO_T *g_pPtbl;													// point table
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];						// extension point table


APG_T *apg_init(void)
// ----------------------------------------------------------------------------
// MALLOC APT_T STRUCTURE
// Description : APG Structure가 사용할 메모리 공간을 확보한다. 
// Arguments   : none
// Returns     : none
{
	APG_T *pApg;
	
	pApg = (APG_T *)malloc( sizeof(APG_T) );
	memset( (unsigned char *)pApg, 0, sizeof(APG_T) );

	pApg->nFd = -1;
	
	pApg->chRxBuf =  (unsigned char *)malloc (MAX_BUFFER_SIZE);
	pApg->chTxBuf = (unsigned char *)malloc (MAX_BUFFER_SIZE);
	pApg->chPrevBuf = (unsigned char *)malloc (MAX_BUFFER_SIZE);	
	
	memset( pApg->chRxBuf , 0x00, MAX_BUFFER_SIZE );
	memset( pApg->chTxBuf, 0x00, MAX_BUFFER_SIZE );	
	memset( pApg->chPrevBuf, 0x00, MAX_BUFFER_SIZE );	
	
	pApg->nTempWp = 0;
	pApg->nRecvByte = 0;
	pApg->nIndexPrev = 0;

	return pApg;
}


int apg_open(APG_T *p)
// ----------------------------------------------------------------------------
// OPEN FILE SOCKET
// Description : file socket을 open한다. 
// Arguments   : p							Is pointer of APT_T
// Returns     : p->svr_sock				server socket fd
{
	struct sockaddr_in server_addr;
	struct timeval timeo;
	int one = 1;

	// initialize		
	memset( &server_addr, 0, sizeof(server_addr) );
	timeo.tv_sec = 0;
	timeo.tv_usec = 10000;

	// create socket 	
	p->nFd = socket( AF_INET, SOCK_STREAM, 0 );
	if( p->nFd < 0 ) {
		fprintf( stdout, "[AGP] Socket creation error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;					
	}

	// initialize socket 	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons( APG_SVR_PORT );

	// bind error가 발생하는 것을 방지하기 위함.
    if (setsockopt( p->nFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
		fprintf( stdout, "[APG] Socket SO_REUSEADDR error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;	

    }
    
	// bind
	if ( bind ( p->nFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ) {
		fprintf( stdout, "[APG] Socket Bind error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;	
	}

	// listen
	if ( listen (p->nFd, 5) ) {
		fprintf( stdout, "[APG] Socket Listen error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;
	}    
	
	return p->nFd;		
}

int apg_recv_message (int nSockFd, unsigned char *pchBuf)
// ----------------------------------------------------------------------------
// MESSAGE RECEIVE FROM KERNEL
// Description : Data get from kernel socket. 
// Arguments   : nSockFd			Is a client socket 
//				 p					Is a pointer receive buffer.
// Returns     : nRecvBytes			Is a count of byte to receive.
{
	int nRecvBytes = 0;

	nRecvBytes = read(nSockFd, pchBuf, MAX_BUFFER_SIZE - 1);
	
	return nRecvBytes;
}


void apg_send_ack_message(int nSocket, APG_T *p)
{
	int i = 0;
	char chChkSum = 0x00;
	LIB_ACK_MSG_T *pTxAck = NULL;
	
	pTxAck = (LIB_ACK_MSG_T *)p->chTxBuf;

	pTxAck->chStx = '<';
	pTxAck->wLength = sizeof(LIB_ACK_MSG_T);
	pTxAck->wCmd = LIB_ACK_MSG_TYPE;		
	chChkSum = 0;
	for (i = 0; i < pTxAck->wLength; i++)
		chChkSum += p->chTxBuf[i];	
	pTxAck->wChkSum = chChkSum;			
	pTxAck->chEtx = '>';	

	// send message
	send(nSocket, p->chTxBuf, pTxAck->wLength, 0);	
}


void apg_send_fail_message(int nSocket, APG_T *p)
{
	int i = 0;
	char chChkSum = 0x00;
	LIB_FAIL_MSG_T *pTxFail = NULL;

	pTxFail = (LIB_FAIL_MSG_T *)p->chTxBuf;

	pTxFail->chStx = '<';
	pTxFail->wLength = sizeof(LIB_FAIL_MSG_T);
	pTxFail->wCmd = LIB_FAIL_MSG_TYPE;		
	chChkSum = 0;
	for (i = 0; i < pTxFail->wLength; i++)
		chChkSum += p->chTxBuf[i];	
	pTxFail->wChkSum = chChkSum;			
	pTxFail->chEtx = '>';	

	// send message
	send(nSocket, p->chTxBuf, pTxFail->wLength, 0);	
}


void apg_send_info_message(int nSocket, APG_T *p)
{
	int i = 0;
	char chChkSum = 0x00;
	LIB_INFO_MSG_T *pTxInfo = NULL;

	pTxInfo = (LIB_INFO_MSG_T *)p->chTxBuf;

	pTxInfo->chStx = '<';
	pTxInfo->wLength = sizeof(LIB_INFO_MSG_T);
	pTxInfo->wCmd = LIB_INFO_MSG_TYPE;	
	pTxInfo->wPcm = g_nMyPcm;	
	chChkSum = 0;
	for (i = 0; i < pTxInfo->wLength; i++)
		chChkSum += p->chTxBuf[i];	
	pTxInfo->wChkSum = chChkSum;			
	pTxInfo->chEtx = '>';	

	// send message
	send(nSocket, p->chTxBuf, pTxInfo->wLength, 0);	
}


void apg_parsing(int nSocket, APG_T *p, unsigned char *pBuf)
{
	char nMsgType = 0x00;
	LIB_PCM_SET_MSG_T *pRxSet = NULL;
	LIB_PCM_DEF_MSG_T *pRxDef = NULL;
	
	//fprintf( stdout, "[APG] Parsing\n" );
	//fflush(stdout);		
	
	nMsgType = pBuf[3];
	switch(nMsgType){
		// APG로부터 온 set message를 처리합니다. 
		case LIB_SET_MSG_TYPE:
			// get pointer
			pRxSet = (LIB_PCM_SET_MSG_T *)pBuf;

			fprintf( stdout, "[APG] %d %d %f\n" , pRxSet->wPcm, pRxSet->wPno, pRxSet->fValue);
			fflush(stdout);	
			
			// check pcm number
			// if wrong message, send fail message			
			if ( pRxSet->wPcm >= 32 ) {
				apg_send_fail_message(nSocket, p);
				return;
			}

			// check pno number
			// if wrong message, send fail message
			if ( pRxSet->wPno >= 256 ) {
				apg_send_fail_message(nSocket, p);
				return;
			}
			
			// set value 			
			pSet(pRxSet->wPcm, pRxSet->wPno, pRxSet->fValue);
			
			// send ack message
			apg_send_ack_message(nSocket, p);				
			break;
	
		// APG로부터 온 def message를 처리합니다. 
		case LIB_DEF_MSG_TYPE:
			// get pointer
			pRxDef = (LIB_PCM_DEF_MSG_T *)pBuf;
			
			// get pcm number
			if ( pRxDef->wPcm >= 32 ) {
				apg_send_ack_message(nSocket, p);
				return;
			}
			
			// send ack message
			apg_send_ack_message(nSocket, p);		
			break;
		
		// APG로부터 온 information message를 처리합니다.
		case LIB_INFO_MSG_TYPE:
			apg_send_info_message(nSocket, p);		
			break;
			
		default:
			return;
	}
}

void apg_handler(int nSocket, APG_T *p, unsigned char *pchTemp)
{
	int nRecvByte = 0;
	int nOffset = 0;
	int nIndex = 0;
	int nMsgLength = 0;
	
	nRecvByte = p->nRecvByte;

	//fprintf( stdout, "[APG] nRecvByte = %d\n" , nRecvByte);
	//fflush(stdout);	

	// 이전 buffer에 값이 들어있다면, 
	// 이전 buffer를 참조해서 Packet을 만든다. 
	if ( p->nIndexPrev > 0  ) {
		memcpy(pchTemp, p->chRxBuf, nRecvByte);
		memcpy(p->chRxBuf, p->chPrevBuf, p->nIndexPrev);
		memcpy(&p->chRxBuf[p->nIndexPrev], pchTemp, nRecvByte);
		nRecvByte += p->nIndexPrev;
		p->nIndexPrev = 0;
	}	

	while (nOffset < nRecvByte)
	{
		//fprintf( stdout, "[APG] %d / %d / %x\n" , nRecvByte, nOffset, p->chRxBuf[nOffset]);
		//fflush(stdout);	
		
		if ( p->chRxBuf[nOffset] != '<' ) {
			nOffset++;
		}
		
		// calculate message length
		/*
		if ( p->chRxBuf[nOffset + 3] == LIB_REQ_MSG_TYPE) {
			nMsgLength = sizeof(LIB_PCM_REQ_MSG_T);
		}
		*/
		if ( p->chRxBuf[nOffset + 3] == LIB_SET_MSG_TYPE) {
			nMsgLength = sizeof(LIB_PCM_SET_MSG_T);
		}		
		else if ( p->chRxBuf[nOffset + 3] == LIB_INFO_MSG_TYPE) {
			nMsgLength = sizeof(LIB_INFO_MSG_T);
		}						
		else {
			// else인 조건에는 항상 최소한의 길이가 주어져야 한다. 	
			nMsgLength = sizeof(LIB_ACK_MSG_T);
		}
		
		//fprintf( stdout, "[APG] nMsgLength = %d\n" , nMsgLength);
		//fflush(stdout);	

		nIndex = nOffset + nMsgLength;	
		if (nIndex <= nRecvByte) {
			// parsing message
			/*
			if ( p->chRxBuf[nOffset + 3] == LIB_REQ_MSG_TYPE) {
				apg_parsing(nSocket, p, &p->chRxBuf[nOffset]);
			}
			*/
			if ( p->chRxBuf[nOffset + 3] == LIB_SET_MSG_TYPE) {
				apg_parsing(nSocket, p, &p->chRxBuf[nOffset]);
			}			
			else if ( p->chRxBuf[nOffset + 3] == LIB_INFO_MSG_TYPE) {
				apg_parsing(nSocket, p, &p->chRxBuf[nOffset]);
			}				
		}
		else {
			p->nIndexPrev = nRecvByte - nOffset;
			memset(p->chPrevBuf, 0x00, sizeof(p->chPrevBuf));
			memcpy(p->chPrevBuf, &p->chRxBuf[nOffset], p->nIndexPrev);
			break;
		}
		nOffset = nIndex;
	}
	
	// protection code.
	if ( p->nIndexPrev < 0 ) {
		p->nIndexPrev = 0;
	}	
}


void *apg_handler_main(void)
// ----------------------------------------------------------------------------
// APG HANDLER MAIN LOOP
// Description : apg handler main loop
// Arguments   : none
// Returns     : none
{
	APG_T *p_apg;
	unsigned char *pchTemp;
	struct timespec ts;

	int nClientFd = -1;
	int nClientLength = 0;
	struct sockaddr_in client_addr;	
	

	p_apg = apg_init();
	if (p_apg == NULL) {
		fprintf( stdout, "Can't create APG Memory space.\n" );
		fflush(stdout);
		system("sync");
		system("sync");
		system("sync");		
		system("killall duksan");
	}
	
	// open apg management socket
	if ( apg_open(p_apg) < 0 ) {
		// kill application
		sleep(3);
		system("sync");
		system("sync");
		system("sync");
		system("killall duksan");
	}	
	
	// init variable
	pchTemp = (unsigned char *)malloc (MAX_BUFFER_SIZE);
	memset (pchTemp, 0x00,  MAX_BUFFER_SIZE);	
		
	while (1) {
		// ready to accept socket
		nClientFd = accept(p_apg->nFd, (struct sockaddr *)&client_addr, &nClientLength); 
		fprintf( stdout, "[APG] Socket Accept OK.\n");
		fflush( stdout );		
		
		// check accept error
		if ( nClientFd == -1 ) {
			fprintf( stdout, "[APG] Accept Error.\n");
			fflush( stdout );

			// kill application
			sleep(3);
			system("sync");
			system("sync");
			system("sync");
			system("killall duksan");
		}
		
		for (;;) {
			// wait delay.
			sched_yield();
			ts.tv_sec = 0;
			ts.tv_nsec = 5 * M_SEC;				
			nanosleep(&ts, NULL);
			
			// receive message
			p_apg->nRecvByte = apg_recv_message(nClientFd, p_apg->chRxBuf);

			//fprintf( stdout, "[APG] p_apg->nRecvByte  = %d\n", p_apg->nRecvByte  );
			//fflush(stdout);	
		
			// parsing message
            if( p_apg->nRecvByte == 0 ) { 
				fprintf( stdout, "[APG] Socket Close.\n");
				fflush( stdout );
				
				// close socket
                close(nClientFd); 				
                break;
            } 
            else if( p_apg->nRecvByte > 0 ) { 
				apg_handler(nClientFd, p_apg, pchTemp);     	
            }
            else 
            	continue;
		}
		continue;
		free(pchTemp);
	}
}




