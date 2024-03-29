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
#include <sys/time.h>
#include <time.h>
#include <sys/poll.h>										// use poll event

#include "define.h"
#include "queue_handler.h"									// queue handler
#include "FUNCTION.h"
#include "elba_mgr.h"										// elba manager
#include "iface_ccms.h"										// interface ccms



CCMS_T				*pCcms;
//CCMS_PTBL_LIST_T	*g_pPtbl;

extern point_queue ccms_queue;	
extern pthread_mutex_t ccmsQ_mutex;
extern pthread_mutex_t pointTable_mutex;									// point table mutex

extern unsigned char g_cNode[32];
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];
extern int g_nMyPcm;

//unsigned char g_cNodeCCMS[32];


int g_nPnoTable[8] = {0,32,64,96,128,160,192,224};


void ccms_put_queue(point_info *pQueue)
// ----------------------------------------------------------------------------
// INSERT A DATA INTO THE CCMS_QUEUE
// Description : ccms_queue에 data를 추가한다.
// Arguments   : p					Is a point-value pointer.
// Returns     : none
{
	/*
	if ( pQueue->message_type != 1 )
		return;
	*/
	pthread_mutex_lock( &ccmsQ_mutex );
	putq( &ccms_queue, pQueue );
	pthread_mutex_unlock( &ccmsQ_mutex );
}


int ccms_get_queue(CCMS_T *p, int nClientFd, point_info *pQueue)
// ----------------------------------------------------------------------------
// DATA GET CCMS_QUEUE
// Description : ccms_queue에서 data를 꺼내온다.
// Arguments   : p					Is a point-value pointer.
// Returns     : n_ret				Is a queue status type.
{
	int i = 0;
	int n_ret;

	memset( pQueue, 0, sizeof(point_info) );

	pthread_mutex_lock( &ccmsQ_mutex );
	n_ret = getq( &ccms_queue, pQueue );
	pthread_mutex_unlock( &ccmsQ_mutex ); 
	
	if ( n_ret == SUCCESS ) {
		for ( i = 0; i < CCMS_MAX_CLIENT; i++ ) {
			// 접속한 Client만 처리하도록 한다. 
			if ( pQueue->message_type == p->nClientId[i] ) {
				printf("q : %d, %d\n",p->nClientStatus[i],  p->nClientId[i] );
				printf("q : %d, %d\n",i, nClientFd );
				// 접속하였지만, 자신의 Data가 맞으면 처리.
				// 자신의 Data가 아니라면 Queue에 다시 넣는다. 
				if ( p->nClientStatus[i] == nClientFd ) {
					return n_ret;
				}
				else {
					ccms_put_queue(pQueue);
					n_ret = QUEUE_EMPTY;
					return n_ret;
				}
					
			}
		}

		//fprintf( stdout, "+ CCMS Queue data ignore.\n" );
		//fflush( stdout );			
	
		n_ret = QUEUE_EMPTY;
		return n_ret;
	}

	return n_ret;
}


void ccms_sleep(int sec, int msec) 
// ----------------------------------------------------------------------------
// WAIT TIMER
// Description : use select function for timer.
// Arguments   : sec		Is a second value.
//				 usec		Is a micro-second value. 
// Returns     : none
{
    fd_set reads, temps;
    struct timeval tv;
    
    FD_ZERO(&reads);
    FD_SET(0, &reads);
    
    tv.tv_sec = sec;
    tv.tv_usec = 1000 * msec;
    temps = reads;
    
    select( 0, &temps, 0, 0, &tv );
    
	return;
}


CCMS_T *new_ccms(void)
// ----------------------------------------------------------------------------
// CREATE THE CCMS_T STRUTURE 
// Description		: This function is called by ccms_main().
// 					  - create CCMS_T structure
// 					  - initialize variable of CCMS_T structure
// Arguments		: none
// Returns			: sock		Is pointer of CCMS_T structure
{
	CCMS_T *p;
	
	p = (CCMS_T *)malloc( sizeof(CCMS_T) );
	
	memset ( p, 0x00, sizeof(CCMS_T) );
	p->nFd = -1;

	p->nServerId = 0;

	//printf("CCMS_PTBL_LIST_T = %d byte\n", sizeof(CCMS_PTBL_LIST_T)); 
	//g_pPtbl = (CCMS_PTBL_LIST_T *) malloc( sizeof(CCMS_PTBL_LIST_T) );
	///p->pPtbl = g_pPtbl;

	return p;
}



