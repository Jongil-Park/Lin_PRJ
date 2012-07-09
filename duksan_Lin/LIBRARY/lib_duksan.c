/*
 * User�� ����� Library file�� �����Ѵ�.. 
 *  	1. Pset, Pget, Pdef�� �ش��ϴ� API�� ����ؾ� �Ѵ�. 
 *		2. UART�� ����� �����ϵ��� API�� ����ؾ� �Ѵ�.
 *
 *		��ü ����Ʈ���� share memory�� ����ؼ� 1�ʸ��� �ѹ��� �о������ ó���Ͽ���. 
 *		pset, pget, pdef�� API�� socket�� ���ؼ� �����ϵ��� ó���Ͽ���. 
 *
 * version :
 *		0.0.1  jong2ry working.								2011-03-08
 *		0.0.2  jong2ry code clean.							2011-03-08
 *
 *
 * code �ۼ��� ���ǻ���
 *		1. global ������ 'g_' ���λ縦 ����.
 *		2. ������ ������ ������ �Ʒ��� ���� ��Ģ���� �����Ѵ�. 
 *			int					n
 *			short				w
 *			float				f
 *			char				ch
 *			unsigned char       b
*			unsignnd int		un
 *			unsigned short 		uw 
 *			pointer				p
 *			structure			s
 *		3. �Լ����� �ҹ��ڷ� '_' ��ȣ�� ����ؼ� �����Ѵ�. 
 *		4. �Լ��� �Լ� ������ ������ 2�ٷ� �Ѵ�. 
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
#include <sys/time.h>										// gettimeofday();
#include <time.h>
#include <sys/un.h>
#include <sys/poll.h>										// use poll event
#include <sys/ipc.h>
#include <sys/shm.h>


#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

// share memory
#define  KEY_NUM    	 	9527			// share memory key
#define  SHARE_MEM_SIZE    	32786			// point table size

#include "lib_duksan.h"

#define UART2DEVICE 					"/dev/s3c2410_serial2"
#define MAX_BUF_SIZE					4024

#define b4800			0	
#define b9600			1
#define b19200			2
#define b115200			3

// not used hund timer
char _sec = 0;												// sec timer value
char _minute = 0;											// minute timer value
unsigned int g_nMyId = 0;									// My PCM Number

struct timeval now_time;									// gettimeofday now value
struct timeval pre_time;									// gettimeofday pre value

int client_socket;											// library file socket
struct sockaddr_un server_addr;								// library file socket address
point_queue lib_queue;										// lib queue	

//float g_ptbl[32][256];										// temp point table

unsigned char tx_msg[4028];									// tx message buffer
unsigned char rx_msg[4028];									// rx message buffer

int gn_id;													// my pcm number

int gn_sendtype;											// send message type.

float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];		// point-table only value

LIB_UART2_T *g_pUart2 = NULL;

// user application function.
extern void ApgMain(void);
extern void PointDefine(void);
extern void ApgInit(void);


void lib_sleep(int sec, int msec) 
// ----------------------------------------------------------------------------
// WAIT TIMER
// Description : use select function for timer.
// Arguments   : sec		Is a second value.
//				 usec		Is a micro-second value. 
// Returns     : none
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = msec * 1000;                
    select(0,NULL,NULL,NULL,&tv);
    return;
}


void initq(point_queue *p)
// ----------------------------------------------------------------------------
// INITIALIZE QUEUE
// Description : It initialize queue.
// Arguments   : p				Is a queue pointer.
// Returns     : none
{
	p->front = 0;
	p->rear = 0;
	memset( p->data, 0, sizeof(p->data) );
	return;
}

int queue_full(void)
// ----------------------------------------------------------------------------
// RETUREN QUEUE FULL 
// Description : return QUEUE_FULL
// Arguments   : none
// Returns     : QUEUE_FULL
{
	return QUEUE_FULL;
}


int queue_empty(void)
// ----------------------------------------------------------------------------
// RETUREN QUEUE EMPTY 
// Description : return QUEUE_EMPTY
// Arguments   : none
// Returns     : QUEUE_EMPTY
{
	return QUEUE_EMPTY;
}


int push_lib_queue(point_queue* p1, point_info *p2) 
// ----------------------------------------------------------------------------
// DATA PUT QUEUE
// Description : It push data in queue
// Arguments   : p1				Is a queue pointer.
// 				 p2				Is a point-value pointer.
// Returns     : none
{ 
	int prev_rear;

	prev_rear = p1->rear;
	
	p1->rear = (p1->rear + 1) % MAX_QUEUE_SIZE; 
	
	if ( p1->front == p1->rear ) { 
		p1->rear = prev_rear;
		return queue_full();        
	} 
	
	memcpy( &p1->data[p1->rear], p2, sizeof(point_info) );
	return SUCCESS;
}


int pop_lib_queue(point_queue *p1, point_info *p2) 
// ----------------------------------------------------------------------------
// DATA GET QUEUE
// Description : It pop data in queue
// Arguments   : p1				Is a queue pointer.
// 				 p2				Is a point-value pointer.
// Returns     : none
{ 	
	if ( p1->front == p1->rear ) {		
		return queue_empty(); 
	}

	p1->front = (p1->front + 1) % MAX_QUEUE_SIZE; 

	memcpy( p2, &p1->data[p1->front], sizeof(point_info) ); 
	return SUCCESS;
} 


void iPset(int n_data, float f_val)
// ----------------------------------------------------------------------------
// Set point value
// Description : User App���� ��û�� ����, lib_queue�� data���� ���� 
// 				 �޽����� ������. 
// Arguments   : n_data					Px���� ������ ��
// 				 f_val					Value
// Returns     : none
{
	point_info point;

	point.pcm = LIB_PCM(n_data);
	point.pno = LIB_PNO(n_data);
	point.value = f_val;
	point.message_type = LIB_SET_MSG_TYPE;

	push_lib_queue(&lib_queue, &point);	

	fprintf( stdout, ">>> APG iPset %d (%d %d) %f\n", n_data, LIB_PCM(n_data), LIB_PNO(n_data), f_val );
	fflush(stdout);
}


float iPget(int n_data) 
// ----------------------------------------------------------------------------
// Get point value
// Description : User App���� ��û�� ����, g_ptbl�� ���� �����Ѵ�. 
// 				 �׸��� ���ο� ���� �����ϱ� ���� lib_queue�� 
// 				 data���� ��û�ϴ� �޽����� ������. 
// Arguments   : n_data					Py���� ������ ��
// Returns     : value					User App�� ��û�� ��
{
	return g_fExPtbl[LIB_PCM(n_data)][LIB_PNO(n_data)];
}


int lib_uart2_open(int nBaudrate)
{
	struct termios oldtio, newtio;
	
	if (g_pUart2 != NULL) {
		return -1;
	}
	
	g_pUart2 = (LIB_UART2_T *) malloc( sizeof(LIB_UART2_T) );
	memset( g_pUart2, 0, sizeof(LIB_UART2_T) );

	g_pUart2->nFd = -1;
	g_pUart2->nPollState = 0;
	memset( &g_pUart2->sPollEvents, 0, sizeof(struct pollfd) );
	
	/*
	g_pUart2->pchTxBuf = (unsigned char *) malloc (MAX_BUF_SIZE);
	g_pUart2->pchRxBuf = (unsigned char *) malloc (MAX_BUF_SIZE);
	memset( g_pUart2->pchTxBuf, 0, MAX_BUF_SIZE );
	memset( g_pUart2->pchRxBuf, 0, MAX_BUF_SIZE );	
	*/	

	//* uart2 initiailze 
	g_pUart2->nFd = open(UART2DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (g_pUart2->nFd < 0) { 
		free(g_pUart2);
		g_pUart2 = NULL;
		fprintf( stdout, "[USER] Close Uart2\n");
		fprintf( stdout, "[USER] Open Uart2 Fail\n");
		fflush(stdout);			
		return -1; 
	}
	
	if (tcgetattr(g_pUart2->nFd, &oldtio) < 0) {
		free(g_pUart2);
		g_pUart2 = NULL;
		fprintf( stdout, "[USER] Close Uart2\n");
		perror("error in tcgetattr");
		fflush(stdout);			
		return -1; 
	}
	
	bzero(&newtio, sizeof(newtio));
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	switch(nBaudrate) {
		case b4800:
			newtio.c_cflag = B4800 | CS8 | CLOCAL | CREAD;
			break;
		case b9600:
			newtio.c_cflag = B9600| CS8 | CLOCAL | CREAD;
			break;
		case b19200:
			newtio.c_cflag = B19200 | CS8 | CLOCAL | CREAD;
			break;
		case b115200:
			newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
			break;	
		default :
			newtio.c_cflag = B19200 | CS8 | CLOCAL | CREAD;
			break;											
	}

	newtio.c_cflag = B19200 | CS8 | CLOCAL | CREAD;
	
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN]  = 1;
	
	tcflush(g_pUart2->nFd, TCIFLUSH);
	tcsetattr(g_pUart2->nFd,TCSANOW,&newtio);	

	fcntl(g_pUart2->nFd, F_SETFL, FNDELAY); 

	/* poll initiailze */
	g_pUart2->sPollEvents.fd = g_pUart2->nFd; 
	/*
		#define POLLIN 0x0001 // ���� �����Ͱ� �ִ�. 
		#define POLLPRI 0x0002 // ����� ���� ����Ÿ�� �ִ�. 
		#define POLLOUT 0x0004 // ���Ⱑ ����(block)�� �ƴϴ�. 
		#define POLLERR 0x0008 // �����߻� 
		#define POLLHUP 0x0010 // ������ ������ 
		#define POLLNVAL 0x0020 // ���������ڰ� ������ ���� �� ����, Invalid request (�߸��� ��û) 	
	*/
	g_pUart2->sPollEvents.events  = POLLIN | POLLERR;
	g_pUart2->sPollEvents.revents = 0;	

	fprintf( stdout, "[USER] Open Uart2\n");
	fflush(stdout);	
	
	return 1;		
}


