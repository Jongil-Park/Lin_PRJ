///******************************************************************/
// file : ghpSchedSvr.c
// date : 2010.02.24.
// author : jong2ry
///******************************************************************/
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
#include <sys/poll.h>		// use poll event

#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

#include "ghp_app.h"
#include "ghp_WebSvr.h"

unsigned char web2_rx_msg[MAX_BUF_LENGTH];
unsigned char web2_tx_msg[MAX_BUF_LENGTH];

int g_Web2DataCount = 0;
//int g_nTotalLength = 0;

int g_Web2_Pcm[512];
int g_Web2_Pno[512];
float g_Web2Fval[512];
unsigned short g_Web2Data[256];

extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];

extern float pGet(int pcm, int pno);
extern int pSet(int pcm, int pno, float value);


#define WEB2_INIT_SOCK				0
#define WEB2_ACCEPT_SOCK			1
#define WEB2_HANDLER_SOCK			2



/*
slect를 사용해서 TimeWait를 만들었다. 
*/
/*******************************************************************/
void IfaceGhpWeb2SelectSleep(int sec,int msec) 
/*******************************************************************/
{
    struct timeval tv;
    tv.tv_sec=sec;
    tv.tv_usec=msec * 1000;                
    select(0,NULL,NULL,NULL,&tv);
    return;
}



#if 1
/****************************************************************/
static void Web2_convert_float_to_4byte(unsigned char *pData, unsigned char *pTxMsg)
/****************************************************************/
{
    int i = 0;
    unsigned char cData[4];
    
    for (i = 0; i < 4; i++)
        cData[i] = *(pData + i);

    *(pTxMsg + 0) = cData[3];  
    *(pTxMsg + 1) = cData[2];
    *(pTxMsg + 2) = cData[1];
    *(pTxMsg + 3) = cData[0];

/*    
    *(pTxMsg + 0) = cData[1];  
    *(pTxMsg + 1) = cData[0];
    *(pTxMsg + 2) = cData[3];
    *(pTxMsg + 3) = cData[2];
*/    
} 
#endif

#if 1
/****************************************************************/
static void Web2_convert_4byte_to_float(unsigned char *pData, unsigned char *pRxMsg)
/****************************************************************/
{
    int i = 0;
    unsigned char cData[4];
    
    for (i = 0; i < 4; i++)
        cData[i] = *(pRxMsg + i);

    *(pData + 0) = cData[3];  
    *(pData + 1) = cData[2];
    *(pData + 2) = cData[1];
    *(pData + 3) = cData[0];    
} 
#endif

/****************************************************************/
static char Web2_CalculateCheckSum(char *chBuf, int nLength)
/****************************************************************/
{
	int i = 0;
	char chSum = 0;
	
	for(i = 0; i < nLength; i++)
		chSum += *(chBuf + i);
	
	return chSum;
}


/****************************************************************/
static void Web2_swap_short(unsigned short *p)
/****************************************************************/
{
    unsigned char *pCh;
    unsigned char temp1;
    unsigned char temp2;
    
    pCh = (unsigned char *)p;
    temp1 = pCh[0];
    temp2 = pCh[1];
    pCh[0] = temp2;
    pCh[1] = temp1;
}


