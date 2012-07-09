/*
 * file 		: userver_mgr.c
 * creator   	: jong2ry
 *
 *
 * UDP Server와 messsage queue를 사용해서 data를 주고 받는다. 
 *
 *
 * version :
 *		0.0.1  jong2ry working.
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
 
 

////////////////////////////////////////////////////////////////////////////////
//define
#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

/* port we're listening on */
#define PORT 9990
#define MAX_BUF 1024

#define MAXPENDING 5    /* Max connection requests */
#define BUFFSIZE 32
void userver_debug(char *mess) { perror(mess); exit(1); }



void userver_HandleClient(int sock) 
// ----------------------------------------------------------------------------
// SOCKET FACTORY
{
	char buffer[BUFFSIZE];
	int received = -1;
	//point_info point;
	//int nResult;
	struct timespec ts;
	UCLIENT_MSG *pMsg;
	
	pMsg = (UCLIENT_MSG *)&buffer[0];
	
	for(;;) {
		// check for message
		if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
			userver_debug("Failed to receive additional bytes from client");
		}
			
		if (received > 0) {
			// wait 3msec
			ts.tv_sec = 0;
			ts.tv_nsec = 3 * 1000000;				
			nanosleep(&ts, NULL);	
								
			//To do... 
			// Send back received data
			if (send(sock, buffer, received, 0) != received) {
				userver_debug("Failed to send bytes to client");
			}					
		}
		else if ( received == 0 ) {
			close(sock);
			return;
		}
	}	
	return;
}     
 
void *userver_main(void* arg)
{
	int nSockOpt;
	int serversock, clientsock;
	struct sockaddr_in echoserver, echoclient;
	

	/* Create the TCP socket */
	if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		userver_debug("Failed to create socket");
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
		userver_debug("Failed to bind the server socket");
	}
	/* Listen on the server socket */
	if (listen(serversock, MAXPENDING) < 0) {
		userver_debug("Failed to listen on server socket");
	}  

	/* Run until cancelled */
	while (1) {
		unsigned int clientlen = sizeof(echoclient);
		/* Wait for client connection */
		if ((clientsock = accept(serversock, (struct sockaddr *) &echoclient, &clientlen)) < 0) {
			userver_debug("Failed to accept client connection");
		}
		
		fprintf(stdout, "Client connected: %s\n", inet_ntoa(echoclient.sin_addr));
		userver_HandleClient(clientsock);
	}
		
	system("killall duksan");		
}	
