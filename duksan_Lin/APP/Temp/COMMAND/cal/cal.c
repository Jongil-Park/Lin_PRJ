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
unsigned char tx_msg[32];

int main(int argc, char* argv[]);
void cmd_handler(int argc, char *argv[]);
void cmd_save(int argc, char *argv[]);
void cmd_init(int argc, char *argv[]);
void cmd_load(int argc, char *argv[]);
void cmd_zero(int argc, char *argv[]);
void cmd_grp(int argc, char *argv[]);
void cmd_display(int argc, char *argv[]);
void cmd_setvalue(int argc, char* argv[]);

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

	if ( strncmp(argv[0], "cal", 3 ) == 0) {
		cmd_handler(argc, argv);		
	}			
	
	close( client_socket);
	return SUCCESS;
}


void cmd_handler(int argc, char* argv[])
{	
	if ( argc == 1 ) {
		cmd_display(argc, argv);
	}
	else if ( argc == 2 ) {
		if ( strncmp(argv[1], "load", 4 ) == 0) {
			cmd_load(argc, argv);		
		}			
		else if ( strncmp(argv[1], "init", 4 ) == 0) {
			cmd_init(argc, argv);		
		}			
		else if ( strncmp(argv[1], "save", 4 ) == 0) {
			cmd_save(argc, argv);		
		}		
		else if ( strncmp(argv[1], "zero", 4 ) == 0) {
			cmd_zero(argc, argv);		
		}		
	}
	else if ( argc == 3 ) {
		cmd_grp(argc, argv);
	}
	else if ( argc == 11 ) {
		cmd_setvalue(argc, argv);
	}
}


void cmd_display(int argc, char* argv[])
// message : || CMD(1) || 'C' || 'A' || 'L' || 'D' ||
{
	memset(tx_msg, 0, sizeof(tx_msg));

	tx_msg[0] = COMMAND_CAL;
	tx_msg[1] = 'C';
	tx_msg[2] = 'A';
	tx_msg[3] = 'L';
	tx_msg[4] = 'D';

	// Send Message
	if( write( client_socket, tx_msg, 5 ) <= 0) {
		printf("[ERROR] Send in %s()\n", __FUNCTION__);
		return;
	}	

	return;	
}


void cmd_grp(int argc, char* argv[])
// message : || CMD(1) || 'C' || 'A' || 'L' || grp || [type] ||
{
	char ch_buf;
	memset(tx_msg, 0, sizeof(tx_msg));

	tx_msg[0] = COMMAND_CAL;
	tx_msg[1] = 'C';
	tx_msg[2] = 'A';
	tx_msg[3] = 'L';
	ch_buf = (char)argv[1][1];
	//printf("%s\n", argv[1]);
	//printf("%c\n", ch_buf);
	tx_msg[4] = ch_buf - 0x30;		//atoi(ch_buf);

	if ( strncmp(argv[2], "vi", 2 ) == 0) {
		tx_msg[5] = CMD_CAL_TYPE_VI;
	}		
	else if ( strncmp(argv[2], "ci", 2 ) == 0) {
		tx_msg[5] = CMD_CAL_TYPE_CI;
	}		
	else if ( strncmp(argv[2], "jpt", 3 ) == 0) {
		tx_msg[5] = CMD_CAL_TYPE_JPT;
	}		
	else if ( strncmp(argv[2], "vo", 2 ) == 0) {
		tx_msg[5] = CMD_CAL_TYPE_VO;
	}		

	// Send Message
	if( write( client_socket, tx_msg, 6 ) <= 0) {
		printf("[ERROR] Send in %s()\n", __FUNCTION__);
		return;
	}	

	return;	
}


void cmd_load(int argc, char* argv[])
// message : || CMD(1) || 'C' || 'A' || 'L' || 'L' ||
{
	memset(tx_msg, 0, sizeof(tx_msg));

	tx_msg[0] = COMMAND_CAL;
	tx_msg[1] = 'C';
	tx_msg[2] = 'A';
	tx_msg[3] = 'L';
	tx_msg[4] = 'L';

	// Send Message
	if( write( client_socket, tx_msg, 5 ) <= 0) {
		printf("[ERROR] Send in %s()\n", __FUNCTION__);
		return;
	}	

	return;	
}


