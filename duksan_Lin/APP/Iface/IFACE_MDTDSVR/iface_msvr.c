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
#include <sys/poll.h>		// use poll event

/****************************************************************/
#include "define.h"
#include "queue_handler.h"
#include "crc16.h"
#include "iface_msvr.h"

#define UART2DEVICE 					"/dev/s3c2410_serial2"
#define MAX_BUF_SIZE					2048

#define     MSVR_CMD_SET            'S'
#define     MSVR_CMD_ACK            'A'
#define     MSVR_CMD_REPORT         'R'
#define     MSVR_CMD_NOTICE         'N'

unsigned char g_Msvr_rxbuf[MAX_BUF_SIZE];
unsigned char g_Msvr_txbuf[MAX_BUF_SIZE];



char g_Msvr_Init[] = "atz";
char g_Msvr_OK[] = "OK";
char g_Msvr_Connect[] = "CONNECT";
char g_Msvr_Ready[] = "at+crm=129";
char g_Msvr_PhoneNum[] = "ATD01046270596";
char g_Msvr_CR = 0x0d;
char g_Msvr_LF = 0x0a;


typedef struct {
    unsigned char cStx;
    unsigned char cLength;
    unsigned char cCmd;
    unsigned short wPcm;
    unsigned short wPno;
    float fVal;
    unsigned char cEtx;
}__attribute__((packed)) _MDTD_PKT_DATA;

typedef struct {
    unsigned char cStx;
    unsigned char cLength;
    unsigned char cCmd;
    unsigned char cEtx;
}__attribute__((packed)) _MDTD_PKT_ACK;




/*******************************************************************/
static void Msvr_Sleep(int sec,int msec) 
/*******************************************************************/
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = msec * 1000; 
    select(0,NULL,NULL,NULL,&tv);
    return;
}


/****************************************************************/
static void  Msvr_On_Send_Byte(void *p, unsigned char TxChar)
/****************************************************************/
{
	IfaceMsvr_T *uart2;
	
	//printf("[%x]\n", TxChar);

	uart2 = (IfaceMsvr_T *) p;
	write( uart2->fd, &TxChar, 1);
}


/****************************************************************/
static int Msvr_On_Send(void *p)
/****************************************************************/
{
	int i = 0;
	IfaceMsvr_T *uart2;
	
	uart2 = (IfaceMsvr_T *) p;

	write( uart2->fd, uart2->pTxbuf, uart2->sendLength);
	
	/*
	for ( i = 0; i < uart2->sendLength ; i++) {
		Msvr_Sleep(0, 500);
		Msvr_On_Send_Byte(p, uart2->pTxbuf[i]);
	}
	*/

	printf("TX = ");
	for (i = 0; i < uart2->sendLength; i++) {
		printf("%x ", uart2->pTxbuf[i]);
	}
	printf("\n");
	
	return 0;
}



/****************************************************************/
/* receive data function */
static int IFaceHT_On_Recv(void *p)
/****************************************************************/
{
	int i = 0;
	int iRet = 0;;
	int iBufWp = 0;
	int iLoopCnt = 0;
	IfaceMsvr_T *uart2;
	unsigned char *pRxbuf;
	
	uart2 = (IfaceMsvr_T *) p;
	pRxbuf = (unsigned char *)uart2->pRxbuf;
	memset( pRxbuf, 0, MAX_BUF_SIZE );



	while(1) {
		uart2->pollState = poll(					// poll()을 호출하여 event 발생 여부 확인     
			(struct pollfd*)&uart2->pollEvents,		// event 등록 변수
			1,  									// 체크할 pollfd 개수
			10);  								// time out 시간 (ms)	

		if ( 0 < uart2->pollState) {                            // 발생한 event 가 있음
			if ( uart2->pollEvents.revents & POLLIN) {			// event 가 자료 수신?
				iRet = read(uart2->fd, &pRxbuf[iBufWp], 32);
				iBufWp += iRet;
				iLoopCnt = 0;
				for ( i = 0; i < iRet; i++) {
					printf("%x - %c \n", pRxbuf[i], pRxbuf[i]);	
				}
			}
			else if ( uart2->pollEvents.revents & POLLERR) {	// event 가 에러?
				printf( "Event is Error. Terminal Broken.\n");
				return -1;
			}
		}
		else {
			if ( 5 < iLoopCnt++ ) {
				uart2->recvLength = iBufWp;
				printf("time out %d\n", iBufWp);
				
				if (iBufWp > 0) {
					/*
					printf("Receive OK\n");
					printf("RX(%d) = ", uart2->recvLength);
					for (i = 0; i < uart2->recvLength ;i++) {
						printf("%x - %c \n", pRxbuf[i], pRxbuf[i]);	
					}
					*/
				}
				//	uart2->on_recv_hadler(uart2);
				
				return iBufWp;
			}
		}
	}	

}


