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

#include "define.h"
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

	if ( strncmp( argv[0], "apset", 5 ) == 0 ) {
		cmd_handler(argc, argv);
	}
	else {
		fprintf(stderr, "Unknown Command. Use following Syntax\n");
		fprintf(stderr, "./pset [pcm] [pno] [value]\n");
		fflush(stderr);
	}
	close(client_socket);   
	return SUCCESS;
}


/*****************************************************/
void cmd_handler(int argc, char* argv[])
/*****************************************************/
{
	//Variables
	unsigned short w_pcm = 0;
	unsigned short w_pno = 0;
	float f_value = 0;
	
	memset(tx_msg, 0, sizeof(tx_msg));
	
	if(argc != 2) {
		fprintf(stderr, "Incorrect arguments\n");
		fprintf(stderr, "./apset [value]\n");
		fflush(stderr);
		return;
	}

	f_value = atof(argv[1]);
	//fprintf(stderr, "f_value %f\n", f_value);
	//fflush(stderr);
	
	// Make Send Message, and Send Message
	tx_msg[0] = COMMAND_APSET;
	memcpy(&tx_msg[1], &w_pcm, sizeof(w_pcm));
	memcpy(&tx_msg[3], &w_pno, sizeof(w_pno));
	memcpy(&tx_msg[5], &f_value, sizeof(f_value));
	
	if( write( client_socket, tx_msg, 9 ) <= 0) {
		fprintf(stderr,"[ERROR] Send in handleCmdPset()\n");
		fflush(stderr);
		return;
	}
	return;
}



