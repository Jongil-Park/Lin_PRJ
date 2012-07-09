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
#include <sys/poll.h>										// use poll event

#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"
#include "queue_handler.h"									// queue handler
#include "iface_handler.h"									// interface ccms

#if 0

IFH_T *g_pIfh;

extern CCMS_T				*pCcms;
extern CCMS_PTBL_LIST_T		*g_pPtbl;



void ifh_ccms_status()
{
	FILE *fp;
	char chPer = 0x25;		// %
	char chChk = 0x22;		// "
	int nIndex = 0; 
	int nCount = 2;

	fp = fopen("/httpd/ccms_ctrl.html", "w");
	if( fp == NULL ) {
		return;		
	}

	fprintf(fp, "<head>");
	
	// 항상 서버로부터 갱신되도록 하는 코드
	fprintf(fp, "<meta http-equiv=%cExpires%c content=%c0%c/> ",
			chChk, chChk, chChk, chChk);
	fprintf(fp, "<meta http-equiv=%cPragma%c content=%cno-cache%c/>",
			chChk, chChk, chChk, chChk);

	fprintf(fp, "<title>");
	fprintf(fp, "IDC CCMS Client Control");
	fprintf(fp, "</title>");
	fprintf(fp, "</head>");
	fprintf(fp, "<body>");

	fprintf(fp, "<br>");
	fprintf(fp, "<h1><b>");
	fprintf(fp, "IDC CCMS Client Control");
	fprintf(fp, "</b></h1>");
	fprintf(fp, "<b>Update :: 2010-11-10 Test Time</b>");
	fprintf(fp, "<hr color =silver size=3>");

	fprintf(fp, "<table width=100%c height=30 border=1 bgcolor=ddffdd cellpadding=0 cellspacing=0>", chPer);
	fprintf(fp, "<tr bgcolor=ddffdd>");

	// Client index
	fprintf(fp, "<td width=50%c ALIGN=CENTER>", chPer);
	fprintf(fp, " 이 름 ");
	fprintf(fp, "</td>");

	// onoff
	fprintf(fp, "<td width=20%c ALIGN=CENTER>",chPer);
	fprintf(fp,	" 현재상태 ");	
	fprintf(fp, "</td>");

	
	// schedule check
	fprintf(fp, "<td width=30%c ALIGN=CENTER>",chPer);
	fprintf(fp,	" 제어값 ");	
	fprintf(fp, "</td>");

	fprintf(fp, "</tr>");
	fprintf(fp, "</table>");

	fprintf(fp, "<TABLE width=100%c height=30  height=30 border=1 cellpadding=0 cellspacing=0>",chPer);
	
	for ( nIndex = 0; nIndex < nCount; nIndex++) {
		fprintf(fp, "<tr>");
		// name
		fprintf(fp, "<td width=50%c ALIGN=CENTER>", chPer);
		fprintf(fp, " FAN_2_2_0_%d ", nIndex);
		fprintf(fp, "</td>");

		// status
		fprintf(fp, "<td width=20%c ALIGN=CENTER>",chPer);
		fprintf(fp,	" Not Yet ");	
		fprintf(fp, "</td>");

		// control
		fprintf(fp, "<td width=30%c ALIGN=CENTER>",chPer);
fprintf(fp, "<table><tr><td>");
fprintf(fp, "<a href=%cccms_ctrl_02_02_%d_0001.html%c target=%c_top%c style=%ctext-decoration: none;%c>",
	chChk, nIndex, chChk, chChk, chChk, chChk, chChk);
fprintf(fp, "<img src=%cbt_on.jpg%c alt=%c%c border=%c0%calign=%cabsbottom%c /></a>",
	chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);
fprintf(fp, "</td><td>");
fprintf(fp, "<a href=%cccms_ctrl_02_02_%d_0000.html%c target=%c_top%c style=%ctext-decoration: none;%c>",
	chChk, nIndex, chChk, chChk, chChk, chChk, chChk);
fprintf(fp, "<img src=%cbt_off.jpg%c alt=%c%c border=%c0%calign=%cabsbottom%c /></a>",
	chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);
fprintf(fp, "</td></tr></table>");
		fprintf(fp, "</td>");
		fprintf(fp, "</tr>");
	}
	
	fprintf(fp, "</table>");
	fprintf(fp, "</body>");
	fclose(fp);
}



void ifh_sleep(int sec, int msec) 
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
    select( 0, NULL, NULL, NULL, &tv );
	return;
}


