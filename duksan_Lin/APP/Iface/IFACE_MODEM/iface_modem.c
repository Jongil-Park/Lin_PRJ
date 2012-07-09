/* file : iface_modem.c
 *
 * CCMS Client가 NET32, TCP로 연결되지 않았을 경우 MODEM을 통해 
 * Server와 연결하도록 한다. 
 * MODEM의 TCP연결을 통해서 Server와 연결하도록 하고, 
 * 연결이 실패하거나 중요한 신호가 발생하면 SMS로 문자를 보내도록 한다. 
 *
 * !!!!! MODEM 설정시 주의사항
 * MODEM의 플로우컨트롤을 사용하지 않도록 해야 한다. 
 * IDC / GCU와 연결시에 9600bps로 연결하도고 해야 Serial Data가 깨지지 않는다. 
 *
*/
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

/*****************************************************************************/
#include "define.h"
#include "queue_handler.h"
#include "FUNCTION.h"
#include "iface_ccms_client.h"
#include "iface_modem.h"


/*****************************************************************************/
#define UART2DEVICE 					"/dev/s3c2410_serial2"
#define MAX_BUF_SIZE					2048

/*****************************************************************************/
extern int 				g_nMyPcm;
extern PTBL_INFO_T 		*g_pPtbl;
extern unsigned char	g_cNode[32];

/*****************************************************************************/
// momdem 연결상태 
int g_nModemConnected = 0;

// temporarily point-table
float prePoint[256];

// Tx, Rx Buffer
static unsigned char g_ifacModem_rxbuf[MAX_BUF_SIZE];
static unsigned char g_ifacModem_txbuf[MAX_BUF_SIZE];

// ASCII point-table
/*
char g_chAsciiTable[] = { 
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
	':', ';', '<', '=', '>', '?', '@',
	'A','B','C','D','E','F','G','H','I','J','K','L','M','N',
	'O','P','Q','R','S','T','U','V','W','X','Y','Z'};
*/

// modem command
char g_Modem_Atz[] = "atz";
char g_Modem_Ate0[] = "ate0";
char g_Modem_Crm[] = "at+crm=251";
char g_Modem_Atd[] = "atd1501";
char g_Modem_Connect[] = "at$tcpopen=125.7.234.182,9104";
char g_Modem_Close[] = "at$tcpclose";
char g_Modem_Exit[] = "at$tcpexit";
char g_Modem_Ack[] = "at$tcpwrite=3C413035";
char g_Modem_Data[] = "at$tcpwrite=3C4E30303030303030303030303030";
char g_Modem_CR = 13;
char g_Modem_LF = 10;


/*******************************************************************/
// delay function 
static void IfaceModem_Sleep(int sec,int msec) 
/*******************************************************************/
{
    fd_set reads, temps;
    struct timeval tv;
    
    FD_ZERO(&reads);
    FD_SET(0, &reads);
    
    tv.tv_sec = sec;
    tv.tv_usec = 1000 * msec;
    temps = reads;
    
    select( 0, &temps, 0, 0, &tv );
    
	return;
}



/****************************************************************/
// send data 
static int IfaceModem_On_Send(void *p)
/****************************************************************/
{
	int i = 0;
	IfaceModem_T *uart2;
	
	uart2 = (IfaceModem_T *) p;
	//write( uart2->fd, uart2->txbuf, uart2->sendLength);
	for (i = 0; i < uart2->sendLength; i++) {
		write( uart2->fd, &uart2->txbuf[i], 1);		
		IfaceModem_Sleep(0,30);
	}

	printf("TX = ");
	for (i = 0; i < uart2->sendLength; i++) {
		printf("%x ", uart2->txbuf[i]);
	}
	printf("\n");
	
	return 0;
}


