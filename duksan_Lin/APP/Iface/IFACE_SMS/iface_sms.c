/* file : iface_sms.c
 *
 * 사용자가 정한 포인트의 값이 변동이 되면 
 * SMS을 보내도록 한다. 
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
#include "iface_sms.h"


/*****************************************************************************/
#define UART2DEVICE 					"/dev/s3c2410_serial2"
#define MAX_BUF_SIZE					2048

/*****************************************************************************/
extern int 				g_nMyPcm;
extern PTBL_INFO_T 		*g_pPtbl;
extern unsigned char	g_cNode[32];

/*****************************************************************************/
// temporarily point-table
int prePoint[256];

// Tx, Rx Buffer
static unsigned char g_ifacSms_rxbuf[MAX_BUF_SIZE];
static unsigned char g_ifacSms_txbuf[MAX_BUF_SIZE];
static unsigned char g_ifacSms_msgbuf[128];

// ASCII point-table
/*
char g_chAsciiTable[] = { 
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
	':', ';', '<', '=', '>', '?', '@',
	'A','B','C','D','E','F','G','H','I','J','K','L','M','N',
	'O','P','Q','R','S','T','U','V','W','X','Y','Z'};
*/

// modem command
char g_Sms_Atz[] = "atz";
char g_Sms_Ate0[] = "ate0";

char g_Sms_CR = 13;
char g_Sms_LF = 10;

char g_Sms_Reset[] = "AT$BWMODE=RESET";
char g_Sms_WhoRU[] = "at$phonenum?";
char g_Sms_Header[] = "at$smsmo=";
char g_Sms_TI[] = "4098";
char g_Sms_MsgEncode[] = "16";
char g_Sms_Reply[] = "1";
char g_Sms_Priority[] = "1";
char g_Sms_Comma[] = ",";
char g_Sms_Data[] = "[알림]경보가발생하였습니다 - ";

char g_MyAdmin[] = "01091292214";
char g_Sms_MyPhone[] = "01046270669";

typedef struct {
    unsigned int nPcm;
    unsigned int nPno;
} IfaceSms_WatchPoint_T;


#define IFACE_SMS_POINTCNT            2
#define IFACE_SMS_POINT_NAME_SIZE     20

IfaceSms_WatchPoint_T g_MyPoint[2] = {
{0, 1},
{0, 2}
}; 

char g_Sms_PointName[2][20] = {
"덕산메카시스 경보01",
"덕산메카시스 경보02"
};

