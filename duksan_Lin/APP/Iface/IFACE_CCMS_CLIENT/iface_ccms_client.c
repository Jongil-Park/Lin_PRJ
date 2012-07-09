#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
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
#include <sys/poll.h>		// use poll event

#include "define.h"
#include "queue_handler.h"									// queue handler
#include "FUNCTION.h"


int 				g_nCCMS_Client_Status;
CCMS_PTBL_LIST_T	*g_pPtbl;

extern point_queue ccms_queue;	
extern pthread_mutex_t ccmsQ_mutex;
extern int g_nMyPcm;

//extern 	PTBL_INFO_T *g_pPtbl;
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];

extern int g_nPnoTable[8];

extern unsigned char 	g_cNode[32];



void cclnt_put_queue(point_info *pQueue)
// ----------------------------------------------------------------------------
// INSERT A DATA INTO THE CCMS_QUEUE
// Description : ccms_queue에 data를 추가한다.
// Arguments   : p					Is a point-value pointer.
// Returns     : none
{
	pthread_mutex_lock( &ccmsQ_mutex );
	putq( &ccms_queue, pQueue );
	pthread_mutex_unlock( &ccmsQ_mutex );
}


int cclnt_get_queue(point_info *pQueue)
// ----------------------------------------------------------------------------
// DATA GET CCMS_QUEUE
// Description : ccms_queue에서 data를 꺼내온다.
// Arguments   : pQueue				Is a point-value pointer.
// Returns     : n_ret				Is a queue status type.
{
	int n_ret;

	memset( pQueue, 0, sizeof(point_info) );

	pthread_mutex_lock( &ccmsQ_mutex );
	n_ret = getq( &ccms_queue, pQueue );
	pthread_mutex_unlock( &ccmsQ_mutex ); 
	
	return n_ret;
}


void cclnt_sleep(int sec, int msec) 
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

unsigned int cclnt_get_id(void)
{
	FILE *fp;
	char bufId[32];
	unsigned int nId = 0;;
	
	if((fp = fopen("ccms_id", "r+")) == NULL) {
		fprintf(stdout, "ccms_id open fail.\n");
		fflush(stdout);
		nId = 0;
		return nId;			
	}

	while(!feof(fp)) {
		fscanf(fp, "%s", bufId);	
	}	
	
	fclose(fp);
	fprintf(stdout, "Read File :: %s\n", bufId);
	fflush(stdout);

	nId = (unsigned int) atoi(bufId);

	return nId;
}


CCMS_CLIENT_T *new_cclnt(void)
// ----------------------------------------------------------------------------
// CREATE THE CCMS_CLIENT_T STRUTURE 
// Description		: This function is called by ccms_client_main().
// 					  - create CCMS_CLIENT_T structure
// 					  - initialize variable of CCMS_CLIENT_T structure
// Arguments		: none
// Returns			: sock		Is pointer of CCMS_CLIENT_T structure
{
	CCMS_CLIENT_T *p;
	
	p = (CCMS_CLIENT_T *)malloc( sizeof(CCMS_CLIENT_T) );
	
	memset ( p, 0x00, sizeof(CCMS_CLIENT_T) );
	p->nFd = -1;

	p->nClientId = g_nMyPcm;

	p->nIndexPno = 0;
	p->nIndexSend = 0;

	printf("CCMS_PTBL_LIST_T = %d byte\n", sizeof(CCMS_PTBL_LIST_T)); 
	g_pPtbl = (CCMS_PTBL_LIST_T *) malloc( sizeof(CCMS_PTBL_LIST_T) );
	p->pPtbl = g_pPtbl;

	return p;
}


