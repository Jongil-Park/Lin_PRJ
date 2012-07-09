/*
 * file 		: dbsvr_mgr.c
 * creator   	: jong2ry
 *
 *
 * point table의 모든 값을 인터페이스로 올리기 위해 사용한다. 
 *
 *
 * version :
 *		0.0.1  jong2ry working.
 *		0.0.2  jong2ry code clean.
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
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <sys/poll.h>		// use poll event

////////////////////////////////////////////////////////////////////////////////
//define
#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

#define ACCEPT_NONBLOCKING

////////////////////////////////////////////////////////////////////////////////
// extern variable
extern pthread_mutex_t pointTable_mutex;										// point table mutex

// point and node
extern int g_nMyPcm;															// my pcm number
extern int g_nMultiDdcFlag;														// mode select flag
extern unsigned char g_cNode[MAX_NET32_NUMBER];									// node
extern PTBL_INFO_T *g_pPtbl;													// point table
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];						// extension point table


////////////////////////////////////////////////////////////////////////////////
// global variable
WORD g_wSendPcmId = 0;															// PCM number index



DBSVR_T *dbsvr_init(void)
// ----------------------------------------------------------------------------
// CREATE THE DBSVR_T STRUTURE 
// Description		: This function is called by dbsvr_main().
// 					  - create DBSVR_T structure
// 					  - initialize variable of DBSVR_T structure
// Arguments		: none
// Returns			: sock		Is pointer of DBSVR_T structure
{
	DBSVR_T *sock;
	
	sock = (DBSVR_T *)malloc( sizeof(DBSVR_T) );
	
	sock->nFd = -1;

	sock->rxbuf = (unsigned char *) malloc (MAX_BUFFER_SIZE);
	sock->txbuf = (unsigned char *) malloc (MAX_BUFFER_SIZE);
	
	memset (sock->rxbuf, 0x00, MAX_BUFFER_SIZE);
	memset (sock->txbuf, 0x00, MAX_BUFFER_SIZE);
	
	sock->nRecvByte = 0;
		
	memset( sock->targetIp, 0, sizeof(sock->targetIp) );		
	memcpy( sock->targetIp, "125.7.234.169", sizeof("125.7.234.169") );

	return sock;
}


void dbsvr_close(DBSVR_T *p)
// ----------------------------------------------------------------------------
// CLOSE THE DBSVR_T STRUTURE 
// Description		: This function is called by dbsvr_main().
// 					  - close socket and initialize variable of DBSVR_T structure
// Arguments		: p			Is a pointer to the DBSVR_T structure.
// Returns			: 1			If the call was successful
//         			 -1			If not
{
	close(p->nFd);

	p->nFd = -1;
	p->nRecvByte = 0;

	memset( p->rxbuf, 0, MAX_BUFFER_SIZE );
	memset( p->txbuf, 0, MAX_BUFFER_SIZE );
}


int dbsvr_open(DBSVR_T *p)
// ----------------------------------------------------------------------------
// OPEN THE DBSVR_T STRUTURE 
// Description		: This function is called by dbsvr_main().
// 					  - create socket and connect DB SErver
// Arguments		: p			Is a pointer to the DBSVR_T structure.
// Returns			: 1			If the call was successful
//              	 -1			If not
{
	int nRes = 0;
	struct sockaddr_in server_addr;
	struct timeval timeo;

#ifdef ACCEPT_NONBLOCKING
	long arg; 		
	fd_set myset; 
	struct timeval tv; 
	int valopt; 
	socklen_t lon;
#endif		
		
	memset( &server_addr, 0, sizeof(server_addr) );
	timeo.tv_sec = 0;
	timeo.tv_usec = 10000;
	
	p->nFd = socket( AF_INET, SOCK_STREAM, 0 );
	if( p->nFd < 0 ) {
		fprintf( stdout, "+ Socket creation error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;					
	}

	setsockopt( p->nFd, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo) );

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr( p->targetIp );
	server_addr.sin_port = htons( DBSVR_PORT );

	fprintf( stdout, "+ Try Connect to DB Server %s\n", p->targetIp );
	fflush( stdout );

	/*
	if ( connect(p->nFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 ) {
		return -1;
	}
	else {
		fprintf( stdout, "+ Connect to DB Server %s Ok\n", p->targetIp );
		fflush( stdout );
		return 1;
	}	
	*/

