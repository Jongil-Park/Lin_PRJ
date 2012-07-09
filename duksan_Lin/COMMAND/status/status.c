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

void cmd_handler(int argc, char* argv[]);

int main(int argc, char* argv[])
{
   struct sockaddr_un   server_addr;

   client_socket  = socket( PF_FILE, SOCK_STREAM, 0);
   if ( -1 == client_socket) {
      fprintf(stderr,  "socket 积己 角菩\n");
	  fflush(stderr);
      exit( 1);
   }

   memset( &server_addr, 0, sizeof( server_addr));
   server_addr.sun_family  = AF_UNIX;
   strcpy( server_addr.sun_path, FILE_SERVER);

   if ( -1 == connect( client_socket, (struct sockaddr*)&server_addr, sizeof( server_addr) ) ) {
      fprintf(stderr, "立加 角菩\n");
	  fflush(stderr);
      exit( 1);
   }

	if ( strncmp( argv[0], "status", 6 ) == 0 ) {
		cmd_handler(argc, argv);
	}
	else {
		fprintf(stderr, "Unknown Command. Use following Syntax\n");
		fprintf(stderr, "status\n");
		fflush(stderr);
	}
	close(client_socket);   
	return SUCCESS;
}


/*****************************************************/
void cmd_handler(int argc, char* argv[])
/*****************************************************/
{
	int n_cnt = 0;

	memset(tx_msg, 0, sizeof(tx_msg));

	// Make Send Message, and Send Message
	tx_msg[n_cnt++] = COMMAND_STATUS;
	tx_msg[n_cnt++] = 'S';
	tx_msg[n_cnt++] = 'T';
	tx_msg[n_cnt++] = 'A';
	tx_msg[n_cnt++] = 'T';
	tx_msg[n_cnt++] = 'U';
	tx_msg[n_cnt++] = 'S';
	
	if( write( client_socket, tx_msg, n_cnt ) <= 0) {
		fprintf(stderr,"[ERROR] Send in handleCmdPset()\n");
		fflush(stderr);
		return;
	}

	return;
}