/****************************************************************/
static unsigned short Web2_RxData_to_Command(unsigned char *pData)
/****************************************************************/
{
    unsigned short nCmd = 0;
    _REQ_GET_CNT_DATA *pReq;
            
    // 2010.01.12에 새로이 추가되는 protocol. jong2ry
    pReq = (_REQ_GET_CNT_DATA *) pData;
    
/*
    // debug code. delete me...!!
    if(ghp_sanyo_tcp == MSG_ON)
    {
        printf("stx = %x\n", pReq->stx); 
        printf("len = %x\n", pReq->length);
        printf("cmd = %x\n", pReq->cmd);
        printf("pcm = %x\n", pReq->pcm);      
        printf("pno = %x\n", pReq->pno);
        printf("cnt = %x\n", pReq->cnt);
        printf("chksum = %x\n", pReq->chksum);
        printf("etx = %x\n", pReq->etx);
    }
*/    
    
    if(pReq->stx == '<')
    {
        // jogn2ry. 2010.03.03
        nCmd = pReq->cmd;
        Web2_swap_short(&nCmd);
        
        if (nCmd == GHPWEB_GET_CNT_DATA)
        {
            nCmd = GHPWEB_GET_CNT_DATA;
            return nCmd;
        }          
    }

    // 기존에 사용되는 protocol    
    nCmd = *(pData + ADDR_COMMAND) | *(pData + ADDR_COMMAND + 1);
/*    
    if(ghp_sanyo_tcp == MSG_ON)
    {
        printf("GHP-Web : Data Command = %d, (0x%x, 0x%x)\n",
            nCmd, *(pData + ADDR_COMMAND), *(pData + ADDR_COMMAND + 1));
    } 
*/    
    return nCmd;
}

/****************************************************************/
/* Web2_RxData_to_Count(unsigned char *pData)
 * Rx Data Format 
 * |CMD(2)|Cnt(2)|PCM(2)|Point(2)|......|Chksum(2)|
 */
static unsigned short Web2_RxData_to_Count(unsigned char *pData)
/****************************************************************/
{
    unsigned short nCnt = 0;
    _REQ_GET_CNT_DATA *pReq;
            
    // 2010.01.12에 새로이 추가되는 protocol. jong2ry
    pReq = (_REQ_GET_CNT_DATA *) pData;
    if(pReq->stx == '<')
    {
        nCnt = pReq->length;
        Web2_swap_short(&nCnt);        
        return nCnt;          
    }    
    
    // 기존에 사용되는 protocol
    nCnt = *(pData + ADDR_DATACNT) | *(pData + ADDR_DATACNT + 1);
    
    /*
    if(ghp_sanyo_tcp == MSG_ON)
    {
        printf("GHP-Web : Data Cnt = %d, (0x%x, 0x%x)\n",
            nCnt, *(pData + ADDR_DATACNT), *(pData + ADDR_DATACNT + 1));
    }
    */ 
    
    return nCnt;
}

/****************************************************************/
/* Web2_Calualate_TotLength(int nCnt, int nCmd)
 *  Calculate Total receive data length.
 */
static int Web2_Calculate_TotLength(int nCnt, int nCmd)
/****************************************************************/
{
    int nTotLength = 0;
    int nDataLength = 0;

    if (nCmd == GHPWEB_GET_DATA)
    {
        nTotLength = SIZE_WEB_CMD + SIZE_WEB_CNT + SIZE_WEB_CHKSUM; 
        nDataLength = nCnt * 4;
        nTotLength = nTotLength + nDataLength;

        //if(ghp_sanyo_tcp == MSG_ON)
        //    printf("GHP-Web : Total Data Length = %d\n", nTotLength);

        return nTotLength;
    }
    else if (nCmd == GHPWEB_SET_DATA)
    {
        nTotLength = SIZE_WEB_CMD + SIZE_WEB_CNT + SIZE_WEB_CHKSUM;
        nDataLength = nCnt * 8;
        nTotLength = nTotLength + nDataLength;

        //if(ghp_sanyo_tcp == MSG_ON)
        //    printf("%s(%d) : Total Data Length = %d\n", __FUNCTION__, __LINE__, nTotLength);

        return nTotLength;
    }
    else if (nCmd == TCP_MSG_GET)
    {
        nTotLength = MSG_GET_LENGTH_OLD;

        //if(ghp_sanyo_tcp == MSG_ON)
        //    printf("%s(%d) : Total Data Length = %d\n", __FUNCTION__, __LINE__, nTotLength);

        return nTotLength;
    }
    else if (nCmd == TCP_MSG_SET)
    {
        nTotLength = MSG_SET_LENGTH_OLD;

        //if(ghp_sanyo_tcp == MSG_ON)
        //    printf("%s(%d) : Total Data Length = %d\n", __FUNCTION__, __LINE__, nTotLength);

        return nTotLength;
    }
    else if (nCmd == GHPWEB_GET_CNT_DATA)
    {
        nTotLength = nCnt;

        //if(ghp_sanyo_tcp == MSG_ON)
        //    printf("%s(%d) : Total Data Length = %d\n", __FUNCTION__, __LINE__, nTotLength);

        return nTotLength;
    }    
    else
    {
        printf("%s(%d) : Command Error(%d)\n", __FUNCTION__, __LINE__, nCmd);
        return FAIL;   
    }
}

