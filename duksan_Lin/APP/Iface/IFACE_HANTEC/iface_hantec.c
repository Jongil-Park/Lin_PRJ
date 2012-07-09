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
#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"
#include "queue_handler.h"									// queue handler
#include "crc16.h"
#include "iface_hantec.h"

#define UART2DEVICE 					"/dev/s3c2410_serial2"
#define MAX_BUF_SIZE					2048

#define IFACE_HT_CTRL_RELAY				1
#define IFACE_HT_CTRL_GROUP				2
#define IFACE_HT_CTRL_PATTERN			3
#define IFACE_HT_CTRL_READ_RELAY		4
#define IFACE_HT_CTRL_READ_GROUP		5
#define IFACE_HT_CTRL_READ_PATTERN		6
#define IFACE_HT_CTRL_INFO				7

#define MAX_PCM_COUNT					32

#define IFACE_HT_DIV_OFFSET				0x00
#define IFACE_HT_GROUP_OFFSET			0x40
#define IFACE_HT_PATTERN_OFFSET			0x60

// point-table�� ���� pcm�� pno�� �����Ѵ�.
#define ISPCM(x)		(x>>8)
#define ISPNO(x)		(x&0xff)

// pno�� �޾Ƽ� �������� ��ȣ�� ����ġ ��ȣ�� �����մϴ�. 
#define IS_DIV(x)		(x)				
#define IS_DIVNO(x)		(((x-1)>>2) + IFACE_HT_DIV_OFFSET)	
#define IS_DIVSW(x)		(((x-1)&0x3)+1)		

// pno�� �޾Ƽ� �׷��� ��ȣ�� ����ġ ��ȣ�� �����մϴ�.
#define IS_GROUP(x)		(x)				
#define IS_GROUPNO(x)	(((x-1)>>2) + IFACE_HT_GROUP_OFFSET)	
#define IS_GROUPSW(x)	(((x-1)&0x3)+1)	

// pno�� �޾Ƽ� ������ ��ȣ�� ����ġ ��ȣ�� �����մϴ�.
#define IS_PATTERN(x)	(x-128)			
#define IS_PATTERNNO(x)	((((x-128)-1)>>2) + IFACE_HT_PATTERN_OFFSET)	
#define IS_PATTERNSW(x)	((((x-128)-1)&0x3)+1)

// 0�̸�, Group�� Pattern�� �����ϴ� PCM��ȣ�� ��Ÿ���� ���̰�, 
// 1�̸� �� ������ ���������ϴ� PCM ��ȣ�� ��Ÿ���� ���̴�. 
#define IS_CTRLTYPE(x)	(x&0x1)		

// PCM ��ȣ�κ��� SLAVE ID ã�´�. 
#define IS_SLAVEID(x)	((x)>>1)

// Tx, Rx Buffer
//static unsigned char g_ifacHT_rxbuf[MAX_BUF_SIZE];
//static unsigned char g_ifacHT_txbuf[MAX_BUF_SIZE];

extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];
extern float g_fPreExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];

extern int pSet(int pcm, int pno, float value);
extern float pGet(int pcm, int pno);

int g_nCount = 0;
int g_nSlaveOffet = 0;

/*******************************************************************/
/* delay function */
void IfaceHT_Sleep(int sec,int msec) 
/*******************************************************************/
{
    struct timeval tv;
    
    tv.tv_sec=sec;
    tv.tv_usec=msec;                
    select(0,NULL,NULL,NULL,&tv);
			    
    return;
}


/****************************************************************/
/* send data */
int IFaceHT_On_Send(void *p)
/****************************************************************/
{
	int i = 0;
	IfaceHT_T *uart2;

	fprintf( stdout, ">> 1\n");
	fflush(stdout);
	
	uart2 = (IfaceHT_T *) p;
	write( uart2->fd, uart2->txbuf, uart2->sendLength);

	printf("TX = ");
	for (i = 0; i < uart2->sendLength; i++) {
		printf("%x ", uart2->txbuf[i]);
	}
	printf("\n");
	
	return 0;
}


/****************************************************************/
/* receive data function */
int IFaceHT_On_Recv(void *p)
/****************************************************************/
{
	int iRet = 0;;
	int iBufWp = 0;
	int iLoopCnt = 0;
	IfaceHT_T *uart2;
	unsigned char *pRxbuf;
	
	uart2 = (IfaceHT_T *) p;
	pRxbuf = (unsigned char *)uart2->rxbuf;
	memset( pRxbuf, 0, MAX_BUF_SIZE );

	while(1) {
		uart2->pollState = poll(					// poll()�� ȣ���Ͽ� event �߻� ���� Ȯ��     
			(struct pollfd*)&uart2->pollEvents,		// event ��� ����
			1,  									// üũ�� pollfd ����
			100);  									// time out �ð� (ms)	

		if ( 0 < uart2->pollState) {                            // �߻��� event �� ����
			if ( uart2->pollEvents.revents & POLLIN) {			// event �� �ڷ� ����?
				iRet = read(uart2->fd, &pRxbuf[iBufWp], 32);
				iBufWp += iRet;
				iLoopCnt = 0;
			}
			else if ( uart2->pollEvents.revents & POLLERR) {	// event �� ����?
				printf( "Event is Error. Terminal Broken.\n");
				return -1;
			}
		}
		else {
			if ( 5 < iLoopCnt++ ) {
				uart2->recvLength = iBufWp;
				printf("time out %d\n", iBufWp);
				
				if (iBufWp > 0)
					IFaceHT_On_Recv_Handler(uart2);
				
				return iBufWp;
			}
		}
	}	

}


