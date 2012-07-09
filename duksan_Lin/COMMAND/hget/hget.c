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

void handleCmdHget(int argc, char* argv[]);

int main(int argc, char* argv[])
{
   //int   client_socket;
   struct sockaddr_un   server_addr;

   client_socket  = socket( PF_FILE, SOCK_STREAM, 0);
   if( -1 == client_socket) {
      printf( "socket 积己 角菩\n");
      exit( 1);
   }

   memset( &server_addr, 0, sizeof( server_addr));
   server_addr.sun_family  = AF_UNIX;
   strcpy( server_addr.sun_path, FILE_SERVER);

   if( -1 == connect( client_socket, (struct sockaddr*)&server_addr, sizeof( server_addr) ) ) {

      printf( "立加 角菩\n");
      exit( 1);
   }

	if(strncmp(argv[0], "hget", 4) == 0) {
		handleCmdHget(argc, argv);
	}
	else if(strncmp(argv[0], "./hget", 6) == 0) {
		handleCmdHget(argc, argv);
	}
	else {
		printf("Unknown Command. Use following Syntax\n");
		printf("./hget [pcm]\n");
	}
	close( client_socket);   
	return SUCCESS;
}


/*****************************************************/
void 
handleCmdHget(int argc, char* argv[])
/*****************************************************/
{
	//Variables
	short pcm;
	int recv_index;
	int recv_length;
	int retry_count;
	struct timespec ts;
	
	//Initialize
	pcm = 0;
	recv_index = 0;
	recv_length = 0;
	retry_count = 0;
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; //0.1sec

	memset(tx_msg, 0, sizeof(tx_msg));
	memset(rx_msg, 0, sizeof(rx_msg));
	
	if(argc != 2) {
		printf("Incorrect arguments\n");
		printf("./hget [pcm]\n");
		return;
	}

	pcm = atoi(argv[1]);

	//Check Validation of Arguments
    if(pcm < 0 || pcm >= MAX_NET32_NUMBER) {
		printf("Invalid PCM [%d] in handleCmdHset()\n", pcm);
		return;
	}

	/* Make Send Message */
	/* | COMMAND_PSET(1byte) | pcm(2byte) | pno(2byte) | fvalue(4byte) | */
	tx_msg[0] = COMMAND_HGET;
	memcpy(&tx_msg[1], &pcm, sizeof(pcm));

	//Send Message
	if( write( client_socket, tx_msg, 3 ) <= 0) {
		printf("[ERROR] Send in handleCmdHget()\n");
		return;
	}	
	return;
}