/****************************************************************/
// parsing receive data 
static int IfaceModem_On_Recv_Handler(void *pIfaceModem) 
/****************************************************************/
{
	int i = 0;
	int iSlaveId = 0;
	int iLength = 0;
	//int iPcm = 0;
	//int iPno = 0;
	//float fVal[4];
	IfaceModem_T *p = NULL;
	unsigned char *pBuf;
	//unsigned short calCrc16;
	//unsigned short recvCrc16;
	unsigned char cType = 0;
	//unsigned char *pData;
	//unsigned int iCnt = 0;	
	//unsigned int iDataCnt = 0;
	int nChkVal = -1;
		
	/* Initialize */
	p = (IfaceModem_T *)pIfaceModem;
	pBuf = (unsigned char *) p->rxbuf;
	iSlaveId = pBuf[0];
	iLength =  p->recvLength;
	cType = pBuf[1];

	printf("RX(%d) = ", p->recvLength);
	for (i = 0; i < p->recvLength ;i++) {
		printf("%c ", pBuf[i]);	
	}
	printf("\n");
	

	nChkVal = -1;
	for (i = 0; i < p->recvLength ;i++) {
		if ( pBuf[i] == 'A' &&  pBuf[i+1] == 'D' 
			&& pBuf[i+2] == 'D' &&  pBuf[i+3] == 'A'
			&& pBuf[i+4] == 'T' &&  pBuf[i+5] == 'A' )
			return IFACE_RTN_TCPREAD;				
	}		
	
	
	nChkVal = -1;
	for (i = 0; i < p->recvLength ;i++) {
		// Get 'ok' Message
		if ( pBuf[i] == 'o' &&  pBuf[i+1] == 'k' )
			nChkVal = IFACE_RTN_OK;
		// Get 'CONNECT' Message
		else if ( pBuf[i] == 'C' &&  pBuf[i+1] == 'O' 
			&& pBuf[i+2] == 'N' &&  pBuf[i+3] == 'N'
			&& pBuf[i+4] == 'E' &&  pBuf[i+5] == 'C'
			&&  pBuf[i+6] == 'T')
			return IFACE_RTN_CONNECT;	
		// TCPOPEN
		else if ( pBuf[i] == 'T' &&  pBuf[i+1] == 'C' 
			&& pBuf[i+2] == 'P' &&  pBuf[i+3] == 'O'
			&& pBuf[i+4] == 'P' &&  pBuf[i+5] == 'E'
			&&  pBuf[i+6] == 'N')
			return IFACE_RTN_TCPOPEN;				
		// ERROR
		else if ( pBuf[i] == 'E' &&  pBuf[i+1] == 'R' 
			&& pBuf[i+2] == 'R' &&  pBuf[i+3] == 'O'
			&& pBuf[i+4] == 'R')
			return IFACE_RTN_ERROR;	
		// TCPCLOSE
		else if ( pBuf[i] == 'C' &&  pBuf[i+1] == 'L' 
			&& pBuf[i+2] == 'O' &&  pBuf[i+3] == 'S'
			&& pBuf[i+4] == 'E' &&  pBuf[i+5] == 'D' )
			return IFACE_RTN_TCPCLOSED;			
		else if ( pBuf[i] == 'A' &&  pBuf[i+1] == 'D' 
			&& pBuf[i+2] == 'D' &&  pBuf[i+3] == 'A'
			&& pBuf[i+4] == 'T' &&  pBuf[i+5] == 'A' )
			return IFACE_RTN_TCPREAD;				
		// TCPSENDDONE
		else if ( pBuf[i] == 'S' &&  pBuf[i+1] == 'E' 
			&& pBuf[i+2] == 'N' &&  pBuf[i+3] == 'D')
			return IFACE_RTN_TCPSENDDONE;				
		// Get 'ok' Message
		else if ( pBuf[i] == 'O' &&  pBuf[i+1] == 'K' )
			nChkVal = IFACE_RTN_OK;			
	}	
	
	return nChkVal;
}


/****************************************************************/
// receive data function 
static int IfaceModem_On_Recv(void *p)
/****************************************************************/
{
	//int i = 0;
	int iRet = 0;;
	int iBufWp = 0;
	int iLoopCnt = 0;
	IfaceModem_T *uart2;
	unsigned char *pRxbuf;
	
	uart2 = (IfaceModem_T *) p;
	pRxbuf = (unsigned char *)uart2->rxbuf;
	memset( pRxbuf, 0, MAX_BUF_SIZE );

	/*
	IfaceModem_Sleep(1, 0);
	iRet = read(uart2->fd, pRxbuf, 32);
	
	printf("iRet = %d\n", iRet);	
	for ( i = 0; i < iRet; i++)
		printf("%c ,", pRxbuf[i]);	
	printf("\n\n");
	return iRet;
	*/
	

	while(1) {
		uart2->pollState = poll(					// poll()을 호출하여 event 발생 여부 확인     
			(struct pollfd*)&uart2->pollEvents,		// event 등록 변수
			1,  									// 체크할 pollfd 개수
			100);  									// time out 시간 (ms)	

		if ( 0 < uart2->pollState) {                            // 발생한 event 가 있음
			if ( uart2->pollEvents.revents & POLLIN) {			// event 가 자료 수신?
				iRet = read(uart2->fd, &pRxbuf[iBufWp], 32);
				iBufWp += iRet;
				iLoopCnt = 0;
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
				return iBufWp;
			}
		}
	}	
}