/****************************************************************/
/* parsing receive data */
void IFaceHT_On_Recv_Handler(void *pIFaceHT) 
/****************************************************************/
{
	int i = 0;
	int iSlaveId = 0;
	int iLength = 0;
	int iPcm = 0;
	int iPno = 0;
	float fVal[4];
	IfaceHT_T *p = NULL;
	unsigned char *pBuf;
	unsigned short calCrc16;
	unsigned short recvCrc16;
	unsigned char cType = 0;
	unsigned char *pData;
	unsigned int iCnt = 0;	
	unsigned int iDataCnt = 0;
		
	// Initialize
	p = (IfaceHT_T *)pIFaceHT;
	pBuf = (unsigned char *) p->rxbuf;
	iSlaveId = pBuf[0];
	iLength =  p->recvLength;
	cType = pBuf[1];

	// debug
	printf("RX(%d) = ", p->recvLength);
	for (i = 0; i < p->recvLength ;i++) {
		printf("%x ", pBuf[i]);	
	}
	printf("\n");
	
	// jong2ry 2011_0308
	// p->recvLength ���� 2���� ������ CRC üũ���� ������ �߻��ϹǷ�
	// �ڵ带 �߰��Ѵ�. 
	if ( p->recvLength < 3 )
		return;
	
	// get crc value
	calCrc16 = IsCrc16( pBuf, (iLength - 2) );
	recvCrc16 = ((pBuf[iLength - 2] << 8) & 0xff00) + (pBuf[iLength - 1]  & 0xff);
	
	// check crc
	if ( calCrc16 == recvCrc16 ) {
		switch(cType) {
			// MODBUS READ MESSAGE
			case 0x03: {
				iDataCnt = pBuf[2];
				// read relay
				if ( iDataCnt == 0x80 ) {
					pData = (unsigned char *) &p->rxbuf[3];							
					iPcm = (iSlaveId * 2) + 1;
					
					printf("iPcm = %d\n", iPcm);					
					printf("iDataCnt = %d\n", iDataCnt);
										
					for ( iCnt = 0; iCnt < iDataCnt; iCnt++ ) {

						if ( ( *(pData+iCnt) & 0xf0) == 0xf0 ) {
							
							printf("iCnt = %d. Val = %d\n", 
								iCnt, (*(pData+iCnt) & 0x0f) );
								
							fVal[0] = (float) (*(pData+iCnt) & 0x01);
							fVal[1] = (float) ((*(pData+iCnt)>>1) & 0x01);
							fVal[2] = (float) ((*(pData+iCnt)>>2) & 0x01);
							fVal[3] = (float) ((*(pData+iCnt)>>3) & 0x01);

							iPno = ((iCnt - 1) * 2) + 1;
							
							// ���簪�� �������� ���ٸ�,
							// ����ڰ� ��� ���� ���� �ƴϱ� ������
							// �������Ʈ ���̺��� ������Ʈ�Ѵ�. 
							// �׷��� ������ ����� �������� �ʴ´�. 
							if ( g_fExPtbl[iPcm][iPno] == g_fPreExPtbl[iPcm][iPno] ) {
								pSet(iPcm, iPno, fVal[0]);
								g_fPreExPtbl[iPcm][iPno] = fVal[0];
							}
							iPno++;
							
							if ( g_fExPtbl[iPcm][iPno] == g_fPreExPtbl[iPcm][iPno] ) {
								pSet(iPcm, iPno, fVal[1]);
								g_fPreExPtbl[iPcm][iPno] = fVal[1];
							}
							iPno++;
							
							if ( g_fExPtbl[iPcm][iPno] == g_fPreExPtbl[iPcm][iPno] ) {
								pSet(iPcm, iPno, fVal[2]);
								g_fPreExPtbl[iPcm][iPno] = fVal[2];
							}
							iPno++;
							
							if ( g_fExPtbl[iPcm][iPno] == g_fPreExPtbl[iPcm][iPno] ) {
								pSet(iPcm, iPno, fVal[3]);
								g_fPreExPtbl[iPcm][iPno] = fVal[3];
							}
							
							iPno = ((iCnt - 1) * 2) + 1;
							printf("pno = %d, val = %f\n",iPno++, fVal[0]);
							printf("pno = %d, val = %f\n",iPno++, fVal[1]);
							printf("pno = %d, val = %f\n",iPno++, fVal[2]);
							printf("pno = %d, val = %f\n",iPno++, fVal[3]);
						}	
					}
				}
				// read group
				else if ( iDataCnt == 0x40 ) {
					pData = (unsigned char *) &p->rxbuf[3];							
					iPcm = (iSlaveId * 2);
					
					printf("iPcm = %d\n", iPcm);					
					printf("iDataCnt = %d\n", iDataCnt);

					for ( iCnt = 0; iCnt < iDataCnt; iCnt++ ) {	
						
						if ( ( *(pData+iCnt) & 0xf0) == 0xf0 ) {
							fVal[0] = (float) (*(pData+iCnt) & 0x01);
							fVal[1] = (float) ((*(pData+iCnt)>>1) & 0x01);
							fVal[2] = (float) ((*(pData+iCnt)>>2) & 0x01);
							fVal[3] = (float) ((*(pData+iCnt)>>3) & 0x01);
							
							iPno = ((iCnt - 1) * 2) + 1;
							printf("pno = %d, val = %f\n",iPno++, fVal[0]);
							printf("pno = %d, val = %f\n",iPno++, fVal[1]);
							printf("pno = %d, val = %f\n",iPno++, fVal[2]);
							printf("pno = %d, val = %f\n",iPno++, fVal[3]);
						}
					}
				}
				// read pattern
				else if ( iDataCnt == 0x0c ) {
					pData = (unsigned char *) &p->rxbuf[3];							
					iPcm = (iSlaveId * 2);
					
					printf("iPcm = %d\n", iPcm);					
					printf("iDataCnt = %d\n", iDataCnt);

					for ( iCnt = 0; iCnt < iDataCnt; iCnt++ ) {	
						
						if ( ( *(pData+iCnt) & 0xf0) == 0xf0 ) {
							fVal[0] = (float) (*(pData+iCnt) & 0x01);
							fVal[1] = (float) ((*(pData+iCnt)>>1) & 0x01);
							fVal[2] = (float) ((*(pData+iCnt)>>2) & 0x01);
							fVal[3] = (float) ((*(pData+iCnt)>>3) & 0x01);
							
							iPno = ((iCnt - 1) * 2) + 1;
							printf("pno = %d, val = %f\n",iPno++, fVal[0]);
							printf("pno = %d, val = %f\n",iPno++, fVal[1]);
							printf("pno = %d, val = %f\n",iPno++, fVal[2]);
							printf("pno = %d, val = %f\n",iPno++, fVal[3]);
						}
					}					
				}				
				break;
			}	

			case 0x11: {
				iDataCnt = pBuf[2];
				iPcm = (iSlaveId * 2) + 1;
				if ( iDataCnt == 0x03 ) {
					printf("FULL2WAY %d. State = %c\n", 
						iSlaveId, pBuf[5] );
					
					if ( pBuf[5] == 'Z' ) 
						pSet(iPcm, 0, 0);
					else 
						pSet(iPcm, 0, 1);
				}
				break;	
			}						
		}
	}

}


