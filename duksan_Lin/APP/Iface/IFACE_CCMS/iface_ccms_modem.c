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
#include "iface_ccms_modem.h"								// interface ccms


CCMS_MODEM_T				*pCcmsModem;
unsigned char 				g_cNodeModem[32];

extern point_queue ccms_queue;	
extern pthread_mutex_t ccmsQ_mutex;
extern pthread_mutex_t pointTable_mutex;									// point table mutex

extern unsigned char g_cNode[32];
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];
extern int g_nMyPcm;

float g_fTestPrevValue[4];


void ccms_modem_sleep(int sec, int msec) 
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



CCMS_MODEM_T *new_ccms_modem(void)
// ----------------------------------------------------------------------------
// CREATE THE CCMS_MODEM_T STRUTURE 
// Description		: This function is called by ccms_main().
// 					  - create CCMS_MODEM_T structure
// 					  - initialize variable of CCMS_MODEM_T structure
// Arguments		: none
// Returns			: sock		Is pointer of CCMS_MODEM_T structure
{
	CCMS_MODEM_T *p;
	
	p = (CCMS_MODEM_T *)malloc( sizeof(CCMS_MODEM_T) );
	
	memset ( p, 0x00, sizeof(CCMS_MODEM_T) );
	p->nFd = -1;

	p->nServerId = 0;
	
	memset(g_cNodeModem, 0x00, sizeof(g_cNodeModem));

	return p;
}



int ccms_modem_open(CCMS_MODEM_T *p)
// ----------------------------------------------------------------------------
// OPEN THE CCMS_MODEM_T Socket
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
		fprintf( stdout, "+ CCMS MODEM Socket creation error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;					
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons( CCMS_MODEM_PORT );

    if (setsockopt( p->nFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
		fprintf( stdout, "+ CCMS MODEM Socket SO_REUSEADDR error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;	

    }

	if ( bind ( p->nFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ) {
		fprintf( stdout, "+ CCMS MODEM Socket Bind error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;	
	}

	if ( listen (p->nFd, 5) ) {
		fprintf( stdout, "+ CCMS MODEM Socket Listen error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;
	}

	FD_ZERO(&p->reads);
	FD_SET(p->nFd, &p->reads);
	p->nFdMax = p->nFd;

	return p->nFd;
}


int ccms_modem_chk_fd(CCMS_MODEM_T *p)
{
	int nFd = 0;
	struct timeval timeout;

	p->temps = p->reads;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	// select function.
	if ( select(p->nFdMax+1, &p->temps, 0, 0, &timeout) == -1 ) {
		fprintf( stdout, "+ CCMS MODEM Socket Select error\n" );
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


void ccms_modem_open_client(CCMS_MODEM_T *p, int nClientFd)
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
			p->nAliveCnt[nIndex] = 6;
			fprintf( stdout, "+ CCMS MODEM Open client %d, fd = %d\n" , nIndex, nClientFd);
			fflush( stdout );
			return;
		}
	}
}


void ccms_modem_close_client(CCMS_MODEM_T *p, int nClientFd)
// ----------------------------------------------------------------------------
// CLOSE CLIENT
// Description		: 접속한  Client에 상태값을 초기화 시킨다. 
// Arguments		: p					Is a pointer to the CCMS_MODEM_T structure.
// 					  nClientFd			Client Fd
// Returns			: none
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == nClientFd ) {
			fprintf( stdout, "+ CCMS MODEM Close client %d, fd = %d\n" , nIndex, nClientFd);
			fflush( stdout );
			p->nClientStatus[nIndex] = 0;
			p->nClientId[nIndex] = 0;
			return;
		}
	}
}



void ccms_modem_close(CCMS_MODEM_T *p, int nClientFd)
// ----------------------------------------------------------------------------
// CLOSE THE CLIENT SOCKET
// Description		: Close client socket.
// Arguments		: p					Is a pointer to the CCMS_T structure.
// 					  nClientFd			Client Fd
// Returns			: none.
{
	FD_CLR(nClientFd, &p->reads);
	close(nClientFd);
	ccms_modem_close_client(p, nClientFd);
	fprintf( stdout, "CCMS MODEM Close Client  = %d \n", nClientFd);
	fflush( stdout );
}


void ccms_modem_set_client_id(CCMS_MODEM_T *p, int nClientFd, char chId)
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == nClientFd ) {
			p->nClientId[nIndex] = chId;
			return;
		}
	}
}


void ccms_modem_register_node(CCMS_MODEM_T *p, int nClientFd, char chId)
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == nClientFd ) {
			p->nClientId[nIndex] = chId;
			p->nAliveCnt[nIndex] = 6;
			return;
		}
	}
}