int ccms_open(CCMS_T *p)
// ----------------------------------------------------------------------------
// OPEN THE CCMS_T Socket
// Description		: This function is called by ccms_main().
// Arguments		: p			Is a pointer to the CCMS_T structure.
// Returns			: 1			If the call was successful
//              	 -1			If not
{
	struct sockaddr_in server_addr;
	struct timeval timeo;
	int one = 1;
		
	memset( &server_addr, 0, sizeof(server_addr) );
	timeo.tv_sec = 0;
	timeo.tv_usec = 10000;
	
	p->nFd = socket( AF_INET, SOCK_STREAM, 0 );
	if( p->nFd < 0 ) {
		fprintf( stdout, "+ CCMS Socket creation error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;					
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons( CCMS_PORT );
	//server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//server_addr.sin_port = htons( 9922 );

    if (setsockopt( p->nFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
		fprintf( stdout, "+ CCMS Socket SO_REUSEADDR error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;	

    }

	if ( bind ( p->nFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ) {
		fprintf( stdout, "+ CCMS Socket Bind error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;	
	}

	if ( listen (p->nFd, 5) ) {
		fprintf( stdout, "+ CCMS Socket Listen error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;
	}

	FD_ZERO(&p->reads);
	FD_SET(p->nFd, &p->reads);
	p->nFdMax = p->nFd;

	return p->nFd;
}


int ccms_chk_fd(CCMS_T *p)
{
	int nFd = 0;
	struct timeval timeout;

	p->temps = p->reads;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	// select function.
	if ( select(p->nFdMax+1, &p->temps, 0, 0, &timeout) == -1 ) {
		fprintf( stdout, "+ CCMS Socket Select error\n" );
		fflush( stdout );
		//exit(1);
		system("killall duksan");
	}

	// check fd.
	for ( nFd = 0; nFd < p->nFdMax + 1; nFd++ ) {
		if ( FD_ISSET(nFd, &p->temps) ) {
			//fprintf( stdout, "+ CCMS Socket Select nFd = %d\n" , nFd);
			//fflush( stdout );
			return nFd;
		}
	}

	return -1;
}


void ccms_open_client(CCMS_T *p, int nClientFd)
// ----------------------------------------------------------------------------
// OPEN CLIENT
// Description		: 접속한  Client에 상태값을 변화시킨다. 
// Arguments		: p					Is a pointer to the CCMS_T structure.
// 					  nClientFd			Client Fd
// Returns			: none
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == 0 ) {
			p->nClientStatus[nIndex] = nClientFd;
			fprintf( stdout, "+ Open client %d, fd = %d\n" , nIndex, nClientFd);
			fflush( stdout );
			return;
		}
	}
}


void ccms_close_client(CCMS_T *p, int nClientFd)
// ----------------------------------------------------------------------------
// CLOSE CLIENT
// Description		: 접속한  Client에 상태값을 초기화 시킨다. 
// Arguments		: p					Is a pointer to the CCMS_T structure.
// 					  nClientFd			Client Fd
// Returns			: none
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == nClientFd ) {
			fprintf( stdout, "+ Close client %d, fd = %d\n" , nIndex, nClientFd);
			fflush( stdout );
			p->nClientStatus[nIndex] = 0;
			p->nClientId[nIndex] = 0;
			return;
		}
	}
}

/*
CCMS_BUFFER_T *ccms_get_buffer(CCMS_T *p, int nClientFd)
// ----------------------------------------------------------------------------
// OPEN BUFFER FOR CLIENT
// Description		: 접속한  Client에 Buffer를 할당한다. 
// Arguments		: p					Is a pointer to the CCMS_T structure.
// 					  nClientFd			Client Fd
// Returns			: buffer pointer 	Is a pointer to the CCMS_BUFFER_T structure.
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == nClientFd ) {
			return &p->RxBuf[nIndex];
		}
	}
	return NULL;
}
*/

/*
void ccms_open_buffer(CCMS_T *p, int nClientFd)	
// ----------------------------------------------------------------------------
// CLOSE BUFFER
// Description		: 접속한  Client에 Buffer를 초기화한다. 
// Arguments		: p					Is a pointer to the CCMS_T structure.
// 					  nClientFd			Client Fd
// Returns			: none
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == nClientFd ) {
			p->RxBuf[nIndex].nIndexPrev = 0;
			fprintf( stdout, "+ Open buffer %d, fd = %d\n" , nIndex, nClientFd);
			fflush( stdout );
			return;
		}
	}
}
*/

/*
void ccms_close_buffer(CCMS_T *p, int nClientFd)	
// ----------------------------------------------------------------------------
// RETURN BUFFER POINTER
// Description		: 접속한  Client가 사용하는 buffer의 pointer를 리턴한다. 
// Arguments		: p					Is a pointer to the CCMS_T structure.
// 					  nClientFd			Client Fd
// Returns			: buffer pointer 	Is a pointer to the CCMS_BUFFER_T structure.
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == nClientFd ) {
			p->RxBuf[nIndex].nIndexPrev = 0;
			fprintf( stdout, "+ Close buffer %d, fd = %d\n" , nIndex, nClientFd);
			fflush( stdout );
			return;
		}
	}
}
*/







void ccms_close(CCMS_T *p, int nClientFd)
// ----------------------------------------------------------------------------
// CLOSE THE CLIENT SOCKET
// Description		: Close client socket.
// Arguments		: p					Is a pointer to the CCMS_T structure.
// 					  nClientFd			Client Fd
// Returns			: none.
{
	FD_CLR(nClientFd, &p->reads);
	close(nClientFd);
	ccms_close_client(p, nClientFd);
	//ccms_close_buffer(p, nClientFd);
	fprintf( stdout, "CCMS Close Client  = %d \n", nClientFd);
	fflush( stdout );
}



void ccms_register_node(CCMS_T *p, int nClientFd, char chId)
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == nClientFd ) {
			p->nClientId[nIndex] = chId;
			p->nAliveCnt[nIndex] = 5;
			return;
		}
	}
}