IFH_T *new_ifh(void)
// ----------------------------------------------------------------------------
// CREATE THE IFH_T STRUTURE 
// Description		: This function is called by ccms_main().
// 					  - create IFH_T structure
// 					  - initialize variable of IFH_T structure
// Arguments		: none
// Returns			: sock		Is pointer of IFH_T structure
{
	IFH_T *p;
	
	p = (IFH_T *)malloc( sizeof(CCMS_T) );
	
	memset ( p, 0x00, sizeof(CCMS_T) );
	p->nFd = -1;

	return p;
}



int ifh_open(IFH_T *p)
// ----------------------------------------------------------------------------
// OPEN THE IFH_T Socket
// Description		: This function is called by ifh_main().
// Arguments		: p			Is a pointer to the IFH_T structure.
// Returns			: 1			If the call was successful
//              	 -1			If not
{
	struct sockaddr_in server_addr;
	struct timeval timeo;
	int one = 1;
		
	memset( &server_addr, 0, sizeof(server_addr) );
	timeo.tv_sec = 0;
	timeo.tv_usec = 10000;
	
	p->nFd = socket( AF_INET, SOCK_STREAM, 0 );
	if( p->nFd < 0 ) {
		fprintf( stdout, "+ IFH Socket creation error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;					
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons( IFH_PORT );

    if (setsockopt( p->nFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
		fprintf( stdout, "+ IFH Socket SO_REUSEADDR error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;	

    }

	if ( bind ( p->nFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ) {
		fprintf( stdout, "+ IFH Socket Bind error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;	
	}

	if ( listen (p->nFd, 5) ) {
		fprintf( stdout, "+ IFH Socket Listen error\n" );
		fflush( stdout );
		close( p->nFd );
		return -1;
	}

	FD_ZERO(&p->reads);
	FD_SET(p->nFd, &p->reads);
	p->nFdMax = p->nFd;

	return p->nFd;
}

int ifh_chk_fd(IFH_T *p)
{
	int nFd = 0;
	struct timeval timeout;

	p->temps = p->reads;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	// select function.
	if ( select(p->nFdMax+1, &p->temps, 0, 0, &timeout) == -1 ) {
		fprintf( stdout, "+ IFH Socket Select error\n" );
		fflush( stdout );
		//exit(1);
		system("killall duksan");
	}

	// check fd.
	for ( nFd = 0; nFd < p->nFdMax + 1; nFd++ ) {
		if ( FD_ISSET(nFd, &p->temps) ) {
			return nFd;
		}
	}

	return -1;
}



void ifh_open_client(IFH_T *p, int nClientFd)
// ----------------------------------------------------------------------------
// OPEN CLIENT
// Description		: 접속한  Client에 상태값을 변화시킨다. 
// Arguments		: p					Is a pointer to the IFH_T structure.
// 					  nClientFd			Client Fd
// Returns			: none
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == 0 ) {
			p->nClientStatus[nIndex] = nClientFd;
			fprintf( stdout, "+ Open client %d, fd = %d\n" , nIndex, nClientFd);
			fflush( stdout );
			return;
		}
	}
}


void ifh_close_client(IFH_T *p, int nClientFd)
// ----------------------------------------------------------------------------
// CLOSE CLIENT
// Description		: 접속한  Client에 상태값을 초기화 시킨다. 
// Arguments		: p					Is a pointer to the IFH_T structure.
// 					  nClientFd			Client Fd
// Returns			: none
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == nClientFd ) {
			fprintf( stdout, "+ Close client %d, fd = %d\n" , nIndex, nClientFd);
			fflush( stdout );
			p->nClientStatus[nIndex] = 0;
			p->nClientId[nIndex] = 0;
			return;
		}
	}
}



IFH_BUFFER_T *ifh_get_buffer(IFH_T *p, int nClientFd)
// ----------------------------------------------------------------------------
// OPEN BUFFER FOR CLIENT
// Description		: 접속한  Client에 Buffer를 할당한다. 
// Arguments		: p					Is a pointer to the IFH_T structure.
// 					  nClientFd			Client Fd
// Returns			: buffer pointer 	Is a pointer to the IFH_BUFFER_T structure.
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == nClientFd ) {
			return &p->RxBuf[nIndex];
		}
	}
	return NULL;
}


