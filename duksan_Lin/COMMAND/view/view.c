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
#include <sys/un.h>
#include <sys/poll.h>		// use poll event

#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"
#include "cmd_handler.h"

int client_socket;
unsigned char tx_msg[32];

int main(int argc, char* argv[]);
void handleCmdView(int argc, char* argv[]);
void handleCmdViewTCP(int argc, char* argv[]);
void handleCmdViewNET32(int argc, char* argv[]);
void handleCmdViewADC(int argc, char* argv[]);
void handleCmdViewSUBIO(int argc, char* argv[]);


int main(int argc, char* argv[])
{
	struct sockaddr_un   server_addr;
	
	client_socket  = socket( PF_FILE, SOCK_STREAM, 0);
	if( -1 == client_socket) {
		printf( "socket create fail\n");
		exit( 1);
	}
	
	memset( &server_addr, 0, sizeof( server_addr));
	server_addr.sun_family  = AF_UNIX;
	strcpy( server_addr.sun_path, FILE_SERVER);
	
	if( -1 == connect( client_socket, (struct sockaddr*)&server_addr, sizeof( server_addr) ) ) {
		printf( "command server connect fail\n");
		exit( 1);
	}

	if(strncmp(argv[0], "vw", 2) == 0) {
		if(strncmp(argv[1], "tcp", 3) == 0)	
			handleCmdViewTCP(argc, argv);		
		else if(strncmp(argv[1], "net32", 5) == 0)	
			handleCmdViewNET32(argc, argv);		
		else if(strncmp(argv[1], "adc", 3) == 0)	
			handleCmdViewADC(argc, argv);		
		else if(strncmp(argv[1], "subio", 5) == 0)	
			handleCmdViewSUBIO(argc, argv);		
	}	

	if(strncmp(argv[0], "view", 4) == 0) {
		if(strncmp(argv[1], "tcp", 3) == 0)	
			handleCmdViewTCP(argc, argv);		
		else if(strncmp(argv[1], "net32", 5) == 0)	
			handleCmdViewNET32(argc, argv);		
		else if(strncmp(argv[1], "adc", 3) == 0)	
			handleCmdViewADC(argc, argv);		
		else if(strncmp(argv[1], "subio", 5) == 0)	
			handleCmdViewSUBIO(argc, argv);		
	}
	
	close( client_socket);
	return SUCCESS;
}

/*****************************************************/
// message : || CMD(1) || 'V' || '-' || 'T' || [1/0] ||
void handleCmdViewTCP(int argc, char* argv[])
/*****************************************************/
{	
	memset(tx_msg, 0, sizeof(tx_msg));

	if(argc < 3) {
		fprintf(stderr,"Incorrect Arguments\n");
		fprintf(stderr,"vw tcp [on][off]\n");
		fprintf(stderr,"view tcp [on][off]\n");
		fflush(stderr);
		return;
	}

	//Make Send Message
	tx_msg[0] = COMMAND_VIEW;
	tx_msg[1] = 'V';
	tx_msg[2] = '-';
	tx_msg[3] = 'T';
	if( strncmp(argv[2], "on", 2 ) == 0)
		tx_msg[4] = 1;
	else
		tx_msg[4] = 0;

	// Send Message
	if( write( client_socket, tx_msg, 5 ) <= 0) {
		printf("[ERROR] Send in %s()\n", __FUNCTION__);
		return;
	}	

	return;		
}


/*****************************************************/
// message : || CMD(1) || 'V' || '-' || 'N' || [1/0] ||
void handleCmdViewNET32(int argc, char* argv[])
/*****************************************************/
{	
	memset(tx_msg, 0, sizeof(tx_msg));

	if( argc < 3 ) {
		fprintf(stderr,"Incorrect Arguments\n");
		fprintf(stderr,"vw net32 [on][off]\n");
		fprintf(stderr,"view net32 [on][off]\n");
		fflush(stderr);
		return;
	}

	tx_msg[0] = COMMAND_VIEW;
	tx_msg[1] = 'V';
	tx_msg[2] = '-';
	tx_msg[3] = 'N';
	if( strncmp(argv[2], "on", 2) == 0 )
		tx_msg[4] = 1;
	else
		tx_msg[4] = 0;

	// Send Message
	if( write( client_socket, tx_msg, 5 ) <= 0) {
		fprintf(stderr, "[ERROR] Send in %s()\n", __FUNCTION__);
		fflush(stderr);
		return;
	}	

	return;	
}


/*****************************************************/
// message : || CMD(1) || 'V' || '-' || 'A' || [1/0] ||
void handleCmdViewADC(int argc, char* argv[])
/*****************************************************/
{	
	memset(tx_msg, 0, sizeof(tx_msg));

	if( argc < 3 ) {
		fprintf(stderr,"Incorrect Arguments\n");
		fprintf(stderr,"vw adc [on][off]\n");
		fprintf(stderr,"view adc [on][off]\n");
		fflush(stderr);
		return;
	}

	tx_msg[0] = COMMAND_VIEW;
	tx_msg[1] = 'V';
	tx_msg[2] = '-';
	tx_msg[3] = 'A';
	if( strncmp(argv[2], "on", 2) == 0 )
		tx_msg[4] = 1;
	else
		tx_msg[4] = 0;

	// Send Message
	if( write( client_socket, tx_msg, 5 ) <= 0) {
		fprintf(stderr, "[ERROR] Send in %s()\n", __FUNCTION__);
		fflush(stderr);
		return;
	}	

	return;	
}


/*****************************************************/
// message : || CMD(1) || 'V' || '-' || 'S' || [1/0] ||
void handleCmdViewSUBIO(int argc, char* argv[])
/*****************************************************/
{	
	memset(tx_msg, 0, sizeof(tx_msg));

	if( argc < 3 ) {
		fprintf(stderr,"Incorrect Arguments\n");
		fprintf(stderr,"vw subio [on][off]\n");
		fprintf(stderr,"view subio [on][off]\n");
		fflush(stderr);
		return;
	}

	tx_msg[0] = COMMAND_VIEW;
	tx_msg[1] = 'V';
	tx_msg[2] = '-';
	tx_msg[3] = 'S';
	if( strncmp(argv[2], "on", 2) == 0 )
		tx_msg[4] = 1;
	else
		tx_msg[4] = 0;

	// Send Message
	if( write( client_socket, tx_msg, 5 ) <= 0) {
		fprintf(stderr, "[ERROR] Send in %s()\n", __FUNCTION__);
		fflush(stderr);
		return;
	}	

	return;	
}


