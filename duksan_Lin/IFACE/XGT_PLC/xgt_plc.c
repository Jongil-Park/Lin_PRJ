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

#include "api.h"
#include "xgt_plc.h"

unsigned char 	g_chTxMsg[XGT_BUFFER_SIZE];
unsigned char 	g_chRxMsg[XGT_BUFFER_SIZE];

unsigned short	g_nInvokeId;								// plc에서 message를 체크하기 위한 index id

#include "plc_1.c"
#include "plc_2.c"
#include "plc_3.c"
#include "plc_m.c"


int g_nPlcSelect = 0;										// plc index


//-----------------------------------------------------------------------------
int xgt_connect_plc(char *pIp)
//-----------------------------------------------------------------------------
{
	int nSocket;
	struct sockaddr_in server_addr;
	struct timeval timeo;
	int res; 
	
#ifdef ACCEPT_NONBLOCKING
	long arg; 		
	fd_set myset; 
	struct timeval tv; 
	int valopt; 
	socklen_t lon;
#endif
	
	memset( &server_addr, 0, sizeof(server_addr) );
	timeo.tv_sec = 3;
	timeo.tv_usec = 0;
	
	nSocket = socket( AF_INET, SOCK_STREAM, 0 );
	if( nSocket < 0 ) {
		fprintf( stdout, "+ Socket creation error\n" );
		fflush( stdout );
		close( nSocket);
		return -1;					
	}	
	
	setsockopt( nSocket, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo) );

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr( pIp );
	server_addr.sin_port = htons( XGT_PLC_PORT );

	fprintf( stdout, "try connect %s\n", plc_1_ip );
	fflush( stdout );	