void ifh_open_buffer(IFH_T *p, int nClientFd)	
// ----------------------------------------------------------------------------
// CLOSE BUFFER
// Description		: 접속한  Client에 Buffer를 초기화한다. 
// Arguments		: p					Is a pointer to the IFH_T structure.
// 					  nClientFd			Client Fd
// Returns			: none
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == nClientFd ) {
			p->RxBuf[nIndex].nIndexPrev = 0;
			fprintf( stdout, "+ Open buffer %d, fd = %d\n" , nIndex, nClientFd);
			fflush( stdout );
			return;
		}
	}
}


void ifh_close_buffer(IFH_T *p, int nClientFd)	
// ----------------------------------------------------------------------------
// RETURN BUFFER POINTER
// Description		: 접속한  Client가 사용하는 buffer의 pointer를 리턴한다. 
// Arguments		: p					Is a pointer to the IFH_T structure.
// 					  nClientFd			Client Fd
// Returns			: none.
{
	int nIndex = 0; 

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( p->nClientStatus[nIndex] == nClientFd ) {
			p->RxBuf[nIndex].nIndexPrev = 0;
			fprintf( stdout, "+ Close buffer %d, fd = %d\n" , nIndex, nClientFd);
			fflush( stdout );
			return;
		}
	}
}



void ifh_close(IFH_T *p, int nClientFd)
// ----------------------------------------------------------------------------
// CLOSE THE CLIENT SOCKET
// Description		: Close client socket.
// Arguments		: p					Is a pointer to the IFH_T structure.
// 					  nClientFd			Client Fd
// Returns			: none.
{
	FD_CLR(nClientFd, &p->reads);
	close(nClientFd);
	ifh_close_client(p, nClientFd);
	ifh_close_buffer(p, nClientFd);
	fprintf( stdout, "IFH Close Client  = %d \n", nClientFd);
	fflush( stdout );
}



void ifh_parse_pkt(IFH_T *p, int nSockFd, unsigned char *pData) 
// ----------------------------------------------------------------------------
// PARSE PACKET
// Description		: 전송받은 Packet을 Parse 한다. 
// Arguments		: nSockFd			Client Socket
// 					  pData				Is a pointer to th Rxbuffer
// Returns			: none.
{
	int nCode = 0;
	IFH_PKT_CCMS_T 	*pCcmsT;
	//IFH_PKT_ACK_T	*pAckT;	
	//int nRtnSend = 0;
	//unsigned char chTxBuf[32];
	unsigned char chRemoteId = 0;
	unsigned char chPcm = 0;
	unsigned char chPno = 0;
	unsigned short wValue = 0;

	nCode = pData[1];

	switch (nCode) {
	// CODE CCMS
	case IFH_CODE_HTTPD:
		pCcmsT = (IFH_PKT_CCMS_T *)pData;
		if ( pCcmsT->chStx == '<' && pCcmsT->chEtx == '>') {

			chRemoteId = pCcmsT->chRemoteId;
			chPcm = pCcmsT->chPcm;
			chPno = pCcmsT->chPno;
			wValue = pCcmsT->wData;

			fprintf( stdout, "Recv chRemoteId = %d \n", chRemoteId);
			fprintf( stdout, "Recv chPcm = %d \n", chPcm);
			fprintf( stdout, "Recv chPno = %d \n", chPno);
			fprintf( stdout, "Recv wValue = %d \n", wValue);
			fflush( stdout );

			/*
			pAckT = (IFH_PKT_ACK_T *)chTxBuf;
			pAckT->chStx = '<';
			pAckT->chCode = IFH_CODE_HTTPD;
			pAckT->chdummy = 0x00;
			pAckT->chEtx = '>';
			
			nRtnSend = send(nSockFd, pAckT, sizeof(IFH_PKT_ACK_T), 0 );
			*/
			ifh_sleep(1,0);
			ifh_ccms_status();
		}
		break;
	}
}


