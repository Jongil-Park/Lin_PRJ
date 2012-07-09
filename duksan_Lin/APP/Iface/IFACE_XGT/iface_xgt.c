#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
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

#include "define.h"
#include "queue_handler.h"									// queue handler



void IfcXGT_sleep(int sec, int msec) 
// ----------------------------------------------------------------------------
// WAIT TIMER
// Description : use select function for timer.
// Arguments   : sec		Is a second value.
//				 usec		Is a micro-second value. 
// Returns     : none
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


IFC_XGT_T *New_IfcXGT(void)
// ----------------------------------------------------------------------------
// CREATE THE IFC_XGT_T STRUTURE 
// Description		: This function is called by ccms_client_main().
// 					  - create IFC_XGT_T structure
// 					  - initialize variable of IFC_XGT_T structure
// Arguments		: none
// Returns			: sock		Is pointer of IFC_XGT_T structure
{
	IFC_XGT_T *p;
	
	p = (IFC_XGT_T *)malloc( sizeof(IFC_XGT_T) );
	
	memset ( p, 0x00, sizeof(IFC_XGT_T) );
	p->nFd = -1;

	return p;
}


int IfcXGT_Open(IFC_XGT_T *p)
// ----------------------------------------------------------------------------
// OPEN THE IFC_XGT_T Socket
// Description		: This function is called by ccms_client_main().
// Arguments		: p			Is a pointer to the IFC_XGT_T structure.
// Returns			: 1			If the call was successful
//              	 -1			If not
{
	struct sockaddr_in server_addr;
	struct timeval timeo;
	int one = 1;
	
	long arg; 		
	int res; 
	fd_set myset; 
	struct timeval tv; 
	int valopt; 
	socklen_t lon;

	
	memset( &server_addr, 0, sizeof(server_addr) );
	timeo.tv_sec = 0;
	timeo.tv_usec = 10000;

	p->nFd = socket( AF_INET, SOCK_STREAM, 0 );
	if( p->nFd < 0 ) {
		fprintf( stdout, "+ CCMS Socket creation error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;					
	}

    if (setsockopt( p->nFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
		fprintf( stdout, "+ CCMS Socket SO_REUSEADDR error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;	
    }

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("192.168.0.12");
	server_addr.sin_port = htons( 2004 );

	// Set non-blocking 
	// 출처:[LINUX] linux에서 network 접속시 non-block으로 연결하기..
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
			fprintf(stdout, "EINPROGRESS in connect() - selecting\n"); 
			do { 
				tv.tv_sec = 5;  // 15초후에 빠져나오도록 한다.
				tv.tv_usec = 0; 
				FD_ZERO(&myset); 
				FD_SET(p->nFd, &myset); 
				res = select(p->nFd+1, NULL, &myset, NULL, &tv); 
				if (res < 0 && errno != EINTR) { 
					fprintf(stdout, "Error connecting %d - %s\n", errno, strerror(errno)); 
					fflush( stdout );
					close( p->nFd );
					return -1; 
				} 
				else if (res > 0) { 
					// Socket selected for write 
					lon = sizeof(int); 
					if (getsockopt(p->nFd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
						fprintf(stdout, "Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
						fflush( stdout );
						close( p->nFd );
						return -1;
					} 
					// Check the value returned... 
					if (valopt) { 
						fprintf(stdout, "Error in delayed connection() %d - %s\n", valopt, strerror(valopt)); 
						fflush( stdout );
						close( p->nFd );
						return -1;
					} 
					break; 
				} 
				else { 
					fprintf(stdout, "Timeout in select() - Cancelling!\n"); 
					fflush( stdout );
					close( p->nFd );
					return -1;
				} 
			} while (1); 
		} 
		else { 
			fprintf(stdout, "Error connecting %d - %s\n", errno, strerror(errno)); 
			fflush( stdout );
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

	return p->nFd;
}


void IfcXGT_Close(IFC_XGT_T *p)
{
	printf("%s()\n", __FUNCTION__);
	
	close(p->nFd);
	
	memset ( p, 0x00, sizeof(CCMS_CLIENT_T) );
	p->nFd = -1;
}


void IfcXGT_MakeHeader(IFC_XGT_T *p)
{
	XGT_HEADER_T *pHeader;
	
	pHeader = (XGT_HEADER_T *)p->chTxBuf;

	memcpy(&pHeader->chCompany[0], "LSIS-XGT", sizeof("LSIS-XGT"));
	pHeader->chReserved = 0;
	pHeader->chPLC_Info = 0;
	pHeader->chCpu_Info = 0xA0;
	pHeader->chSource = 0x33;
	pHeader->chInvokeId = p->wInvokeId++;
	pHeader->chLength = sizeof(XGT_BIT_APP_T);
	pHeader->chFnetPos = 0x00;
	pHeader->chReserved2 = 0;		
}


int IfcXGT_Send_Pkt(IFC_XGT_T *p)
{
	int nSendLength = 0;
	int nAppLength = 0;
	
	int i = 0;
	int nRetSend = 0;
	CCMS_PKT_T *pTx;
	CCMS_PKT_SET_T *pSetT;
	point_info point;
	int nRemoteId = 0;
	unsigned char chPcm = 0;
	unsigned char chPno = 0;
	unsigned short wValue = 0;
	int nIndexPno = 0;

    fd_set writes, temps;
    struct timeval tv;
    int nResult = 0;
    int fd_max;	
	
	XGT_BIT_APP_T *pApp;
	
	IfcXGT_MakeHeader(p);
	
	pApp = (XGT_BIT_APP_T *)&p->chTxBuf[sizeof(XGT_HEADER_T)];
	
	pApp->wCmd = 0x0054;
	pApp->wType = 0x0014;
	pApp->wReserved = 0x0000;
	pApp->wCnt = 0x0001;
	pApp->wNameLength = 0x0008;
	memcpy(pApp->chName, "%MB00000", sizeof("%MB00000"));
	pApp->wLength = 0x0008;
	
	
	nSendLength = sizeof(XGT_HEADER_T) + sizeof(XGT_BIT_APP_T);
    
    FD_ZERO(&writes);
    FD_SET(p->nFd, &writes);
    fd_max = p->nFd;
    
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    temps = writes;
    
    nResult = select( fd_max + 1, NULL, &temps, NULL, &tv );
 
    if(nResult == 0)  {
        printf("send wait\n");
        return 0;
    }
    else if(nResult == -1)  {
        printf("send select error : \n");
        return -1;
    }
    else  {	
        //printf("upper FD_ISSET\n");
        if(FD_ISSET(p->nFd, &temps))  {	
            nRetSend = send(p->nFd, p->chTxBuf, nSendLength, 0 );

			printf("TX = ");
			for ( i = 0; i< nSendLength; i++)
				printf("%x ", p->chTxBuf[i]);
			printf("\n");            
            
            return nRetSend;
        }
    }		
	
	
	return nRetSend;
}


void *IfcXGT_main(void* arg)
{
	int nRetOpen = 0;
	int nRecvByte = 0;
	int i = 0;
	IFC_XGT_T	*pXgt = NULL;

	signal(SIGPIPE, SIG_IGN);	// Ignore broken_pipe signal
	
	IfcXGT_sleep(1,0);

	pXgt = New_IfcXGT();
	if ( pXgt == NULL ) {
		fprintf( stdout, "Can't create XGT Interface.\n" );
		fflush( stdout );
		//exit(1);
		system("killall duksan");
	}

	while (1) {		
		nRetOpen = IfcXGT_Open(pXgt);
		if ( nRetOpen < 0 ) {
			fprintf( stdout, "Wait connect\n" );
			fflush( stdout );			
			IfcXGT_Close(pXgt);
			continue;
		}
		
		for (;;) {			
			IfcXGT_sleep(3,0);

			if ( IfcXGT_Send_Pkt(pXgt) <= 0 ) {
				IfcXGT_Close(pXgt);
				break;
			}				
			
			IfcXGT_sleep(1,0);
			nRecvByte = read(pXgt->nFd, pXgt->chRxBuf, sizeof(pXgt->chRxBuf));
			printf("RX = ");
			for ( i = 0; i< nRecvByte; i++)
				printf("%x ", pXgt->chRxBuf[i]);
			printf("\n");
			
			/*			
			IfcXGT_sleep(3,0);
			

			nRtnRecv = IfcXGT_RecvPkt(pXgt);

			if ( nRtnRecv == 0 ) {
				IfcXGT_Close(pXgt);
				break;
			}
			*/
			
		}
		continue;
	}

}