#if 0





/****************************************************************/
// check type for message that to send 
static int IfaceModem_On_Send_Handler(void *pIfaceModem)
/****************************************************************/
{
	float fVal = 0;
	unsigned int iPcm = 0;
	unsigned int iPno = 0;
	unsigned int iCtrl = 0;
	unsigned int iCtrlType = 0;
	IfaceModem_T *p = NULL;

	/* Initialize */
	p = (IfaceModem_T *)pIfaceModem;
	iCtrl = p->ctrl;
	iPcm = ISPCM(iCtrl);
	iPno = ISPNO(iCtrl);
	fVal = p->ctrlVal;
	
	printf("iCtrl = %d, Pcm = %d, Pno= %d val = %f\n", 
		iCtrl, iPcm, iPno, p->ctrlVal);

	// 0이면, Group과 Pattern을 제어하는 PCM번호를 나타내는 것이고, 
	// 1이면 각 Relay을 개별제어하는 PCM 번호를 나타내는 것이다. 
	iCtrlType = IS_CTRLTYPE(iPcm);
	
	if ( iCtrlType == 1 ) {
		
		// Relay는 1 ~ 255번의 pno에 맵핑된다. 
		if ( iPno > 0 && iPno < 256 ) {
			p->ctrlType = IFACE_HT_CTRL_RELAY;
			p->on_make_data(p);
			p->on_uart2_send(p);
			return IFACE_HT_CTRL_RELAY;		
		}	
	}
	else {

		// Group number is 1 ~ 128
		if ( iPno > 0 && iPno <= 128 ) {
			p->ctrlType = IFACE_HT_CTRL_GROUP;
			p->on_make_data(p);
			p->on_uart2_send(p);			
			return IFACE_HT_CTRL_GROUP;		
		}
		// Pattern number is 129 ~ 152
		else if ( iPno <= 152 ) {
			p->ctrlType = IFACE_HT_CTRL_PATTERN;
			p->on_make_data(p);
			p->on_uart2_send(p);
			return IFACE_HT_CTRL_PATTERN;		
		}
	}

	return -1;
}
#endif


/****************************************************************/
// creat interface
static IfaceModem_T *New_IfaceModem(void)
/****************************************************************/
{
	IfaceModem_T *uart2;
	
	uart2 = (IfaceModem_T *) malloc( sizeof(IfaceModem_T) );
	memset( uart2, 0, sizeof(IfaceModem_T) );

	uart2->fd = -1;
	uart2->pollState = 0;
	memset( &uart2->pollEvents, 0, sizeof(struct pollfd) );
	
	uart2->recvLength = 0;	
	uart2->sendLength = 0;
	uart2->rxbuf = (unsigned char *)&g_ifacModem_rxbuf;
	uart2->txbuf = (unsigned char *)&g_ifacModem_txbuf;
	memset( uart2->rxbuf, 0, MAX_BUF_SIZE );
	memset( uart2->txbuf, 0, MAX_BUF_SIZE );	

	memset(&prePoint, 0x00, sizeof(prePoint));
	
	return uart2;
}


/****************************************************************/
// close interface
static void IfaceModem_Close(IfaceModem_T *p)
/****************************************************************/
{
	close(p->fd);
	memset( p, 0, sizeof(IfaceModem_T) );
	p->fd = -1;
}


