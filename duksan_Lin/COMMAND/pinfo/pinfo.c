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

void handleCmdPget(int argc, char* argv[]);

int main(int argc, char* argv[])
{
   //int   client_socket;
   struct sockaddr_un   server_addr;

   client_socket  = socket( PF_FILE, SOCK_STREAM, 0);
   if( -1 == client_socket) {
      printf( "socket 积己 角菩n");
      exit( 1);
   }

   memset( &server_addr, 0, sizeof( server_addr));
   server_addr.sun_family  = AF_UNIX;
   strcpy( server_addr.sun_path, FILE_SERVER);

   if( -1 == connect( client_socket, (struct sockaddr*)&server_addr, sizeof( server_addr) ) ) {

      printf( "立加 角菩n");
      exit( 1);
   }

	if(strncmp(argv[0], "pinfo", 5) == 0) {
		handleCmdPget(argc, argv);
	}
	else if(strncmp(argv[0], "./pinfo", 7) == 0) {
		handleCmdPget(argc, argv);
	}
	else {
		printf("Unknown Command. Use following Syntax(%d)\n", argc);
		printf("%s\n", argv[0]);
		printf("./pinfo [pcm] [pno] [number]\n");
	}
	
	close( client_socket);
	return SUCCESS;
}


/*****************************************************/
void handleCmdPget(int argc, char* argv[])
/*****************************************************/
{
	//Variables
	int i;
	short pcm;
	short pno;
	short number;
	int recv_index;
	int recv_length;
	int retry_count;
	int count;
	struct timespec ts;
	//point_info point;
	
	//Initialize
	i = 0;
	pcm = 0;
	pno = 0;
	number = 0;
	recv_index = 0;
	recv_length = 0;
	retry_count = 0;
	count = 0;
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; //0.1sec
	//memset(&point, 0, sizeof(point));
	//
	memset(tx_msg, 0, sizeof(tx_msg));
	memset(rx_msg, 0, sizeof(rx_msg));
	
	if(argc < 3) {
		printf("Incorrect Arguments\n");
		printf("./pinfo [pcm] [pno] [number]\n");
		return;
	}
	
	pcm = atoi(argv[1]);
	pno = atoi(argv[2]);
	if (argc == 4)
		number = atoi(argv[3]);
	else
		number = 0;

	//Check Validation of Arguments
    if(pcm < 0 || pcm >= MAX_NET32_NUMBER) {
		printf("Invalid PCM [%d] in handleCmdPget()\n", pcm);
		return;
	}

	if(pno < 0 || pno >= MAX_POINT_NUMBER) {
		printf("Invalid PNO [%d] in handleCmdPget()\n", pno);
		return;
	}

	if(number < 0 || (pno + number) > MAX_POINT_NUMBER) {
		printf("Invalid Number [%d] in handleCmdPget()\n", number);
		return;
	}

	//Make Send Message
	tx_msg[0] = COMMAND_PGET_MULTI_POINT;
	memcpy(&tx_msg[1], &pcm, sizeof(pcm));
	memcpy(&tx_msg[3], &pno, sizeof(pno));
	memcpy(&tx_msg[5], &number, sizeof(number));

	// Send Message
	if( write( client_socket, tx_msg, 8 ) <= 0) {
		printf("[ERROR] Send in %s()\n", __FUNCTION__);
		return;
	}	
	/*
	if(send(client_socket, tx_msg, 8, 0) <= 0) {
		printf("[ERROR] Send in handleCmdPget()\n");
		return;
	}
	*/
	
	return;
}






#if 0

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
//
#include "define.h"
#include "queue.h"
#include "point_manager.h"
#include "cmd_handler.h"


int client_socket;
unsigned char tx_msg[MAX_BUFFER];
unsigned char rx_msg[MAX_BUFFER];	

void handleCmdPget(int argc, char* argv[]);


#if 0
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
		printf("./pinfo [pcm] [pno] [number]\n");
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
		if(strncmp(argv[0], "pinfo", 5) == 0)
		{
			handleCmdPget(argc, argv);
		}
		else if(strncmp(argv[0], "./pinfo", 7) == 0)
		{
			handleCmdPget(argc, argv);
		}
		else
		{
			printf("Unknown Command. Use following Syntax(%d)\n", argc);
			printf("%s\n", argv[0]);
			printf("./pinfo [pcm] [pno] [number]\n");
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
void 
handleCmdPget(int argc, char* argv[])
/*****************************************************/
{	
	//Variables
	int i;
	short pcm;
	short pno;
	int number;
	int recv_index;
	int recv_length;
	int retry_count;
	int count;
	struct timespec ts;
	point_info point;
	
	//Initialize
	i = 0;
	pcm = 0;
	pno = 0;
	number = 0;
	recv_index = 0;
	recv_length = 0;
	retry_count = 0;
	count = 0;
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; //0.1sec
	memset(&point, 0, sizeof(point));
	//
	memset(tx_msg, 0, sizeof(tx_msg));
	memset(rx_msg, 0, sizeof(rx_msg));
	
	if(argc < 3)
	{
		printf("Incorrect Arguments\n");
		printf("./pinfo [pcm] [pno] [number]\n");
		return;
	}
	
	pcm = atoi(argv[1]);
	pno = atoi(argv[2]);
	if (argc == 4)
		number = atoi(argv[3]);
	else
		number = 0;

	//Check Validation of Arguments
    if(pcm < 0 || pcm >= MAX_NET32_NUMBER)
	{
		printf("Invalid PCM [%d] in handleCmdPget()\n", pcm);
		return;
	}

	if(pno < 0 || pno >= MAX_POINT_NUMBER)
	{
		printf("Invalid PNO [%d] in handleCmdPget()\n", pno);
		return;
	}

	if(number < 0 || (pno + number) > MAX_POINT_NUMBER)
	{
		printf("Invalid Number [%d] in handleCmdPget()\n", number);
		return;
	}

	//Make Send Message
	tx_msg[0] = COMMAND_PGET_MULTI_POINT;
	memcpy(&tx_msg[1], &pcm, sizeof(pcm));
	memcpy(&tx_msg[3], &pno, sizeof(pno));
	memcpy(&tx_msg[5], &number, sizeof(number));

	// Send Message
	if(send(client_socket, tx_msg, 7, 0) <= 0)
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


#endif


