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
#include <pthread.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <strings.h>
#include <netdb.h>

typedef struct
{
	unsigned char c_pno;
	unsigned char c_factoryReset;	
	unsigned char c_type;
	unsigned char c_length;
	unsigned int  n_val;
	unsigned char c_chksum;
}__attribute__((packed)) PNT_DEV_MSG_T;

// point-define¿« type	
#define DFN_PNT_VR						0
#define DFN_PNT_DO						1
#define DFN_PNT_VO						2
#define DFN_PNT_DI2S					3
#define DFN_PNT_JPT						4
#define DFN_PNT_VI						5
#define DFN_PNT_CI						6
#define DFN_PNT_ID						7

#define  DEV_FILENAME					"/dev/s3c2410_adc"

int g_nMyPcm = 0;

void Make_Prompt_Shell(int pcm)
// ----------------------------------------------------------------------------
// PROMPT FILE INITIALIZE
{
	FILE *fp = NULL;

	if( (fp = fopen("/etc/rc.d/rc.hostname", "w")) == NULL ) {
		printf("[ERROR] File Open with Option 'r'\n");
		return;		
	}
	else {
		fprintf(fp,"#!/bin/sh\n");
		fprintf(fp,"/bin/hostname \'[PCM%02d]\'\n", pcm);
		fprintf(fp,"chmod 777 / \n");
		fclose(fp);
		printf("Change Prompt Id %d\n", pcm);
	}
}

int main(int argc, char* argv[])
{
	int i = 0;
	int n_fd = -1;
	int n_ret = 0;
	PNT_DEV_MSG_T *p_msg;
	unsigned char c_buf[12];
	
	// open device
	n_fd = open( DEV_FILENAME, O_RDWR );					

	if ( n_fd < 0) {
		// g_nMyPcm number set zero. and prompt change
		g_nMyPcm = 0;
		Make_Prompt_Shell(g_nMyPcm);
		fprintf(stdout, "[ERROR] GPIO Driver open error\n"); 
		fprintf(stdout, "Set Pcm Number = %d\n", g_nMyPcm); 
		fflush(stdout);
		return -1;
	}
	
	// initialize
	memset( c_buf, 0x00, sizeof(c_buf) );
	p_msg = (PNT_DEV_MSG_T *) &c_buf;

	// make message
	p_msg->c_pno = 0;
	p_msg->c_type = DFN_PNT_ID;
	p_msg->c_length = (char)sizeof(PNT_DEV_MSG_T);
	p_msg->c_chksum = 0;
	for( i = 0; i < (p_msg->c_length - 1); i++) {
		p_msg->c_chksum -= c_buf[i];
	}

	// wirte message
	write( n_fd, c_buf, 8 );

	// buffer initialize and read message
	memset( c_buf, 0x00, sizeof(c_buf) );
	n_ret = read( n_fd, c_buf, 8 );

	// set g_nMyPcm number. and prompt change
	g_nMyPcm = p_msg->n_val;
	Make_Prompt_Shell(g_nMyPcm);

	return -1;
}



