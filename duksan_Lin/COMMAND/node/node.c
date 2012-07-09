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

void handleCmdNode(int argc, char* argv[]);

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

	if(strncmp(argv[0], "node", 4) == 0) {
		handleCmdNode(argc, argv);
	}
	else {
		printf("Unknown Command. Use following Syntax(%d)\n", argc);
		printf("node \n");
	}
	
	close( client_socket);
	return SUCCESS;
}


/*****************************************************/
void handleCmdNode(int argc, char* argv[])
/*****************************************************/
{
	memset(tx_msg, 0, sizeof(tx_msg));
	
	if ( argc < 1 ) {
		printf("Incorrect Arguments\n");
		printf("node \n");
		return;
	}

	// Make Send Message
	tx_msg[0] = COMMAND_NODE;
	tx_msg[1] = 'N';

	// Send Message
	if( write( client_socket, tx_msg, 2 ) <= 0) {
		printf("[ERROR] Send in %s()\n", __FUNCTION__);
		return;
	}	

	return;
}