/****************************************************************/
static IfaceMsvr_T *New_Msvr(void)
/****************************************************************/
{
	IfaceMsvr_T *uart2;
	
	uart2 = (IfaceMsvr_T *) malloc( sizeof(IfaceMsvr_T) );
	memset( uart2, 0, sizeof(IfaceMsvr_T) );

	uart2->fd = -1;
	uart2->pollState = 0;
	memset( &uart2->pollEvents, 0, sizeof(struct pollfd) );
	
	uart2->recvLength = 0;	
	uart2->sendLength = 0;
	uart2->pRxbuf = (unsigned char *)&g_Msvr_rxbuf;
	uart2->pTxbuf = (unsigned char *)&g_Msvr_txbuf;
	memset( uart2->pRxbuf, 0, MAX_BUF_SIZE );
	memset( uart2->pTxbuf, 0, MAX_BUF_SIZE );	

	return uart2;
}


/****************************************************************/
static void Msvr_Close(IfaceMsvr_T *p)
/****************************************************************/
{
	close(p->fd);
	memset( p, 0, sizeof(IfaceMsvr_T) );
	p->fd = -1;
}


/****************************************************************/
static int Msvr_Open(IfaceMsvr_T *p)
/****************************************************************/
{
	struct termios oldtio, newtio;
	
	/* uart2 initiailze */
	p->fd = open(UART2DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (p->fd < 0) { 
		printf("Serial UART2 Open fail\n");
		return -1; 
	}
	else
		printf("Serial UART2 Open success\n");
	
	if (tcgetattr(p->fd, &oldtio) < 0) {
		perror("error in tcgetattr");
		return -1; 
	}
	
	bzero(&newtio, sizeof(newtio));
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
	
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN]  = 1;
	
	tcflush(p->fd, TCIFLUSH);
	tcsetattr(p->fd,TCSANOW,&newtio);	

	fcntl(p->fd, F_SETFL, FNDELAY); 

	/* poll initiailze */
	p->pollEvents.fd = p->fd; 
	/*
		#define POLLIN 0x0001 // 읽을 데이터가 있다. 
		#define POLLPRI 0x0002 // 긴급한 읽을 데이타가 있다. 
		#define POLLOUT 0x0004 // 쓰기가 봉쇄(block)가 아니다. 
		#define POLLERR 0x0008 // 에러발생 
		#define POLLHUP 0x0010 // 연결이 끊겼음 
		#define POLLNVAL 0x0020 // 파일지시자가 열리지 않은 것 같은, Invalid request (잘못된 요청) 	
	*/
	p->pollEvents.events  = POLLIN | POLLERR;
	p->pollEvents.revents = 0;	
	
	return 1;
}


static void Msvr_Send_Atz(void *p)
{
	IfaceMsvr_T *pMsvr;
	
	pMsvr = (IfaceMsvr_T *) p;
	

	fprintf(stdout, "Send ATZ\n");
	fflush(stdout);
	pMsvr->sendLength = sizeof(g_Msvr_Init);
	memcpy(pMsvr->pTxbuf, g_Msvr_Init, pMsvr->sendLength);
	Msvr_On_Send(p);

	fprintf(stdout, "Send CR\n");
	//pMsvr->sendLength = sizeof(g_Msvr_CR);
	//memcpy(pMsvr->pTxbuf, g_Msvr_CR, pMsvr->sendLength);
	Msvr_On_Send_Byte(p, g_Msvr_CR);
}



/****************************************************************/
void *Iface_Msvr_Main(void* arg)
/****************************************************************/
{
	//int iLoopCnt = 0;
	IfaceMsvr_T *pMsvr = NULL;

	pMsvr = New_Msvr();
	if (pMsvr == NULL) {
		printf("Can't create MDTD Server interface.\n");
		//exit(1);
		system("killall duksan");
	}

	Msvr_Sleep(1, 0);

	printf("Start MDTD Server interface.\n");

	while (1) {
		// open uart2
		if ( Msvr_Open(pMsvr) < 0 ) {
			Msvr_Sleep(3, 0);
			Msvr_Close(pMsvr);
			continue;	
		}


		for (;;) {
			fprintf(stdout, "wait 5 sec\n");
			fflush(stdout);
			Msvr_Sleep(5, 0);

			Msvr_Send_Atz(pMsvr);
			//Msvr_Sleep(0, 100);
			IFaceHT_On_Recv(pMsvr);

		}
	}

	//exit(1);
	system("killall duksan");
}