/****************************************************************/
/* check type for message that to send */
int IFaceHT_On_Send_Handler(void *pIFaceHT)
/****************************************************************/
{
	float fVal = 0;
	unsigned int iPcm = 0;
	unsigned int iPno = 0;
	unsigned int iCtrl = 0;
	unsigned int iCtrlType = 0;
	IfaceHT_T *p = NULL;

	/* Initialize */
	p = (IfaceHT_T *)pIFaceHT;
	iCtrl = p->ctrl;
	iPcm = ISPCM(iCtrl);
	iPno = ISPNO(iCtrl);
	fVal = p->ctrlVal;

	// 0�̸�, Group�� Pattern�� �����ϴ� PCM��ȣ�� ��Ÿ���� ���̰�, 
	// 1�̸� �� Relay�� ���������ϴ� PCM ��ȣ�� ��Ÿ���� ���̴�. 
	iCtrlType = IS_CTRLTYPE(iPcm);

	printf("iCtrl = %d, iCtrlType = %d, Pcm = %d, Pno= %d val = %f\n", 
		iCtrl, iCtrlType, iPcm, iPno, p->ctrlVal);
	
	if ( iCtrlType == 1 ) {
		// Relay�� 1 ~ 255���� pno�� ���εȴ�. 
		if ( iPno > 0 && iPno < 256 ) {
			p->ctrlType = IFACE_HT_CTRL_RELAY;
			IFaceHT_On_Make_Data(p);
			IFaceHT_On_Send(p);
			return IFACE_HT_CTRL_RELAY;		
		}	
	}
	else {
		printf("Ignore pattern and group control!!\n");
		return -1;

		// Group number is 1 ~ 128
		if ( iPno == 0 ) {
			return -1;
		}
		else if ( iPno > 0 && iPno <= 128 ) {
			p->ctrlType = IFACE_HT_CTRL_GROUP;
			IFaceHT_On_Make_Data(p);
			IFaceHT_On_Send(p);			
			return IFACE_HT_CTRL_GROUP;		
		}
		// Pattern number is 129 ~ 152
		else if ( iPno <= 152 ) {
			p->ctrlType = IFACE_HT_CTRL_PATTERN;
			IFaceHT_On_Make_Data(p);
			IFaceHT_On_Send(p);
			return IFACE_HT_CTRL_PATTERN;		
		}
	}

	return -1;
}