/****************************************************************/
static int Web2_Make_TxMsg_GetCntMode(unsigned char *pRcvData, unsigned char *pData, int nCnt)
/****************************************************************/
{
    int i = 0;
    int cnt = 0;
    unsigned short loopCnt = 0;
    unsigned short size = 0;
    unsigned short pcm = 0;
    unsigned short pno = 0;
    unsigned short valueSize = 0;
    int headerSize = 0;
    unsigned char cChkSum;
    //Point *p;
    _REQ_GET_CNT_DATA *pReq;
    _RES_GET_CNT_DATA_HEADER *pRes;

    memset(&g_Web2Data, 0x00, sizeof(g_Web2Data));
    pReq = (_REQ_GET_CNT_DATA *) pRcvData;

    // get PCM
    //nPcm = pReq->pcm % 32;
    pcm = pReq->pcm;
    Web2_swap_short(&pcm);
    pcm = pcm%32;
    // debug code. delete me.
    //if(ghp_sanyo_tcp == MSG_ON)     printf("nPcm = %x\n", pcm);    

    // get point number
    pno = pReq->pno;
    Web2_swap_short(&pno);
    //if(ghp_sanyo_tcp == MSG_ON)     printf("pno = %x\n", pno);
    
    //  get for-loop count
    loopCnt = pReq->cnt;
    Web2_swap_short(&loopCnt);
    //if(ghp_sanyo_tcp == MSG_ON)     printf("loopCnt 1 = %x\n", loopCnt);
    loopCnt = pno + loopCnt;
    //if(ghp_sanyo_tcp == MSG_ON)     printf("loopCnt 2 = %x\n", loopCnt);

    // get value. value-type is unsigned short (2 byte)
    cnt = 0;
    for (i = pno; i < loopCnt; i++)
    {
        // get point-table pointer.
        //if (pcm == MyId)        p = &ptbl[i];
        //else                    p = &g_fExPtbl[pcm][i];
        
        g_Web2Data[cnt++] = (unsigned short)(g_fExPtbl[pcm][i]* 100);
        Web2_swap_short(&g_Web2Data[cnt - 1]);
        // debug code. delete me.
        //printf("g_Web2Data[%d] = %x\n", i, g_Web2Data[cnt - 1]);    
    }    
    
    // get tx-message header.
    valueSize = pReq->cnt;
    Web2_swap_short(&valueSize); 
    valueSize = valueSize * sizeof(unsigned short);
    headerSize = sizeof(_RES_GET_CNT_DATA_HEADER);
    size = valueSize + headerSize;
    pRes = (_RES_GET_CNT_DATA_HEADER *) pData; 
    pRes->stx = '<';
    // 3을 더한것은 '>'과 chksum의 byte 수이다. 
    pRes->length = size + 3;                Web2_swap_short(&pRes->length);
    pRes->cmd = GHPWEB_GET_CNT_DATA;        Web2_swap_short(&pRes->cmd);
    pRes->pcm = pReq->pcm;                  
    pRes->pno = pReq->pno;                  
    pRes->cnt = pReq->cnt;                    
    memcpy(&pData[headerSize], &g_Web2Data[0], valueSize);

    // Get CheckSum Data.
    cChkSum = Web2_CalculateCheckSum(web2_tx_msg, size);
	web2_tx_msg[size] = 0x00;
	web2_tx_msg[size + 1] = cChkSum; 
    size += 2;
    web2_tx_msg[size++] = '>';
    
    // debug code. delete me.
    //if(ghp_sanyo_tcp == MSG_ON)     printf("size = %x\n", size);    

    return size;
}