void ccms_alive_check(CCMS_T *p)
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] !=  0) {
			//printf("Index = %d, p->nAliveCnt[nIndex] = %d\n", nIndex, p->nAliveCnt[nIndex]);
			if ( (p->nAliveCnt[nIndex]--) == 0 ) {
				ccms_close(p,  p->nClientStatus[nIndex]);
			}
			continue;
		}
	}	
}



void ccms_parse_pkt(CCMS_T *p, int nSockFd, unsigned char *pData) 
// ----------------------------------------------------------------------------
// PARSE PACKET
// Description		: 전송받은 Packet을 Parse 한다. 
// Arguments		: nSockFd			Client Socket
// 					  pData				Is a pointer to th Rxbuffer
// Returns			: none.
{
	int i = 0;
	int nType = 0;
	CCMS_PKT_T *pPkt;
	CCMS_PKT_ACK_T *pAck;
	CCMS_PKT_SET_T *pSetT;
	int nRtnSend = 0;
	unsigned char chTxBuf[32];
	point_info point;
	unsigned char chRemoteId = 0;
	unsigned char chPcm = 0;
	unsigned char chPno = 0;
	unsigned short wValue = 0;

	nType = pData[1];

	switch (nType) {
	// Notice Packet.
	case 'N':
		pPkt = (CCMS_PKT_T *)pData;
		if ( pPkt->chStx == '<' && pPkt->chEtx == '>') {

			ccms_register_node(p, nSockFd, pPkt->chId);

			/*
			fprintf( stdout, "Recv Index = %d \n", pPkt->wIndex);
			fprintf( stdout, "Recv Index = %d \n", pPkt->wIndex);
			fprintf( stdout, "Recv Data[0] = %f \n", pPkt->fData[0]);
			fprintf( stdout, "Recv Data[1] = %f \n", pPkt->fData[1]);
			fflush( stdout );
			*/

			for ( i = 0; i < CCMS_PKT_DATA_CNT; i++ ) {
				point.pcm = pPkt->chId;
				point.pno = g_nPnoTable[pPkt->wIndex] + i;
				point.value = pPkt->fData[i];
				
				if (g_fExPtbl[point.pcm][point.pno] == point.value )
					continue;
				
				pthread_mutex_lock( &pointTable_mutex );
				g_fExPtbl[point.pcm][point.pno] = point.value;
				pthread_mutex_unlock( &pointTable_mutex );
				elba_push_queue( &point );					
			}

			memset(chTxBuf, 0x00, sizeof(chTxBuf));
			if ( ccms_get_queue(p, nSockFd, &point) == SUCCESS ) {
				
				chRemoteId = (unsigned char)point.message_type;
				chPcm = (unsigned char)point.pcm;
				chPno = (unsigned char)point.pno;
				wValue = (unsigned short)point.value;

				pSetT = (CCMS_PKT_SET_T *)chTxBuf;
				pSetT->chStx = '<';
				pSetT->chCmd = 'S';
				pSetT->chId = chRemoteId;
				pSetT->chPcm = chPcm;
				pSetT->chPno = chPno;
				pSetT->wData = wValue;
				pSetT->chEtx = '>';

				fprintf( stdout, "Set %d %d %d %f\n", 
						 point.message_type, point.pcm, 
						 point.pno, point.value);
				fflush( stdout );

				nRtnSend = send(nSockFd, pSetT, sizeof(CCMS_PKT_SET_T), 0 );
			}
			else {
				pAck = (CCMS_PKT_ACK_T *)chTxBuf;
				pAck->chStx = '<';
				pAck->chCmd = 'A';
				pAck->chId = pPkt->chId;
				pAck->chEtx = '>';
				
				nRtnSend = send(nSockFd, pAck, sizeof(CCMS_PKT_ACK_T), 0 );
			}
		}
		break;

	case 'S':
		pSetT = (CCMS_PKT_SET_T *)pData;
		if ( pSetT->chStx == '<' && pSetT->chEtx == '>') {
			point.pcm = (short)pSetT->chPcm;
			point.pno = (short)pSetT->chPno;
			point.value = (float)pSetT->wData;
			point.message_type = (int)pSetT->chId;

			fprintf( stdout, "Recv 'S' Command\n");
			fflush( stdout );
							
			fprintf( stdout, "Recv  %d %d %d %f\n", 
					 point.message_type, point.pcm, 
					 point.pno, point.value);
			fflush( stdout );

			if ( point.pcm != g_nMyPcm ) {
				ccms_put_queue(&point);
				fprintf( stdout, "Put Queue\n");
				fflush( stdout );				
			}

			pSet(point.pcm, point.pno, point.value);

			pAck = (CCMS_PKT_ACK_T *)chTxBuf;
			pAck->chStx = '<';
			pAck->chCmd = 'A';
			pAck->chId = pSetT->chId;
			pAck->chEtx = '>';
			
			nRtnSend = send(nSockFd, pAck, sizeof(CCMS_PKT_ACK_T), 0 );
		}
		break;
	}
}