void cmd_init(int argc, char* argv[])
// message : || CMD(1) || 'C' || 'A' || 'L' || 'I' ||
{
	memset(tx_msg, 0, sizeof(tx_msg));

	tx_msg[0] = COMMAND_CAL;
	tx_msg[1] = 'C';
	tx_msg[2] = 'A';
	tx_msg[3] = 'L';
	tx_msg[4] = 'I';

	// Send Message
	if( write( client_socket, tx_msg, 5 ) <= 0) {
		printf("[ERROR] Send in %s()\n", __FUNCTION__);
		return;
	}	

	return;	
}


void cmd_save(int argc, char* argv[])
// message : || CMD(1) || 'C' || 'A' || 'L' || 'S' ||
{
	memset(tx_msg, 0, sizeof(tx_msg));

	tx_msg[0] = COMMAND_CAL;
	tx_msg[1] = 'C';
	tx_msg[2] = 'A';
	tx_msg[3] = 'L';
	tx_msg[4] = 'S';

	// Send Message
	if( write( client_socket, tx_msg, 5 ) <= 0) {
		printf("[ERROR] Send in %s()\n", __FUNCTION__);
		return;
	}	

	return;	
}

void cmd_zero(int argc, char* argv[])
// message : || CMD(1) || 'C' || 'A' || 'L' || 'Z' ||
{
	memset(tx_msg, 0, sizeof(tx_msg));

	tx_msg[0] = COMMAND_CAL;
	tx_msg[1] = 'C';
	tx_msg[2] = 'A';
	tx_msg[3] = 'L';
	tx_msg[4] = 'Z';

	// Send Message
	if( write( client_socket, tx_msg, 5 ) <= 0) {
		printf("[ERROR] Send in %s()\n", __FUNCTION__);
		return;
	}	

	return;	
}


void cmd_setvalue(int argc, char* argv[])
// message : || CMD(1) || 'C' || 'A' || 'L' || 'S' || grp || [type] || [val_1] || ...|| [val_8] ||
{
	char ch_buf;
	memset(tx_msg, 0, sizeof(tx_msg));

	tx_msg[0] = COMMAND_CAL;
	tx_msg[1] = 'C';
	tx_msg[2] = 'A';
	tx_msg[3] = 'L';
	ch_buf = (char)argv[1][1];
	tx_msg[4] = ch_buf - 0x30;		//atoi(ch_buf);

	// make type
	if ( strncmp(argv[2], "jpt_hi", 6 ) == 0) {
		tx_msg[5] = CMD_CAL_TYPE_JPT_HI;
		tx_msg[6] = atoi(argv[3]) - 1400;
		tx_msg[7] = atoi(argv[4]) - 1400;
		tx_msg[8] = atoi(argv[5]) - 1400;
		tx_msg[9] = atoi(argv[6]) - 1400;
		tx_msg[10] = atoi(argv[7]) - 1400;
		tx_msg[11] = atoi(argv[8]) - 1400;
		tx_msg[12] = atoi(argv[9]) - 1400;
		tx_msg[13] = atoi(argv[10]) - 1400;
	}	
	else if ( strncmp(argv[2], "jpt_lo", 6 ) == 0) {
		tx_msg[5] = CMD_CAL_TYPE_JPT_LO;
		tx_msg[6] = atoi(argv[3]) - 800;
		tx_msg[7] = atoi(argv[4]) - 800;
		tx_msg[8] = atoi(argv[5]) - 800;
		tx_msg[9] = atoi(argv[6]) - 800;
		tx_msg[10] = atoi(argv[7]) - 800;
		tx_msg[11] = atoi(argv[8]) - 800;
		tx_msg[12] = atoi(argv[9]) - 800;
		tx_msg[13] = atoi(argv[10]) - 800;
	}			
	else if ( strncmp(argv[2], "vo", 2 ) == 0) {
		tx_msg[5] = CMD_CAL_TYPE_VO;
		tx_msg[6] = atoi(argv[3]);
		tx_msg[7] = atoi(argv[4]);
		tx_msg[8] = atoi(argv[5]);
		tx_msg[9] = atoi(argv[6]);
		tx_msg[10] = atoi(argv[7]);
		tx_msg[11] = atoi(argv[8]);
		tx_msg[12] = atoi(argv[9]);
		tx_msg[13] = atoi(argv[10]);
	}		
	else 
		return;



	// Send Message
	if( write( client_socket, tx_msg, 14 ) <= 0) {
		printf("[ERROR] Send in %s()\n", __FUNCTION__);
		return;
	}	

	return;	
}