void ifh_recv_pkt(IFH_T *p, int nSockFd, IFH_BUFFER_T *pBuf, int nRecvLength) 
// ----------------------------------------------------------------------------
// RECEIVE PACKET
// Description		: 전송받은 Packet을 정리한다.  
// Arguments		: nSockFd			Client Socket
// 					  pData				Is a pointer to th Rxbuffer
// 					  nRecvLength		Client로 부터 전송받은 Packet의 길이
// Returns			: none.
{
	int nIndex = 0;
	int nOffset = 0;
	int nPktLength = 0;
	unsigned char chTemp[CCMS_BUFFER_SIZE];

	// 이전 buffer에 값이 들어있다면, 
	// 이전 buffer를 참조해서 Packet을 만든다. 
	if ( pBuf->nIndexPrev > 0  ) {
		memcpy(chTemp, pBuf->chRxBuf, nRecvLength);
		memcpy(pBuf->chRxBuf, pBuf->chPrevBuf, pBuf->nIndexPrev);
		memcpy(&pBuf->chRxBuf[pBuf->nIndexPrev], chTemp, nRecvLength);
		nRecvLength += pBuf->nIndexPrev;
		pBuf->nIndexPrev = 0;
	}

	while (nOffset < nRecvLength) {

		if ( pBuf->chRxBuf[nOffset] != '<' ) {
			nOffset++;
		}

		// 전체 Message 길이를 정한다.  
		if ( pBuf->chRxBuf[nOffset + 1] == IFH_CODE_HTTPD ) {
			nPktLength = sizeof(IFH_PKT_CCMS_T);
		}
		if ( pBuf->chRxBuf[nOffset + 1] == IFH_CODE_PCTRL ) {
			nPktLength = sizeof(IFH_PKT_PCTRL_T);
		}

		else
			continue;

		nIndex = nOffset + nPktLength;	
		if (nIndex <= nRecvLength) {
			//ifh_parse_pkt(p, nSockFd, &pBuf->chRxBuf[nOffset]);
		}
		else {
			pBuf->nIndexPrev = nRecvLength - nOffset;
			memset(pBuf->chPrevBuf, 0x00, sizeof(pBuf->chPrevBuf));
			memcpy(pBuf->chPrevBuf, &pBuf->chRxBuf[nOffset], pBuf->nIndexPrev);
			break;
		}
		nOffset = nIndex;
	}
	
	// protection code.
	if ( pBuf->nIndexPrev < 0 ) {
		pBuf->nIndexPrev = 0;
	}

	return;
}


void *ifh_main(void* arg)
{
	IFH_T 				*pIfh;
	int 				nChkFd = 0;
	int 				nClientFd = 0;
	int 				nClientLength = 0;
	struct sockaddr_in 	client_addr;
	unsigned char 		chTemp[128];
	int 				nRetRecv = 0;
	IFH_BUFFER_T 		*pRxBuf;

	ifh_sleep(1,0);
	
	pIfh = new_ifh();
	g_pIfh = pIfh;

	if ( pIfh == NULL ) {
		fprintf( stdout, "Can't create IFH Interface.\n" );
		fflush( stdout );
		//exit(1);
		system("killall duksan");
	}

	while (1) {
		if ( ifh_open(pIfh) < 0 ) {
			ifh_sleep(1,0);
			continue;
		}
		
		for ( ;; ) {
			// check select fd
			nChkFd = ifh_chk_fd(pIfh);
			if ( nChkFd < 0 ) {
				continue;
			}

			// 연결 요청인 경우 처리 
			if ( nChkFd == pIfh->nFd ) {
				nClientLength = sizeof(client_addr);
				nClientFd = accept( pIfh->nFd, 
									(struct sockaddr *)&client_addr,
								    &nClientLength );
				FD_SET(nClientFd, &pIfh->reads);

				if ( pIfh->nFdMax < nClientFd ) 
					pIfh->nFdMax = nClientFd;
				// open structure
				ifh_open_client(pIfh, nClientFd);
				ifh_open_buffer(pIfh, nClientFd);

				fprintf( stdout, "IFH Accept client nClientFd = %d\n", nClientFd );
				fflush( stdout );
			}
			else {
				nRetRecv = read(nChkFd, chTemp, sizeof(chTemp));

				// client close
				if ( nRetRecv == 0 ) {
					ifh_close(pIfh, nChkFd);
					continue;
				}

				// client error
				if ( nRetRecv < 0 ) {
					continue;
				}

				// get rx buffer strcuture
				pRxBuf = ifh_get_buffer(pIfh, nChkFd);
				if ( pRxBuf == NULL ) {
					fprintf( stdout, "IFH Buffer Error \n");
					ifh_close(pIfh, nChkFd);
					continue;
				}

				memcpy(&pRxBuf->chRxBuf, 
					   chTemp,
					   nRetRecv);
				ifh_recv_pkt(pIfh, nChkFd, pRxBuf, nRetRecv);
			}
		}

	}
}



#endif