#ifdef ACCEPT_NONBLOCKING
	// Set non-blocking 
	// 출처:[LINUX] linux에서 network 접속시 non-block으로 연결하기..
	if( (arg = fcntl(nSocket, F_GETFL, NULL)) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( nSocket);
		return -1;
	} 
	
	arg |= O_NONBLOCK; 
	if( fcntl(nSocket, F_SETFL, arg) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( nSocket);
		return -1;
	}

	// Trying to connect with timeout 
	res =  connect(nSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
	if (res < 0) { 
		if (errno == EINPROGRESS) { 
			fprintf(stdout, "ELBA Progress in connect() - selecting\n"); 
			fflush( stdout );
			do { 
				tv.tv_sec = 5;  // 5초후에 빠져나오도록 한다.
				tv.tv_usec = 0; 
				FD_ZERO(&myset); 
				FD_SET(nSocket, &myset); 
				res = select(nSocket+1, NULL, &myset, NULL, &tv); 
				if (res < 0 && errno != EINTR) { 
					//fprintf(stdout, "Elba Error connecting %d - %s\n", errno, strerror(errno)); 
					//fflush( stdout );
					close( nSocket );
					return -1; 
				} 
				else if (res > 0) { 
					// Socket selected for write 
					lon = sizeof(int); 
					if (getsockopt(nSocket, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
						//fprintf(stdout, "Elba Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
						//fflush( stdout );
						close( nSocket );
						return -1;
					} 
					// Check the value returned... 
					if (valopt) { 
						fprintf( stdout, "[ELBA] connection() %d - %s\n", valopt, strerror(valopt)); 
						fflush( stdout );							
						close( nSocket );
						return -1;
					} 
					break; 
				} 
				else { 
					//fprintf(stdout, "Elba Timeout in select() - Cancelling!\n"); 
					//fflush( stdout );
					close( nSocket);
					return -1;
				} 
			} while (1); 
		} 
		else { 
			//fprintf(stdout, "Error connecting %d - %s\n", errno, strerror(errno)); 
			//fflush( stdout );
			close( nSocket );
			return -1;
		} 
	} 

	// Set to blocking mode again... 
	if( (arg = fcntl(nSocket, F_GETFL, NULL)) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( nSocket);
		return -1; 
	} 
	
	arg &= (~O_NONBLOCK); 
	if( fcntl(nSocket, F_SETFL, arg) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( nSocket );
		return -1; 
	}
#else
	res =  connect( nSocket, (struct sockaddr *)&server_addr, sizeof(server_addr) ); 
	if (res < 0) { 
		return -1;
	}	
#endif
	
	return nSocket;
}


//-----------------------------------------------------------------------------
void xgt_close_plc(int nSocket)
//-----------------------------------------------------------------------------
{
	close (nSocket);
	
	printf("close xgt plc socket %d\n", nSocket);
}


//-----------------------------------------------------------------------------
void xgt_make_header(struct XGT_HEADER *pHeader) 
//-----------------------------------------------------------------------------
{
	memcpy(&pHeader->company[0], "LSIS-XGT", 8);
	pHeader->reserved = 0x0000;
	pHeader->info = 0x0000;
	pHeader->cpu = 0xa0;
	pHeader->farme = 0x33;
	pHeader->invoke = g_nInvokeId;
	pHeader->length = 0x0012;		// application instruction length
	pHeader->postion = 0x00;
	pHeader->bcc = 0x00;			// application instruction chksum 
}


//-----------------------------------------------------------------------------
int xgt_read_aio(int nSocket, unsigned int nAddr) 
//-----------------------------------------------------------------------------
{
	int i = 0;
	int nIndex = 0;
	unsigned char chBcc = 0;				// application instruction chksum 
	
	struct XGT_HEADER *pHeader = (struct XGT_HEADER *)g_chTxMsg;
	
	printf("Read AIO nAddr = %d\n", nAddr); 	
	
	xgt_make_header(pHeader);				// make header message
	
	// make command (read request : 0x0054)
	nIndex = sizeof(struct XGT_HEADER);
	g_chTxMsg[nIndex] = 0x00;
	chBcc += g_chTxMsg[nIndex++];
	g_chTxMsg[nIndex] = 0x54;
	chBcc += g_chTxMsg[nIndex++];
	
	// make type (dword type :  0x0003)
	g_chTxMsg[nIndex] = 0x00;
	chBcc += g_chTxMsg[nIndex++];
	g_chTxMsg[nIndex] = 0x03;	
	chBcc += g_chTxMsg[nIndex++];
	
	// don't care
	g_chTxMsg[nIndex] = 0x00;
	chBcc += g_chTxMsg[nIndex++];
	g_chTxMsg[nIndex] = 0x00;	
	chBcc += g_chTxMsg[nIndex++];	
	
	// block
	g_chTxMsg[nIndex] = 0x00;
	chBcc += g_chTxMsg[nIndex++];
	g_chTxMsg[nIndex] = 0x01;	
	chBcc += g_chTxMsg[nIndex++];	
		
	// variable name length
	g_chTxMsg[nIndex] = 0x00;
	chBcc += g_chTxMsg[nIndex++];	
	g_chTxMsg[nIndex] = 0x06;			
	chBcc += g_chTxMsg[nIndex++];	

	// variable name 
	g_chTxMsg[nIndex] = '%';
	chBcc += g_chTxMsg[nIndex++];	
	g_chTxMsg[nIndex] = 'M';
	chBcc += g_chTxMsg[nIndex++];	
	g_chTxMsg[nIndex] = 'X';
	chBcc += g_chTxMsg[nIndex++];	
	g_chTxMsg[nIndex] = '0' + (nAddr / 100);
	chBcc += g_chTxMsg[nIndex++];	
	g_chTxMsg[nIndex] = '0' + ((nAddr / 10) % 10);
	chBcc += g_chTxMsg[nIndex++];	
	g_chTxMsg[nIndex] = '0' + (nAddr % 10);
	chBcc += g_chTxMsg[nIndex++];	

	// variable name 
	g_chTxMsg[nIndex] = 0x00;
	chBcc += g_chTxMsg[nIndex++];	
	g_chTxMsg[nIndex] = 0x32;				// 한번에 50개의 data를 읽어온다. 
	chBcc += g_chTxMsg[nIndex++];	

	// set bcc
	pHeader->bcc = chBcc;
	
	// debug message
	printf("xgt tx : ");
	for ( i = 0; i < nIndex; i++ ) {
		printf("%x ", g_chTxMsg[i]);
	}		
	printf("\n");

	send(nSocket, g_chTxMsg, nIndex, 0);
	
	return 1;	
}


//-----------------------------------------------------------------------------
int xgt_read_dio(int nSocket, unsigned int nAddr) 
//-----------------------------------------------------------------------------
{
	//int i = 0;
	//unsigned char chBcc = 0;				// application instruction chksum 
	//int nLength = 0;
	
	struct XGT_HEADER *pHeader = (struct XGT_HEADER *)g_chTxMsg;
	
	printf("Read DIO nAddr = %d\n", nAddr); 	
	
	xgt_make_header(pHeader);				// make header message
	
	return 1;		
}


//-----------------------------------------------------------------------------
void PointDefine(void)
//-----------------------------------------------------------------------------
{
	
}


//-----------------------------------------------------------------------------
void ApgInit(void)
//-----------------------------------------------------------------------------
{
	printf("xgt plc interface\n");
}


//-----------------------------------------------------------------------------
void SecondTimer(void)
//-----------------------------------------------------------------------------
{
	switch(g_nPlcSelect) {
		case XGT_PLC_1:
			xgt_plc_1_process();		
			g_nPlcSelect = XGT_PLC_2;
			break;

		default :									
			g_nPlcSelect = XGT_PLC_1;
			break;					
	}
}


//-----------------------------------------------------------------------------
void MinuteTimer(void)
//-----------------------------------------------------------------------------
{
}


//-----------------------------------------------------------------------------
void ApgMain(void)
//-----------------------------------------------------------------------------
{
	static char sec = 0, minute =0;  

	if ( sec == _sec )
		return;
	sec = _sec;
	SecondTimer();
	
	if ( minute == _minute )
		return;
	minute = _minute;
	MinuteTimer();
}



#if 0
// share memory variable
int   shm_id;												// share memory id
void *shm_addr;												// share memory address

float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];		// point-table read only value