//void ccms_recv_pkt(CCMS_T *p, int nSockFd, CCMS_BUFFER_T *pBuf, int nRecvLength) 
void ccms_recv_pkt(CCMS_T *p, int nSockFd, int nRecvLength) 
// ----------------------------------------------------------------------------
// RECEIVE PACKET
// Description		: 전송받은 Packet을 정리한다.  
// Arguments		: nSockFd			Client Socket
// 					  pData				Is a pointer to th Rxbuffer
// 					  nRecvLength		Client로 부터 전송받은 Packet의 길이
// Returns			: none.
{
	int nIndex = 0;
	int nOffset = 0;
	int nPktLength = 0;
	unsigned char chTemp[CCMS_BUFFER_SIZE];

	// 이전 buffer에 값이 들어있다면, 
	// 이전 buffer를 참조해서 Packet을 만든다. 
	if ( p->nIndexPrev > 0  ) {
		memcpy(chTemp, p->chRxBuf, nRecvLength);
		memcpy(p->chRxBuf, p->chPrevBuf, p->nIndexPrev);
		memcpy(&p->chRxBuf[p->nIndexPrev], chTemp, nRecvLength);
		nRecvLength += p->nIndexPrev;
		p->nIndexPrev = 0;
	}

	while (nOffset < nRecvLength) {
		if ( p->chRxBuf[nOffset] != '<' ) {
			nOffset++;
		}
		
		if ( p->chRxBuf[nOffset + 1] == 'N' ) {
			nPktLength = 136;
		}
		else if ( p->chRxBuf[nOffset + 1] == 'S' ) {
			nPktLength = 4;
		}
		else
			continue;

		nIndex = nOffset + nPktLength;	
		if (nIndex <= nRecvLength) {
			ccms_parse_pkt(p, nSockFd, &p->chRxBuf[nOffset]);
		}
		else {
			p->nIndexPrev = nRecvLength - nOffset;
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

	return;
}


void ccms_chk_client(CCMS_T *p)
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] != 0 ) {
			fprintf( stdout, "> Open client %d, fd = %d, Id = %d \n" , 
					 nIndex, 
					 p->nClientStatus[nIndex],
					 p->nClientId[nIndex] );
			fflush( stdout );
		}
	}
	//fprintf( stdout, "\n");
	//fflush( stdout );
}