int lib_uart2_send(unsigned char *pTx, unsigned int nLength, int nDebug)
{
	int i = 0;
	
	if (g_pUart2 == NULL || g_pUart2->nFd <= 0) {
		return -1;
	}
		
	write( g_pUart2->nFd, pTx, nLength);

	if ( nDebug > 0 ) {
		fprintf( stdout, "Tx = ");
		for (i = 0; i < nLength; i++) {
			fprintf( stdout, "%x ", pTx[i]);
		}
		fprintf( stdout, "\n");
		fflush(stdout);	
	}
	return 0;
}


int lib_uart2_recv(unsigned char *pRx, unsigned int nLength, int nDebug)
{
	int nRecvByte = 0;;
	int nBufWp = 0;
	int iLoopCnt = 0;
	
	while(1) {
		g_pUart2->nPollState = poll(					// poll()�� ȣ���Ͽ� event �߻� ���� Ȯ��     
			(struct pollfd*)&g_pUart2->sPollEvents,		// event ��� ����
			1,  										// üũ�� pollfd ����
			100);  										// time out �ð� (ms)	

		if ( 0 < g_pUart2->nPollState) {                            // �߻��� event �� ����
			if ( g_pUart2->sPollEvents.revents & POLLIN) {			// event �� �ڷ� ����?
				nRecvByte = read(g_pUart2->nFd, &pRx[nBufWp], 32);
				nBufWp += nRecvByte;
				iLoopCnt = 0;
			}
			else if ( g_pUart2->sPollEvents.revents & POLLERR) {	// event �� ����?
				fprintf( stdout, "[USER] Event is Error. Terminal Broken.\n");
				fflush(stdout);					
				return -1;
			}
		}
		else {
			if ( 5 < iLoopCnt++ ) {
				
				fprintf( stdout, "[USER] Time Out %d.\n", nBufWp);
				fflush(stdout);	
								
				if (nBufWp == 0)
					nBufWp = -1;	
					
				return nBufWp;
			}
		}
	}	
}


