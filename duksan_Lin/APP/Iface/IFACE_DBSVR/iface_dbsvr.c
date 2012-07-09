
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

#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"
#include "queue_handler.h"									// queue handler
#include "log_mgr.h"										// log manager

BYTE g_dbsvr_rxmsg[MAX_BUFFER_SIZE];
BYTE g_dbsvr_txmsg[MAX_BUFFER_SIZE];

WORD g_dbsvr_pcm = 0;

extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];

void dbsvr_sleep(int sec, int msec) 
// ----------------------------------------------------------------------------
// WAIT TIMER
// Description : use select function for timer.
// Arguments   : sec		Is a second value.
//				 usec		Is a micro-second value. 
// Returns     : none
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = msec * 1000;
    select( 0, NULL, NULL, NULL, &tv );
	return;
}


DBSVR_T *new_dbsvr(void)
// ----------------------------------------------------------------------------
// CREATE THE ELBA_T STRUTURE 
// Description		: This function is called by dbsvr_main().
// 					  - create DBSVR_T structure
// 					  - initialize variable of DBSVR_T structure
// Arguments		: none
// Returns			: sock		Is pointer of DBSVR_T structure
{
	DBSVR_T *sock;
	
	sock = (DBSVR_T *)malloc( sizeof(DBSVR_T) );
	
	sock->fd = -1;
	sock->rxbuf = (unsigned char *)&g_dbsvr_rxmsg;
	sock->txbuf = (unsigned char *)&g_dbsvr_txmsg;
	sock->tempWp = 0;
	sock->recvLength = 0;
	sock->bufSize = MAX_BUFFER_SIZE;
		
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
	close(p->fd);

	p->fd = -1;
	p->tempWp = 0;
	p->recvLength = 0;

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
	struct sockaddr_in server_addr;
	struct timeval timeo;
		
	memset( &server_addr, 0, sizeof(server_addr) );
	timeo.tv_sec = 0;
	timeo.tv_usec = 10000;
	
	p->fd = socket( AF_INET, SOCK_STREAM, 0 );
	if( p->fd < 0 ) {
		fprintf( stdout, "+ Socket creation error\n" );
		fflush( stdout );
		close( p->fd );
		return -1;					
	}

	setsockopt( p->fd, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo) );

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr( p->targetIp );
	server_addr.sin_port = htons( 9000 );

	fprintf( stdout, "+ Try Connect to DB Server %s\n", p->targetIp );
	fflush( stdout );

	if ( connect(p->fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 ) {
		return -1;
	}
	else {
		fprintf( stdout, "+ Connect to DB Server %s Ok\n", p->targetIp );
		fflush( stdout );
		return 1;
	}	
}


void dbsvr_make_msg(DBSVR_T *p)
{
	int i = 0;
	float fvalue = 0;
	DBSVR_DATA_MSG_T *pMsg = (DBSVR_DATA_MSG_T *)p->txbuf;
	
	if ( g_dbsvr_pcm >= 32 ) {
		g_dbsvr_pcm = 0;
	}
	
	pMsg->stx = '<';
	pMsg->func = 'R';
	pMsg->length = sizeof(DBSVR_DATA_MSG_T);
	pMsg->pcm = g_dbsvr_pcm;

	for ( i = 0; i< 256; i++ ) {
		fvalue = g_fExPtbl[g_dbsvr_pcm][i];
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
	}
	pMsg->etx = '>';

	//fprintf( stdout, "pMsg->pcm = %d \n" , pMsg->pcm);
	//fflush( stdout );

	g_dbsvr_pcm++;
}


int dbsvr_send(DBSVR_T *p)
{
	if ( send(p->fd, p->txbuf, sizeof(DBSVR_DATA_MSG_T), 0) < 0 )  {
		fprintf( stdout, "DBSVR Send Error \n");
		fflush( stdout );
		return -1;
	}
	else {
		//fprintf( stdout, "Send Message \n");
		//fflush( stdout );
	}
		 
	return 0;
}


int dbsvr_recv(DBSVR_T *p)
{
	int ret = 0;

	ret = recv(p->fd, p->rxbuf, MAX_BUFFER_SIZE, 0);
		 
	return ret;
}


void dbsvr_handler(DBSVR_T *p, int nLength)
{
	DBSVR_ACK_MSG_T *pMsg;

	//fprintf( stdout, "Receive Message %d / sizeof(DBSVR_ACK_MSG_T) = %d\n", nLength, sizeof(DBSVR_ACK_MSG_T));
	//fflush( stdout );

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
	int ret = 0;
	DBSVR_T *pDbsvr;

	pDbsvr = new_dbsvr();
	if ( pDbsvr == NULL ) {
		fprintf( stdout, "Can't create DBSVR Interface.\n" );
		fflush( stdout );
		//exit(1);
		system("killall duksan");
	}

	while(1) {
		if ( dbsvr_open( pDbsvr ) < 0 ) {
			dbsvr_sleep( 3, 0 );
			dbsvr_close( pDbsvr );
			continue;	
		}
		
		for ( ;; ) {
			dbsvr_make_msg(pDbsvr);

			dbsvr_send(pDbsvr);
			
			// send 후에 150 mSec를 기다린다. 
			//dbsvr_sleep( 0, 20 );
			dbsvr_sleep( 0, 150 );

			ret = dbsvr_recv(pDbsvr);
			if ( ret == 0 ) {
				fprintf( stdout, "Close DBSVR Socket\n" );
				fflush( stdout );
				dbsvr_sleep( 3, 0 );
				dbsvr_close( pDbsvr );
				break;
			}
			else if ( ret > 0 ) {
				dbsvr_handler(pDbsvr, ret);
			}
			continue;
		}

		continue;
	}

}



