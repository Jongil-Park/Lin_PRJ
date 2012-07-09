///******************************************************************/
// file : ghp_WebSvr.h
// date : 2010.03.03.
// author : jong2ry
///******************************************************************/
#ifndef		IFACE_GHP_WEB_H_
#define		IFACE_GHP_WEB_H_

#define GHPWEB_GET_DATA             0x0A
#define GHPWEB_GET_CNT_DATA         0x020B          // 2010.01.12 add.
#define GHPWEB_SET_DATA             0x0E
#define TCP_MSG_GET                 0x09
#define TCP_MSG_SET                 0x0D

#define ADDR_COMMAND            	0
#define ADDR_DATACNT            	2

#define MSG_GET_LENGTH_OLD      	12
#define MSG_SET_LENGTH_OLD      	12
#define PACKET_SIZE_OLD         	2

#define SIZE_WEB_CMD            	2
#define SIZE_WEB_CNT            	2
#define SIZE_WEB_CHKSUM         	2
#define SIZE_HEADER             	4

#define MSG_ON                  	1
#define MSG_OFF                 	0

#define GHPWEB_SERVER_PORT      	4300        //TCP Server Port
#define GHPWEB2_SERVER_PORT      	4400        //TCP2 Server Port

//static void convert_float_to_4byte(unsigned char *pData, unsigned char *pTxMsg);
//static void convert_4byte_to_float(unsigned char *pData, unsigned char *pRxMsg);
//static char CalculateCheckSum(char *chBuf, int nLength);
//static void swap_short(unsigned short *p);
//static unsigned short RxData_to_Command(unsigned char *pData);
//static unsigned short RxData_to_Count(unsigned char *pData);
//static int Calculate_TotLength(int nCnt, int nCmd);
//static int Make_TxMsg_GetCntMode(unsigned char *pRcvData, unsigned char *pData, int nCnt);
//static int Make_TxMsg_GetMode(unsigned char *pRcvData, unsigned char *pData, int nCnt);
//static int Make_TxMsg_GetMode_Old(unsigned char *pRcvData, unsigned char *pData, int nCnt);
//static int Make_TxMsg_SetMode(unsigned char *pRcvData, unsigned char *pData, int nCnt);
//static int Make_TxMsg_SetMode_Old(unsigned char *pRcvData, unsigned char *pData, int nCnt);
void *webserver_main(void);
void web_handler(int fd, fd_set *reads);


void *web2server_main(void);
void web2_handler(int fd, fd_set *reads);
//static char Web2_CalculateCheckSum(char *chBuf, int nLength);
#endif