void ccms_modem_parse_pkt(CCMS_MODEM_T *p, int nSockFd, unsigned char *pData) 
// ----------------------------------------------------------------------------
// PARSE PACKET
// Description		: 전송받은 Packet을 Parse 한다. 
// Arguments		: nSockFd			Client Socket
// 					  pData				Is a pointer to th Rxbuffer
// Returns			: none.
{
	//int i = 0;
	int nType = 0;
	int nRetSend = 0;
	//CCMS_PKT_T *pPkt;
	//CCMS_PKT_ACK_T *pAck;
	//CCMS_PKT_SET_T *pSetT;
	//int nRtnSend = 0;
	unsigned char chTxBuf[32];
	point_info point;
	//unsigned char chRemoteId = 0;
	unsigned char chPcm = 0;
	unsigned char chPno = 0;
	float *pValue;
	unsigned char chVal[4];
	

	nType = pData[1];

	switch (nType) {
	// Ack Packet.	
	case 'A':
		chPcm = ((p->chRxBuf[2] - 0x30) * 10 ) + (p->chRxBuf[3] - 0x30);
		printf("Receive Message 'ACK' PCM (%d)\n", chPcm);
		ccms_modem_register_node(p, nSockFd, chPcm);
		if ( g_fExPtbl[1][21] != g_fTestPrevValue[0] ) {
			g_fTestPrevValue[0]  = g_fExPtbl[1][21] ;
			chTxBuf[0] = 2;
			chTxBuf[1] = 1;
			chTxBuf[2] = 0;
			chTxBuf[3] = 1;			
			if (g_fExPtbl[1][21] > 0)
				chTxBuf[3] = 1;			
			else
				chTxBuf[3] = 0;			
							
			nRetSend = send(nSockFd, chTxBuf, 4, 0 );	
			printf("nRetSend = %d\n", nRetSend);			
		}

		if ( g_fExPtbl[1][22] != g_fTestPrevValue[1] ) {
			g_fTestPrevValue[1]  = g_fExPtbl[1][22] ;
			chTxBuf[0] = 2;
			chTxBuf[1] = 2;
			chTxBuf[2] = 0;
			if (g_fExPtbl[1][22] > 0)
				chTxBuf[3] = 1;			
			else
				chTxBuf[3] = 0;			
			
			nRetSend = send(nSockFd, chTxBuf, 4, 0 );	
			printf("nRetSend = %d\n", nRetSend);
		}
		break;
		
	// Notice Packet.
	case 'N':
		chPcm = ((p->chRxBuf[2] - 0x30) * 10 ) + (p->chRxBuf[3] - 0x30);
		chPno = ((p->chRxBuf[4] - 0x30) * 100 ) + 
			((p->chRxBuf[5] - 0x30) * 10) + 
			(p->chRxBuf[6] - 0x30);
		
		ccms_modem_register_node(p, nSockFd, chPcm);		
		printf("Receive Message 'NOTICE' PCM (%d, %d)\n", chPcm, chPno);


		if ( p->chRxBuf[7] > 0x40) 			
			chVal[0] = ((p->chRxBuf[7] - 0x41 + 10) * 0x10 );
		else
			chVal[0] = ((p->chRxBuf[7] - 0x30) * 0x10 );
			
		if ( p->chRxBuf[8] > 0x40) 			
			chVal[0] = (p->chRxBuf[8] - 0x41 + 10) + chVal[0];
		else
			chVal[0] = (p->chRxBuf[8] - 0x30) + chVal[0];			

		if ( p->chRxBuf[9] > 0x40) 			
			chVal[1] = ((p->chRxBuf[9] - 0x41 + 10) * 0x10 );
		else
			chVal[1] = ((p->chRxBuf[9] - 0x30) * 0x10 );
			
		if ( p->chRxBuf[10] > 0x40) 			
			chVal[1] = (p->chRxBuf[10] - 0x41 + 10) + chVal[1];
		else
			chVal[1] = (p->chRxBuf[10] - 0x30) + chVal[1];			

		if ( p->chRxBuf[11] > 0x40) 			
			chVal[2] = ((p->chRxBuf[11] - 0x41 + 10) * 0x10 );
		else
			chVal[2] = ((p->chRxBuf[11] - 0x30) * 0x10 );
			
		if ( p->chRxBuf[12] > 0x40) 			
			chVal[2] = (p->chRxBuf[12] - 0x41 + 10) + chVal[2];
		else
			chVal[2] = (p->chRxBuf[12] - 0x30) + chVal[2];			

		if ( p->chRxBuf[13] > 0x40) 			
			chVal[3] = ((p->chRxBuf[13] - 0x41 + 10) * 0x10 );
		else
			chVal[3] = ((p->chRxBuf[13] - 0x30) * 0x10 );
			
		if ( p->chRxBuf[14] > 0x40) 			
			chVal[3] = (p->chRxBuf[14] - 0x41 + 10) + chVal[3];
		else
			chVal[3] = (p->chRxBuf[14] - 0x30) + chVal[3];			
		
		pValue = (float *)chVal;
		//printf("pVal = %f\n", *pValue);

		point.pcm = chPcm;
		point.pno = chPno;
		point.value = *pValue;

		//printf("point.value = %f\n", point.value);

		if (g_fExPtbl[point.pcm][point.pno] == point.value )
			break;
		
		
		pthread_mutex_lock( &pointTable_mutex );
		g_fExPtbl[point.pcm][point.pno] = point.value;
		pthread_mutex_unlock( &pointTable_mutex );
		elba_push_queue( &point );	
	
		printf("CCMS MODEM Write g_fExPtbl[%d][%d] = %f\n", 
			point.pcm, 
			point.pno, 
			g_fExPtbl[point.pcm][point.pno]);		
		
		break;		
	}
}


