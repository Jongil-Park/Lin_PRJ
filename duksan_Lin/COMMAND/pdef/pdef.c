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

	if ( strncmp( argv[0], "pdef", 4 ) == 0 ) {
		cmd_handler(argc, argv);
	}
	else {
		fprintf(stderr, "Use following Syntax\n");
		fprintf(stderr, "type = VR, DO, VO, DI2S, JPT, VI, CI\n");
		fprintf(stderr, "pdef [pcm] [pno] [type] [hyst] [scale] [offset] [min] [max]\n");
		fflush(stderr);
	}
	close(client_socket);   
	return SUCCESS;
}



void cmd_handler(int argc, char* argv[])
{
	//int i = 0;
	unsigned short w_pcm = 0;
	unsigned short w_pno = 0;
	unsigned short w_type = 0;
	float f_hyst = 0;
	float f_min = 0;
	float f_max = 0;
	float f_scale = 0;
	float f_offset = 0;
	
	memset(tx_msg, 0, sizeof(tx_msg));

	// Make Send Message, and Send Message
	if ( argc == 3 ) {
		w_pcm = (unsigned short) atoi(argv[1]);
		w_pno = (unsigned short) atoi(argv[2]);

		tx_msg[0] = COMMAND_PDEF;
		memcpy(&tx_msg[1], &w_pcm, sizeof(w_pcm));
		memcpy(&tx_msg[3], &w_pno, sizeof(w_pno));

		if( write( client_socket, tx_msg, 5 ) <= 0) {
			fprintf(stderr,"[ERROR] Send in pdef command\n");
			fflush(stderr);
			return;
		}
		return;
	}

	if ( argc == 9 ) {
		w_pcm = (unsigned short) atoi(argv[1]);
		w_pno = (unsigned short) atoi(argv[2]);

		if ( strncmp( argv[3], "vr", 2 ) == 0 ) {
			w_type = DFN_PNT_VR;
		}
		else if ( strncmp( argv[3], "VR", 2 ) == 0 ) {
			w_type = DFN_PNT_VR;
		}
		else if ( strncmp( argv[3], "do", 2 ) == 0 ) {
			w_type = DFN_PNT_DO;
		}
		else if ( strncmp( argv[3], "DO", 2 ) == 0 ) {
			w_type = DFN_PNT_DO;
		}
		else if ( strncmp( argv[3], "vo", 2 ) == 0 ) {
			w_type = DFN_PNT_VO;
		}
		else if ( strncmp( argv[3], "VO", 2 ) == 0 ) {
			w_type = DFN_PNT_VO;
		}
		else if ( strncmp( argv[3], "di2s", 4 ) == 0 ) {
			w_type = DFN_PNT_DI2S;
		}
		else if ( strncmp( argv[3], "DI2S", 4 ) == 0 ) {
			w_type = DFN_PNT_DI2S;
		}
		else if ( strncmp( argv[3], "jpt", 3 ) == 0 ) {
			w_type = DFN_PNT_JPT;
		}
		else if ( strncmp( argv[3], "JPT", 3 ) == 0 ) {
			w_type = DFN_PNT_JPT;
		}
		else if ( strncmp( argv[3], "vi", 2 ) == 0 ) {
			w_type = DFN_PNT_VI;
		}
		else if ( strncmp( argv[3], "VI", 2 ) == 0 ) {
			w_type = DFN_PNT_VI;
		}
		else if ( strncmp( argv[3], "ci", 2 ) == 0 ) {
			w_type = DFN_PNT_CI;
		}
		else if ( strncmp( argv[3], "CI", 2 ) == 0 ) {
			w_type = DFN_PNT_CI;
		}

		f_hyst = atof(argv[4]);
		f_scale = atof(argv[5]);
		f_offset = atof(argv[6]);
		f_min = atof(argv[7]);
		f_max = atof(argv[8]);

		tx_msg[0] = COMMAND_PDEF;
		memcpy(&tx_msg[1], &w_pcm, sizeof(w_pcm));
		memcpy(&tx_msg[3], &w_pno, sizeof(w_pno));
		memcpy(&tx_msg[5], &w_type, sizeof(w_type));
		memcpy(&tx_msg[7], &f_hyst, sizeof(f_hyst));
		memcpy(&tx_msg[11], &f_scale, sizeof(f_scale));
		memcpy(&tx_msg[15], &f_offset, sizeof(f_offset));
		memcpy(&tx_msg[19], &f_min, sizeof(f_min));
		memcpy(&tx_msg[23], &f_max, sizeof(f_max));
		
		if( write( client_socket, tx_msg, 27 ) <= 0) {
			fprintf(stderr,"[ERROR] Send in pdef command\n");
			fflush(stderr);
			return;
		}

		//for ( i = 0; i < 27; i++ ) 
		//	fprintf( stderr, "0x%x ", tx_msg[i] );
		//fprintf( stderr, "\n");
		//fflush(stderr);

		return;
	}

	fprintf(stderr, "Use following Syntax\n");
	fprintf(stderr, "type = VR, DO, VO, DI2S, JPT, VI, CI\n");
	fprintf(stderr, "pdef [pcm] [pno] [type] [hyst] [scale] [offset] [min] [max]\n");
	fflush(stderr);
	
	return;
}