int cclnt_open(CCMS_CLIENT_T *p)
// ----------------------------------------------------------------------------
// OPEN THE CCMS_CLIENT_T Socket
// Description		: This function is called by ccms_client_main().
// Arguments		: p			Is a pointer to the CCMS_CLIENT_T structure.
// Returns			: 1			If the call was successful
//              	 -1			If not
{
	struct sockaddr_in server_addr;
	struct timeval timeo;
	int one = 1;
	
	long arg; 		
	int res; 
	fd_set myset; 
	struct timeval tv; 
	int valopt; 
	socklen_t lon;

	
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

    if (setsockopt( p->nFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
		fprintf( stdout, "+ CCMS Socket SO_REUSEADDR error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;	
    }

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("125.7.234.182");
	server_addr.sin_port = htons( CCMS_PORT );

	// Set non-blocking 
	// 출처:[LINUX] linux에서 network 접속시 non-block으로 연결하기..
	if( (arg = fcntl(p->nFd, F_GETFL, NULL)) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( p->nFd );
		return -1;
	} 
	
	arg |= O_NONBLOCK; 
	if( fcntl(p->nFd, F_SETFL, arg) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( p->nFd );
		return -1;
	}

	// Trying to connect with timeout 
	res =  connect(p->nFd, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
	if (res < 0) { 
		if (errno == EINPROGRESS) { 
			fprintf(stdout, "EINPROGRESS in connect() - selecting\n"); 
			do { 
				tv.tv_sec = 5;  // 15초후에 빠져나오도록 한다.
				tv.tv_usec = 0; 
				FD_ZERO(&myset); 
				FD_SET(p->nFd, &myset); 
				res = select(p->nFd+1, NULL, &myset, NULL, &tv); 
				if (res < 0 && errno != EINTR) { 
					fprintf(stdout, "Error connecting %d - %s\n", errno, strerror(errno)); 
					fflush( stdout );
					close( p->nFd );
					return -1; 
				} 
				else if (res > 0) { 
					// Socket selected for write 
					lon = sizeof(int); 
					if (getsockopt(p->nFd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
						fprintf(stdout, "Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
						fflush( stdout );
						close( p->nFd );
						return -1;
					} 
					// Check the value returned... 
					if (valopt) { 
						fprintf(stdout, "Error in delayed connection() %d - %s\n", valopt, strerror(valopt)); 
						fflush( stdout );
						close( p->nFd );
						return -1;
					} 
					break; 
				} 
				else { 
					fprintf(stdout, "Timeout in select() - Cancelling!\n"); 
					fflush( stdout );
					close( p->nFd );
					return -1;
				} 
			} while (1); 
		} 
		else { 
			fprintf(stdout, "Error connecting %d - %s\n", errno, strerror(errno)); 
			fflush( stdout );
			close( p->nFd );
			return -1;
		} 
	} 

	// Set to blocking mode again... 
	if( (arg = fcntl(p->nFd, F_GETFL, NULL)) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( p->nFd );
		return -1; 
	} 
	
	arg &= (~O_NONBLOCK); 
	if( fcntl(p->nFd, F_SETFL, arg) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( p->nFd );
		return -1; 
	}

	return p->nFd;
}


int cclnt_send_pkt(CCMS_CLIENT_T *p)
{
	int i = 0;
	int nRetSend = 0;
	CCMS_PKT_T *pTx;
	CCMS_PKT_SET_T *pSetT;
	point_info point;
	int nRemoteId = 0;
	unsigned char chPcm = 0;
	unsigned char chPno = 0;
	unsigned short wValue = 0;
	int nIndexPno = 0;

    fd_set writes, temps;
    struct timeval tv;
    int nResult = 0;
    int fd_max;
    
    FD_ZERO(&writes);
    FD_SET(p->nFd, &writes);
    fd_max = p->nFd;
    
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    temps = writes;
    
    
    //nResult = select( p->nFd, &temps, 0, 0, &tv );
	
	if ( cclnt_get_queue(&point) == SUCCESS ) {
		nRemoteId = (int)point.message_type;
		chPcm = (unsigned char)point.pcm;
		chPno = (unsigned char)point.pno;
		wValue = (unsigned short)point.value;

		pSetT = (CCMS_PKT_SET_T *)p->chTxBuf;
		pSetT->chStx = '<';
		pSetT->chCmd = 'S';
		pSetT->chId = (unsigned char)nRemoteId;
		pSetT->chPcm = chPcm;
		pSetT->chPno = chPno;
		pSetT->wData = wValue;
		pSetT->chEtx = '>';

		fprintf( stdout, "Set %d %d %d %f\n", 
				 point.message_type, point.pcm, 
				 point.pno, point.value);
		fflush( stdout );

		//nRetSend = send(p->nFd, pSetT, sizeof(CCMS_PKT_SET_T), 0 );		 
		
	    //nResult = select( fd_max + 1, &temps, 0, 0, &tv );
	    nResult = select( fd_max + 1, NULL, &temps, NULL, &tv );
	 
	    if(nResult == 0)  {
	        printf("send wait\n");
	        return 0;
	    }
	    else if(nResult == -1)  {
	        printf("send select error : \n");
	        return -1;
	    }
	    else  {	
	        //printf("upper FD_ISSET\n");
	        if(FD_ISSET(p->nFd, &temps))  {	
	            nRetSend = send(p->nFd, pSetT, sizeof(CCMS_PKT_SET_T), 0 );	
				//printf("nRetSend = %d\n", nRetSend);	
	            return nRetSend;
	        }
	    }		
	}
	else {
		
		if ( p->nIndexPno++ >= 7 )
			p->nIndexPno = 0;
		
		pTx = (CCMS_PKT_T *)p->chTxBuf;	
		pTx->chStx = '<';
		pTx->chCmd = 'N';
		pTx->wIndex = p->nIndexPno;
		pTx->chId = p->nClientId;
		pTx->wDummy = 0;
			
		nIndexPno = g_nPnoTable[p->nIndexPno];
		
		for ( i = 0; i < CCMS_PKT_DATA_CNT; i++) {
			pTx->fData[i] = g_fExPtbl[p->nClientId][nIndexPno + i];
		}
		
		pTx->chEtx = '>';
	
		//nRetSend = send(p->nFd, pTx, sizeof(CCMS_PKT_T), 0 );
	 
	    //nResult = select( fd_max + 1, &temps, 0, 0, &tv );
	    nResult = select( fd_max + 1, NULL, &temps, NULL, &tv );
	 
	    if(nResult == 0)  {
	        printf("send wait\n");
	        return 0;
	    }
	    else if(nResult == -1)  {
	        printf("send select error : \n");
	        return -1;
	    }
	    else  {	
	        //printf("upper FD_ISSET\n");
	        if(FD_ISSET(p->nFd, &temps))  {	
	            nRetSend = send(p->nFd, pTx, sizeof(CCMS_PKT_T), 0 );
				//printf("nRetSend = %d\n", nRetSend);	
	            return nRetSend;
	        }
	    }		

		//fprintf( stdout, ">> %d  length = %d\n", nIndexPno, sizeof(CCMS_PKT_T) );
		//fflush( stdout );	
	}
	
	return nRetSend;
}


void cclnt_parse_pkt(unsigned char *pBuf) 
{
	int nType = 0;
	//CCMS_PKT_T *pPkt;
	//CCMS_PKT_ACK_T *pAck;
	CCMS_PKT_SET_T *pSetT;
	//int nRtnSend = 0;
	//unsigned char chTxBuf[32];
	//point_info point;
	//unsigned short wRemoteId = 0;
	//unsigned char chPcm = 0;
	//unsigned char chPno = 0;
	//unsigned short wValue = 0;

	nType = pBuf[1];

	switch (nType) {
	// Notice Packet.
	case 'S':
		pSetT = (CCMS_PKT_SET_T *)pBuf;
		if ( pSetT->chStx == '<' && pSetT->chEtx == '>') {
			fprintf( stdout, "Set = %d, %d, %d \n", 
					 pSetT->chPcm,
					 pSetT->chPno,
					 pSetT->wData);
			fflush( stdout );

			pSet(pSetT->chPcm, pSetT->chPno, pSetT->wData);
		}
		break;
	}	
}


int cclnt_recv_pkt(CCMS_CLIENT_T *p)
{
	int nRecvByte = 0;
	int nIndex = 0;
	int nOffset = 0;
	int nPktLength = 0;
	unsigned char chTemp[CCMS_BUFFER_SIZE];

    fd_set reads, temps;
    struct timeval tv;
    int nResult = 0;
    int fd_max;
    
    FD_ZERO(&reads);
    FD_SET(p->nFd, &reads);
    fd_max = p->nFd;
    
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    temps = reads;

    nResult = select( fd_max + 1, &temps, 0, 0, &tv );
 
    if(nResult == 0)  {
        printf("receive wait\n");
        return 0;
    }
    else if(nResult == -1)  {
        printf("receive select error : \n");
        return -1;
    }
    else  {	
        //printf("upper FD_ISSET\n");
        if(FD_ISSET(p->nFd, &temps))  {	
             //nRetSend = send(p->nFd+1, pSetT, sizeof(CCMS_PKT_SET_T), 0 );	
             nRecvByte = read(p->nFd, p->chRxBuf, sizeof(p->chRxBuf));
        }
    }	
	    
	 //nRecvByte = read(p->nFd, p->chRxBuf, sizeof(p->chRxBuf));

	if ( nRecvByte <= 0) {
		return nRecvByte;
	}

	// 이전 buffer에 값이 들어있다면, 
	// 이전 buffer를 참조해서 Packet을 만든다. 
	if ( p->nIndexPrev > 0  ) {
		memcpy(chTemp, p->chRxBuf, nRecvByte);
		memcpy(p->chRxBuf, p->chPrevBuf, p->nIndexPrev);
		memcpy(&p->chRxBuf[p->nIndexPrev], chTemp, nRecvByte);
		nRecvByte += p->nIndexPrev;
		p->nIndexPrev = 0;
	}

	while (nOffset < nRecvByte)
	{
		if ( p->chRxBuf[nOffset] != '<' ) {
			nOffset++;
		}
		
		if ( p->chRxBuf[nOffset + 1] == 'N' ) {
			nPktLength = 520;
		}
		else if ( p->chRxBuf[nOffset + 1] == 'S' ) {
			nPktLength = 8;
		}
		else if ( p->chRxBuf[nOffset + 1] == 'A' ) {
			nPktLength = 4;
		}
		else
			continue;

		nIndex = nOffset + nPktLength;	
		if (nIndex <= nRecvByte) {
			//ccms_parse_pkt(nSockFd, &pBuf->chRxBuf[nOffset]);
			if ( p->chRxBuf[nOffset + 1] == 'S' ) {
				cclnt_parse_pkt(&p->chRxBuf[nOffset]);
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


	return nRecvByte;
}


void cclnt_close(CCMS_CLIENT_T *p)
{
	printf("%s()\n", __FUNCTION__);
	
	close(p->nFd);
	
	memset ( p, 0x00, sizeof(CCMS_CLIENT_T) );
	p->nFd = -1;

	//p->nClientId = cclnt_get_id();
	p->nClientId = g_nMyPcm;

	p->nIndexPno = 0;
	p->nIndexSend = 0;	
}


int cclnt_master_check(void)
{
	if ( g_cNode[0] > 0 )
		return 1;
	else
		return -1;
}

int cclnt_get_status(void)
{
	return g_nCCMS_Client_Status;
}

void *ccms_client_main(void* arg)
{
	//int i = 0;
	int nRetOpen = 0;
	int nRtnRecv = 0;
	CCMS_CLIENT_T	*pCclnt = NULL;

	signal(SIGPIPE, SIG_IGN);	// Ignore broken_pipe signal
	
	cclnt_sleep(1,0);

	pCclnt = new_cclnt();
	if ( pCclnt == NULL ) {
		fprintf( stdout, "Can't create CCMS Client Interface.\n" );
		fflush( stdout );
		//exit(1);
		system("killall duksan");
	}

	while (1) {
		g_nCCMS_Client_Status = 0;
	
		if ( cclnt_master_check() > 0 ) {
			cclnt_sleep(3,0);
			continue;
		}
		
		nRetOpen = cclnt_open(pCclnt);
		if ( nRetOpen < 0 ) {
			fprintf( stdout, "Wait connect\n" );
			fflush( stdout );			
			cclnt_sleep(3,0);
			continue;
		}
		
		for (;;) {
		
			g_nCCMS_Client_Status = 1;
			
			//fprintf( stdout, "Check Me\n" );
			//fflush( stdout );
			
			if ( cclnt_send_pkt(pCclnt) <= 0 ) {
				cclnt_close(pCclnt);
				break;
			}				
			
			cclnt_sleep(0,50);
			

			nRtnRecv = cclnt_recv_pkt(pCclnt);

			if ( nRtnRecv == 0 ) {
				cclnt_close(pCclnt);
				break;
			}
			
			if ( cclnt_master_check() > 0 ) {
				cclnt_close(pCclnt);
				break;
			}	
		}
		continue;
	}
	syslog_record(SYSLOG_DESTROY_CCMS_CLIENT);	
}