/****************************************************************/
/* check alive slave id */
int IFaceHT_On_Alive_Handler(void *pIFaceHT)
/****************************************************************/
{
	int ret = -1;
	unsigned char pcm = 0;
	IfaceHT_T *p = NULL;
	//unsigned char *pSlave = NULL;
	int iLoopcnt;
	int iAlivecnt;
	unsigned char cAliveId[16];
	
	/* Initialize */
	p = (IfaceHT_T *)pIFaceHT;
	iAlivecnt = 0;
	iLoopcnt = 0;
	memset( &cAliveId, 0, sizeof(cAliveId) );

	/* It fine alive slave id */
	for ( pcm = 0; pcm < MAX_PCM_COUNT; pcm=pcm+2 ) {
		if (g_fExPtbl[pcm][0] == 1 ) {
			ret++;
			cAliveId[iAlivecnt++] = 1;
		}
		else {
			cAliveId[iAlivecnt++] = 0;
		}
	}
	
	return ret;
}


/****************************************************************/
int IFaceHT_On_Change_SlaveId(void *pIFaceHT)
/****************************************************************/
{
	//int ret = -1;
	unsigned char pcm = 0;
	IfaceHT_T *p = NULL;
	int iLoopcnt;
	int iAlivecnt;
	unsigned char cAliveId[16];
	
	// Initialize 
	p = (IfaceHT_T *)pIFaceHT;
	iAlivecnt = 0;
	iLoopcnt = 0;
	memset( &cAliveId, 0, sizeof(cAliveId) );

	// It fine alive slave id 
	for ( pcm = 0; pcm < MAX_PCM_COUNT; pcm = pcm+2 ) {
		if (g_fExPtbl[pcm][0] == 1 ) {
			//printf("1  cAliveId[%d] On\n", iAlivecnt);
			cAliveId[iAlivecnt++] = 1;
		}
		else {
			//printf("2  cAliveId[%d] Off\n", iAlivecnt);
			cAliveId[iAlivecnt++] = 0;
		}
	}
	
	p->readId++;
	for ( iLoopcnt = p->readId; iLoopcnt < 16; iLoopcnt++) {
		if ( cAliveId[iLoopcnt] == 1 ) {
			p->readId = iLoopcnt;
			//printf("3  p->readId = %d\n", p->readId);
			return p->readId;
		}
	} 

	for ( iLoopcnt = 0; iLoopcnt < 16; iLoopcnt++) {
		if ( cAliveId[iLoopcnt] == 1 ) {
			p->readId = iLoopcnt;
			//printf("4  p->readId = %d\n", p->readId);
			return p->readId;
		}
	} 

	return -1;
}


