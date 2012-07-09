/*
 * file 		: elba_mgr.c
 * creator   	: tykim
 *
 *
 * TCP/IP �� ���� Application���� data ��� 
 *  	1. elba protocol�� ���� data ó��.
 *  	2. 3istation �� 3imini�� ���� ȣȯ�� ����.
 *
 *
 * version :
 *		0.0.1  tykim working.
 *		0.0.2  jong2ry code clean.
 *
 *
 * code �ۼ��� ���ǻ���
 *		1. global ������ 'g_' ���λ縦 ����.
 *		2. ������ ������ ������ �Ʒ��� ���� ��Ģ���� �����Ѵ�. 
 *			int					n
 *			short				w
 *			long				l
 *			float				f
 *			char				ch
 *			unsigned char       b
 *			unsignnd int		un
 *			unsigned short 		uw 
 *			pointer				p
 *			structure			s or nothing
 *		3. �������� ù���ڴ� ����ڷ� ����Ѵ�. 
 *		4. ���������� �� ������ ������ ������� ����ڷ� �Ѵ�. 
 *			ex > g_nTest, g_bFlag, g_fPointTable
 *		5. �Լ����� �ҹ��ڷ� '_' ��ȣ�� ����ؼ� �����Ѵ�. 
 *		6. �Լ��� �Լ� ������ ������ 2�ٷ� �Ѵ�. 
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
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <sys/poll.h>		// use poll event
#include <malloc.h>

////////////////////////////////////////////////////////////////////////////////
//define
#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

#include "elba_mgr.h"										

#define ACCEPT_NONBLOCKING				1
#define RECEIVE_USE_SELECT				1

////////////////////////////////////////////////////////////////////////////////
//extern variable
extern pthread_mutex_t pointTable_mutex;										// point table mutex

extern point_queue elba_queue;													// elba queue
extern pthread_mutex_t elbaQ_mutex;												// elba mutex

// point and node
extern int g_nMyPcm;															// my pcm number
extern int g_nMultiDdcFlag;														// mode select flag
extern unsigned char g_cNode[MAX_NET32_NUMBER];									// node
extern PTBL_INFO_T *g_pPtbl;													// point table
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];						// extension point table


////////////////////////////////////////////////////////////////////////////////
// global variable
const char g_chElbaHead[5] = "{HEAD";											// elba head message
const char g_chElbaTail[5] = "TAIL}";											// elba tail message
int gc_dbg_txmsg = 0;															// ntx display flag	
int gc_dbg_rxmsg = 0;															// nrx display flag		
int gc_view_elba = 0;															// elba display flag
int gc_elba_conntecion_flag = -1;												// elba connect flag
_ELBA_STATUS_T	g_ElbaStatus;													// log elba status


void elba_push_queue(point_info *p)
// ----------------------------------------------------------------------------
// PUT IN ELBA QUEUE
// Description	: data put in elba queue
// Arguments	: p				Is a pointer of point_info structure.
// Returns		: none
{
	pthread_mutex_lock( &elbaQ_mutex );
	putq( &elba_queue, p );
	pthread_mutex_unlock( &elbaQ_mutex );	
}


int elba_pop_queue(point_info *p)
// ----------------------------------------------------------------------------
// GET IN ELBA QUEUE
// Description	: data get in elba queue
// Arguments	: p				Is a pointer of point_info structure.
// Return 		: nRet			Is a status of queue
{
	int nRet;

	memset( p, 0, sizeof(point_info) );
	pthread_mutex_lock( &elbaQ_mutex );
	nRet = getq( &elba_queue, p );
	pthread_mutex_unlock( &elbaQ_mutex );
	
	return nRet;
}


void elba_make_tx_message(unsigned char *p, point_info sPoint)
// ----------------------------------------------------------------------------
// MAKE MESSAGE THAT SEND TO 3ISTATION
// Description	: It make message to send 3iS.
// Arguments	: p				Is a pointer to send transmit message
//				  point			Is a structure to send value of point table
// Return 		: none
{
	unsigned char chTotalLength;
	float fValue;

	chTotalLength = 0x19;
	fValue = elba_change_byte_order(sPoint.value);

	memcpy(p + 0,  g_chElbaHead,	 5);
	memset(p + 5,  0x0D,     		 1);
	memset(p + 6,  0xfe,          	 1);
	memset(p + 7,  0xfe,          	 1);
	memset(p + 8,  0x20,          	 1);
	memset(p + 9,  0x0C,          	 1);
	memset(p + 10, 0xfe,          	 1);
	memset(p + 11, 0xfe,          	 1);
	memcpy(p + 12, &sPoint.pcm,    	 1);
	memcpy(p + 13, &sPoint.pno,    	 1);
	memcpy(p + 14, &fValue,		 	 4);
	memcpy(p + 19, &chTotalLength, 	 1);
	memcpy(p + 20, g_chElbaTail,	 5);
	
	return;
}


float elba_change_byte_order(float fValue)
// ----------------------------------------------------------------------------
// CHANGE BYTE ORDER
// Description	: fValue�� byte order�� �ٲپ �����Ѵ�. 
// Arguments	: fValue		Is a float value
// Return 		: fRtnValue		float value
{
	unsigned char *p1;
	unsigned char *p2;
	float fRtnValue;
	
	p1 = (char*) &fValue;
	p2 = (char*) &fRtnValue;
	
	p2[0] = p1[3];
	p2[1] = p1[2];
	p2[2] = p1[1];
	p2[3] = p1[0];

	return fRtnValue;
}
	

int elba_send_message(ELBA_T *p)
// ----------------------------------------------------------------------------
//  SEND A DATA TO THE 3ISTATION
// Description	: This function is called to send the data to 3iStation.
// Arguments	: p				Is a pointer to the ELBA_T structure. 
// Return 		: nIndexBuf		Is value to return send() function.
{
	int i = 0;
	point_info sPoint;
	int nResult = 0;
	int nIndexBuf = 0;
	unsigned char *pchBuf;
	int nTempBufSize = 0;
	unsigned char chTempBuf[25];
	struct  timespec ts;
	
	// initialize
	pchBuf = p->bTxBuf;
	nTempBufSize = sizeof(chTempBuf);
	memset(pchBuf, 0, ELBA_BUF_SIZE);
	ts.tv_sec = 0;
	ts.tv_nsec = 1 * M_SEC;

	// get data from elba queue
	for(i = 0; i < ELBA_TX_MESSAGE_COUNT; i++) {
		nResult = elba_pop_queue( &sPoint );

		if(nResult != SUCCESS)  {
			nanosleep(&ts, NULL);	
			break;
		}
		
		elba_make_tx_message(chTempBuf, sPoint);
		memcpy( (pchBuf + nIndexBuf) , chTempBuf, nTempBufSize);
		nIndexBuf = nIndexBuf + nTempBufSize;		
	}
	
	//if there's no data to send
	if(nIndexBuf <= 0)
		return nIndexBuf;
	
	// send message
	if(send(p->nFd, pchBuf, nIndexBuf, 0) < 0) 
		return -1;
		
	g_ElbaStatus.nNtx += nIndexBuf;		
		
	// console debug
	if( gc_dbg_txmsg ) {
		fprintf(stdout, "TxD(%d) : \n", nIndexBuf);
		for(i = 1; i < nIndexBuf + 1; i++) {
			fprintf(stdout, "0x%02x ", pchBuf[i-1]);
			if((i % 25) == 0) {
				fprintf(stdout, "\n");
				fflush(stdout);
			}
		}
		fprintf(stdout, "\n");
		fflush(stdout);
	}
	
	return nIndexBuf;
}


unsigned char elba_get_chksum8(void *p, size_t len)
// ----------------------------------------------------------------------------
// GET CHKSUM8 VALUE
// Description	: fValue�� byte order�� �ٲپ �����Ѵ�. 
// Arguments	: p				Is a pointer of message
//				  len			Is a length of message
// Return 		: chSum			Is a chksum8 value.
{
	unsigned char chSum, *pchMsg = p;

	chSum = 0;
	while ( len-- )
		chSum -= *pchMsg++;
		
	return	chSum;
}


void elba_send_time_message(ELBA_T *p)
// ----------------------------------------------------------------------------
// TIME MESSAGE SEND TO SFMC
// Description	: SFMC�� �ð������� �����Ѵ�. 
// Arguments	: p				Is a pointer to the ELBA_T structure. 
// Return 		: none
{
	int i = 0;
	unsigned char *pBuf;
	unsigned char chTotalLength;
	time_t the_time;
	struct tm *pTm;

	// initialize
	pBuf = p->bTxBuf;
	memset(p->bTxBuf, 0, ELBA_BUF_SIZE);
	time(&the_time);
	pTm = localtime(&the_time);
	
	// make message	
	chTotalLength = 0x19;
	memcpy(pBuf + 0,  g_chElbaHead,			 5);
	memset(pBuf + 5,  0x0f,    		 1);
	memset(pBuf + 6,  0xfe,        	 1);
	memset(pBuf + 7,  0xfe,        	 1);
	memset(pBuf + 8,  0x20,        	 1);
	memset(pBuf + 9,  0x0C,        	 1);
	memset(pBuf + 10, 0xfe,        	 1);
	memset(pBuf + 11, 0xfe,        	 1);
	pBuf[12] = 2000 - (pTm->tm_year+1900);
	pBuf[13] = pTm->tm_mon+1;
	pBuf[14] = pTm->tm_mday;
	pBuf[15] = pTm->tm_hour;
	pBuf[16] = pTm->tm_min;
	pBuf[17] = pTm->tm_wday;
	pBuf[18] = elba_get_chksum8(&pBuf[4], pBuf[4]);
	memcpy(pBuf + 19, &chTotalLength, 	 1);
	memcpy(pBuf + 20, g_chElbaTail,		 	 5);

	// send message
	if( send(p->nFd, pBuf, chTotalLength, 0) < 0 ) 
		return;
	
	g_ElbaStatus.nNtx += chTotalLength;		

	// console debug
	if( gc_dbg_txmsg ) {
		fprintf(stdout, "TxD Time (%d) : \n", chTotalLength);
		for( i = 0; i < chTotalLength; i++ ) 
			fprintf(stdout, "0x%02x ", pBuf[i]);
		fprintf(stdout, "\n");
		fflush(stdout);
	}
}


int elba_get_station_ipaddr(char* stationip)
// ----------------------------------------------------------------------------
// GET A 3ISTATION IP ADDRESS 
// Description	: This function is called by elba_init() to get 3iStation ipaddress.
// Arguments	: stationip		Is a pointer to get 3iStation ipaddress		
// Returns		: SUCCESS		If the call was successful
//				  ERROR			If not
{
	FILE* fp = NULL;
	int cnt = 0;
	cmdinfo gc_argument[CFG_COMMAND_COUNT];

	memset( gc_argument, 0, sizeof(gc_argument) );
	
	// search file and copy StationIP.
	if( (fp = fopen( "/duksan/CONFIG/config.dat", "r" )) == NULL ) {
		fclose( fp );
		fprintf( stdout, "[ELBA ERROR1] /duksan/CONFIG/config.dat not opened \n" );
		fflush( stdout );
		return ERROR;
	} 
	else {
		while( !feof(fp) ) {
			fscanf( fp, 
					"%s %s\n", 
					(char *)&gc_argument[cnt].name, (char *)&gc_argument[cnt].value );
				
			if ( strncmp( (char *)&gc_argument[cnt].name, "stationip", 5 ) == 0 ) {
				memcpy( stationip, (char *)&gc_argument[cnt].value, CFG_VALUE_SIZE );
				fclose( fp );		
				return SUCCESS;	
			}
			else
				cnt++;
		}				
	}

	fprintf( stdout, "[ELBA ERROR2] StationIP not found \n" );
	fflush( stdout );
	fclose( fp );
	return ERROR;		
}


ELBA_T *elba_init(void)
// ----------------------------------------------------------------------------
// CREATE THE ELBA_T STRUTURE 
// Description	: This function is called by elba_main().
// 					- create ELBA_T structure
// 					- initialize variable of ELBA_T structure
// Arguments	: none			
// Returns		: pElba			Is pointer of ELBA_T structure
{
	ELBA_T *pElba;
	
	pElba = (ELBA_T *)malloc( sizeof(ELBA_T) );
	
	pElba->nFd = -1;
	
	pElba->bRxBuf =  (unsigned char *)malloc (ELBA_BUF_SIZE);
	pElba->bTxBuf = (unsigned char *)malloc (ELBA_BUF_SIZE);
	pElba->bPrevBuf = (unsigned char *)malloc (ELBA_BUF_SIZE);	
	
	memset( pElba->bRxBuf , 0x00, NET32_BUF_SIZE );
	memset( pElba->bTxBuf, 0x00, NET32_BUF_SIZE );	
	memset( pElba->bPrevBuf, 0x00, NET32_BUF_SIZE );	
	
	pElba->nTempWp = 0;
	pElba->nRecvLength = 0;
	pElba->nIndexPrev = 0;
	pElba->nTimeRequest = ELBA_TIME_WAIT;
	
	if ( elba_get_station_ipaddr(pElba->chServerIp) == ERROR ) {
		fprintf(stdout, "Can't get stationIP from config.dat file.\nPlease check config.dat file\n");
		fflush(stdout);
		return NULL;
	}

	return pElba;
}


void elba_close(ELBA_T *p)
// ----------------------------------------------------------------------------
// CLOSE THE ELBA SOCKET
// Description	: close socket.
// Arguments	: p				Is a pointer to the ELBA_T structure. 
// Return 		: none
{
	close(p->nFd);

	p->nFd = -1;
	p->nTempWp = 0;
	p->nRecvLength = 0;

	memset( p->bPrevBuf, 0, ELBA_BUF_SIZE );
	memset( p->bRxBuf, 0, ELBA_BUF_SIZE );
	memset( p->bTxBuf, 0, ELBA_BUF_SIZE );

	// write connect log
	if (gc_elba_conntecion_flag != SYSLOG_DISCONNECT_ELBA) {
		gc_elba_conntecion_flag = SYSLOG_DISCONNECT_ELBA;
		syslog_record(SYSLOG_DISCONNECT_ELBA);
	}
}


int elba_open(ELBA_T *p)
// ----------------------------------------------------------------------------
// TRY CONNECT TO 3ISTATION
// Description	: create socket and connect 3iStation
// Arguments	: p				Is a pointer to the ELBA_T structure. 
// Return 		: 1				If the call was successful
//				  -1			If not
{
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
	
	p->nFd = socket( AF_INET, SOCK_STREAM, 0 );
	if( p->nFd < 0 ) {
		fprintf( stdout, "+ Socket creation error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;					
	}

	setsockopt( p->nFd, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo) );

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr( p->chServerIp );
	server_addr.sin_port = htons( ELBA_SERVER_PORT );

	fprintf( stdout, "\n[ELBA] Try Connect to 3iStation %s\n", p->chServerIp );
	fflush( stdout );	

#ifdef ACCEPT_NONBLOCKING
	// Set non-blocking 
	// ��ó:[LINUX] linux���� network ���ӽ� non-block���� �����ϱ�..
	if( (arg = fcntl(p->nFd, F_GETFL, NULL)) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( p->nFd );
		return -1;
	} 
	
	arg |= O_NONBLOCK; 
	if( fcntl(p->nFd, F_SETFL, arg) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( p->nFd );
		return -1;
	}

	// Trying to connect with timeout 
	res =  connect(p->nFd, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
	if (res < 0) { 
		if (errno == EINPROGRESS) { 
			fprintf(stdout, "ELBA Progress in connect() - selecting\n"); 
			fflush( stdout );
			do { 
				tv.tv_sec = 5;  // 5���Ŀ� ������������ �Ѵ�.
				tv.tv_usec = 0; 
				FD_ZERO(&myset); 
				FD_SET(p->nFd, &myset); 
				res = select(p->nFd+1, NULL, &myset, NULL, &tv); 
				if (res < 0 && errno != EINTR) { 
					//fprintf(stdout, "Elba Error connecting %d - %s\n", errno, strerror(errno)); 
					//fflush( stdout );
					close( p->nFd );
					return -1; 
				} 
				else if (res > 0) { 
					// Socket selected for write 
					lon = sizeof(int); 
					if (getsockopt(p->nFd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
						//fprintf(stdout, "Elba Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
						//fflush( stdout );
						close( p->nFd );
						return -1;
					} 
					// Check the value returned... 
					if (valopt) { 
						fprintf( stdout, "[ELBA] connection() %d - %s\n", valopt, strerror(valopt)); 
						fflush( stdout );							
						close( p->nFd );
						return -1;
					} 
					break; 
				} 
				else { 
					//fprintf(stdout, "Elba Timeout in select() - Cancelling!\n"); 
					//fflush( stdout );
					close( p->nFd );
					return -1;
				} 
			} while (1); 
		} 
		else { 
			//fprintf(stdout, "Error connecting %d - %s\n", errno, strerror(errno)); 
			//fflush( stdout );
			close( p->nFd );
			return -1;
		} 
	} 

	// Set to blocking mode again... 
	if( (arg = fcntl(p->nFd, F_GETFL, NULL)) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( p->nFd );
		return -1; 
	} 
	
	arg &= (~O_NONBLOCK); 
	if( fcntl(p->nFd, F_SETFL, arg) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( p->nFd );
		return -1; 
	}
#else
	res =  connect(p->nFd, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
	if (res < 0) { 
		return -1;
	}
#endif

	return p->nFd;	
}


void elba_parsing (unsigned char *pBuf)
// ----------------------------------------------------------------------------
// PARSING MESSAGE
// Description	: Message�� parsing �Ѵ�. 
// 
// Arguments	: pBuf				Is a pointer to the receive message buffer 
// Returns		: none
{
	//int i = 0;
	int nIsTimeMsg = 0;
	int nType = 0;
	point_info sPoint;
	struct timeval set_time;
	time_t     tm_st;
	time_t     tm_nd;
	double     d_diff;
	struct tm  user_stime;	
	

	nType = pBuf[8];
	nIsTimeMsg = pBuf[5];

	if ( nIsTimeMsg == 0x0f ) {
		if ( nType == 0x40 ) {
			// ������ �Ǵ� �ð�. 1970-01-01 00:00
			user_stime.tm_year   = 1970 - 1900;   // ���� :�⵵�� 1900����� ����
			user_stime.tm_mon    = 1      -1;      // ���� :���� 0���� ����
			user_stime.tm_mday   = 1;
			user_stime.tm_hour   = 0;
			user_stime.tm_min    = 0;
			user_stime.tm_sec    = 0;
			user_stime.tm_isdst  = 0;              // ��� Ÿ�� ��� ����
			tm_st = mktime( &user_stime );
			
			user_stime.tm_year   = (2000 + pBuf[12])  -1900;   // ���� :�⵵�� 1900����� ����
			user_stime.tm_mon    = (int)pBuf[13] - 1;      // ���� :���� 0���� ����
			user_stime.tm_mday   = (int)pBuf[14];
			user_stime.tm_hour   = (int)pBuf[15];
			user_stime.tm_min    = (int)pBuf[16];
			user_stime.tm_sec    = 0;
			user_stime.tm_isdst  = 0;              // ��� Ÿ�� ��� ����
			tm_nd = mktime( &user_stime );	
			
			d_diff   = difftime( tm_nd, tm_st );
			fprintf( stdout, "[Time Set] d_diff %f \n",	d_diff );
			gettimeofday( &set_time, NULL );
			set_time.tv_sec = (long)d_diff;
			fprintf( stdout, "[Time Set] set_time.tv_sec %ld \n", set_time.tv_sec );
			settimeofday( &set_time, NULL );
			fprintf( stdout, "[Time Set] %d-%d-%d %d:%d \n", 
					 pBuf[12], pBuf[13], pBuf[14], pBuf[15], pBuf[16] );
			fflush( stdout);

			system( "hwclock --systohc" );
		}						
		
		if ( gc_view_elba ) {
			fprintf(stdout, "\nGet Time  ");
			fflush(stdout);
		}		
		return;		
	}

	switch (nType) {
	// Request Message
	// ex message > 0x7b 0x48 0x45 0x41 0x44 0x09 0xfe 0xfe 0x00 0x08 0xfe 0xfe 0x00 0x01 0xf6 0x15 0x54 0x41 0x49 0x4c 0x7d
	case 0x00:
		// get point information.
		sPoint.pcm = pBuf[12];
		sPoint.pno = pBuf[13];
		pthread_mutex_lock( &pointTable_mutex );
		sPoint.value = g_fExPtbl[sPoint.pcm][sPoint.pno];	
		pthread_mutex_unlock( &pointTable_mutex );
		
		// point_table�� ���� �׻� update�� ���� ������ �ִ� DB�̱� ������
		// point_table�� ���� �ٷ� �÷������൵ ������ ���̶�� �����Ѵ�.
		// ���� net32�� ����Ǿ� �ִٸ�, 
		// Net32�� ���� Device�̱� ������ ���� 3iS���� ��û�� ó���ϰ�
		// Queue�� ����ؼ� ���ο� ���� �����ϵ��� �Ѵ�.
		// �׷��Ƿ� Net32�� ����Ǿ� �ִٸ�, 
		// 3iS�� ��û�� ���� 2���� ������ ������ �Ǵ� ���̴�.
		
		// only alive pcm
		/*
		for ( i = 0; i < 32; i++ ) {
			if ( g_cNode[i] > 0 && i == sPoint.pcm && sPoint.pcm != g_nMyPcm )	{
				sPoint.message_type = NET32_TYPE_REQUIRE;
				net32_push_queue(&sPoint);		// send to net32_queue	
			}
		}	
		*/
		sPoint.message_type = NET32_TYPE_REQUIRE;
		net32_push_queue(&sPoint);		// send to net32_queue	
		
		sPoint.message_type = NET32_TYPE_REPORT;
		elba_push_queue(&sPoint);			// send to elba queue.

		if ( gc_view_elba ) {
			fprintf(stdout, "\nRequest %d, %d  ", sPoint.pcm, sPoint.pno);
			fflush(stdout);
		}		
		break;
			
	// Commnad Message
	case 0x40:
		// get point information.
		sPoint.pcm = pBuf[12];
		sPoint.pno = pBuf[13];
		memcpy( &sPoint.value, (pBuf + 14), sizeof(float) );
		sPoint.value = elba_change_byte_order(sPoint.value);

		// Interface for PLC
		//pthread_mutex_lock( &plcQ_mutex );
		//putq( &plc_message_queue, &sPoint );
		//pthread_mutex_unlock( &plcQ_mutex );	
		//g_iChkControl = 1;

		// ���� Net32�� ����Ѵٸ�, 
		// point_table�� ���� �������� �ʰ� Net32�� data�� ������.
		// ������ Net32�� ������� �ʴ� ��쿡��, 
		// point_table�� ���� �����ϰ� �� ���� 3iS�� ������.
		sPoint.message_type = NET32_TYPE_COMMAND;
		pSet(sPoint.pcm, sPoint.pno, sPoint.value);
		
		if ( g_nMultiDdcFlag )	{
			pthread_mutex_lock( &pointTable_mutex );
			g_fExPtbl[sPoint.pcm][sPoint.pno] = sPoint.value;
			pthread_mutex_unlock( &pointTable_mutex );
			elba_push_queue(&sPoint);								
		}

		if ( gc_view_elba ) {
			fprintf(stdout, "\nCommand %d, %d  %0.1f", sPoint.pcm, sPoint.pno, sPoint.value);
			fflush(stdout);
		}		
		break;
				
	default:
		break;
	}		
}