#ifdef ACCEPT_NONBLOCKING
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
	nRes =  connect(p->nFd, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
	if (nRes < 0) { 
		if (errno == EINPROGRESS) { 
			fprintf(stdout, "DBSVR Progress in connect() - selecting\n"); 
			fflush( stdout );
			do { 
				tv.tv_sec = 5;  // 5초후에 빠져나오도록 한다.
				tv.tv_usec = 0; 
				FD_ZERO(&myset); 
				FD_SET(p->nFd, &myset); 
				nRes = select(p->nFd+1, NULL, &myset, NULL, &tv); 
				if (nRes < 0 && errno != EINTR) { 
					//fprintf(stdout, "DBSVR Error connecting %d - %s\n", errno, strerror(errno)); 
					//fflush( stdout );
					close( p->nFd );
					return -1; 
				} 
				else if (nRes > 0) { 
					// Socket selected for write 
					lon = sizeof(int); 
					if (getsockopt(p->nFd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
						//fprintf(stdout, "DBSVR Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
						//fflush( stdout );
						close( p->nFd );
						return -1;
					} 
					// Check the value returned... 
					if (valopt) { 
						fprintf( stdout, "[DBSVR] connection() %d - %s\n", valopt, strerror(valopt)); 
						fflush( stdout );							
						close( p->nFd );
						return -1;
					} 
					break; 
				} 
				else { 
					//fprintf(stdout, "DBSVR Timeout in select() - Cancelling!\n"); 
					//fflush( stdout );
					close( p->nFd );
					return -1;
				} 
			} while (1); 
		} 
		else { 
			//fprintf(stdout, "Error connecting %d - %s\n", errno, strerror(errno)); 
			//fflush( stdout );
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
	
#else
	nRes =  connect(p->nFd, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
	if (nRes < 0) { 
		return -1;
	}
#endif	
	
	return p->nFd;
}


void dbsvr_make_messge(DBSVR_T *p)
{
	int i = 0;
	//float fvalue = 0;
	DBSVR_DATA_MSG_T *pMsg = (DBSVR_DATA_MSG_T *)p->txbuf;
	
	if ( g_wSendPcmId >= 32 ) {
		g_wSendPcmId = 0;
	}
	
	pMsg->stx = '<';
	pMsg->func = 'R';
	pMsg->length = sizeof(DBSVR_DATA_MSG_T);
	pMsg->pcm = g_wSendPcmId;

	for ( i = 0; i< 256; i++ ) {
		// get value
		pthread_mutex_lock( &pointTable_mutex );
		pMsg->fValue[i] = g_fExPtbl[g_wSendPcmId][i];
		pthread_mutex_unlock( &pointTable_mutex );

		/*
		if ( fvalue >= 0 ) {
			pMsg->value[i] = (WORD) fvalue;
			pMsg->value[i] = pMsg->value[i] & 0x7FFF;
			pMsg->value[i] = pMsg->value[i];
		}
		else {
			fvalue = fvalue * -1;
			pMsg->value[i] = (WORD) fvalue;
			pMsg->value[i] = pMsg->value[i] & 0x7FFF;
			pMsg->value[i] = pMsg->value[i] + 0x1000;
		}
		*/
	}
	pMsg->etx = '>';

	//fprintf( stdout, "pMsg->pcm = %d \n" , pMsg->pcm);
	//fflush( stdout );

	g_wSendPcmId++;
}


int dbsvr_send_message(DBSVR_T *p)
{
	int nSendByte = 0;
	
	nSendByte = send(p->nFd, p->txbuf, sizeof(DBSVR_DATA_MSG_T), 0);
	
	return nSendByte;
}


int dbsvr_recv_message(DBSVR_T *p)
{
	int nRecvByte = 0;

	nRecvByte = recv(p->nFd, p->rxbuf, MAX_BUFFER_SIZE, 0);
		 
	return nRecvByte;
}


void dbsvr_handler(DBSVR_T *p, int nLength)
{
	DBSVR_ACK_MSG_T *pMsg;

	if ( nLength != sizeof(DBSVR_ACK_MSG_T) ) {
		return;
	}

	pMsg = (DBSVR_ACK_MSG_T *)p->rxbuf;


	if ( pMsg->stx == '<' && pMsg->etx == '>' ) {
		if ( pMsg->pcm == 0xffff &&  pMsg->pno == 0xffff ) {
			return;
		}

		pSet(pMsg->pcm, pMsg->pno, pMsg->value);
		fprintf( stdout, "%x %x %x \n", 
				 pMsg->pcm, pMsg->pno, pMsg->value);
		fflush( stdout );
	}
}



void *dbsvr_main(void* arg)
{
	DBSVR_T *pDbsvr;
	int nSendRtn = 0;
	struct  timespec ts;

	pDbsvr = dbsvr_init();
	if ( pDbsvr == NULL ) {
		fprintf( stdout, "Can't create DBSVR Interface.\n" );
		fflush( stdout );
		system("sync");
		system("sync");
		system("sync");
		system("killall duksan");
	}

	while(1) {
		if ( dbsvr_open( pDbsvr ) < 0 ) {
			sleep(3);
			dbsvr_close( pDbsvr );
			continue;	
		}
		
		for ( ;; ) {
			// message를 생성한 후에 send로 보낸다. 
			dbsvr_make_messge(pDbsvr);
			nSendRtn = dbsvr_send_message(pDbsvr);
			
			if ( nSendRtn  < 0 ) {
				fprintf( stdout, "Close DBSVR Socket\n" );
				fflush( stdout );
				sleep(3);
				dbsvr_close( pDbsvr );
				break;				
			}
			
			// send 후에 150 mSec를 기다린다. 
			ts.tv_sec = 0;
			ts.tv_nsec = 150 * M_SEC;
			nanosleep(&ts, NULL);	

			pDbsvr->nRecvByte = dbsvr_recv_message(pDbsvr);
			if ( pDbsvr->nRecvByte == 0 ) {
				fprintf( stdout, "Close DBSVR Socket\n" );
				fflush( stdout );
				sleep(3);
				dbsvr_close( pDbsvr );
				break;
			}
			else if ( pDbsvr->nRecvByte > 0 ) {
				dbsvr_handler(pDbsvr, pDbsvr->nRecvByte);
			}
			continue;
		}

		continue;
	}

	free(pDbsvr->rxbuf);
	free(pDbsvr->txbuf);
}