/****************************************************************/
void IFaceHT_On_Make_Data(void *pIFaceHT)
/****************************************************************/
{
	//int i = 0;
	//int iSlaveId = 0;
	int iLength = 0;
	unsigned short crc16;
	IfaceHT_T *p = NULL;	
	IfaceHT_ReadMsg_T *pRead;
	IfaceHT_WriteMsg_T *pWrite;
	IfaceHT_InfoMsg_T *pReport;
	unsigned char pcm = 0;
	unsigned char pno = 0;	
	unsigned char cDiv = 0;
	unsigned char cDivNo = 0;
	unsigned char cDivSw = 0;
	unsigned char cGrp = 0;
	unsigned char cGrpNo = 0;
	unsigned char cGrpSw = 0;
	unsigned char cPttn = 0;
	unsigned char cPttnNo = 0;
	unsigned char cPttnSw = 0;
	unsigned char cRelayVal = 0;
	
	p = (IfaceHT_T *)pIFaceHT;
	pRead = (IfaceHT_ReadMsg_T *)p->txbuf;
	pWrite = (IfaceHT_WriteMsg_T *)p->txbuf;
	pReport = (IfaceHT_InfoMsg_T *)p->txbuf;

	switch (p->ctrlType) {
		case IFACE_HT_CTRL_RELAY: 	
			printf("IFACE_HT_CTRL_RELAY\n");
			pcm = ISPCM(p->ctrl);
			pno = ISPNO(p->ctrl);
			printf("PCM = %d, PNO = %d\n", pcm, pno);
			
			cDiv = IS_DIV(pno);
			cDivNo = IS_DIVNO(pno);
			cDivSw = IS_DIVSW(pno);
			
			if (p->ctrlVal == 1)
				cRelayVal = 0x10 + (cDivSw * 2) - 2 + 1;
			else
				cRelayVal = 0x10 + (cDivSw * 2) - 2 + 0;
			
			printf("cDiv = %d, cDivNo = %d, cDivSw = %d, cRelayVal = %d\n", 
				cDiv, cDivNo, cDivSw, cRelayVal);
				
			iLength = sizeof(IfaceHT_WriteMsg_T);
			pWrite->id = IS_SLAVEID(pcm) + g_nSlaveOffet;
			pWrite->cmd = 0x06; 			// FIX command
			pWrite->addrHi = 0x00;
			pWrite->addrLo = cDivNo;
			pWrite->dataHi = 0x00;
			pWrite->dataLo = cRelayVal;		
			crc16 = IsCrc16( pWrite, (iLength - 2) );
			pWrite->crcHi = (crc16>>8) & 0xff;	
			pWrite->crcLo = crc16 &0xff;	
			p->sendLength = iLength;	
			break;
			
		case IFACE_HT_CTRL_GROUP: 	
			printf("IFACE_HT_CTRL_GROUP\n");
			pcm = ISPCM(p->ctrl);
			pno = ISPNO(p->ctrl);
			cGrp = IS_GROUP(pno);
			cGrpNo = IS_GROUPNO(pno);
			cGrpSw = IS_GROUPSW(pno);	

			if (p->ctrlVal == 1)
				cRelayVal = 0x10 + (cGrpSw * 2) - 2 + 1;
			else
				cRelayVal = 0x10 + (cGrpSw * 2) - 2 + 0;			
			
			printf("PCM = %d, PNO = %d\n", pcm, pno);	
			printf("cGrp = %d, cGrpNo = %d, cGrpSw = %d, cRelayVal = %d\n", 
				cGrp, cGrpNo, cGrpSw, cRelayVal);					

			iLength = sizeof(IfaceHT_WriteMsg_T);
			pWrite->id = IS_SLAVEID(pcm) + g_nSlaveOffet;
			pWrite->cmd = 0x06; 			// FIX command
			pWrite->addrHi = 0x00;
			pWrite->addrLo = cGrpNo + IFACE_HT_GROUP_OFFSET;
			pWrite->dataHi = 0x00;
			pWrite->dataLo = cRelayVal;		
			crc16 = IsCrc16( pWrite, (iLength - 2) );
			pWrite->crcHi = (crc16>>8) & 0xff;	
			pWrite->crcLo = crc16 &0xff;	
			p->sendLength = iLength;					
			break;
			
		case IFACE_HT_CTRL_PATTERN: 	
			printf("IFACE_HT_CTRL_PATTERN\n");
			pcm = ISPCM(p->ctrl);
			pno = ISPNO(p->ctrl);
			cPttn = IS_PATTERN(pno);
			cPttnNo = IS_PATTERNNO(pno);
			cPttnSw = IS_PATTERNSW(pno);	

			if (p->ctrlVal == 1)
				cRelayVal = 0x10 + (cGrpSw * 2) - 2 + 1;
			else
				cRelayVal = 0x10 + (cGrpSw * 2) - 2 + 0;			
			
			printf("PCM = %d, PNO = %d\n", pcm, pno);	
			printf("cPttn = %d, cPttnNo = %d, cPttnSw = %d, cRelayVal = %d\n", 
				cPttn, cPttnNo, cPttnSw, cRelayVal);							

			iLength = sizeof(IfaceHT_WriteMsg_T);
			pWrite->id = IS_SLAVEID(pcm) + g_nSlaveOffet;
			pWrite->cmd = 0x06; 			// FIX command
			pWrite->addrHi = 0x00;
			pWrite->addrLo = cPttnNo + IFACE_HT_PATTERN_OFFSET;
			pWrite->dataHi = 0x00;
			pWrite->dataLo = cRelayVal;		
			crc16 = IsCrc16( pWrite, (iLength - 2) );
			pWrite->crcHi = (crc16>>8) & 0xff;	
			pWrite->crcLo = crc16 &0xff;	
			p->sendLength = iLength;					
			break;
			
		/* read whole relay data */
		case IFACE_HT_CTRL_READ_RELAY: 	
			iLength = sizeof(IfaceHT_ReadMsg_T);
			pRead->id = p->readId  + g_nSlaveOffet;
			pRead->cmd = 0x03; 			// FIX command
			pRead->addrHi = 0x00;
			pRead->addrLo = 0x80;		// FIX address
			pRead->dataHi = 0x00;
			pRead->dataLo = 0x01;		// FIX data count
			crc16 = IsCrc16( pRead, (iLength - 2) );
			pRead->crcHi = (crc16>>8) & 0xff;	
			pRead->crcLo = crc16 &0xff;	
			p->sendLength = iLength;
			break;
			
		/* read whole group data */
		case IFACE_HT_CTRL_READ_GROUP: 	
			iLength = sizeof(IfaceHT_ReadMsg_T);
			pRead->id = p->readId + g_nSlaveOffet;
			pRead->cmd = 0x03; 			// FIX command
			pRead->addrHi = 0x00;
			pRead->addrLo = IFACE_HT_GROUP_OFFSET;		// FIX address
			pRead->dataHi = 0x00;
			pRead->dataLo = 0x20;		// FIX data count
			crc16 = IsCrc16( pRead, (iLength - 2) );
			pRead->crcHi = (crc16>>8) & 0xff;	
			pRead->crcLo = crc16 &0xff;	
			p->sendLength = iLength;
			break;		

		/* read whole pattern data */
		case IFACE_HT_CTRL_READ_PATTERN: 	
			iLength = sizeof(IfaceHT_ReadMsg_T);
			pRead->id = p->readId + g_nSlaveOffet;
			pRead->cmd = 0x03; 			// FIX command
			pRead->addrHi = 0x00;
			pRead->addrLo = IFACE_HT_PATTERN_OFFSET;		// FIX address
			pRead->dataHi = 0x00;
			pRead->dataLo = 0x06;		// FIX data count
			crc16 = IsCrc16( pRead, (iLength - 2) );
			pRead->crcHi = (crc16>>8) & 0xff;	
			pRead->crcLo = crc16 &0xff;	
			p->sendLength = iLength;
			break;						

		/* read full2way state */	
		case IFACE_HT_CTRL_INFO: 	
			iLength = sizeof(IfaceHT_InfoMsg_T);
			pReport->id = p->readId  + g_nSlaveOffet;
			pReport->cmd = 0x11; 		// FIX command
			crc16 = IsCrc16( pReport, (iLength - 2) );
			pReport->crcHi = (crc16>>8) & 0xff;	
			pReport->crcLo = crc16 &0xff;	
			p->sendLength = iLength;
			break;
	}
}