int elba_recv_message(ELBA_T *p)
// ----------------------------------------------------------------------------
// RECEIVE A DATA FROM 3ISTATION
// Description	: 3iStation���� ���� Message�� �޴´�. 
// 
// Arguments	: p				Is a pointer to the ELBA_T structure. 
// Returns		: recv_length	Is length to receive the data from 3iStation
//			     				or ELBA_RECV_TIMEOUT or ELBA_RECV_ERROR
{
	int i = 0;
	int nRecvByte = 0;
	int nIndex = 0;
	int nOffset = 0;
	int nPktLength = 0;
	unsigned char chTemp[ELBA_BUF_SIZE];
    fd_set reads, temps;
    struct timeval tv;
    int nResult = 0;
    int fd_max;
    struct  timespec ts;

#ifdef RECEIVE_USE_SELECT
	// initialize select variable    
	FD_ZERO(&reads);
    FD_SET(p->nFd, &reads);
    fd_max = p->nFd;

	// set timeout value
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    temps = reads;

	// select function.
    nResult = select( fd_max + 1, &temps, 0, 0, &tv );
    if(nResult == 0)  {
		//printf("elba time out\n");
        return ELBA_RECV_ERROR;
    }
    else if(nResult == -1)  {
		//printf("elba recv error\n");
        return ELBA_RECV_ERROR;
    }
    else  {	
        if(FD_ISSET(p->nFd, &temps))  {	
			nRecvByte = read(p->nFd, p->bRxBuf, (ELBA_BUF_SIZE/6) );
        }
    }	
#else
	nRecvByte = recv(p->nFd,  p->bRxBuf, (ELBA_BUF_SIZE/6), 0);
#endif	 

	// if 'nRecvByte' is zero, return ELBA_RECV_ERROR.
	if ( nRecvByte == 0) {
		return 0;
	}
	
	// if 'nRecvByte' is zero, return ELBA_RECV_ERROR.	
	if ( nRecvByte < 0) {
		return ELBA_RECV_ERROR;
	}
	
	g_ElbaStatus.nNrx += nRecvByte;

	// console debug
	if ( gc_dbg_rxmsg ) {
		fprintf(stdout, "RxD(%d) : \n", nRecvByte);
		for(i = 0; i < nRecvByte; i++)
		{
			fprintf(stdout, "0x%02x ", p->bRxBuf[i]);
			if ( p->bRxBuf[i-1] == 0x4c && p->bRxBuf[i] == 0x7d)
				fprintf(stdout, "\n");
			fflush(stdout);
		}
	}	

	// ���� buffer�� ���� ����ִٸ�, 
	// ���� buffer�� �����ؼ� Packet�� �����. 
	if ( p->nIndexPrev > 0  ) {
		memcpy(chTemp, p->bRxBuf, nRecvByte);
		memcpy(p->bRxBuf, p->bPrevBuf, p->nIndexPrev);
		memcpy(&p->bRxBuf[p->nIndexPrev], chTemp, nRecvByte);
		nRecvByte += p->nIndexPrev;
		p->nIndexPrev = 0;
	}

	while (nOffset < nRecvByte)
	{
		if ( p->bRxBuf[nOffset] != '{' ) {
			nOffset++;
		}
		
		// message type is request
		if ( p->bRxBuf[nOffset + 5] == 0x09) {
			nPktLength = 21;
		}
		// message type is time
		else if ( p->bRxBuf[nOffset + 5] == 0x0f) {
			nPktLength = 25;
			p->nTimeRequest = ELBA_TIME_REQUEST;
		}		
		// message type is command
		else if ( p->bRxBuf[nOffset + 5] == 0x0d) {
			nPktLength = 25;
		}
		else  
			nPktLength = 21;

		nIndex = nOffset + nPktLength;	
		if (nIndex <= nRecvByte) {
			ts.tv_sec = 0;
			ts.tv_nsec = 1 * M_SEC;			
			nanosleep(&ts, NULL);	
			if ( p->bRxBuf[nOffset + 8] == 0x00 || p->bRxBuf[nOffset + 8] == 0x40) {
				elba_parsing(&p->bRxBuf[nOffset]);
			}
		}
		else {
			p->nIndexPrev = nRecvByte - nOffset;
			memset(p->bPrevBuf, 0x00, sizeof(p->bPrevBuf));
			memcpy(p->bPrevBuf, &p->bRxBuf[nOffset], p->nIndexPrev);
			break;
		}
		nOffset = nIndex;
	}
	
	// protection code.
	if ( p->nIndexPrev < 0 ) {
		p->nIndexPrev = 0;
	}

	return nRecvByte;
}