void xgt_copy_ptbl(void)
//-----------------------------------------------------------------------------
{
	memcpy((float *)g_fExPtbl, (float *)shm_addr, SHARE_MEM_SIZE );		// 공유메모리 복사
	printf("xgt copy share memory\n");
}


int xgt_init_sharememory(void)
//-----------------------------------------------------------------------------
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


int main(int argc, char *argv[]) 
//-----------------------------------------------------------------------------
{
	int i = 0;
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
		
	memset(g_chTxMsg, 0x00, XGT_BUFFER_SIZE);
	memset(g_chRxMsg, 0x00, XGT_BUFFER_SIZE);
			
	g_chTxMsg[0] = 1;
			
	while(1) {
		xgt_copy_ptbl();					// copy point table
		sleep(1);
		switch(nPlcSelect) {
			case XGT_PLC_1:
				xgt_plc_1_process();		
				nPlcSelect = XGT_PLC_2;
				break;
// 현장에서 체크한 후에 주석처리를 제거합니다. 
/*				
			case XGT_PLC_2:
				xgt_plc_2_process();		
				nPlcSelect = XGT_PLC_3;
				break;
				
			case XGT_PLC_3:
				xgt_plc_3_process();		
				nPlcSelect = XGT_PLC_M;
				break;
				
			case XGT_PLC_M:
				xgt_plc_m_process();		
				nPlcSelect = XGT_PLC_1;
				break;
*/				
			default :									
				nPlcSelect = XGT_PLC_1;
				break;					
		}
	}
}

#endif