/****************************************************************/
void IFaceHT_On_Dummy_Handler(void *pIFaceHT) 
/****************************************************************/
{
	unsigned int iLength;
	unsigned short crc16;
	IfaceHT_T *p = NULL;
	IfaceHT_WriteMsg_T *pWrite;
	
	p = (IfaceHT_T *)pIFaceHT;
	pWrite = (IfaceHT_WriteMsg_T *)p->txbuf;

	iLength = sizeof(IfaceHT_WriteMsg_T);
	pWrite->id = p->readId  + g_nSlaveOffet;
	pWrite->cmd = 0x06; 		// FIX command
	pWrite->addrHi = 0x00;
	pWrite->addrLo = 0x3F;		// FIX dummy addr (last address)
	pWrite->dataHi = 0x00;
	pWrite->dataLo = 0x16;		// FIX dummy value (last switch off)
	crc16 = IsCrc16( pWrite, (iLength - 2) );
	pWrite->crcHi = (crc16>>8) & 0xff;	
	pWrite->crcLo = crc16 & 0xff;	
	p->sendLength = iLength;		
	
	IFaceHT_On_Send(p);
	IFaceHT_On_Recv(p);
}


/****************************************************************/
int IFaceHT_On_Observer(void *pIFaceHT)
/****************************************************************/
{
	unsigned int iMaxLoop = 0;
	unsigned int iCntLoop = 0;
	//int ret = -1;
	IfaceHT_T *p = NULL;
	float *pNowVal = NULL;
	float *pPreVal = NULL;
	
	/* Initialize */
	p = (IfaceHT_T *)pIFaceHT;
	pNowVal = (float *)p->nowPntTable;	
	pPreVal = (float *)p->prePntTable;

	/* It get maximum loop count */
	iMaxLoop = MAX_NET32_NUMBER * MAX_POINT_NUMBER;

	/* It find control data in point-table */ 
	for ( iCntLoop = 0; iCntLoop < iMaxLoop; iCntLoop++ ) {
		if ( *(pNowVal + iCntLoop) != 	*(pPreVal + iCntLoop) ) {
			*(pPreVal + iCntLoop) = *(pNowVal + iCntLoop);
			p->ctrl = iCntLoop;
			p->ctrlVal = *(pNowVal + iCntLoop);
			return iCntLoop;
		}
	}
	
	return -1;
}

/****************************************************************/
IfaceHT_T *New_IfaceHT(void)
/****************************************************************/
{
	IfaceHT_T *uart2;
	
	uart2 = (IfaceHT_T *) malloc( sizeof(IfaceHT_T) );
	memset( uart2, 0, sizeof(IfaceHT_T) );

	uart2->fd = -1;
	uart2->pollState = 0;
	memset( &uart2->pollEvents, 0, sizeof(struct pollfd) );
	
	uart2->recvLength = 0;	
	uart2->sendLength = 0;
	uart2->rxbuf = (unsigned char *) malloc (MAX_BUF_SIZE);
	uart2->txbuf = (unsigned char *) malloc (MAX_BUF_SIZE);
	memset( uart2->rxbuf, 0, MAX_BUF_SIZE );
	memset( uart2->txbuf, 0, MAX_BUF_SIZE );	

	uart2->ctrl = 0;
	uart2->ctrlVal = 0;

	uart2->nowPntTable = (float *)&g_fExPtbl;
	uart2->prePntTable = (float *)&g_fPreExPtbl;
	memcpy( uart2->prePntTable, uart2->nowPntTable, sizeof(g_fExPtbl) );

	uart2->readId = 0;

	return uart2;
}


/****************************************************************/
void IfaceHT_Close(IfaceHT_T *p)
/****************************************************************/
{
	close(p->fd);
	memset( p, 0, sizeof(IfaceHT_T) );
	p->fd = -1;
}


/****************************************************************/
int IfaceHT_Open(IfaceHT_T *p)
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

	newtio.c_cflag = B19200 | CS8 | CLOCAL | CREAD;
	
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN]  = 1;
	
	tcflush(p->fd, TCIFLUSH);
	tcsetattr(p->fd,TCSANOW,&newtio);	

	fcntl(p->fd, F_SETFL, FNDELAY); 

	/* poll initiailze */
	p->pollEvents.fd = p->fd; 
	/*
		#define POLLIN 0x0001 // ���� �����Ͱ� �ִ�. 
		#define POLLPRI 0x0002 // ����� ���� ����Ÿ�� �ִ�. 
		#define POLLOUT 0x0004 // ���Ⱑ ����(block)�� �ƴϴ�. 
		#define POLLERR 0x0008 // �����߻� 
		#define POLLHUP 0x0010 // ������ ������ 
		#define POLLNVAL 0x0020 // ���������ڰ� ������ ���� �� ����, Invalid request (�߸��� ��û) 	
	*/
	p->pollEvents.events  = POLLIN | POLLERR;
	p->pollEvents.revents = 0;	
	
	return 1;
}