/****************************************************************/
static int IfaceModem_Open(IfaceModem_T *p)
/****************************************************************/
{
	struct termios oldtio, newtio;

	p->fd = open(UART2DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (p->fd < 0) { 
		fprintf(stderr, "[GHP] Serial g_iUart2Fd Open fail\n");
		fflush(stderr);
		return -1 ; 
	}
	else {
		fprintf(stderr, "[GHP] Serial g_iUart2Fd Open success\n");
		fflush(stderr);
	}
	
	if (tcgetattr(p->fd, &oldtio) < 0)
	{
		//perror("error in tcgetattr");
		fprintf(stderr, "[GHP] error in tcgetattr\n");
		fflush(stderr);
		return -1 ; 
	}
	
	bzero(&newtio, sizeof(newtio));
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
	newtio.c_cflag &= ~CRTSCTS;
	
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


static void IfaceModem_SendCR(IfaceModem_T *p)
{
	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Modem_CR);
	memcpy(p->txbuf , &g_Modem_CR, p->sendLength); 
	
	IfaceModem_On_Send(p);
}


static void IfaceModem_SendLF(IfaceModem_T *p)
{
	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Modem_LF);
	memcpy(p->txbuf , &g_Modem_LF, p->sendLength); 
	
	IfaceModem_On_Send(p);
}


static int IfaceModem_SendATZ(IfaceModem_T *p)
{
	int nRtnVal = 0;
	
	printf("%s\n", g_Modem_Atz);

	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Modem_Atz) - 1;
	memcpy(p->txbuf , g_Modem_Atz, p->sendLength); 
	
	IfaceModem_On_Send(p);
	
	IfaceModem_SendCR(p);
	IfaceModem_SendLF(p);
	
	if ( IfaceModem_On_Recv(p) > 0 ) {
		nRtnVal = IfaceModem_On_Recv_Handler(p);
	}
		
	if ( nRtnVal == IFACE_RTN_OK ) {
		printf(" 'OK' Message receive success\n");
		return 1;
	}
	else { 
		printf(" 'OK' Message receive fail\n");
		return -1;
	}	
}


static int IfaceModem_SendATE0(IfaceModem_T *p)
{
	int nRtnVal = 0;
	
	printf("%s\n", g_Modem_Ate0);

	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Modem_Ate0) - 1;
	memcpy(p->txbuf , g_Modem_Ate0, p->sendLength); 
	
	IfaceModem_On_Send(p);
	IfaceModem_SendCR(p);
	IfaceModem_SendLF(p);
	
	if ( IfaceModem_On_Recv(p) > 0 ) {
		nRtnVal = IfaceModem_On_Recv_Handler(p);
	}
		
	if ( nRtnVal == IFACE_RTN_OK ) {
		printf(" 'OK' Message receive success\n");
		return 1;
	}
	else { 
		printf(" 'OK' Message receive fail\n");
		return -1;
	}	
}


static int IfaceModem_SendCRM(IfaceModem_T *p)
{
	int nRtnVal = 0;
	
	printf("%s\n", g_Modem_Crm);
	
	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Modem_Crm) - 1 ;
	memcpy(p->txbuf , g_Modem_Crm, p->sendLength); 
	
	IfaceModem_On_Send(p);	
	IfaceModem_SendCR(p);
	IfaceModem_SendLF(p);

	if ( IfaceModem_On_Recv(p) > 0 ) {
		nRtnVal = IfaceModem_On_Recv_Handler(p);
	}
		
	if ( nRtnVal == IFACE_RTN_OK ) {
		printf(" 'OK' Message receive success\n");
		return 1;
	}
	else { 
		printf(" 'OK' Message receive fail\n");
		return -1;
	}	
}

static int IfaceModem_SendATD(IfaceModem_T *p)
{
	int nRtnVal = 0;
	int nRetryCnt = 0;
	
	printf("%s\n", g_Modem_Atd);
	
	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Modem_Atd) - 1 ;
	memcpy(p->txbuf , g_Modem_Atd, p->sendLength); 
		
	IfaceModem_On_Send(p);		
	IfaceModem_SendCR(p);	
	IfaceModem_SendLF(p);

	for ( nRetryCnt = 0; nRetryCnt < 3; nRetryCnt++ ) {
		if ( IfaceModem_On_Recv(p) > 0 ) {
			nRtnVal = IfaceModem_On_Recv_Handler(p);
		}

		if ( nRtnVal == IFACE_RTN_CONNECT ) {
			printf(" 'CONNECT' Message receive success\n");
			return 1;
		}
		else { 
			printf(" 'CONNECT' Message receive fail\n");
			IfaceModem_Sleep(3, 0);		
			continue;
		}			
	}	
	
	return -1;
}