/*******************************************************************/
// delay function 
static void IfaceSms_Sleep(int sec,int msec) 
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
static int IfaceSms_On_Send(void *p)
/****************************************************************/
{
	int i = 0;
	IfaceSms_T *uart2;
	
	uart2 = (IfaceSms_T *) p;
	//write( uart2->fd, uart2->txbuf, uart2->sendLength);
	for (i = 0; i < uart2->sendLength; i++) {
		write( uart2->fd, &uart2->txbuf[i], 1);		
		IfaceSms_Sleep(0,30);
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
static int IfaceSms_On_Recv_Handler(void *pIfaceSms) 
/****************************************************************/
{
	int i = 0;
	int iSlaveId = 0;
	int iLength = 0;
	//int iPcm = 0;
	//int iPno = 0;
	//float fVal[4];
	IfaceSms_T *p = NULL;
	unsigned char *pBuf;
	//unsigned short calCrc16;
	//unsigned short recvCrc16;
	unsigned char cType = 0;
	//unsigned char *pData;
	//unsigned int iCnt = 0;	
	//unsigned int iDataCnt = 0;
	int nChkVal = -1;
		
	/* Initialize */
	p = (IfaceSms_T *)pIfaceSms;
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
		// Get 'ok' Message
		if ( pBuf[i] == 'o' &&  pBuf[i+1] == 'k' )
			nChkVal = IFACE_SMS_RTN_OK;
		// Get '$006'
		else if ( pBuf[i] == '$' &&  pBuf[i+1] == '0' 
			&& pBuf[i+2] == '0' &&  pBuf[i+3] == '6')
			return IFACE_SMS_RTN_006;				
		// Get 'ok' Message
		else if ( pBuf[i] == 'O' &&  pBuf[i+1] == 'K' )
			nChkVal = IFACE_SMS_RTN_OK;			
	}	
	
	return nChkVal;
}


/****************************************************************/
// receive data function 
static int IfaceSms_On_Recv(void *p)
/****************************************************************/
{
	//int i = 0;
	int iRet = 0;;
	int iBufWp = 0;
	int iLoopCnt = 0;
	IfaceSms_T *uart2;
	unsigned char *pRxbuf;
	
	uart2 = (IfaceSms_T *) p;
	pRxbuf = (unsigned char *)uart2->rxbuf;
	memset( pRxbuf, 0, MAX_BUF_SIZE );

	while(1) {
		uart2->pollState = poll(					// poll()을 호출하여 event 발생 여부 확인     
			(struct pollfd*)&uart2->pollEvents,		// event 등록 변수
			1,  									// 체크할 pollfd 개수
			200);  									// time out 시간 (ms)	

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






/****************************************************************/
// creat interface
static IfaceSms_T *New_IfaceSms(void)
/****************************************************************/
{
	IfaceSms_T *uart2;
	
	uart2 = (IfaceSms_T *) malloc( sizeof(IfaceSms_T) );
	memset( uart2, 0, sizeof(IfaceSms_T) );

	uart2->fd = -1;
	uart2->pollState = 0;
	memset( &uart2->pollEvents, 0, sizeof(struct pollfd) );
	
	uart2->recvLength = 0;	
	uart2->sendLength = 0;
	uart2->rxbuf = (unsigned char *)&g_ifacSms_rxbuf;
	uart2->txbuf = (unsigned char *)&g_ifacSms_txbuf;
	memset( uart2->rxbuf, 0, MAX_BUF_SIZE );
	memset( uart2->txbuf, 0, MAX_BUF_SIZE );	

	memset(&prePoint, 0x00, sizeof(prePoint));
	
	return uart2;
}


/****************************************************************/
// close interface
static void IfaceSms_Close(IfaceSms_T *p)
/****************************************************************/
{
	close(p->fd);
	memset( p, 0, sizeof(IfaceSms_T) );
	p->fd = -1;
}


/****************************************************************/
static int IfaceSms_Open(IfaceSms_T *p)
/****************************************************************/
{
	struct termios oldtio, newtio;

	p->fd = open(UART2DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (p->fd < 0) { 
		fprintf(stderr, "[SMS] Serial g_iUart2Fd Open fail\n");
		fflush(stderr);
		return -1 ; 
	}
	else {
		fprintf(stderr, "[SMS] Serial g_iUart2Fd Open success\n");
		fflush(stderr);
	}
	
	if (tcgetattr(p->fd, &oldtio) < 0)
	{
		//perror("error in tcgetattr");
		fprintf(stderr, "[SMS] error in tcgetattr\n");
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


static void IfaceSms_SendCR(IfaceSms_T *p)
{
	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Sms_CR);
	memcpy(p->txbuf , &g_Sms_CR, p->sendLength); 
	
	IfaceSms_On_Send(p);
}


static void IfaceSms_SendLF(IfaceSms_T *p)
{
	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Sms_LF);
	memcpy(p->txbuf , &g_Sms_LF, p->sendLength); 
	
	IfaceSms_On_Send(p);
}


static int IfaceSms_SendATZ(IfaceSms_T *p)
// ----------------------------------------------------------------------------
// SEND 'ATZ' MESSAGE
// Description : It send 'atz' message to CDMA Modem
// Arguments   : p			Is a pointer of IfaceSms_T structure.
// Returns     : 1			Is a success (It get 'OK' message)
//				 -1			Is a fail
{
	int nRtnVal = 0;
	
	printf("%s\n", g_Sms_Atz);

	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Sms_Atz) - 1;
	memcpy(p->txbuf , g_Sms_Atz, p->sendLength); 
	
	IfaceSms_On_Send(p);
	
	IfaceSms_SendCR(p);
	IfaceSms_SendLF(p);
	
	if ( IfaceSms_On_Recv(p) > 0 ) {
		nRtnVal = IfaceSms_On_Recv_Handler(p);
	}
		
	if ( nRtnVal == IFACE_SMS_RTN_OK ) {
		printf(" 'OK' Message receive success\n");
		return 1;
	}
	else { 
		printf(" 'OK' Message receive fail\n");
		return -1;
	}	
}


static int IfaceSms_SendATE0(IfaceSms_T *p)
// ----------------------------------------------------------------------------
// SEND 'ATE0' MESSAGE
// Description : It send 'atz' message to CDMA Modem
// Arguments   : p			Is a pointer of IfaceSms_T structure.
// Returns     : 1			Is a success (It get 'OK' message)
//				 -1			Is a fail
{
	int nRtnVal = 0;
	
	printf("%s\n", g_Sms_Ate0);

	memset( p->txbuf, 0, MAX_BUF_SIZE );	
	
	p->sendLength = sizeof(g_Sms_Ate0) - 1;
	memcpy(p->txbuf , g_Sms_Ate0, p->sendLength); 
	
	IfaceSms_On_Send(p);
	IfaceSms_SendCR(p);
	IfaceSms_SendLF(p);
	
	if ( IfaceSms_On_Recv(p) > 0 ) {
		nRtnVal = IfaceSms_On_Recv_Handler(p);
	}
		
	if ( nRtnVal == IFACE_SMS_RTN_OK ) {
		printf(" 'OK' Message receive success\n");
		return 1;
	}
	else { 
		printf(" 'OK' Message receive fail\n");
		return -1;
	}	
}


static char HexToASCII(char *pBuf, int nShift, int nAnd)
{
	char chVal, chConverter;

	chConverter = (pBuf[0] & nAnd) >> nShift;

	if( (chConverter >= 0xa) && (chConverter <= 0xf) )
		chVal = chConverter + 'A' -0xa;
	else
		chVal = chConverter + '0';
        
    return  chVal;
}


static int IfaceSms_MakeMsg(unsigned char *pTx, int nIndex) 
{
    int i = 0;
    //int j = 0;
    int nLength = 0;
    
    memset(pTx, 0x00, sizeof(pTx));

    // 경보 알림 Message
    for (i = 0; i < strlen(g_Sms_Data); i++)
    {
		pTx[nLength++] = HexToASCII(&g_Sms_Data[i], 4, 0xf0);				    // Hi-Byte Checking
		pTx[nLength++] = HexToASCII(&g_Sms_Data[i], 0, 0x0f);				    // Low-Byte Checking
    }
        
    // 경보Point 이름.
    for (i = 0; i < IFACE_SMS_POINT_NAME_SIZE; i++)
    {
   		pTx[nLength++] = HexToASCII(&g_Sms_PointName[nIndex][i], 4, 0xf0);		// Hi-Byte Checking
		pTx[nLength++] = HexToASCII(&g_Sms_PointName[nIndex][i], 0, 0x0f);		// Low-Byte Checking
    }
    
    return nLength;	
	
}

static int IfaceSms_SendData(IfaceSms_T *p, int nIndex) 
{
    int nPt = 0;
    int nRtnVal = 0;
    int nDataLength = 0;
    unsigned char *pMsg;
    
    memset( p->txbuf, 0, MAX_BUF_SIZE );
    
    pMsg = (unsigned char *)p->txbuf;
    
    // create data message. and get data message length.
    nDataLength = IfaceSms_MakeMsg(g_ifacSms_msgbuf, nIndex);
    
    // create message send to BSM modem.
    memcpy(pMsg + nPt, &g_Sms_Header, strlen(g_Sms_Header));
    nPt += strlen(g_Sms_Header);      

	memcpy(pMsg + nPt, &g_MyAdmin, strlen(g_MyAdmin));
    nPt += strlen(g_MyAdmin);      

    memcpy(pMsg + nPt, &g_Sms_Comma, strlen(g_Sms_Comma));
    nPt += strlen(g_Sms_Comma);      

    memcpy(pMsg + nPt, &g_Sms_MyPhone, strlen(g_Sms_MyPhone));
    nPt += strlen(g_Sms_MyPhone);      

    memcpy(pMsg + nPt, &g_Sms_Comma, strlen(g_Sms_Comma));
    nPt += strlen(g_Sms_Comma);      

    memcpy(pMsg + nPt, &g_Sms_TI, strlen(g_Sms_TI));
    nPt += strlen(g_Sms_TI);      

    memcpy(pMsg + nPt, &g_Sms_Comma, strlen(g_Sms_Comma));
    nPt += strlen(g_Sms_Comma);      

    memcpy(pMsg + nPt, &g_Sms_MsgEncode, strlen(g_Sms_MsgEncode));
    nPt += strlen(g_Sms_MsgEncode);      

    memcpy(pMsg + nPt, &g_Sms_Comma, strlen(g_Sms_Comma));
    nPt += strlen(g_Sms_Comma);      

    memcpy(pMsg + nPt, &g_Sms_Reply, strlen(g_Sms_Reply));
    nPt += strlen(g_Sms_Reply);      

    memcpy(pMsg + nPt, &g_Sms_Comma, strlen(g_Sms_Comma));
    nPt += strlen(g_Sms_Comma);      

    memcpy(pMsg + nPt, &g_Sms_Priority, strlen(g_Sms_Priority));
    nPt += strlen(g_Sms_Priority);      

    memcpy(pMsg + nPt, &g_Sms_Comma, strlen(g_Sms_Comma));
    nPt += strlen(g_Sms_Comma); 

    memcpy(pMsg + nPt, g_ifacSms_msgbuf, nDataLength);
    nPt += nDataLength; 


	p->sendLength = nPt;
	
	IfaceSms_On_Send(p);
	IfaceSms_SendCR(p);
	IfaceSms_SendLF(p);
	
	IfaceSms_Sleep(10, 0);
	
	if ( IfaceSms_On_Recv(p) > 0 ) {
		nRtnVal = IfaceSms_On_Recv_Handler(p);
	}
		
	if ( nRtnVal == IFACE_SMS_RTN_006 ) {
		printf(" 'OK' Message receive success\n");
		return 1;
	}
	else { 
		printf(" 'OK' Message receive fail\n");
		return -1;
	}		
 	
}




/****************************************************************/
void *IfaceSms_Main(void* arg)
/****************************************************************/
{
	int nStatus = 0;
	int nIndexChk = 0;
	int nRetryCnt = 0;
	int nPcm = 0;
	int nPno = 0;
	//int nRecvCnt = 0;
	//point_info point;
	IfaceSms_T *pSms = NULL;

	pSms = New_IfaceSms();
	if (pSms == NULL) {
		printf("Can't create SMS interface.\n");
		//exit(1);
		system("killall duksan");
	}

	IfaceSms_Sleep(2, 0);

	printf("Start SMS interface.\n");
	while (1) {
					
		/* open uart2 */
		if ( IfaceSms_Open(pSms) < 0 ) {
			IfaceSms_Sleep(3, 0);
			IfaceSms_Close(pSms);
			continue;	
		}
		
		nStatus = IFACE_SMS_READY;
		for (;;) {
			switch(nStatus) {
			// MODEM으로 통신을 연결하기 위한 준비를 한다. 
			case IFACE_SMS_READY:
				printf("\n\n= IFACE_SMS_READY =============================\n");
				if ( IfaceSms_SendATZ(pSms) > 0 ) {
					IfaceSms_Sleep(1, 0);		
					nStatus = IFACE_SMS_CHECK_VALUE;
				}
				
				if ( IfaceSms_SendATE0(pSms) > 0 ) {
					IfaceSms_Sleep(1, 0);		
					nStatus = IFACE_SMS_CHECK_VALUE;
				}
								
				break;

			case IFACE_SMS_CHECK_VALUE:
				nIndexChk = 0;
				nStatus = IFACE_SMS_READY;
				for ( nIndexChk = 0; nIndexChk < IFACE_SMS_POINTCNT; nIndexChk++) {
					nPcm = g_MyPoint[nIndexChk].nPcm;
					nPno = g_MyPoint[nIndexChk].nPno; 
					if ( prePoint[nIndexChk] != (int)pGet(nPcm, nPno) ) {
						prePoint[nIndexChk] = (int)pGet(nPcm, nPno);
						if ( prePoint[nIndexChk] > 0 ) {
							nStatus = IFACE_SMS_SEND_MSG;
							break;
						}
					}
				}
				break;

			case IFACE_SMS_SEND_MSG:
				for ( nRetryCnt = 0; nRetryCnt < 3; nRetryCnt++ ) {
					if (IfaceSms_SendData(pSms, nIndexChk) > 0)
						break;	
					IfaceSms_Sleep(10, 0);						
				}
				IfaceSms_Sleep(5, 0);									
				nStatus = IFACE_SMS_CLOSE;
				break;

			// SMS 연결을 종료한다. 
			case IFACE_SMS_CLOSE:
				printf("\n\n= IFACE_SMS_CLOSE =============================\n");
				nStatus = IFACE_MODEM_READY;
				break;	
				
			default :
				nStatus = IFACE_MODEM_READY;
				break;								
			}
		}
	}
	syslog_record(SYSLOG_DESTROY_IFACE_SMS);	
}