static int Web2_Make_TxMsg_GetMode(unsigned char *pRcvData, unsigned char *pData, int nCnt)
{
    int i = 0;
    int nOffset = 0;
    int nSize = 0;
    int nPcm = 0;
    unsigned char cChkSum;
    
    memcpy(pData, pRcvData, SIZE_HEADER);
    nSize = SIZE_HEADER;
    
    // Get point value.
    for (i = 0; i < nCnt; i++)
    {
        nOffset = i * 4;
        g_Web2_Pcm[i] = (*(pRcvData + SIZE_HEADER + nOffset) << 8) + (*(pRcvData + SIZE_HEADER + nOffset + 1));  
        g_Web2_Pno[i] = *(pRcvData + SIZE_HEADER + nOffset + 2) | *(pRcvData + SIZE_HEADER + nOffset + 3);
        nPcm = g_Web2_Pcm[i] % 32; 
        g_Web2Fval[i] = pGet(nPcm, g_Web2_Pno[i]);
        //printf("Pcm = %d(%d), Pno = %d, fVal = %f\n", g_Web2_Pcm[i], nPcm, g_Web2_Pno[i], g_Web2Fval[i]);
    } 
    
    // Get Tx message Data.
    for (i = 0; i < nCnt; i++)    
    {
        nOffset = i * 8;
        
        *(pData + SIZE_HEADER + nOffset) = (0xff00 & g_Web2_Pcm[i]) >> 8;  
        *(pData + SIZE_HEADER + nOffset + 1) = 0x00ff & g_Web2_Pcm[i];
        *(pData + SIZE_HEADER + nOffset + 2) = (0xff00 & g_Web2_Pno[i]) >> 8;
        *(pData + SIZE_HEADER + nOffset + 3) = 0x00ff & g_Web2_Pno[i];
        
        //printf("fVal(%d) = %f\n", i, g_Web2Fval[i]);
        Web2_convert_float_to_4byte((unsigned char *)&g_Web2Fval[i], (pData + SIZE_HEADER + nOffset + 4));

        nSize += 8;
    }

    // Get CheckSum Data.
    cChkSum = Web2_CalculateCheckSum(web2_tx_msg, nSize);
	web2_tx_msg[nSize] = 0x00;
	web2_tx_msg[nSize + 1] = cChkSum; 
    nSize += 2;
    
    return nSize;
}



static int Web2_Make_TxMsg_GetMode_Old(unsigned char *pRcvData, unsigned char *pData, int nCnt)
{
    //int i = 0;
    int nOffset = 0;
    int nSize = 0;
    int nPcm = 0;
    int nPno = 0;
    float fVal = 0;
    unsigned char cChkSum;
    
    // Get point value.
    nOffset = PACKET_SIZE_OLD;        // 2 is Old Packet Header size
    nPcm = (*(pRcvData + nOffset) << 8) + (*(pRcvData + nOffset + 1));  
    nPcm = nPcm % 32;
    nPno = (*(pRcvData + nOffset + 2) << 8) + (*(pRcvData + nOffset + 3));
    fVal = pGet(nPcm, nPno);
    //printf("Pcm = %d, Pno = %d, fVal = %f\n", nPcm, nPno, fVal);

    nPcm = (*(pRcvData + nOffset) << 8) + (*(pRcvData + nOffset + 1)); 
    
    *(pData) = (0xff00 & TCP_MSG_GET) >> 8;
    *(pData + 1) = TCP_MSG_GET;    
    *(pData + 2) = (0xff00 & nPcm) >> 8;  
    *(pData + 3) = 0x00ff & nPcm;
    *(pData + 4) = (0xff00 & nPno) >> 8;
    *(pData + 5) = 0x00ff & nPno;
                                        
    Web2_convert_float_to_4byte((unsigned char *)&fVal, (pData + 6));

    // Get CheckSum Data.
    cChkSum = Web2_CalculateCheckSum(web2_tx_msg, 10);
	nSize = 10; 
    web2_tx_msg[nSize] = 0x00;
	web2_tx_msg[nSize + 1] = cChkSum; 
    
    // 12 is Tx message length. (Hard cording.)
    nSize = 12;  
    return nSize;
}