static int IfaceModem_SendCONNECT(IfaceModem_T *p)
{
	int nRtnVal = 0;
	int nRetryCnt = 0;
	
	printf("%s\n", g_Modem_Connect);
	
	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Modem_Connect) - 1;
	memcpy(p->txbuf , g_Modem_Connect, p->sendLength); 
			
	IfaceModem_On_Send(p);		
	IfaceModem_SendCR(p);
	IfaceModem_SendLF(p);

	for ( nRetryCnt = 0; nRetryCnt < 3; nRetryCnt++ ) {
		if ( IfaceModem_On_Recv(p) > 0 ) {
			nRtnVal = IfaceModem_On_Recv_Handler(p);
		}

		if ( nRtnVal == IFACE_RTN_TCPOPEN || nRtnVal == IFACE_RTN_OK) {
			printf(" 'TCPOPEN' Message receive success\n");
			return 1;
		}
		else { 
			printf(" 'TCPOPEN' Message receive fail\n");
			IfaceModem_Sleep(3, 0);		
			continue;
		}			
	}	
	
	return -1;
}

static int IfaceModem_SendCLOSE(IfaceModem_T *p)
{
	int nRtnVal = 0;
	
	printf("%s\n", g_Modem_Close);
	
	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Modem_Close) - 1 ;
	memcpy(p->txbuf , g_Modem_Close, p->sendLength); 
		
	IfaceModem_On_Send(p);		
	IfaceModem_SendCR(p);
	IfaceModem_SendLF(p);
	
	if ( IfaceModem_On_Recv(p) > 0 ) {
		nRtnVal = IfaceModem_On_Recv_Handler(p);
	}
		
	if ( nRtnVal == IFACE_RTN_OK ) {
		printf(" 'OK' Message receive success\n");
		return 1;
	}
	else { 
		printf(" 'OK' Message receive fail\n");
		return -1;
	}		
}

static int IfaceModem_SendEXIT(IfaceModem_T *p)
{
	int nRtnVal = 0;
	
	printf("%s\n", g_Modem_Exit);
	
	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Modem_Exit) - 1 ;
	memcpy(p->txbuf , g_Modem_Exit, p->sendLength); 
		
	IfaceModem_On_Send(p);		
	IfaceModem_SendCR(p);
	IfaceModem_SendLF(p);

	if ( IfaceModem_On_Recv(p) > 0 ) {
		nRtnVal = IfaceModem_On_Recv_Handler(p);
	}
		
	if ( nRtnVal == IFACE_RTN_OK ) {
		printf(" 'OK' Message receive success\n");
		return 1;
	}
	else { 
		printf(" 'OK' Message receive fail\n");
		return -1;
	}	
}

static void IfaceModem_SendACK(IfaceModem_T *p)
{
	//int nRtnVal = 0;
	
	printf("%s g_nMyPcm (%d)\n", g_Modem_Ack, g_nMyPcm);
	
	g_Modem_Ack[17] = g_nMyPcm/10 + 0x30;
	g_Modem_Ack[19] = g_nMyPcm%10 + 0x30;
	
	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Modem_Ack) - 1 ;
	memcpy(p->txbuf , g_Modem_Ack, p->sendLength); 
		
	IfaceModem_On_Send(p);		
	IfaceModem_SendCR(p);
	IfaceModem_SendLF(p);
}

