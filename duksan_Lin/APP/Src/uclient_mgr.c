/*
 * file 		: uclient_mgr.c
 * creator   	: jong2ry
 *
 *
 * UDP Server�� messsage queue�� ����ؼ� data�� �ְ� �޴´�. 
 *
 *
 * version :
 *		0.0.1  jong2ry working.
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
#include <sys/time.h>
#include <time.h>
#include <sys/poll.h>		// use poll event
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <sys/stat.h> 
 

////////////////////////////////////////////////////////////////////////////////
//define
#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

/* port we're listening on */
#define PORT 9991
#define MAX_BUF 128

#define MAXPENDING 5    /* Max connection requests */
#define BUFFSIZE 32

void uclient_debug(char *mess) { perror(mess); return; }

extern pthread_mutex_t uclientQ_mutex;
extern point_queue uclient_queue;




void uclient_push_queue(point_info *p)
// ----------------------------------------------------------------------------
// PUSH DATA IN UCLIENT_QUEUE
{
	pthread_mutex_lock( &uclientQ_mutex );
	putq( &uclient_queue, p );
	pthread_mutex_unlock( &uclientQ_mutex );
}


int uclient_pop_queue(point_info *p)
// ----------------------------------------------------------------------------
// POP DATA IN UCLIENT_QUEUE
{
	int n_ret;

	memset( p, 0, sizeof(point_info) );
	pthread_mutex_lock( &uclientQ_mutex );
	n_ret = getq( &uclient_queue, p );
	pthread_mutex_unlock( &uclientQ_mutex );

	return n_ret;
}


void uclient_HandleClient(int sock) 
// ----------------------------------------------------------------------------
// SOCKET FACTORY
{
	char buffer[BUFFSIZE];
	int received = -1;
	point_info point;
	int nResult;
	struct timespec ts;
	UCLIENT_MSG *pMsg;
	
	pMsg = (UCLIENT_MSG *)&buffer[0];
	
	for(;;) {
		// check for message
		if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
			uclient_debug("Failed to receive additional bytes from client");
		}
			
		if (received > 0) {
			// udp client���� ��û�ϴ� ���� �׻� 4byte�̴�. 
			// 4byte�� ���� ������, ���� message�� �ٽ� ������. 
			if ( received == 4 ) {
				nResult = uclient_pop_queue( &point );
				if(nResult != SUCCESS) {
					// wait 3msec
					ts.tv_sec = 0;
					ts.tv_nsec = 3 * 1000000;				
					nanosleep(&ts, NULL);	
							
					// Send back received data
					if (send(sock, buffer, received, 0) != received) {
						uclient_debug("Failed to send bytes to client");
					}						
					continue;
				}
				
				pMsg->stx = '<';
				pMsg->length = sizeof(UCLIENT_MSG);
				pMsg->cmd = point.message_type;
				pMsg->addr = point.pcm;
				pMsg->pno = point.pno;
				pMsg->value = point.value;
				pMsg->etx = '>';
				
				if (send(sock, buffer, sizeof(UCLIENT_MSG), 0) != sizeof(UCLIENT_MSG)) {
					uclient_debug("Failed to send bytes to client");
				}	
				continue;					
			}
			else {
				// Send back received data
				if (send(sock, buffer, received, 0) != received) {
					uclient_debug("Failed to send bytes to client");
				}					
			}			
		}
		else if ( received == 0 ) {
			close(sock);
			return;
		}
	}	
	return;
}


void *uclient_main(void* arg)
// ----------------------------------------------------------------------------
// THREAD MAIN
{
	int nSockOpt;
	int serversock, clientsock;
	struct sockaddr_in echoserver, echoclient;
	

	/* Create the TCP socket */
	if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		uclient_debug("Failed to create socket");
	}
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
	echoserver.sin_family = AF_INET;                  /* Internet/IP */
	echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
	echoserver.sin_port = htons(PORT);      		  /* server port */

	// prevent bind error
	nSockOpt = 1;
	setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, &nSockOpt, sizeof(nSockOpt));

	/* Bind the server socket */
	if (bind(serversock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0) {
		uclient_debug("Failed to bind the server socket");
	}
	/* Listen on the server socket */
	if (listen(serversock, MAXPENDING) < 0) {
		uclient_debug("Failed to listen on server socket");
	}  

	/* Run until cancelled */
	while (1) {
		unsigned int clientlen = sizeof(echoclient);
		/* Wait for client connection */
		if ((clientsock = accept(serversock, (struct sockaddr *) &echoclient, &clientlen)) < 0) {
			uclient_debug("Failed to accept client connection");
		}
		
		fprintf(stdout, "Client connected: %s\n", inet_ntoa(echoclient.sin_addr));
		uclient_HandleClient(clientsock);
	}
		
	system("killall duksan");		
}	

