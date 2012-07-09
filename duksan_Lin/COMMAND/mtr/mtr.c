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

void handleCmdMtr(int argc, char* argv[]);

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

	if(strncmp(argv[0], "mtr", 3) == 0) {
		handleCmdMtr(argc, argv);
	}
	else {
		printf("Unknown Command. Use following Syntax(%d)\n", argc);
		printf("%s\n", argv[0]);
		printf("mtr niq [ff] \n");
		printf("mtr noq [ff] \n");
	}
	
	close( client_socket);
	return SUCCESS;
}


/*****************************************************/
void handleCmdMtr(int argc, char* argv[])
/*****************************************************/
{
	//Variables
	int i;
	short niq;
	short noq;
	int recv_index;
	int recv_length;
	int retry_count;
	int count;
	struct timespec ts;
	
	//Initialize
	i = 0;
	recv_index = 0;
	recv_length = 0;
	retry_count = 0;
	count = 0;
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; //0.1sec

	memset(tx_msg, 0, sizeof(tx_msg));
	memset(rx_msg, 0, sizeof(rx_msg));
	
	if ( argc < 2 ) {
		printf("Incorrect Arguments\n");
		printf("mtr niq [ff] \n");
		printf("mtr noq [ff] \n");
		return;
	}

	if ( strncmp(argv[1], "niq", 3) == 0 ) {

		if ( argc == 2  ) {
			printf("mtr niq\n");
			niq = 0x00;
		}
		else if ( argc == 3 && (strncmp(argv[2], "ff", 2) == 0) ) {
			printf("mtr niq ff\n");
			niq = 0xff;
		}

		// Make Send Message
		tx_msg[0] = COMMAND_MTR;
		tx_msg[1] = 'I';
		memcpy(&tx_msg[2], &niq, sizeof(niq));

		// Send Message
		if( write( client_socket, tx_msg, 4 ) <= 0) {
			printf("[ERROR] Send in %s()\n", __FUNCTION__);
			return;
		}	
	}
	else if( strncmp(argv[1], "noq", 3) == 0 ) {

		if ( argc == 2  ) {
			printf("mtr noq\n");
			noq = 0x00;
		}
		else if ( argc == 3 && (strncmp(argv[2], "ff", 2) == 0) ) {
			printf("mtr noq ff\n");
			noq = 0xff;
		}

		// Make Send Message
		tx_msg[0] = COMMAND_MTR;
		tx_msg[1] = 'O';
		memcpy(&tx_msg[2], &noq, sizeof(noq));

		// Send Message
		if( write( client_socket, tx_msg, 4 ) <= 0) {
			printf("[ERROR] Send in %s()\n", __FUNCTION__);
			return;
		}	
	}
	return;
}