void elba_save_file(void) 
{
	FILE *fp = NULL;
	
	fp = fopen("/duksan/FILE/elba", "w");
	if( fp == NULL ) {
		return;		
	}	
	
	fprintf(fp,	"Elba Count\n" );
	if ( g_ElbaStatus.nStatus > 0 )
		fprintf(fp,	"+ STATUS : CONNECTED\n"); 
	else 
		fprintf(fp,	"+ STATUS : DISCONNECTED\n");
		 
	fprintf(fp,	"+ Ntx : %d Bytes\n+ Nrx : %d Bytes\n\n", 
		g_ElbaStatus.nNtx,
		g_ElbaStatus.nNrx );
		
	g_ElbaStatus.nNtx = 0;
	g_ElbaStatus.nNrx = 0;
	
	fclose(fp);
}


void *elba_main(void* arg)
// ----------------------------------------------------------------------------
// ELBA_THREAD MANAGER
// Description	: This function is called by main().
// 					- create ELBA_T structure
//					- call elba_open() and create socket
// 					- receive data from 3iStation and parsing data.
// 					- data send to 3iStation.
// Arguments   : arg				Thread Argument
{
	ELBA_T *pElba;
	int nRtnVal = 0;
	time_t     tm_nd;
	struct tm *tm_ptr;	
	int nPreSec = 0;	

	signal(SIGPIPE, SIG_IGN);								// Ignore broken_pipe signal.

	// create elba structure.
	pElba = elba_init();
	if ( pElba == NULL ) {
		fprintf( stdout, "Can't create ELBA.\n" );
		fflush( stdout );
		syslog_record(SYSLOG_DESTROY_ELBA);
		system("sync");
		system("sync");
		system("sync");		
		system("killall duksan");
	}

	while (1) {
		// Initialize Elba-Status variable
		g_ElbaStatus.nStatus = 0;
		g_ElbaStatus.nNtx = 0;
		g_ElbaStatus.nNrx = 0;		
		elba_save_file();
		
		sleep(3);		
		if ( elba_open( pElba ) < 0 ) {
			elba_close( pElba );
			continue;	
		}
		
		fprintf( stdout, "Connected 3iStation.\n" );
		fflush( stdout );	
		
		// write connect log
		if (gc_elba_conntecion_flag != SYSLOG_CONNECT_ELBA) {
			gc_elba_conntecion_flag = SYSLOG_CONNECT_ELBA;
			syslog_record(SYSLOG_CONNECT_ELBA);
		}		

		// Initialize Elba-Status variable
		g_ElbaStatus.nStatus = 1;
		g_ElbaStatus.nNtx = 0;
		g_ElbaStatus.nNrx = 0;
		
		for(;;) {
			sched_yield();
			
			//1�ʸ��� Elba�� ���� Data�� ���ΰ�ģ��. 
			time(&tm_nd);
			tm_ptr = localtime(&tm_nd);
			if ( nPreSec != tm_ptr->tm_sec ) {
				nPreSec = tm_ptr->tm_sec;
				elba_save_file();
			}
			
			// send message
			elba_send_message( pElba );
			
			// Time Flag�� üũ�ؼ� SFMC�� ���� �ð��� �����Ѵ�. 
			if ( pElba->nTimeRequest == ELBA_TIME_REQUEST ) {
				elba_send_time_message( pElba );
				pElba->nTimeRequest = ELBA_TIME_RESPONSE;
			}
			
			// receive message
			nRtnVal = elba_recv_message( pElba );

			// socket close
			if ( nRtnVal == 0 ) {
				fprintf( stdout, "lost host server\n" );
				fflush( stdout );
				sleep(3);
				elba_close( pElba );				
				break;				
			}
			// socket error
			else if ( nRtnVal < 0 ) {
				sched_yield();
			}
			// receive data
			else {
			}
			continue;
		}
	}
	syslog_record(SYSLOG_DESTROY_ELBA);

	free(pElba->bRxBuf);
	free(pElba->bTxBuf);
	free(pElba->bPrevBuf);
	
	system("sync");
	system("sync");
	system("sync");	
	system("killall duksan");
}