static int IfaceModem_SendDATA(IfaceModem_T *p, point_info *pPnt)
{
	int i = 0;
	int nRtnVal = 0;
	int nRetryCnt = 0;
	char chValue[8];
	unsigned char chFloat[4];
		
	memset( p->txbuf, 0, MAX_BUF_SIZE );	

	g_Modem_Data[17] = g_nMyPcm/10 + 0x30;
	g_Modem_Data[19] = g_nMyPcm%10 + 0x30;
	
	g_Modem_Data[21] = pPnt->pno/100 + 0x30;
	if ( g_Modem_Data[21] == 0x30 )
		g_Modem_Data[23] = pPnt->pno/10 + 0x30;
	else if ( g_Modem_Data[21] == 0x31 )
		g_Modem_Data[23] = (pPnt->pno/10 - 10) + 0x30;		
	else if ( g_Modem_Data[21] == 0x32 )
		g_Modem_Data[23] = (pPnt->pno/10 - 20) + 0x30;				
	g_Modem_Data[25] = pPnt->pno%10 + 0x30;


	//&chFloat = (unsigned char *) &(pPnt->value);
	memcpy(chFloat, &(pPnt->value), sizeof(float));
	sprintf(chValue, "%02X%02X%02X%02X", chFloat[0], chFloat[1], chFloat[2], chFloat[3]);

	printf("%s [%x %x %x %x %x %x %x %x]\n", chValue, 
		chValue[0], chValue[1], 
		chValue[2], chValue[3], 
		chValue[4], chValue[5], 
		chValue[6], chValue[7]	);

	for ( i = 0; i < 8; i++ ) {
		g_Modem_Data[26 + (i * 2)] = (chValue[i]/16) + 0x30;
		g_Modem_Data[27 + (i * 2)] = (chValue[i]%16) + 0x30;
	}
	
	p->sendLength = sizeof(g_Modem_Data) - 1 ;
	memcpy(p->txbuf , g_Modem_Data, p->sendLength); 
		
	IfaceModem_On_Send(p);		
	IfaceModem_SendCR(p);
	IfaceModem_SendLF(p);
	
	printf("%s [%d]\n", g_Modem_Data, sizeof(g_Modem_Data));	
	

	for ( nRetryCnt = 0; nRetryCnt < 2; nRetryCnt++ ) {
		if ( IfaceModem_On_Recv(p) > 0 ) {
			nRtnVal = IfaceModem_On_Recv_Handler(p);
		}

		if ( nRtnVal == IFACE_RTN_TCPSENDDONE || nRtnVal == IFACE_RTN_OK) {
			return IFACE_RTN_TCPSENDDONE;	
		}
		else if ( nRtnVal == IFACE_RTN_TCPCLOSED) {
			return IFACE_RTN_TCPCLOSED;	
		}
		else if ( nRtnVal == IFACE_RTN_ERROR) {
			return IFACE_RTN_ERROR;	
		}		
		else { 
			printf(" 'TCPDONE' Message receive fail\n");
			IfaceModem_Sleep(3, 0);		
			continue;
		}	
	}	
	
	return IFACE_RTN_TCPSENDDONE;
}





int IfaceModem_Master_Check(void)
{
	if ( g_cNode[0] > 0 )
		return 1;
	else
		return -1;
}



void IfaceModem_On_Recv_Parse(IfaceModem_T *p)
{
	int i = 0;
	unsigned char *pBuf;
	int nPcm = 0;
	int nPno = 0;
	int nValue = 0;

	/* Initialize */
	pBuf = (unsigned char *) p->rxbuf;	
	
	printf("RX(%d) = ", p->recvLength);
	for (i = 0; i < p->recvLength ;i++) {
		printf("%c ", pBuf[i]);	
	}
	printf("\n");
	
	
	for (i = 0; i < p->recvLength ;i++) {
		if ( pBuf[i] == 'A' &&  pBuf[i+1] == 'D' 
			&& pBuf[i+2] == 'D' &&  pBuf[i+3] == 'A'
			&& pBuf[i+4] == 'T' &&  pBuf[i+5] == 'A' ) {
				printf(">> pBuf[i+6] = %c\n", pBuf[i+6]);
				printf(">> pBuf[i+7] = %c\n", pBuf[i+7]);
				printf(">> pBuf[i+8] = %c\n", pBuf[i+8]);
				printf(">> pBuf[i+9] = %c\n", pBuf[i+9]);
				printf(">> pBuf[i+10] = %c\n", pBuf[i+10]);
				printf(">> pBuf[i+11] = %c\n", pBuf[i+11]);
				printf(">> pBuf[i+12] = %c\n", pBuf[i+12]);
				printf(">> pBuf[i+13] = %c\n", pBuf[i+13]);
				printf(">> pBuf[i+14] = %c\n", pBuf[i+14]);
				printf(">> pBuf[i+15] = %c\n", pBuf[i+15]);

				if ( pBuf[i+6] == '=' && pBuf[i+7] == '0' && pBuf[i+9] == '0' ) {		
					nPcm = pBuf[i+8] - 0x30;
					nPno = pBuf[i+10] - 0x30;
					nValue = pBuf[i+14] - 0x30;
					pSet(nPcm, nPno, nValue);
				}
			}
	}	
}