void IFaceHT_Guri(void)
{
	pSet(0, 0, 0);
	pSet(2, 0, 1);
	pSet(4, 0, 0);
	pSet(6, 0, 0);
	pSet(8, 0, 0);
	pSet(10, 0, 0);
	pSet(12, 0, 0);
	pSet(14, 0, 0);
	pSet(16, 0, 0);
	pSet(18, 0, 0);
	pSet(20, 0, 0);
	pSet(22, 0, 0);
	pSet(24, 0, 0);
	pSet(26, 0, 0);
	pSet(28, 0, 0);
	pSet(30, 0, 0);
}


void IFaceHT_KimChun(void)
{
	pSet(0, 0, 0);
	pSet(2, 0, 1);
	pSet(4, 0, 0);
	pSet(6, 0, 0);
	pSet(8, 0, 0);
	pSet(10, 0, 0);
	pSet(12, 0, 0);
	pSet(14, 0, 0);
	pSet(16, 0, 0);
	pSet(18, 0, 0);
	pSet(20, 0, 0);
	pSet(22, 0, 0);
	pSet(24, 0, 0);
	pSet(26, 0, 0);
	pSet(28, 0, 0);
	pSet(30, 0, 0);

	printf("KimChun Point Table Init 2010-1008\n");
}


void IFaceHT_OhSong(void)
{
	pSet(0, 0, 0);
	pSet(2, 0, 1);
	pSet(4, 0, 1);
	pSet(6, 0, 1);
	pSet(8, 0, 0);
	pSet(10, 0, 0);
	pSet(12, 0, 0);
	pSet(14, 0, 0);
	pSet(16, 0, 0);
	pSet(18, 0, 0);
	pSet(20, 0, 0);
	pSet(22, 0, 0);
	pSet(24, 0, 0);
	pSet(26, 0, 0);
	pSet(28, 0, 0);
	pSet(30, 0, 0);
	
	printf("OhSong Point Table Init 2010-1008\n");
}


void IFaceHT_PointSet(void)
{
	int i = 0;
	int nPcm = 0;
	
	pSet(0, 0, 0);
	
	if ( g_nCount > 0 ) {
		for (i = 0; i < g_nCount; i++) {
			nPcm = ( 1 + i) * 2;			
			fprintf(stdout, "Set Pcm :: %d\n", nPcm);
			fflush(stdout);			
			pSet(nPcm, 0, 1);
		}
	}
	else {
	}	
}

/*
int IfaceHT_TimeLine (void)
{
	time_t     tm_nd;
	struct tm *tm_ptr;
	
	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);

	if ( (tm_ptr->tm_mon + 1) == 12 )
		return -1;

	if ( (tm_ptr->tm_mon + 1) >= 1 && (tm_ptr->tm_mday) >= 6 ) {
		printf("Ignore TimeLine  %d / %d)\n", (tm_ptr->tm_mon + 1), (tm_ptr->tm_mday) );
		return 1;
	}
	else
		return -1;
}
*/

void IfaceHT_GetConfig(void)
{
	int i = 0;
	FILE *fp;
	char bufName[32];
	char bufId[32];
	int nPcm = 0;
	
	if((fp = fopen("Light.cfg", "r+")) == NULL) {
		fprintf(stdout, "Light.cfg open fail.\n");
		fflush(stdout);
		g_nCount = 0;
		return;			
	}

	while(!feof(fp)) {
		fscanf(fp, "%s %s", bufName, bufId);	
	}	
	
	fclose(fp);
	fprintf(stdout, "Read File :: %s, %s\n", bufName, bufId);
	fflush(stdout);

	g_nCount = atoi(bufId);

	pSet(0, 0, 0);
	
	if ( g_nCount > 0 ) {
		for (i = 0; i < g_nCount; i++) {
			nPcm = ( 1 + i) * 2;			
			fprintf(stdout, "Set Pcm :: %d\n", nPcm);
			fflush(stdout);			
			pSet(nPcm, 0, 1);
		}
	}
	else {
	}
		
}

