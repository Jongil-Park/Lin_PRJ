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
#include <sys/time.h>										// gettimeofday();
#include <time.h>
#include <sys/un.h>
#include <sys/poll.h>										// use poll event
#include <sys/ipc.h>
#include <sys/shm.h>

int 	g_nSocket;
unsigned char 	*g_pchTxMsg;
unsigned char 	*g_pchRxMsg;

unsigned short	g_nInvokeId;						// plc에서 message를 체크하기 위한 index id



// share memory variable
int   shm_id;
void *shm_addr;

// share memory
#define  	KEY_NUM    	 			9527			// share memory key
#define  	SHARE_MEM_SIZE    		32786			// point table size

#define 	MAX_NET32_NUMBER		32
#define		MAX_POINT_NUMBER		256

#define		XGT_PLC_1				1
#define		XGT_PLC_2				2
#define		XGT_PLC_3				3
#define		XGT_PLC_M				4

// point table
float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];		// point-table only value


struct XGT_HEADER {
	char company[8];
	unsigned short reserved;
	unsigned short info;
	unsigned char cpu;
	unsigned char farme;
	unsigned short invoke;
	unsigned short length;
	unsigned char postion;
	unsigned char reserved2;
}__attribute__ ((packed)) ;

void xgt_copy_ptbl(void)
{
	memcpy((float *)g_fExPtbl, (float *)shm_addr, SHARE_MEM_SIZE );		// 공유메모리 복사
	printf("xgt copy share memory\n");
}


int xgt_init_sharememory(void)
{
	// share memory init
	if ( -1 == ( shm_id = shmget( (key_t)KEY_NUM, SHARE_MEM_SIZE, IPC_CREAT|0666))) {
		fprintf(stdout, "XGT Share Memory Initialize \t\t\t [  FAIL  ]\n");
		fflush(stdout);	
		return -1;
	} 
	
	if ( ( void *)-1 == ( shm_addr = shmat( shm_id, ( void *)0, 0))) {
		fprintf(stdout, "XGT Share Memory Add \t\t\t\t [  FAIL  ]\n");
		fflush(stdout);	
		return -1;
	} 	
	printf("xgt init share memory\n");
	return 1;
}


int xgt_connect_plc(int nPlcSelect) 
{
	
	
	
	return 1;	
}

void xgt_make_header(struct XGT_HEADER *pHeader) 
{
	memcpy(pHeader->company, "LSIS-XGT", 8);
	pHeader->reserved = 0x0000;
	pHeader->info = 0x0000;
	pHeader->cpu = 0xa0;
	pHeader->farme = 0x33;
	pHeader->invoke = g_nInvokeId;
	pHeader->length = 0x0000;		// application instruction length
	pHeader->postion = 0x00;
	pHeader->reserved2 = 0x00;
}


void xgt_make_message(void) 
{
	
}

int xgt_read_aio(int nPlcSelect) 
{
	int nLength = 0;
	struct XGT_HEADER *pHeader = (struct XGT_HEADER *)g_pchTxMsg;
	
	xgt_make_header(pHeader);				// make header message
	//nLength = xgt_make_message();
	
	return 1;	
}



int xgt_read_dio(int nPlcSelect) 
{
	
	
	
	return 1;	
}


int xgt_close_plc(int nPlcSelect) 
{
	
	
	
	return 1;	
}


int main(int argc, char *argv[]) 
{
	int       length, socket_size, index, number;
	fd_set    read_fds;
	int nShareRtn = 0;						// share memory를 초기화 리턴값.
	int nPlcSelect = 0;						// 통신하고자 하는 PLC를 선택한다. 
	
	printf("xgt plc interface\n");

	nShareRtn = xgt_init_sharememory();		
	if ( nShareRtn < 0 )
		return -1;

	// initialize
	nPlcSelect = XGT_PLC_1;
	g_nInvokeId = 0;
	g_pchTxMsg = (unsigned char *)malloc (1024);
	g_pchRxMsg = (unsigned char *)malloc (1024);
		
	while(1) {
		xgt_copy_ptbl();					// copy point table
		sleep(1);
		
		switch(nPlcSelect) {
			case XGT_PLC_1:
				xgt_connect_plc(nPlcSelect);
				xgt_read_aio(nPlcSelect);
				xgt_read_dio(nPlcSelect);
				xgt_close_plc(nPlcSelect);
				nPlcSelect = XGT_PLC_2;
				break;
				
			case XGT_PLC_2:
				xgt_connect_plc(nPlcSelect);
				xgt_read_aio(nPlcSelect);
				xgt_read_dio(nPlcSelect);
				xgt_close_plc(nPlcSelect);
				nPlcSelect = XGT_PLC_3;
				break;

			case XGT_PLC_3:
				xgt_connect_plc(nPlcSelect);
				xgt_read_aio(nPlcSelect);
				xgt_read_dio(nPlcSelect);
				xgt_close_plc(nPlcSelect);
				nPlcSelect = XGT_PLC_M;
				break;		

			case XGT_PLC_M:
				xgt_connect_plc(nPlcSelect);
				xgt_read_aio(nPlcSelect);
				xgt_read_dio(nPlcSelect);
				xgt_close_plc(nPlcSelect);
				nPlcSelect = XGT_PLC_1;
				break;										
		}
	}
}