void *ccms_main(void* arg)
{
	int nChkFd = 0;
	int nClientFd = -1;
	int nClientLength = 0;
	int nRetRecv = 0;
	struct sockaddr_in client_addr;
	unsigned char chTemp[1024];
	//CCMS_BUFFER_T *pRxBuf;
	//int nLoopCnt = 0;


	signal(SIGPIPE, SIG_IGN);	// Ignore broken_pipe signal
	
	//ccms_sleep(1,0);

	pCcms = new_ccms();
	if ( pCcms == NULL ) {
		fprintf( stdout, "Can't create CCMS Interface.\n" );
		fflush( stdout );
		//exit(1);
		system("killall duksan");
	}

	while (1) {
		ccms_open(pCcms);

		//ccms_chk_client(pCcms);		

		for (;;) {
			//printf("CCMS Loop \n"); 
			
			ccms_alive_check(pCcms);

			nChkFd = ccms_chk_fd(pCcms);
			if ( nChkFd < 0 ) {
				continue;
			}

			// 연결 요청인 경우 처리 
			if ( nChkFd == pCcms->nFd ) {
				nClientLength = sizeof(client_addr);
				nClientFd = accept( pCcms->nFd, 
									(struct sockaddr *)&client_addr,
								    &nClientLength );
				FD_SET(nClientFd, &pCcms->reads);

				if ( pCcms->nFdMax < nClientFd ) 
					pCcms->nFdMax = nClientFd;

				fprintf( stdout, "CCMS Accept client nClientFd = %d\n", nClientFd );
				fflush( stdout );
				
				// open structure
				ccms_open_client(pCcms, nClientFd);
			}
			else  {
				nRetRecv = read(nChkFd, chTemp, sizeof(chTemp));
				
				if ( nRetRecv <= 0 ) {
					fprintf( stdout, "CCMS nChkFd = %d  nRetRecv = %d \n", nChkFd, nRetRecv);
					fflush( stdout );
				}

				// client close
				if ( nRetRecv == 0 ) {
					ccms_close(pCcms, nChkFd);
					continue;
				}

				// client error
				if ( nRetRecv < 0 ) {
					continue;
				}
				
				/*
				// get rx buffer strcuture
				pRxBuf = ccms_get_buffer(pCcms, nChkFd);
				if ( pRxBuf == NULL ) {
					fprintf( stdout, "CCMS Buffer Error \n");
					ccms_close(pCcms, nChkFd);
					continue;
				}
				*/
					
				//pRxBuf = (unsigned char) pCcms->chRxBuf;
				memcpy(pCcms->chRxBuf, 
					   chTemp,
					   nRetRecv);
				ccms_recv_pkt(pCcms, nChkFd, nRetRecv);
			}
		} // for (;;)
	} // while(1)
	syslog_record(SYSLOG_DESTROY_CCMS_SERVER);	
}