/****************************************************************/
void *IfaceModem_Main(void* arg)
/****************************************************************/
{
	int i = 0;
	int nStatus = 0;
	int nRtnVal = 0;
	//int nRetryCnt = 0;
	int nRecvCnt = 0;
	point_info point;
	IfaceModem_T *pMd = NULL;

	pMd = New_IfaceModem();
	if (pMd == NULL) {
		printf("Can't create HANTEC interface.\n");
		//exit(1);
		system("killall duksan");
	}

	IfaceModem_Sleep(2, 0);

	printf("Start Modem interface.\n");
	while (1) {
					
		/* open uart2 */
		if ( IfaceModem_Open(pMd) < 0 ) {
			IfaceModem_Sleep(3, 0);
			IfaceModem_Close(pMd);
			continue;	
		}
		
		nStatus = IFACE_MODEM_READY;
		for (;;) {

			// net32 master가 연결되어 있다면 MODEM에 접속하지 않는다. 
			if ( IfaceModem_Master_Check() > 0 ) {
				//IfaceModem_Sleep(3,0);
				if ( nStatus != IFACE_MODEM_READY) {
					if ( IfaceModem_SendCLOSE(pMd) < 0)
						IfaceModem_Sleep(1, 0);		
	
					if ( IfaceModem_SendEXIT(pMd) < 0)
						IfaceModem_Sleep(1, 0);						
				}
				IfaceModem_Sleep(1, 0);
				nStatus = IFACE_MODEM_READY;
				continue;
			}				
			
			// ccms master가 연결되어 있다면 MODEM에 접속하지 않는다. 
			if ( cclnt_get_status() > 0 ) {
				//IfaceModem_Sleep(3,0);
				if ( nStatus != IFACE_MODEM_READY) {
					if ( IfaceModem_SendCLOSE(pMd) < 0)
						IfaceModem_Sleep(1, 0);		
	
					if ( IfaceModem_SendEXIT(pMd) < 0)
						IfaceModem_Sleep(1, 0);						
				}	
				IfaceModem_Sleep(1, 0);			
				nStatus = IFACE_MODEM_READY;
				continue;
			}				
			
			switch(nStatus) {
			// MODEM으로 통신을 연결하기 위한 준비를 한다. 
			case IFACE_MODEM_READY:
				printf("\n\n= IFACE_MODEM_READY =============================\n");
				if ( IfaceModem_SendATZ(pMd) > 0 ) {
					nStatus = IFACE_MODEM_CONNECT;
				}
				else { 
					IfaceModem_Sleep(1, 0);		
					nStatus = IFACE_MODEM_READY;
					break;
				}
				
				if ( IfaceModem_SendATE0(pMd) > 0 ) {
					nStatus = IFACE_MODEM_CONNECT;
				}
				else { 
					IfaceModem_Sleep(1, 0);		
					nStatus = IFACE_MODEM_READY;
				}				
				
				break;
			
			// MODEM을 통해 CCMS 서버에 접속한다. 
			case IFACE_MODEM_CONNECT:
				printf("\n\n= IFACE_MODEM_CONNECT =============================\n");
				// TCP Exit 명령을 먼저 내보내서 연결되어 있을 TCP를 종료한다. 
				if ( IfaceModem_SendCLOSE(pMd) < 0)
					IfaceModem_Sleep(2, 0);		

				if ( IfaceModem_SendEXIT(pMd) < 0)
					IfaceModem_Sleep(2, 0);		
				
				// ATZ 명령을 통해 모뎀을 초기화한다. 
				if ( IfaceModem_SendATZ(pMd) < 0 ) {
					IfaceModem_Sleep(1, 0);		
					nStatus = IFACE_MODEM_READY;
					continue;
				}				
				
				// ATE 명령을 통해 에코를 방지한다. 
				if ( IfaceModem_SendATE0(pMd) < 0 ) {
					IfaceModem_Sleep(1, 0);		
					nStatus = IFACE_MODEM_READY;
				}					
				
				// CRM 명령을 통해 TCP 모드로 변경한다. 
				if ( IfaceModem_SendCRM(pMd) < 0 ) {
					IfaceModem_Sleep(1, 0);		
					nStatus = IFACE_MODEM_READY;
					continue;
				}					
							
				// ATD 명령을 통해 패킷접속명령을 내린다.
				if ( IfaceModem_SendATD(pMd) < 0 ) {
					IfaceModem_Sleep(1, 0);		
					nStatus = IFACE_MODEM_READY;
					continue;
				}	
								
				// CONNECT 명령을 서버에 접속한다. 
				if ( IfaceModem_SendCONNECT(pMd) < 0 ) {
					IfaceModem_Sleep(1, 0);		
					nStatus = IFACE_MODEM_READY;
					continue;
				}	
				
				g_nModemConnected = 1;
				IfaceModem_Sleep(1, 0);	
				nStatus = IFACE_MODEM_SEND_ACK;
				break;

			// MODEM을 통해 서버로 전송될 DATA를 보낸다. 
			// 약.. 4분 동안 아무런 DATA가 없다면 접속을 중단한다. 
			case IFACE_MODEM_SEND_DATA:
				printf("\n\n= IFACE_MODEM_SEND_DATA =============================\n");
				printf("PCM = %d, PNO = %d, Value = %f\n", point.pcm, point.pno, point.value);
				nRtnVal = IfaceModem_SendDATA(pMd, &point);
				
				if ( nRtnVal == IFACE_RTN_TCPREAD ) {
					printf("Recv Data\n");
					IfaceModem_On_Recv_Parse(pMd);
					nStatus = IFACE_MODEM_RECV_DATA;
				}				
				
				if ( nRtnVal == IFACE_RTN_TCPCLOSED)
					nStatus = IFACE_MODEM_CLOSE;
				else
					nStatus = IFACE_MODEM_RECV_DATA;
				break;
							
			// MODEM을 통해 서버로부터 전송될 DATA를 기다린다. 
			// 일정시간동안 DATA가 전송되지 않으면 접속실패로 간주한다. 
			case IFACE_MODEM_RECV_DATA:
				IfaceModem_Sleep(1, 0);	
				printf("\n\n= IFACE_MODEM_RECV_DATA =============================\n");
				if ( IfaceModem_On_Recv(pMd) > 0 ) {
					nRtnVal = IfaceModem_On_Recv_Handler(pMd);
				}
				
				if ( nRtnVal == IFACE_RTN_TCPREAD ) {
					printf("Recv Data\n");
					IfaceModem_On_Recv_Parse(pMd);
				}
				
				if ( nRtnVal == IFACE_RTN_TCPCLOSED ) {
					nStatus = IFACE_MODEM_CLOSE;
					break;
				}					

				if ( nRtnVal == IFACE_RTN_ERROR) {
					nStatus = IFACE_MODEM_CLOSE;
					break;
				}
				
				if ( g_nModemConnected == 0 ) {
					nStatus = IFACE_MODEM_CLOSE;
					break;
				}
				else 
					nStatus = IFACE_MODEM_RECV_DATA;
				
				if ( ++nRecvCnt > 5 ) {
					nStatus = IFACE_MODEM_SEND_ACK;		
					break;
				}

				for ( i = 0; i < 32; i++) {
					if ( prePoint[i] != g_pPtbl[i].f_val ) {
						prePoint[i] = g_pPtbl[i].f_val;
						point.pcm = g_nMyPcm;
						point.pno = i;
						point.value = prePoint[i];
						nStatus = IFACE_MODEM_SEND_DATA;	
						break;						
					}
				}
												
				break;

			// 5초마다 한번씩 ACK 보내서 TCP가 끊어지지 않도록 한다. 
			case IFACE_MODEM_SEND_ACK:
				printf("\n\n= IFACE_MODEM_SEND_ACK =============================\n");
				nRecvCnt = 0;
				IfaceModem_SendACK(pMd);
				nStatus = IFACE_MODEM_RECV_DATA;
				break;

				
			// TCP 연결을 종료한다. 
			case IFACE_MODEM_CLOSE:
				printf("\n\n= IFACE_MODEM_CLOSE =============================\n");
				if ( IfaceModem_SendCLOSE(pMd) < 0)
					IfaceModem_Sleep(1, 0);		

				if ( IfaceModem_SendEXIT(pMd) < 0)
					IfaceModem_Sleep(1, 0);		
					
				nStatus = IFACE_MODEM_READY;
				break;				
			}
		}
	}
	syslog_record(SYSLOG_DESTROY_IFACE_MODEM);	
}