static int Web2_Make_TxMsg_SetMode(unsigned char *pRcvData, unsigned char *pData, int nCnt)
{
    int i = 0;
    int nOffset = 0;
    int nSize = 0;
    int nPcm = 0;
    //unsigned char cChkSum;
    
    memcpy(pData, pRcvData, SIZE_HEADER);
    nSize = SIZE_HEADER;
    
    // Set point value. 
    if (nCnt == 1) 
    {
        nOffset = i * 4;
        
        g_Web2_Pcm[i] = (*(pRcvData + SIZE_HEADER + nOffset) << 8) + (*(pRcvData + SIZE_HEADER + nOffset + 1));
        g_Web2_Pno[i] = *(pRcvData + SIZE_HEADER + nOffset + 2) | *(pRcvData + SIZE_HEADER + nOffset + 3);
        
        Web2_convert_4byte_to_float((unsigned char *)&g_Web2Fval[i], (pRcvData + SIZE_HEADER + nOffset + 4));
        nPcm = g_Web2_Pcm[i] % 32; 
        printf("GHP-Web : Pcm = %d(%d), Pno = %d, fVal = %f\n", g_Web2_Pcm[i], nPcm, g_Web2_Pno[i], g_Web2Fval[i]);
        pSet(nPcm, g_Web2_Pno[i], g_Web2Fval[i]);
    } 
    nSize += 8; 
    
    memcpy(pData, pRcvData, nSize);
    
    return nSize;
}



static int Web2_Make_TxMsg_SetMode_Old(unsigned char *pRcvData, unsigned char *pData, int nCnt)
{
    //int i = 0;
    int nOffset = 0;
    int nSize = 0;
    int nPcm = 0;
    int nPno = 0;
    //unsigned char cChkSum;
    
    nOffset = PACKET_SIZE_OLD;        // 2 is Old Packet Header size
    nPcm = (*(pRcvData + nOffset) << 8) + (*(pRcvData + nOffset + 1));
    nPno = *(pRcvData + nOffset + 2) | *(pRcvData +  nOffset + 3);
    
    Web2_convert_4byte_to_float((unsigned char *)&g_Web2Fval[0], (pRcvData + nOffset + 4));        // 4 is pcm, pno byte count.
    nPcm = nPcm % 32; 
    printf("GHP-Web : Pcm = %d, Pno = %d, fVal = %f\n", nPcm, nPno, g_Web2Fval[0]);
    pSet(nPcm, nPno, g_Web2Fval[0]);

    memcpy(pData, pRcvData, MSG_SET_LENGTH_OLD);
    nSize = MSG_SET_LENGTH_OLD;
    
    return nSize;
}