int lib_uart2_close(int nBaudrate)
{
	free(g_pUart2);
	g_pUart2 = NULL;
	fprintf( stdout, "[USER] Close Uart2\n");
	fflush(stdout);	
}

LIB_T *lib_init(void) 
{
	LIB_T *pLib;
	
	pLib = (LIB_T *)malloc( sizeof(LIB_T) );
	
	pLib->nFd = -1;
	
	pLib->chRxBuf =  (unsigned char *)malloc (LIB_BUF_SIZE);
	pLib->chTxBuf = (unsigned char *)malloc (LIB_BUF_SIZE);
	pLib->chPrevBuf = (unsigned char *)malloc (LIB_BUF_SIZE);	
	
	memset( pLib->chRxBuf , 0x00, LIB_BUF_SIZE );
	memset( pLib->chTxBuf, 0x00, LIB_BUF_SIZE );	
	memset( pLib->chPrevBuf, 0x00, LIB_BUF_SIZE );	
	
	pLib->nTempWp = 0;
	pLib->nRecvByte = 0;
	pLib->nIndexPrev = 0;

	return pLib;
}


int lib_open(LIB_T *p) 
{
	struct sockaddr_in server_addr;
	struct timeval timeo;
	
	long arg; 		
	int res; 
	fd_set myset; 
	struct timeval tv; 
	int valopt; 
	socklen_t lon;
		
	memset( &server_addr, 0, sizeof(server_addr) );
	timeo.tv_sec = 0;
	timeo.tv_usec = 10000;
	
	fprintf(stdout, "%s()\n", __FUNCTION__); 
	fflush( stdout );		
	
	p->nFd = socket( AF_INET, SOCK_STREAM, 0 );
	if( p->nFd < 0 ) {
		fprintf( stdout, "+ [LIB Socket creation error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;					
	}

	setsockopt( p->nFd, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo) );

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	server_addr.sin_port = htons( 9923 );

	//fprintf( stdout, "+ [LIB] Try Connect to 3iStation %s\n", p->chServerIp );
	//fflush( stdout );	

	// Set non-blocking 
	// ��ó:[LINUX] linux���� network ���ӽ� non-block���� �����ϱ�..
	if( (arg = fcntl(p->nFd, F_GETFL, NULL)) < 0) { 
		fprintf(stdout, "LIB Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( p->nFd );
		return -1;
	} 
	
	arg |= O_NONBLOCK; 
	if( fcntl(p->nFd, F_SETFL, arg) < 0) { 
		fprintf(stdout, "LIB Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( p->nFd );
		return -1;
	}

	// Trying to connect with timeout 
	res =  connect(p->nFd, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
	if (res < 0) { 
		if (errno == EINPROGRESS) { 
			//fprintf(stdout, "LIB Progress in connect() - selecting\n"); 
			do { 
				tv.tv_sec = 5;  // 5���Ŀ� ������������ �Ѵ�.
				tv.tv_usec = 0; 
				FD_ZERO(&myset); 
				FD_SET(p->nFd, &myset); 
				res = select(p->nFd+1, NULL, &myset, NULL, &tv); 
				if (res < 0 && errno != EINTR) { 
					//fprintf(stdout, "LIB Error connecting %d - %s\n", errno, strerror(errno)); 
					//fflush( stdout );
					close( p->nFd );
					return -1; 
				} 
				else if (res > 0) { 
					// Socket selected for write 
					lon = sizeof(int); 
					if (getsockopt(p->nFd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
						//fprintf(stdout, "LIB Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
						//fflush( stdout );
						close( p->nFd );
						return -1;
					} 
					// Check the value returned... 
					if (valopt) { 
						fprintf( stdout, "+ [LIB] connection() %d - %s\n", valopt, strerror(valopt)); 
						fflush( stdout );							
						close( p->nFd );
						return -1;
					} 
					break; 
				} 
				else { 
					//fprintf(stdout, "LIB Timeout in select() - Cancelling!\n"); 
					//fflush( stdout );
					close( p->nFd );
					return -1;
				} 
			} while (1); 
		} 
		else { 
			//fprintf(stdout, "LIB Error connecting %d - %s\n", errno, strerror(errno)); 
			//fflush( stdout );
			close( p->nFd );
			return -1;
		} 
	} 

	// Set to blocking mode again... 
	if( (arg = fcntl(p->nFd, F_GETFL, NULL)) < 0) { 
		fprintf(stdout, "LIB Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( p->nFd );
		return -1; 
	} 
	
	arg &= (~O_NONBLOCK); 
	if( fcntl(p->nFd, F_SETFL, arg) < 0) { 
		fprintf(stdout, "LIB Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( p->nFd );
		return -1; 
	}

	return p->nFd;		
}


void lib_close(LIB_T *p)
{
	close(p->nFd);

	p->nFd = -1;
	p->nTempWp = 0;
	p->nRecvByte = 0;

	memset( p->chPrevBuf, 0, LIB_BUF_SIZE );
	memset( p->chRxBuf, 0, LIB_BUF_SIZE );
	memset( p->chTxBuf, 0, LIB_BUF_SIZE );	
	
	fprintf(stdout, "%s()\n", __FUNCTION__); 
	fflush( stdout );	
}


int lib_recevie_message(LIB_T *p, unsigned char  *pchBuf)
{
	int nRecvBytes = 0;
	
	nRecvBytes = read(p->nFd, pchBuf, LIB_BUF_SIZE - 1);
	
	return nRecvBytes;
}


void lib_parsing(unsigned char *pchBuf)
{
	int i = 0;
	char nMsgType = 0x00;
	char chChkSum = 0x00;
	unsigned short wPcm = 0;
	unsigned short wLength = 0;
	LIB_INFO_MSG_T *pPcmInfoMsg = NULL;	

	fprintf( stdout, "[LIB] Parsing\n" );
	fflush(stdout);	

	nMsgType = pchBuf[3];
	switch(nMsgType){
		// ack message �� �ƹ��� ó���� ���� �ʴ´�. 
		case LIB_ACK_MSG_TYPE:
			break;			

		// fail message �� �ƹ��� ó���� ���� �ʴ´�. 
		case LIB_FAIL_MSG_TYPE:
			break;			
						
		// �ڽ��� ID�� �����Ѵ�. 
		case LIB_INFO_MSG_TYPE:
			// get pointer
			pPcmInfoMsg = (LIB_INFO_MSG_T *)pchBuf;
					
			// get message length
			wLength = sizeof(LIB_INFO_MSG_T); 
		
			// check length
			if ( wLength != pPcmInfoMsg->wLength ) {
				g_nMyId = 31;
				return;
			}
					
			// check my pcm number
			g_nMyId = pPcmInfoMsg->wPcm;
			if ( g_nMyId >= 32 ) {
				g_nMyId = 31;
				return;			
			}
			break;			
			
		default:
			return;
	}	
}


void lib_handler(LIB_T *p, unsigned char *pchTemp)
{
	int nRecvByte = 0;
	int nOffset = 0;
	int nIndex = 0;
	int nMsgLength = 0;
	
	nRecvByte = p->nRecvByte;

	// ���� buffer�� ���� ����ִٸ�, 
	// ���� buffer�� �����ؼ� Packet�� �����. 
	if ( p->nIndexPrev > 0  ) {
		memcpy(pchTemp, p->chRxBuf, nRecvByte);
		memcpy(p->chRxBuf, p->chPrevBuf, p->nIndexPrev);
		memcpy(&p->chRxBuf[p->nIndexPrev], pchTemp, nRecvByte);
		nRecvByte += p->nIndexPrev;
		p->nIndexPrev = 0;
	}	
	
	while (nOffset < nRecvByte)
	{
		if ( p->chRxBuf[nOffset] != '<' ) {
			nOffset++;
		}
		
		// calculate message length
		if ( p->chRxBuf[nOffset + 3] == LIB_ACK_MSG_TYPE) {
			nMsgLength = sizeof(LIB_ACK_MSG_T);
		}		
		else if ( p->chRxBuf[nOffset + 3] == LIB_INFO_MSG_TYPE ) {
			nMsgLength = sizeof(LIB_INFO_MSG_T);
		}
		else {
			// else�� ���ǿ��� �׻� �ּ����� ���̰� �־����� �Ѵ�. 	
			nMsgLength = sizeof(LIB_ACK_MSG_T);
		}

		nIndex = nOffset + nMsgLength;	
		if (nIndex <= nRecvByte) {
			// parsing message
			if ( p->chRxBuf[nOffset + 3] == LIB_ACK_MSG_TYPE) {
				lib_parsing(&p->chRxBuf[nOffset]);
			}	
			else if ( p->chRxBuf[nOffset + 3] == LIB_INFO_MSG_TYPE ) {
				lib_parsing(&p->chRxBuf[nOffset]);
			}		
		}
		else {
			p->nIndexPrev = nRecvByte - nOffset;
			memset(p->chPrevBuf, 0x00, sizeof(p->chPrevBuf));
			memcpy(p->chPrevBuf, &p->chRxBuf[nOffset], p->nIndexPrev);
			break;
		}
		nOffset = nIndex;
	}
	
	// protection code.
	if ( p->nIndexPrev < 0 ) {
		p->nIndexPrev = 0;
	}	
}

void lib_send_set_message(LIB_T *p, point_info *pPoint)
{
	int i = 0;
	LIB_PCM_SET_MSG_T *pTx = NULL;
	unsigned short wLength = 0;
	unsigned char chChkSum = 0;
		
	fprintf(stdout, "APG %s()\n", __FUNCTION__);
	fflush(stdout);		
	
	pTx = (LIB_PCM_SET_MSG_T *) p->chTxBuf;
	wLength = sizeof(LIB_PCM_SET_MSG_T);
	
	// check message type
	if ( pPoint->message_type != LIB_SET_MSG_TYPE )
		return;
		
	// make message
	pTx->chStx = '<';
	pTx->wLength = wLength;
	pTx->wCmd = LIB_SET_MSG_TYPE;
	pTx->wPcm = pPoint->pcm;
	pTx->wPno = pPoint->pno;
	pTx->fValue = pPoint->value;
	chChkSum = 0;
	for (i = 0; i < wLength; i++)
		chChkSum += p->chTxBuf[i];	
	pTx->wChkSum = chChkSum;			
	pTx->chEtx = '>';

	// debug		
	printf("Tx = ");	
	for ( i = 0; i < wLength; i++ ) {
		printf("%x ", p->chTxBuf[i]);	
	}
	printf("\n");	
	
	// send message
	send(p->nFd, pTx, wLength, 0);		
}


void lib_send_def_message(LIB_T *p, point_info *pPoint)
{
	fprintf(stdout, "APG %s()\n", __FUNCTION__);
	fflush(stdout);			
}


void lib_send_info_message(LIB_T *p, point_info *pPoint)
{
	int i = 0;
	LIB_INFO_MSG_T *pTx = NULL;
	unsigned short wLength = 0;
	unsigned char chChkSum = 0;
		
	fprintf(stdout, "APG %s()\n", __FUNCTION__);
	fflush(stdout);		
	
	pTx = (LIB_INFO_MSG_T *) p->chTxBuf;
	wLength = sizeof(LIB_INFO_MSG_T);
	
	// check message type
	if ( pPoint->message_type != LIB_INFO_MSG_TYPE )
		return;
				
	// make message
	pTx->chStx = '<';
	pTx->wLength = wLength;
	pTx->wCmd = LIB_INFO_MSG_TYPE;
	pTx->wPcm = pPoint->pcm;
	chChkSum = 0;
	for (i = 0; i < wLength; i++)
		chChkSum += p->chTxBuf[i];	
	pTx->wChkSum = chChkSum;			
	pTx->chEtx = '>';

	// debug		
	printf("Tx = ");	
	for ( i = 0; i < wLength; i++ ) {
		printf("%x ", p->chTxBuf[i]);	
	}
	printf("\n");	
	
	// send message
	send(p->nFd, pTx, wLength, 0);			
}

int main(void)
// ----------------------------------------------------------------------------
// MAIN LOOP
// Description : hund, sec, minute timer�� Value ���� �����Ѵ�. 
// 				 ApgMain() �� ȣ���Ѵ�. 
// Arguments   : sec		Is a second value.
//				 usec		Is a micro-second value. 
// Returns     : none
{
	int n_length = 0;
	int n_chk_time = 0;
	int n5MsecCount = 0;
	unsigned short wReadPcm = 0;
	LIB_T	*pLib = NULL;
	unsigned char *pchTemp;
	point_info point;
	int nReciveByte = 0;

	int   shm_id;
	void *shm_addr;
   
	int nQueueState = 0;
	
	gettimeofday(&now_time, NULL);
	gettimeofday(&pre_time, NULL);

	initq(&lib_queue);
	pchTemp = (unsigned char *)malloc (LIB_BUF_SIZE);
	memset (pchTemp, 0x00,  MAX_BUFFER_SIZE);		
	
	pLib = lib_init();
	if ( pLib == NULL ) {
		fprintf( stdout, "[LIB] Init Error\n" );
		fflush(stdout);		
	}

	// share memory init
	if ( -1 == ( shm_id = shmget( (key_t)KEY_NUM, SHARE_MEM_SIZE, IPC_CREAT|0666))) {
		fprintf(stdout, "APG Share Memory Initialize \t\t\t [  FAIL  ]\n");
		fflush(stdout);	
		return -1;
	} 
	
	if ( ( void *)-1 == ( shm_addr = shmat( shm_id, ( void *)0, 0))) {
		fprintf(stdout, "APG Share Memory Add \t\t\t\t [  FAIL  ]\n");
		fflush(stdout);	
		return -1;
	} 

	lib_sleep( 1, 0 );
	memcpy((float *)g_fExPtbl, (float *)shm_addr, SHARE_MEM_SIZE );		// �����޸� ����
	fprintf(stdout, "APG First Get Value \t\t\t\t [  OK  ]\n");

	// push information message
	point.pcm = 0;
	point.pno = 0;
	point.value = 0;
	point.message_type = LIB_INFO_MSG_TYPE;
	push_lib_queue( &lib_queue, &point );

	while(1) {
		// library socket open
		if ( lib_open( pLib ) < 0 ) {
			lib_close( pLib );
			lib_sleep( 5, 0 );
			continue;	
		}

		ApgInit();
		PointDefine();

		for (;;) {		
			// library queue�� Ȯ���Ѵ�. 			
			nQueueState = pop_lib_queue( &lib_queue, &point );	
			if ( nQueueState == SUCCESS ) {	
				switch( point.message_type ) {
					// send set message
					case LIB_SET_MSG_TYPE:
						lib_send_set_message(pLib, &point);
						lib_sleep( 0, 5 );
						nReciveByte = lib_recevie_message(pLib, pchTemp);
						
						//printf("nReciveByte = %d\n", nReciveByte);
						if ( nReciveByte > 0 )
							lib_handler(pLib, pchTemp);
						continue;
					
					// send point define message
					case LIB_DEF_MSG_TYPE:
						lib_send_def_message(pLib, &point);
						lib_sleep( 0, 5 );
						nReciveByte = lib_recevie_message(pLib, pchTemp);
						
						//printf("nReciveByte = %d\n", nReciveByte);
						if ( nReciveByte > 0 )
							lib_handler(pLib, pchTemp);
						continue;	

					// send information message
					case LIB_INFO_MSG_TYPE:
						lib_send_info_message(pLib, &point);
						lib_sleep( 0, 5 );
						nReciveByte = lib_recevie_message(pLib, pchTemp);
						
						//printf("nReciveByte = %d\n", nReciveByte);
						if ( nReciveByte > 0 )
							lib_handler(pLib, pchTemp);
						continue;	
					
					// default message						
					default:
						continue;
				}
				continue;
			}
			else {
				lib_sleep( 0, 100 );
			}			
			
			// 1�� ������ ���� �ٲ��� Check�Ѵ�. _sec ���� ��ȭ��Ų��. 
			gettimeofday(&now_time, NULL);
			if ( now_time.tv_sec != pre_time.tv_sec ) {
				pre_time.tv_sec = now_time.tv_sec;

				// 1�ʸ��� share memory�� point table�� ���� �����Ѵ�.
				memcpy((float *)g_fExPtbl, (float *)shm_addr, SHARE_MEM_SIZE );		// �����޸� ����
				
				//printf("g_nMyId = %d\n", g_nMyId);
				
				// sec timer value init
				_sec++;
				if ( _sec >= 60 ) {
					_sec = 0;
					_minute++;
			
					// minute timer value init
					if ( _minute >= 60 ) {
						_minute++;
					}
				}
			}
			
			ApgMain();		
			continue;
		}
		lib_close( pLib );
		lib_sleep( 5, 0 );		
		continue;
	}
	free(pchTemp);
	free(pLib->chRxBuf);
	free(pLib->chTxBuf);
	free(pLib->chPrevBuf);

	return 0;
}

#if 0

// ----------------------------------------------------------------------------
// POINT DEFINE FUCNTION 
// Description : �Ʒ��� function���� point define�� ���� function ���̴�. 
// 				 Apg app���� Call�Ǹ�, �ڽ��� Point define ���� �ƴϸ� 
// 				 �����Ѵ�. 
//
// Reference code.
// ---------------------------------------------------------------------------- 
// PdefDo2s(pcm, pno++, INIT_RESUME); 
// PdefVo(pcm, pno++, 0.5, 1, 0, 0, 100);       
// PdefJpt1000(pcm, pno++, 3.0, 0, -30, 130);
// PdefDi2s(pcm, pno++);
// PdefVi(pcm, 11, 2.0,  1,   0,  0, 100);  
// PdefCi(pcm, 16, 1.0, 1.25, -20, 0, 100);
// PdefVr(pcm, pno++, 0.1, -100, 150);


void PdefDo2s(byte pcm, byte pno, byte opt)
// ----------------------------------------------------------------------------
// POINT DEFINE DO
{
	int i = 0;
	int n_cnt = 0;
	int n_pnt = 0;
	int n_length = 0;
	unsigned char c_chksum = 0;
	APG_PDEF_FORMAT_T *p_data;

	memset( tx_msg, 0x00, sizeof(tx_msg) );

	// check pcm number
	if ( pcm != gn_id ) 
		return;

	// Create apg socket
	if ( lib_create_sock() < 0 ) {
		exit(0);
	}

	// change type
	gn_sendtype = LIB_SEND_TYPE_PDEF;

	// get data pointer
	n_pnt = 2;
	p_data = (APG_PDEF_FORMAT_T *) &tx_msg[n_pnt];	
	
	// get value
	p_data->c_cmd = LIB_PDEF_REQUEST;
	p_data->c_pcm = pcm;
	p_data->w_pno = pno;
	p_data->f_type = LIB_PDEF_DO;
	p_data->f_hyst = 0.5;
	p_data->f_scale = 1;
	p_data->f_offset = 0;
	p_data->f_min = 0;
	p_data->f_max = 1;
	
	// set total length
	n_length = 32;
	tx_msg[0] = 0x00;
	tx_msg[1] = n_length;

	// calculate total chksum
	c_chksum = 0;
	for ( i = 0; i < n_length - 1; i++ ) {
		c_chksum -= tx_msg[i];
	}
	tx_msg[n_length - 1] = c_chksum;

	// send message
	if( write( client_socket, tx_msg, n_length ) <= 0) {
		fprintf( stdout, "[ERR] Send in %s()\n", __FUNCTION__ );
		fflush(stdout);
		return;
	}	

	lib_sleep(0, 5);

	// receive message and handler message....!!! 
	n_length = lib_recv_msg();
	if ( n_length > 0 ) {
		lib_handler();
	}

	close(client_socket);
}


void PdefVo(byte pcm, byte pno, float f_hyst, float f_scale, float f_offset, float f_min, float f_max)
// ----------------------------------------------------------------------------
// POINT DEFINE VO
{
	int i = 0;
	int n_cnt = 0;
	int n_pnt = 0;
	int n_length = 0;
	unsigned char c_chksum = 0;
	APG_PDEF_FORMAT_T *p_data;

	memset( tx_msg, 0x00, sizeof(tx_msg) );

	// check pcm number
	if ( pcm != gn_id ) 
		return;

	// Create apg socket
	if ( lib_create_sock() < 0 ) {
		exit(0);
	}

	// change type
	gn_sendtype = LIB_SEND_TYPE_PDEF;


	// get data pointer
	n_pnt = 2;
	p_data = (APG_PDEF_FORMAT_T *) &tx_msg[n_pnt];	
	
	// get value
	p_data->c_cmd = LIB_PDEF_REQUEST;
	p_data->c_pcm = pcm;
	p_data->w_pno = pno;
	p_data->f_type = LIB_PDEF_VO;
	p_data->f_hyst = f_hyst;
	p_data->f_scale = f_scale;
	p_data->f_offset = f_offset;
	p_data->f_min = f_min;
	p_data->f_max = f_max;
	
	// set total length
	n_length = 32;
	tx_msg[0] = 0x00;
	tx_msg[1] = n_length;

	// calculate total chksum
	c_chksum = 0;
	for ( i = 0; i < n_length - 1; i++ ) {
		c_chksum -= tx_msg[i];
	}
	tx_msg[n_length - 1] = c_chksum;

	// send message
	if( write( client_socket, tx_msg, n_length ) <= 0) {
		fprintf( stdout, "[ERR] Send in %s()\n", __FUNCTION__ );
		fflush(stdout);
		return;
	}	

	lib_sleep(0, 5);

	// receive message and handler message....!!! 
	n_length = lib_recv_msg();
	if ( n_length > 0 ) {
		lib_handler();
	}

	close(client_socket);
}


void PdefJpt1000(byte pcm, byte pno, float f_hyst, float f_offset, float f_min, float f_max)
// ----------------------------------------------------------------------------
// POINT DEFINE JPT1000
{
	int i = 0;
	int n_cnt = 0;
	int n_pnt = 0;
	int n_length = 0;
	unsigned char c_chksum = 0;
	APG_PDEF_FORMAT_T *p_data;

	memset( tx_msg, 0x00, sizeof(tx_msg) );

	// check pcm number
	if ( pcm != gn_id ) 
		return;

	// Create apg socket
	if ( lib_create_sock() < 0 ) {
		exit(0);
	}

	// change type
	gn_sendtype = LIB_SEND_TYPE_PDEF;


	// get data pointer
	n_pnt = 2;
	p_data = (APG_PDEF_FORMAT_T *) &tx_msg[n_pnt];	
	
	// get value
	p_data->c_cmd = LIB_PDEF_REQUEST;
	p_data->c_pcm = pcm;
	p_data->w_pno = pno;
	p_data->f_type = LIB_PDEF_JPT1000;
	p_data->f_hyst = f_hyst;
	p_data->f_scale = 1;
	p_data->f_offset = f_offset;
	p_data->f_min = f_min;
	p_data->f_max = f_max;
	
	// set total length
	n_length = 32;
	tx_msg[0] = 0x00;
	tx_msg[1] = n_length;

	// calculate total chksum
	c_chksum = 0;
	for ( i = 0; i < n_length - 1; i++ ) {
		c_chksum -= tx_msg[i];
	}
	tx_msg[n_length - 1] = c_chksum;

	// send message
	if( write( client_socket, tx_msg, n_length ) <= 0) {
		fprintf( stdout, "[ERR] Send in %s()\n", __FUNCTION__ );
		fflush(stdout);
		return;
	}	

	lib_sleep(0, 5);

	// receive message and handler message....!!! 
	n_length = lib_recv_msg();
	if ( n_length > 0 ) {
		lib_handler();
	}

	close(client_socket);
}


void PdefDi2s(byte pcm, byte pno)
// ----------------------------------------------------------------------------
// POINT DEFINE DI2S
{
	int i = 0;
	int n_cnt = 0;
	int n_pnt = 0;
	int n_length = 0;
	unsigned char c_chksum = 0;
	APG_PDEF_FORMAT_T *p_data;

	memset( tx_msg, 0x00, sizeof(tx_msg) );

	// check pcm number
	if ( pcm != gn_id ) 
		return;

	// Create apg socket
	if ( lib_create_sock() < 0 ) {
		exit(0);
	}

	// change type
	gn_sendtype = LIB_SEND_TYPE_PDEF;


	// get data pointer
	n_pnt = 2;
	p_data = (APG_PDEF_FORMAT_T *) &tx_msg[n_pnt];	
	
	// get value
	p_data->c_cmd = LIB_PDEF_REQUEST;
	p_data->c_pcm = pcm;
	p_data->w_pno = pno;
	p_data->f_type = LIB_PDEF_DI2S;
	p_data->f_hyst = 0.5;
	p_data->f_scale = 1;
	p_data->f_offset = 0;
	p_data->f_min = 0;
	p_data->f_max = 1;
	
	// set total length
	n_length = 32;
	tx_msg[0] = 0x00;
	tx_msg[1] = n_length;

	// calculate total chksum
	c_chksum = 0;
	for ( i = 0; i < n_length - 1; i++ ) {
		c_chksum -= tx_msg[i];
	}
	tx_msg[n_length - 1] = c_chksum;

	// send message
	if( write( client_socket, tx_msg, n_length ) <= 0) {
		fprintf( stdout, "[ERR] Send in %s()\n", __FUNCTION__ );
		fflush(stdout);
		return;
	}	

	lib_sleep(0, 5);

	// receive message and handler message....!!! 
	n_length = lib_recv_msg();
	if ( n_length > 0 ) {
		lib_handler();
	}

	close(client_socket);
}


void PdefVi(byte pcm, byte pno, float f_hyst, float f_scale, float f_offset, float f_min, float f_max)
// ----------------------------------------------------------------------------
// POINT DEFINE VI
{
	int i = 0;
	int n_cnt = 0;
	int n_pnt = 0;
	int n_length = 0;
	unsigned char c_chksum = 0;
	APG_PDEF_FORMAT_T *p_data;

	memset( tx_msg, 0x00, sizeof(tx_msg) );

	// check pcm number
	if ( pcm != gn_id ) 
		return;

	// Create apg socket
	if ( lib_create_sock() < 0 ) {
		exit(0);
	}

	// change type
	gn_sendtype = LIB_SEND_TYPE_PDEF;


	// get data pointer
	n_pnt = 2;
	p_data = (APG_PDEF_FORMAT_T *) &tx_msg[n_pnt];	
	
	// get value
	p_data->c_cmd = LIB_PDEF_REQUEST;
	p_data->c_pcm = pcm;
	p_data->w_pno = pno;
	p_data->f_type = LIB_PDEF_VI;
	p_data->f_hyst = f_hyst;
	p_data->f_scale = f_scale;
	p_data->f_offset = f_offset;
	p_data->f_min = f_min;
	p_data->f_max = f_max;
	
	// set total length
	n_length = 32;
	tx_msg[0] = 0x00;
	tx_msg[1] = n_length;

	// calculate total chksum
	c_chksum = 0;
	for ( i = 0; i < n_length - 1; i++ ) {
		c_chksum -= tx_msg[i];
	}
	tx_msg[n_length - 1] = c_chksum;

	// send message
	if( write( client_socket, tx_msg, n_length ) <= 0) {
		fprintf( stdout, "[ERR] Send in %s()\n", __FUNCTION__ );
		fflush(stdout);
		return;
	}	

	lib_sleep(0, 5);

	// receive message and handler message....!!! 
	n_length = lib_recv_msg();
	if ( n_length > 0 ) {
		lib_handler();
	}

	close(client_socket);
}


void PdefCi(byte pcm, byte pno, float f_hyst, float f_scale, float f_offset, float f_min, float f_max)
// ----------------------------------------------------------------------------
// POINT DEFINE CI
{
	int i = 0;
	int n_cnt = 0;
	int n_pnt = 0;
	int n_length = 0;
	unsigned char c_chksum = 0;
	APG_PDEF_FORMAT_T *p_data;

	memset( tx_msg, 0x00, sizeof(tx_msg) );

	// check pcm number
	if ( pcm != gn_id ) 
		return;

	// Create apg socket
	if ( lib_create_sock() < 0 ) {
		exit(0);
	}

	// change type
	gn_sendtype = LIB_SEND_TYPE_PDEF;


	// get data pointer
	n_pnt = 2;
	p_data = (APG_PDEF_FORMAT_T *) &tx_msg[n_pnt];	
	
	// get value
	p_data->c_cmd = LIB_PDEF_REQUEST;
	p_data->c_pcm = pcm;
	p_data->w_pno = pno;
	p_data->f_type = LIB_PDEF_CI;
	p_data->f_hyst = f_hyst;
	p_data->f_scale = f_scale;
	p_data->f_offset = f_offset;
	p_data->f_min = f_min;
	p_data->f_max = f_max;
	
	// set total length
	n_length = 32;
	tx_msg[0] = 0x00;
	tx_msg[1] = n_length;

	// calculate total chksum
	c_chksum = 0;
	for ( i = 0; i < n_length - 1; i++ ) {
		c_chksum -= tx_msg[i];
	}
	tx_msg[n_length - 1] = c_chksum;

	// send message
	if( write( client_socket, tx_msg, n_length ) <= 0) {
		fprintf( stdout, "[ERR] Send in %s()\n", __FUNCTION__ );
		fflush(stdout);
		return;
	}	

	lib_sleep(0, 5);

	// receive message and handler message....!!! 
	n_length = lib_recv_msg();
	if ( n_length > 0 ) {
		lib_handler();
	}


	close(client_socket);
}


void PdefVr(byte pcm, byte pno, float f_hyst, float f_min, float f_max)
// ----------------------------------------------------------------------------
// POINT DEFINE VR
{
	int i = 0;
	int n_cnt = 0;
	int n_pnt = 0;
	int n_length = 0;
	unsigned char c_chksum = 0;
	APG_PDEF_FORMAT_T *p_data;

	memset( tx_msg, 0x00, sizeof(tx_msg) );

	// check pcm number
	if ( pcm != gn_id ) 
		return;

	// Create apg socket
	if ( lib_create_sock() < 0 ) {
		exit(0);
	}

	// change type
	gn_sendtype = LIB_SEND_TYPE_PDEF;


	// get data pointer
	n_pnt = 2;
	p_data = (APG_PDEF_FORMAT_T *) &tx_msg[n_pnt];	
	
	// get value
	p_data->c_cmd = LIB_PDEF_REQUEST;
	p_data->c_pcm = pcm;
	p_data->w_pno = pno;
	p_data->f_type = LIB_PDEF_VR;
	p_data->f_hyst = f_hyst;
	p_data->f_scale = 1;
	p_data->f_offset = 0;
	p_data->f_min = f_min;
	p_data->f_max = f_max;
	
	// set total length
	n_length = 32;
	tx_msg[0] = 0x00;
	tx_msg[1] = n_length;

	// calculate total chksum
	c_chksum = 0;
	for ( i = 0; i < n_length - 1; i++ ) {
		c_chksum -= tx_msg[i];
	}
	tx_msg[n_length - 1] = c_chksum;

	// send message
	if( write( client_socket, tx_msg, n_length ) <= 0) {
		fprintf( stdout, "[ERR] Send in %s()\n", __FUNCTION__ );
		fflush(stdout);
		return;
	}	

	lib_sleep(0, 5);

	// receive message and handler message....!!! 
	n_length = lib_recv_msg();
	if ( n_length > 0 ) {
		lib_handler();
	}

	close(client_socket);
}
#endif