/****************************************************************/
void *IfaceHT_Main(void* arg)
/****************************************************************/
{
	int iLoopCnt = 0;
	//int nRecvByte = 0;
	IfaceHT_T *pHt = NULL;
	struct timespec ts;

	sleep(3);

	pHt = New_IfaceHT();
	if (pHt == NULL) {
		printf("Can't create HANTEC interface.\n");
		system("killall duksan");
	}

	printf("Start HANTEC interface.\n");
	while (1) {
		// open uart2 
		if ( IfaceHT_Open(pHt) < 0 ) {
			sleep(3);
			IfaceHT_Close(pHt);
			continue;	
		}
	
		IfaceHT_GetConfig();
		
		for(;;) {	
			// Test Code
			/*
			// wait delay
			iLoopCnt++;
			printf(".");
			ts.tv_sec = 0;
			//ts.tv_nsec = 100 * M_SEC;				// read
			ts.tv_nsec = 1 * M_SEC;					// poll
			nanosleep(&ts, NULL);	
			nRecvByte = IFaceHT_On_Recv(pHt);	// receive and parsing
			//nRecvByte = read(pHt->fd, pHt->rxbuf, 32);
			fprintf(stdout, "(%d)nRecvByte = %d\n", iLoopCnt, nRecvByte);
			fflush(stdout);
			if ( iLoopCnt >= 1000 )
				iLoopCnt = 0;
			continue;
			*/
						
			// guri interface.
			//IFaceHT_Guri();
			//IFaceHT_KimChun();
			//IFaceHT_OhSong();
			//IFaceHT_PointSet();	
	
			//IfaceHT_Sleep(0, 500);
			// wait delay
			ts.tv_sec = 0;
			ts.tv_nsec = 500 * M_SEC;				
			nanosleep(&ts, NULL);			
			
			// == check alive sequence ==
			// 0, 2, 4, ... 30 PCM�� 0�� pno�� LIU�� ���°��� ��Ÿ����. 
			// It check alive slave id
			if ( IFaceHT_On_Alive_Handler(pHt) < 0 ) {
				//IfaceHT_Sleep(2, 0);
				sleep(2);
				continue; 
			}
			
			// == write sequence ==
			// 1. point-table�� check�ؼ� �ٲ� ���� �ִ����� Ȯ���Ѵ�. 
			//   �ٲ� ���� ���ٸ� relay ���°��� �о�´�. 
			// It check value of control
			if ( IFaceHT_On_Observer(pHt) > 0) {
				printf("\n\nControl light ===============\n");
				if ( IFaceHT_On_Send_Handler(pHt) > 0) {	// send packet
					
					IfaceHT_Sleep(0, 500);
					// wait delay
					//ts.tv_sec = 0;
					//ts.tv_nsec = 500 * M_SEC;				
					//nanosleep(&ts, NULL);			
										
					IFaceHT_On_Recv(pHt);			// receive and parsing
				}	
				printf("Wait delay 2 Sec\n");
				//IfaceHT_Sleep(2, 0);
				sleep(2);
				iLoopCnt = 0;
				continue;
			}		
			
			// for(;;)�� �ѹ� �� �� ���� 500ms�� sleep�� �ϱ� ������
			// �� 5�ʸ��� �ѹ��� Relay�� ���� ���� �о� �´�. 
			if (iLoopCnt++ > 10) {
				iLoopCnt = 0;

				// Light.cfg ������ �о PCM�� �ڵ����� �����մϴ�.
				IFaceHT_PointSet();	
				
				if (IFaceHT_On_Change_SlaveId(pHt) < 0)
					continue;
				
				/* == check full2way sequence ==
				Ȱ��ȭ�� LIU�� full2way�� ���°��� Ȯ���Ѵ�.  
				*/ 
				/*
				printf("\n\nCheck Full2way ===============\n");
				pHt->ctrlType = IFACE_HT_CTRL_INFO;
				pHt->on_make_data(pHt);		// make read packet for check full2way
				IFaceHT_On_Send(pHt);	// send packet
				IFaceHT_On_Recv(pHt);	// receive and parsing		
				*/
			
				/* == read sequence ==
				1. dummy packet �� �����ؼ� relay ���°��� ���ΰ�ħ�ǵ��� �Ѵ�.
				2. �� 2�ʸ� ��ٸ���. 
				3. ��ü relay���� �о������ �Ѵ�. 
				4. ��ü group���� �о������ �Ѵ�. 
				5. ��ü pattern���� �о������ �Ѵ�. 
				*/ 
				/*
				printf("\n\nSend Dummy packet ===============\n");
				IFaceHT_On_Dummy_Handler(pHt);	// send dummy packet
				*/
				printf("\n\nRead Relay ===============\n");
				pHt->ctrlType = IFACE_HT_CTRL_READ_RELAY;
				IFaceHT_On_Make_Data(pHt);		// make read packet for check relay
				IFaceHT_On_Send(pHt);	// send packet
				
				IfaceHT_Sleep(0, 500);
				// wait delay
				//ts.tv_sec = 0;
				//ts.tv_nsec = 500 * M_SEC;				
				//nanosleep(&ts, NULL);			
							
				IFaceHT_On_Recv(pHt);	// receive and parsing
				
				printf("Wait delay 3 Sec\n");
				//IfaceHT_Sleep(3, 0);
				sleep(3);

				/*
				printf("\n\nRead Group ===============\n");
				pHt->ctrlType = IFACE_HT_CTRL_READ_GROUP;
				IFaceHT_On_Make_Data(pHt);		// make read packet for check relay
				IFaceHT_On_Send(pHt);	// send packet
				IFaceHT_On_Recv(pHt);	// receive and parsing

				printf("\n\nRead Pattern ===============\n");
				pHt->ctrlType = IFACE_HT_CTRL_READ_PATTERN;
				IFaceHT_On_Make_Data(pHt);		// make read packet for check relay
				IFaceHT_On_Send(pHt);	// send packet
				IFaceHT_On_Recv(pHt);	// receive and parsing	
				*/
			}
			continue;		
		}
		continue;
	}

	free(pHt->txbuf);
	free(pHt->rxbuf);

	//exit(1);
	system("killall duksan");
}


