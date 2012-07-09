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
unsigned char tx_msg[MAX_BUFFER];
unsigned char rx_msg[MAX_BUFFER];	

int main(int argc, char* argv[]);
void handleCmdDebugTCP(int argc, char* argv[]);
void handleCmdDebugNTX(int argc, char* argv[]);
void handleCmdDebugNRX(int argc, char* argv[]);
void handleCmdDebugSUBIO(int argc, char* argv[]);
void handleCmdDebugIFACE(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	//int   client_socket;
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

	if(strncmp(argv[0], "debug", 5) == 0)
	{
		if(strncmp(argv[1], "tcp", 3) == 0)	
			handleCmdDebugTCP(argc, argv);		
		
		if(strncmp(argv[1], "ntx", 3) == 0)	
			handleCmdDebugNTX(argc, argv);		
		
		if(strncmp(argv[1], "nrx", 3) == 0)	
			handleCmdDebugNRX(argc, argv);	
		
		if(strncmp(argv[1], "subio", 5) == 0)	
			handleCmdDebugSUBIO(argc, argv);	
			
		if(strncmp(argv[1], "iface", 5) == 0)	
			handleCmdDebugIFACE(argc, argv);				
	}			
	
	close( client_socket);
	return SUCCESS;
}

/*****************************************************/
// message : || CMD(1) || 'T' || 'C' || 'P' || [1/0] ||
void 
handleCmdDebugTCP(int argc, char* argv[])
/*****************************************************/
{	
	memset(tx_msg, 0, sizeof(tx_msg));

	if(argc < 3)
	{
		printf("Incorrect Arguments\n");
		printf("./debug tcp [on][off]\n");
		return;
	}

	//Make Send Message
	tx_msg[0] = COMMAND_DEBUG_TCP;
	tx_msg[1] = 'T';
	tx_msg[2] = 'C';
	tx_msg[3] = 'P';
	if(strncmp(argv[2], "on", 2) == 0)
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
// message : || CMD(1) || 'N' || 'T' || 'X' || [1/0] ||
void 
handleCmdDebugNTX(int argc, char* argv[])
/*****************************************************/
{	
	memset(tx_msg, 0, sizeof(tx_msg));

	if(argc < 3)
	{
		printf("Incorrect Arguments\n");
		printf("./debug ntx [on][off]\n");
		return;
	}

	//Make Send Message
	tx_msg[0] = COMMAND_DEBUG_NTX;
	tx_msg[1] = 'N';
	tx_msg[2] = 'T';
	tx_msg[3] = 'X';
	if(strncmp(argv[2], "on", 2) == 0)
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
// message : || CMD(1) || 'N' || 'R' || 'X' || [1/0] ||
void handleCmdDebugNRX(int argc, char* argv[])
/*****************************************************/
{	
	memset(tx_msg, 0, sizeof(tx_msg));

	if(argc < 3)
	{
		printf("Incorrect Arguments\n");
		printf("./debug nrx [on][off]\n");
		return;
	}

	//Make Send Message
	tx_msg[0] = COMMAND_DEBUG_NRX;
	tx_msg[1] = 'N';
	tx_msg[2] = 'R';
	tx_msg[3] = 'X';
	if(strncmp(argv[2], "on", 2) == 0)
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
// message : || CMD(1) || 'S' || 'U' || 'B' || [1/0] ||
void handleCmdDebugSUBIO(int argc, char* argv[])
/*****************************************************/
{	
	memset(tx_msg, 0, sizeof(tx_msg));

	if(argc < 3)
	{
		printf("Incorrect Arguments\n");
		printf("./debug subio [on][off]\n");
		return;
	}

	//Make Send Message
	tx_msg[0] = COMMAND_DEBUG_SUBIO;
	tx_msg[1] = 'S';
	tx_msg[2] = 'U';
	tx_msg[3] = 'B';
	if(strncmp(argv[2], "on", 2) == 0)
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
// message : || CMD(1) || 'S' || 'U' || 'B' || [1/0] ||
void handleCmdDebugIFACE(int argc, char* argv[])
/*****************************************************/
{	
	memset(tx_msg, 0, sizeof(tx_msg));

	if(argc < 3)
	{
		printf("Incorrect Arguments\n");
		printf("./debug iface [on][off]\n");
		return;
	}

	//Make Send Message
	tx_msg[0] = COMMAND_DEBUG_IFACE;
	tx_msg[1] = 'I';
	tx_msg[2] = 'F';
	tx_msg[3] = 'H';
	if(strncmp(argv[2], "on", 2) == 0)
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



#if 0
//
#include "define.h"
#include "queue.h"
#include "point_manager.h"
#include "cmd_handler.h"

int client_socket;
unsigned char tx_msg[MAX_BUFFER];
unsigned char rx_msg[MAX_BUFFER];	

int main(int argc, char* argv[]);
void handleCmdDebugTCP(int argc, char* argv[]);
void handleCmdDebugNTX(int argc, char* argv[]);
void handleCmdDebugNRX(int argc, char* argv[]);

/*****************************************************/
int 
main(int argc, char* argv[])
/*****************************************************/
{
	//Variables
	struct sockaddr_in server_addr_in;
	struct timeval tv;
	char server_ip[15];
	int server_port;

	if(argc == 1)
	{
		printf("No argument. Use following Syntax(%d)\n", argc);
		printf("./debug tcp [on][off]\n");
		return FAIL;
	}
	
	//Initialize
	client_socket = -1;
	memset(&server_addr_in, 0, sizeof(server_addr_in));
	tv.tv_sec = 0;
	tv.tv_usec = 100000; //0.1sec
	strcpy(server_ip, "127.0.0.1");
	server_port = COMMAND_SERVER_PORT;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);

	setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	server_addr_in.sin_family = AF_INET;
	server_addr_in.sin_addr.s_addr = inet_addr(server_ip);
	server_addr_in.sin_port = htons(server_port);

	if(connect(client_socket, (struct sockaddr *) &server_addr_in,
				sizeof(server_addr_in)) >= 0)
	{
		if(strncmp(argv[0], "debug", 5) == 0)
		{
			if(strncmp(argv[1], "tcp", 3) == 0)	
				handleCmdDebugTCP(argc, argv);		
			
			if(strncmp(argv[1], "ntx", 3) == 0)	
				handleCmdDebugNTX(argc, argv);		
			
			if(strncmp(argv[1], "nrx", 3) == 0)	
				handleCmdDebugNRX(argc, argv);		
		}			
		return SUCCESS;
	}
	else
	{
		printf("Connecting to Interface Server Failed\n");
		close(client_socket);
		return FAIL;
	}
}


/*****************************************************/
// message : || CMD(1) || 'T' || 'C' || 'P' || [1/0] ||
void 
handleCmdDebugTCP(int argc, char* argv[])
/*****************************************************/
{	
	//Variables
	struct timespec ts;
	
	//Initialize
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; //0.1sec
	
	memset(tx_msg, 0, sizeof(tx_msg));

	if(argc < 3)
	{
		printf("Incorrect Arguments\n");
		printf("./debug tcp [on][off]\n");
		return;
	}

	//Make Send Message
	tx_msg[0] = COMMAND_DEBUG_TCP;
	tx_msg[1] = 'T';
	tx_msg[2] = 'C';
	tx_msg[3] = 'P';
	if(strncmp(argv[2], "on", 2) == 0)
		tx_msg[4] = 1;
	else
		tx_msg[4] = 0;

	// Send Message
	if(send(client_socket, tx_msg, 5, 0) <= 0)
	{
		printf("[ERROR] Send() in handleCmdPget()\n");
		return;
	}
	
	// close socket
	nanosleep(&ts, NULL);
	close(client_socket);
	return;
}

/*****************************************************/
// message : || CMD(1) || 'N' || 'T' || 'X' || [1/0] ||
void 
handleCmdDebugNTX(int argc, char* argv[])
/*****************************************************/
{	
	struct timespec ts;
	
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; //0.1sec
	
	memset(tx_msg, 0, sizeof(tx_msg));

	if(argc < 3)
	{
		printf("Incorrect Arguments\n");
		printf("./debug tcp [on][off]\n");
		return;
	}

	//Make Send Message
	tx_msg[0] = COMMAND_DEBUG_NTX;
	tx_msg[1] = 'N';
	tx_msg[2] = 'T';
	tx_msg[3] = 'X';
	if(strncmp(argv[2], "on", 2) == 0)
		tx_msg[4] = 1;
	else
		tx_msg[4] = 0;

	// Send Message
	if(send(client_socket, tx_msg, 5, 0) <= 0)
	{
		printf("[ERROR] Send() in handleCmdPget()\n");
		return;
	}
	
	// close socket
	nanosleep(&ts, NULL);
	close(client_socket);
	return;
}

/*****************************************************/
// message : || CMD(1) || 'N' || 'R' || 'X' || [1/0] ||
void 
handleCmdDebugNRX(int argc, char* argv[])
/*****************************************************/
{	
	struct timespec ts;
	
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; //0.1sec
	
	memset(tx_msg, 0, sizeof(tx_msg));

	if(argc < 3)
	{
		printf("Incorrect Arguments\n");
		printf("./debug tcp [on][off]\n");
		return;
	}

	//Make Send Message
	tx_msg[0] = COMMAND_DEBUG_NRX;
	tx_msg[1] = 'N';
	tx_msg[2] = 'R';
	tx_msg[3] = 'X';
	if(strncmp(argv[2], "on", 2) == 0)
		tx_msg[4] = 1;
	else
		tx_msg[4] = 0;

	// Send Message
	if(send(client_socket, tx_msg, 5, 0) <= 0)
	{
		printf("[ERROR] Send() in handleCmdPget()\n");
		return;
	}
	
	// close socket
	nanosleep(&ts, NULL);
	close(client_socket);
	return;
}



#endif