#if 0
/****************************************************************/
void *web2server_main(void)
/****************************************************************/
{
	//Variables
	int i, j;
	//
	int fd_max;
	fd_set reads, temps;
	struct timeval tv;
	int one = 1;
	//
	int server_socket;
	int client_socket;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	//
	int fd;
	int sin_size;
	int server_port;
	int status;

	//Initialize
	i = j = 0;
	//
	fd_max = 0;
	FD_ZERO(&reads);
	FD_ZERO(&temps);
	tv.tv_sec = 0;
	tv.tv_usec = 10000; //0.01sec
	//
	server_socket = 0;
	client_socket = 0;
	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));
	//
	fd = 0;
	sin_size = sizeof(client_addr);
	server_port = GHPWEB2_SERVER_PORT;
	status = INIT_PROCESS;
	
	//Ignore broken_pipe signal
	signal(SIGPIPE, SIG_IGN);
	IfaceGhpWeb2SelectSleep(3, 0);
	
	while(1)
	{
		IfaceGhpWeb2SelectSleep(0, 100);
		
		switch(status)
		{
			case INIT_PROCESS:
			{
				server_socket = socket(PF_INET, SOCK_STREAM, 0);
				printf("Web-Server-2 Started, Port [%d]\n", server_port);
				//
				server_addr.sin_family = AF_INET;
				server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
				server_addr.sin_port = htons(server_port);
				//
				setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
				//
				// bind error가 발생하는 것을 방지하기 위함.
			    setsockopt( server_socket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
				//
				if(bind(server_socket, (struct sockaddr *)&server_addr,
							sizeof(server_addr)) < 0)
				{
					printf("[ERROR] In Bind of Web-Server-2\n");
					close(server_socket);
					status = INIT_PROCESS;
					//sleep(10);
					IfaceGhpWeb2SelectSleep(10, 0);
					break;
				}
				//
				if(listen(server_socket, 10) < 0)
				{
					printf("[ERROR] In Listen of Web-Server-2\n");
					close(server_socket);
					status = INIT_PROCESS;
					//sleep(10);
					IfaceGhpWeb2SelectSleep(10, 0);
					break;
				}
				//
				FD_ZERO(&reads);
				FD_SET(server_socket, &reads);
				fd_max = server_socket;
				//
				status = SELECT_PROCESS;
				break;
			}
			//
			case SELECT_PROCESS:
			{
				//
				temps = reads;
				//
				if(select(fd_max+1, &temps, 0, 0, &tv) == -1)
				{
					printf("[ERROR] In Select of Web-Server-2\n");
					status = INIT_PROCESS;
					//sleep(10);
					IfaceGhpWeb2SelectSleep(10, 0);
					break;
				}
				//
				status = SELECT_PROCESS;
				//
				for(fd = 0; fd < (fd_max + 1); fd++)
				{
					if(FD_ISSET(fd, &temps))
					{	
						//if connection's been requested,
						if(fd == server_socket)
						{
							status = CONNECTION_REQUESTED;
							break;
						}
						else
						{
							status = HANDLE_COMMAND;
							break;
						}
					}
				}
				break;
			}
			//
			case CONNECTION_REQUESTED:
			{
				client_socket = accept(server_socket, (struct sockaddr *)&client_addr,
						               &sin_size);
				//
				if (client_socket == -1)
				{
					printf("[Error] In Accept of Web-Server-2\n");
					status = SELECT_PROCESS;
					break;
				}
				//
				FD_SET(client_socket, &reads);
				if(fd_max < client_socket)
					fd_max = client_socket;
				//
				status = SELECT_PROCESS;
				break;
			}
			//
			case HANDLE_COMMAND:
			{
				web2_handler(fd, &reads);
				status = SELECT_PROCESS;
				break;
			}
		}			
	}
}
#endif






//void web2_handler(int fd, fd_set *reads)
void web2_handler_2(int fd, int rxmsg_length)
{
	//int i = 0;
	//int status = 0;
	//int rxmsg_length = 0;
    int nDataCmd = 0;
    int nTotLength = 0;
    int nSize = 0;
    //unsigned char cChksum;	
	
/*
    // Debug Code. print Rx message
    if(ghp_sanyo_tcp == MSG_ON)
    {
        printf("\nRx Data(%d) = \n", rxmsg_length);
        for (i = 0; i < rxmsg_length; i++)
        {
            printf("%x ", web2_rx_msg[i]);
            if (!((i+1) % 16))
                printf("\n"); 
        }
        printf("\n");
    }
    
*/	
    for (;;)
    {
        nDataCmd = Web2_RxData_to_Command(web2_rx_msg); 
        g_Web2DataCount = Web2_RxData_to_Count(web2_rx_msg);
        nTotLength = Web2_Calculate_TotLength(g_Web2DataCount, nDataCmd);
    
        //printf(">> nDataCmd = %d\n", nDataCmd);        
        //printf(">> g_Web2DataCount = %d\n", g_Web2DataCount);
        //printf(">> nTotLength = %d\n", nTotLength);

        switch(nDataCmd)
        {
            case GHPWEB_GET_DATA:
                nSize = Web2_Make_TxMsg_GetMode(web2_rx_msg, web2_tx_msg, g_Web2DataCount);
                break;
                
            case GHPWEB_SET_DATA:
                nSize = Web2_Make_TxMsg_SetMode(web2_rx_msg, web2_tx_msg, g_Web2DataCount);
                break;
                
            case TCP_MSG_GET:
                nSize = Web2_Make_TxMsg_GetMode_Old(web2_rx_msg, web2_tx_msg, g_Web2DataCount);
                break;
                
            case TCP_MSG_SET:
                nSize = Web2_Make_TxMsg_SetMode_Old(web2_rx_msg, web2_tx_msg, g_Web2DataCount);
                break;

            case GHPWEB_GET_CNT_DATA:
                nSize = Web2_Make_TxMsg_GetCntMode(web2_rx_msg, web2_tx_msg, g_Web2DataCount);
                break;             
                 
            default :
                return;
        }
        
        send(fd, web2_tx_msg, nSize, 0);
/*
        // Debug Code. print Tx message
        if(ghp_sanyo_tcp == MSG_ON)
        {
            printf("Tx Data(%d)\n", nSize);
            for (i = 0; i < nSize; i++)    
            {
                printf("%x ", web2_tx_msg[i]);
                if (!((i+1) % 16))
                    printf("\n"); 
            }    
            printf("\n");
        }    
*/        
        rxmsg_length = rxmsg_length - nTotLength;
        
        if (rxmsg_length > 0)
        {
            memcpy(web2_rx_msg, 
                        web2_rx_msg + nTotLength, 
                        sizeof(web2_rx_msg) - nTotLength); 
            continue;
        }
        else
            break;

    }

    return;	
}