void ccms_modem_recv_pkt(CCMS_MODEM_T *p, int nSockFd, int nRecvLength) 
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
		

	while (nOffset < nRecvLength)
	{
		if ( p->chRxBuf[nOffset] != '<' ) {
			nOffset++;
		}
		
		if ( p->chRxBuf[nOffset + 1] == 'A' ) {
			nPktLength = 4;
		}
		else if ( p->chRxBuf[nOffset + 1] == 'N' ) {
			nPktLength = 15;
		}		
		else
			continue;


		nIndex = nOffset + nPktLength;	
		if (nIndex <= nRecvLength) {
			ccms_modem_parse_pkt(p, nSockFd, &p->chRxBuf[nOffset]);
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


void ccms_modem_alive_check(CCMS_MODEM_T *p)
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] !=  0) {
			//printf("Index = %d, p->nAliveCnt[nIndex] = %d\n", nIndex, p->nAliveCnt[nIndex]);
			if ( (p->nAliveCnt[nIndex]--) == 0 ) {
				ccms_modem_close(p,  p->nClientStatus[nIndex]);
			}
			continue;
		}
	}	
}


void *ccms_modem_main(void* arg)
{
	int i = 0;
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

	pCcmsModem = new_ccms_modem();
	if ( pCcmsModem == NULL ) {
		fprintf( stdout, "Can't create CCMS MODEM Interface.\n" );
		fflush( stdout );
		//exit(1);
		system("killall duksan");
	}

	while (1) {
		ccms_modem_open(pCcmsModem);

		for (;;) {
			
			nChkFd = ccms_modem_chk_fd(pCcmsModem);
			if ( nChkFd < 0 ) {
				ccms_modem_alive_check(pCcmsModem);				
				continue;
			}

			// 연결 요청인 경우 처리 
			if ( nChkFd == pCcmsModem->nFd ) {
				nClientLength = sizeof(client_addr);
				nClientFd = accept( pCcmsModem->nFd, 
									(struct sockaddr *)&client_addr,
								    &nClientLength );
				FD_SET(nClientFd, &pCcmsModem->reads);

				if ( pCcmsModem->nFdMax < nClientFd ) 
					pCcmsModem->nFdMax = nClientFd;

				fprintf( stdout, "CCMS MODEM Accept client nClientFd = %d\n", nClientFd );
				fflush( stdout );
				
				// open structure
				ccms_modem_open_client(pCcmsModem, nClientFd);
			}
			else  {
				nRetRecv = read(nChkFd, chTemp, sizeof(chTemp));
				
				printf("nRetRecv = %d\n", nRetRecv);
				for ( i = 0; i < nRetRecv; i++) {
				printf("%x[%c], ", chTemp[i], chTemp[i]);
				}
				printf("\n");
			
				
				if ( nRetRecv <= 0 ) {
					fprintf( stdout, "CCMS MODEM nChkFd = %d  nRetRecv = %d \n", nChkFd, nRetRecv);
					fflush( stdout );
				}

				// client close
				if ( nRetRecv == 0 ) {
					ccms_modem_close(pCcmsModem, nChkFd);
					continue;
				}

				// client error
				if ( nRetRecv < 0 ) {
					continue;
				}
				
				memcpy(pCcmsModem->chRxBuf, 
					   chTemp,
					   nRetRecv);
				ccms_modem_recv_pkt(pCcmsModem, nChkFd, nRetRecv);
			}
		} // for (;;)
	} // while(1)
	syslog_record(SYSLOG_DESTROY_CCMS_MODEM_SERVER);	
}



