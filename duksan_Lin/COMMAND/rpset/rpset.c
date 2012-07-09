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
      fprintf(stdout,  "[Error] Rpset Can't creat socket\n");
	  fflush(stdout);
      exit( 1);
   }

   memset( &server_addr, 0, sizeof( server_addr));
   server_addr.sun_family  = AF_UNIX;
   strcpy( server_addr.sun_path, FILE_SERVER);

   if ( -1 == connect( client_socket, (struct sockaddr*)&server_addr, sizeof( server_addr) ) ) {
      fprintf(stdout, "[Error] Rpset Can't connect socket\n");
	  fflush(stdout);
      exit( 1);
   }

	if ( strncmp( argv[0], "rpset", 5 ) == 0 ) {
		cmd_handler(argc, argv);
	}
	else {
		fprintf(stdout, "[Error] Unknown Command. Use following Syntax\n");
		fprintf(stdout, "rpset [remote Id] [pcm] [pno] [value]\n");
		fflush(stdout);
	}
	close(client_socket);   
	return SUCCESS;
}


/*****************************************************/
void cmd_handler(int argc, char* argv[])
/*****************************************************/
{
	//Variables
	unsigned short wRemoteId = 0;
	unsigned short wPcm = 0;
	unsigned short wPno = 0;
	unsigned short wValue = 0;
	
	memset(tx_msg, 0, sizeof(tx_msg));
	
	if(argc != 5) {
		fprintf(stdout, "[Error] Incorrect arguments\n");
		fprintf(stdout, "rpset [remote Id] [pcm] [pno] [value]\n");
		fflush(stdout);
		return;
	}

	wRemoteId = (unsigned short)atoi(argv[1]);
	wPcm = (unsigned short)atoi(argv[2]);
	wPno = (unsigned short)atoi(argv[3]);
	wValue = (unsigned short)atoi(argv[4]);
	
	if ( wRemoteId >= 16 ) {
		fprintf(stdout, "[Error] Incorrect arguments\n");
		fprintf(stdout, "RemoteId too big\n");
		fflush(stdout);
		return;
	}

	if ( wPcm >= 32 ) {
		fprintf(stdout, "[Error] Incorrect arguments\n");
		fprintf(stdout, "Pcm number too big\n");
		fflush(stdout);
		return;
	}

	if ( wPno >= 64 ) {
		fprintf(stdout, "[Error] Incorrect arguments\n");
		fprintf(stdout, "pno number too big\n");
		fflush(stdout);
		return;
	}

	/*
	fprintf(stdout, ">> RPSET : %d, %d, %d, %d \n", 
			wRemoteId, wPcm, wPno, wValue);
	fflush(stdout);
	*/

	// Make Send Message, and Send Message
	tx_msg[0] = COMMAND_RPSET;
	memcpy(&tx_msg[1], &wRemoteId, sizeof(wRemoteId));
	memcpy(&tx_msg[3], &wPcm, sizeof(wPcm));
	memcpy(&tx_msg[5], &wPno, sizeof(wPno));
	memcpy(&tx_msg[7], &wValue, sizeof(wValue));
	
	if( write( client_socket, tx_msg, 9 ) <= 0) {
		fprintf(stdout,"[Error] RPSET Can't send message\n");
		fflush(stdout);
		return;
	}

	return;
}