/****************************************************************/
void *web2server_main(void)
/****************************************************************/
{
	//Variables
	int i, j;
	//
	int fd_max;
	fd_set reads, temps;
	struct timeval tv;
	//
	int server_socket;
	int client_socket;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	//
	int fd;
	int sin_size;
	int server_port;
	int status;
	int rxmsg_length = 0;

	//Initialize
	i = j = 0;
	//
	fd_max = 0;
	FD_ZERO(&reads);
	FD_ZERO(&temps);
	tv.tv_sec = 0;
	tv.tv_usec = 10000; //0.01sec
	//
	server_socket = 0;
	client_socket = 0;
	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));
	//
	fd = 0;
	sin_size = sizeof(client_addr);
	server_port = GHPWEB2_SERVER_PORT;
	status = INIT_PROCESS;
	
	//Ignore broken_pipe signal
	signal(SIGPIPE, SIG_IGN);
	IfaceGhpWeb2SelectSleep(3, 0);
	
	while(1)
	{
		IfaceGhpWeb2SelectSleep(0, 50);

		server_socket = socket(PF_INET, SOCK_STREAM, 0);
		printf("Web-Server-2 Started, socket = [%d], Port [%d]\n", server_socket, server_port);
		
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons(server_port);
		
		setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &tv, sizeof(tv));
		
		if(bind(server_socket, (struct sockaddr *)&server_addr,
					sizeof(server_addr)) < 0) {
			printf("[ERROR] In Bind of Web-Server-2\n");
			close(server_socket);
			status = INIT_PROCESS;
			IfaceGhpWeb2SelectSleep(10, 0);
			continue;
		}
		
		if(listen(server_socket, 10) < 0) {
			printf("[ERROR] In Listen of Web-Server-2\n");
			close(server_socket);
			status = INIT_PROCESS;
			IfaceGhpWeb2SelectSleep(10, 0);
			continue;
		}
		
		status = WEB2_ACCEPT_SOCK;
		
		for (;;) {			
			switch (status) {
				// socket accept 를 기다린다.
				case WEB2_ACCEPT_SOCK:
					client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &sin_size);
					if (client_socket == -1) {
						printf("[Error] In Accept of Web-Server-2\n");
						IfaceGhpWeb2SelectSleep(1, 0);
						status = WEB2_ACCEPT_SOCK;
						break;
					}
					else 
						status = WEB2_HANDLER_SOCK;
						printf("Web-Server-2 Connection Ok.\n");
					break;
					
				case WEB2_HANDLER_SOCK:
					//web2_handler(fd, &reads);					
					memset(web2_rx_msg, 0, sizeof(web2_rx_msg));
					rxmsg_length = recv(client_socket, web2_rx_msg, (sizeof(web2_rx_msg)/2), 0);
				
					if ( rxmsg_length == 0 ) {
						close(client_socket);
						printf("Web-Server-2 Close\n");
						status = WEB2_ACCEPT_SOCK;
						break;
					}
					else if ( rxmsg_length > 0 ) {
						web2_handler_2(client_socket, rxmsg_length);
						status = WEB2_HANDLER_SOCK;
						break;					
					}
					else {
						status = WEB2_HANDLER_SOCK;	
						break;
					}				
					break;					
			}
		}
	}		
}




#if 0
/************************************************************/
void web2_handler(int fd, fd_set *reads)
/************************************************************/
{
	//int i = 0;
	//int status = 0;
	int rxmsg_length = 0;
    int nDataCmd = 0;
    int nTotLength = 0;
    int nSize = 0;
    //unsigned char cChksum;	

	memset(web2_rx_msg, 0, sizeof(web2_rx_msg));
	rxmsg_length = recv(fd, web2_rx_msg, sizeof(web2_rx_msg), 0);

	if(rxmsg_length == 0)
	{
		FD_CLR(fd, reads);
		close(fd);
		printf("Web-Server-2 Close\n");
		return;
	}
	
/*
    // Debug Code. print Rx message
    if(ghp_sanyo_tcp == MSG_ON)
    {
        printf("\nRx Data(%d) = \n", rxmsg_length);
        for (i = 0; i < rxmsg_length; i++)
        {
            printf("%x ", web2_rx_msg[i]);
            if (!((i+1) % 16))
                printf("\n"); 
        }
        printf("\n");
    }
    
*/	

    for (;;)
    {
        nDataCmd = Web2_RxData_to_Command(web2_rx_msg); 
        g_Web2DataCount = Web2_RxData_to_Count(web2_rx_msg);
        nTotLength = Web2_Calculate_TotLength(g_Web2DataCount, nDataCmd);
    
        //printf(">> nDataCmd = %d\n", nDataCmd);        
        //printf(">> g_Web2DataCount = %d\n", g_Web2DataCount);
        //printf(">> nTotLength = %d\n", nTotLength);

        switch(nDataCmd)
        {
            case GHPWEB_GET_DATA:
                nSize = Web2_Make_TxMsg_GetMode(web2_rx_msg, web2_tx_msg, g_Web2DataCount);
                break;
                
            case GHPWEB_SET_DATA:
                nSize = Web2_Make_TxMsg_SetMode(web2_rx_msg, web2_tx_msg, g_Web2DataCount);
                break;
                
            case TCP_MSG_GET:
                nSize = Web2_Make_TxMsg_GetMode_Old(web2_rx_msg, web2_tx_msg, g_Web2DataCount);
                break;
                
            case TCP_MSG_SET:
                nSize = Web2_Make_TxMsg_SetMode_Old(web2_rx_msg, web2_tx_msg, g_Web2DataCount);
                break;

            case GHPWEB_GET_CNT_DATA:
                nSize = Web2_Make_TxMsg_GetCntMode(web2_rx_msg, web2_tx_msg, g_Web2DataCount);
                break;             
                 
            default :
                return;
        }
        
        send(fd, web2_tx_msg, nSize, 0);
/*
        // Debug Code. print Tx message
        if(ghp_sanyo_tcp == MSG_ON)
        {
            printf("Tx Data(%d)\n", nSize);
            for (i = 0; i < nSize; i++)    
            {
                printf("%x ", web2_tx_msg[i]);
                if (!((i+1) % 16))
                    printf("\n"); 
            }    
            printf("\n");
        }    
*/        
        rxmsg_length = rxmsg_length - nTotLength;
        
        if (rxmsg_length > 0)
        {
            memcpy(web2_rx_msg, 
                        web2_rx_msg + nTotLength, 
                        sizeof(web2_rx_msg) - nTotLength); 
            continue;
        }
        else
            break;

    }

    return;	
}


#endif



