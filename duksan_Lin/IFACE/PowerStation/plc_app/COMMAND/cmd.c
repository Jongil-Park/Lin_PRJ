
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
//
#include "global.h"
#include "struct.h"
#include "cmd.h"
//
int client_socket;
unsigned char tx_msg[MAX_BUFFER];
unsigned char rx_msg[MAX_BUFFER];	
//
/*****************************************************/
int  main(int argc, char* argv[])
/*****************************************************/
{
	//Variables
	struct sockaddr_in server_addr_in;
	struct timeval tv;
	char server_ip[15];
	int server_port;
	int nDnpType = 0;
	int nDnpPno = 0;
	int nDnpPno1 = 0;
	int nDnpPno2 = 0;
	int nDnpValue = 0;

	//Initialize
	client_socket = -1;
	memset(&server_addr_in, 0, sizeof(server_addr_in));
	tv.tv_sec = 0;
	tv.tv_usec = 100000; //0.1sec
	strcpy(server_ip, "127.0.0.1");
	server_port = COMMAND_SERVER_PORT;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
	//
	setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	//
	server_addr_in.sin_family = AF_INET;
	server_addr_in.sin_addr.s_addr = inet_addr(server_ip);
	server_addr_in.sin_port = htons(server_port);
	//
	//
	if(connect(client_socket, (struct sockaddr *) &server_addr_in,
				sizeof(server_addr_in)) >= 0)
	{
		//printf("Connected to Interface Server : %s, Port : %d\n", server_ip, server_port);
	}
	//
	else
	{
		printf("Connecting to Interface Server Failed\n");
		close(client_socket);
		return ERROR;
	}	
	//
	if(argc == 1)
	{
		printf("No argument. Use following Syntax\n");
		printf("./cmd dnpset [type] [pno] [value]\n");
		printf("./cmd dnpinfo [type] [pno1] [pno2]\n");
		printf("./cmd plc [on / off]\n");
		printf("./cmd dnp [on / off]\n");
		
		close(client_socket);
		return ERROR;
	}
	//
	if (strncmp(argv[1], "plc", 3) == 0 && argc == 3)
	{
		if(strncmp(argv[2], "on", 2) == 0)
		{
			Msg_handler(CMD_PLC_MSG, 1);
		}
		else if (strncmp(argv[2], "off", 3) == 0)
		{
			Msg_handler(CMD_PLC_MSG, 0);
		}
		else
		{
			close(client_socket);
			return ERROR;
		}
	}
	//
	if (strncmp(argv[1], "dnp", 3) == 0 && argc == 3)
	{
		if(strncmp(argv[2], "on", 2) == 0)
		{
			Msg_handler(CMD_DNP_MSG, 1);
		}
		else if (strncmp(argv[2], "off", 3) == 0)
		{
			Msg_handler(CMD_DNP_MSG, 0);
		}
		else
		{
			close(client_socket);
			return ERROR;
		}
	}
	//
	if (strncmp(argv[1], "dnpset", 6) == 0 && argc == 5)
	{
		nDnpType = atoi(argv[2]);
		nDnpPno = atoi(argv[3]);
		nDnpValue = atoi(argv[4]);
		
		if (nDnpType == 0 || nDnpType == 1 || nDnpType == 3 || nDnpType > 4)
		{
			printf("DnpSet Type Error %d\n", nDnpType);
			printf("DnpSet Type ==> DI = 1, DO = 2, AI = 3, AO = 4\n");
			return ERROR;
		}
		printf("DnpSet %d, %d, %d\n", nDnpType, nDnpPno, nDnpValue);
		
		DnpSet_handler(nDnpType, nDnpPno, nDnpValue);
		
		close(client_socket);
		return SUCCESS;
	}
	
	if (strncmp(argv[1], "dnpinfo", 7) == 0 && argc == 5)
	{
		nDnpType = atoi(argv[2]);
		nDnpPno1 = atoi(argv[3]);
		nDnpPno2 = atoi(argv[4]);
		printf("DnpInfo %d, %d, %d\n", nDnpType, nDnpPno1, nDnpPno2);
		
		for (nDnpPno = nDnpPno1; nDnpPno <= nDnpPno2; nDnpPno++)
		{
			DnpInfo_handler(nDnpType, nDnpPno);
			//printf("nDnpPno = %d\n", nDnpPno);
		}
		
		close(client_socket);
		return SUCCESS;
	}

	close(client_socket);
	return SUCCESS;
}

void Msg_handler(int nType, int nValue)
{
	//Variables
	int i = 0;
	int j = 0;
	unsigned char chChkSum = 0;
	int recv_index = 0;
	int retry_count = 0;
	int recv_length = 0; 
	struct timespec ts;

	//Initialize
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; //0.1sec

	tx_msg[i++] = 0x08; //Length
	if (nType == CMD_PLC_MSG)
	{
		tx_msg[i++] = 'P';
		tx_msg[i++] = 'L';
		tx_msg[i++] = 'C';
	}
	else if (nType == CMD_DNP_MSG)
	{
		tx_msg[i++] = 'D';
		tx_msg[i++] = 'N';
		tx_msg[i++] = 'P';
	}
	else
	{
		tx_msg[i++] = 0x00;
		tx_msg[i++] = 0x00;
		tx_msg[i++] = 0x00;
	}
	tx_msg[i++] = 0xfe;
	tx_msg[i++] = nType;
	tx_msg[i++] = nValue;

	for (j = 0; j < i; j++)
		chChkSum += tx_msg[j];
	tx_msg[i++] = chChkSum;

	//Send Message
	if(send(client_socket, tx_msg, i, 0) <= 0)
	{
		printf("[ERROR] Send in handleCmdPset()\n");
		return;
	}

	while(1)
	{
		recv_length = recv(client_socket, rx_msg, sizeof(rx_msg), 0);
		
		if (recv_length > 0)
			break;		
		else if(recv_length == 0)
			break;
		else
		{
			nanosleep(&ts, NULL);
			if(++retry_count > MAX_RETRY_COUNT)
			{
				printf("%s recv failed. something's wrong in Command Server\n", __FUNCTION__);
				return;
			}
		}
	}
}


void DnpSet_handler(int nType, int nPno, int nValue)
{
	//Variables
	int i = 0;
	int j = 0;
	unsigned char chChkSum = 0;
	int recv_index = 0;
	int retry_count = 0;
	int recv_length = 0; 
	struct timespec ts;
	unsigned char chPnoHi = 0x00;
	unsigned char chPnoLo = 0x00;

	if (nPno > 0xff)
	{
		chPnoHi = (unsigned char)(nPno >> 8);
		chPnoLo = (unsigned char)(nPno - (chPnoHi * 0x100));
	}
	else
	{
		chPnoLo = nPno;
	}

	//Initialize
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; //0.1sec

	tx_msg[i++] = 0x0C; //Length
	tx_msg[i++] = 'D';
	tx_msg[i++] = 'S';
	//tx_msg[i++] = 0xfe;
	tx_msg[i++] = nType;
	tx_msg[i++] = chPnoHi;
	tx_msg[i++] = chPnoLo;
	tx_msg[i++] = nValue;
	tx_msg[i++] = 0x00;
	tx_msg[i++] = 0x00;
	tx_msg[i++] = 0x00;
	tx_msg[i++] = 0x00;
	for (j = 0; j < i; j++)
		chChkSum += tx_msg[j];
	tx_msg[i++] = chChkSum;

	//Send Message
	if(send(client_socket, tx_msg, i, 0) <= 0)
	{
		printf("[ERROR] Send in %s\n", __FUNCTION__);
		return;
	}

	while(1)
	{
		recv_length = recv(client_socket, rx_msg, sizeof(rx_msg), 0);
		
		if (recv_length > 0)
			break;		
		else if(recv_length == 0)
			break;
		else
		{
			nanosleep(&ts, NULL);
			if(++retry_count > MAX_RETRY_COUNT)
			{
				printf("%s recv failed. something's wrong in Command Server\n", __FUNCTION__);
				return;
			}
		}
	}
/*
	printf("Recv(%d) :: ", recv_length);
	for (i = 0; i < recv_length; i++)
		printf("%x ", rx_msg[i]);
	printf("\n");
*/
}

void DnpInfo_handler(int nType, int nPno)
{
	//Variables
	int i = 0;
	int j = 0;
	unsigned char chChkSum = 0;
	int recv_index = 0;
	int retry_count = 0;
	int recv_length = 0; 
	struct timespec ts;
	unsigned char chPnoHi = 0x00;
	unsigned char chPnoLo = 0x00;

	if (nPno > 0xff)
	{
		chPnoHi = (unsigned char)(nPno >> 8);
		chPnoLo = (unsigned char)((nPno - (chPnoHi * 0x100)));
	}
	else
	{
		chPnoLo = nPno;
	}

	//printf("chPnoHi = %d, chPhoLo = %d\n", chPnoHi, chPnoLo);

	//Initialize
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; //0.1sec

	tx_msg[i++] = 0x0C; //Length
	tx_msg[i++] = 'D';
	tx_msg[i++] = 'I';
	tx_msg[i++] = 0xfe;
	tx_msg[i++] = nType;
	tx_msg[i++] = chPnoHi;
	tx_msg[i++] = chPnoLo;
	tx_msg[i++] = 0x00;
	tx_msg[i++] = 0x00;
	tx_msg[i++] = 0x00;
	tx_msg[i++] = 0x00;
	for (j = 0; j < i; j++)
		chChkSum += tx_msg[j];
	tx_msg[i++] = chChkSum;

#if 0
	printf("Tx (%d) :: ", i);
	for (j = 0; j < i; j++)
		printf("%x ", tx_msg[j]);
	printf("\n");
#endif

	//Send Message
	if(send(client_socket, tx_msg, i, 0) <= 0)
	{
		printf("[ERROR] Send in %s\n", __FUNCTION__);
		return;
	}

	while(1)
	{
		recv_length = recv(client_socket, rx_msg, sizeof(rx_msg), 0);
		
		if (recv_length > 0)
			break;		
		else if(recv_length == 0)
			break;
		else
		{
			nanosleep(&ts, NULL);
			if(++retry_count > MAX_RETRY_COUNT)
			{
				printf("%s recv failed. something's wrong in Command Server\n", __FUNCTION__);
				return;
			}
		}
	}

#if 0
	printf("Recv(%d) :: ", recv_length);
	for (i = 0; i < recv_length; i++)
		printf("%x ", rx_msg[i]);
	printf("\n");
#endif

	if (recv_length == 0x25 || recv_length == 0x15)
		DnpInfo_Parse(rx_msg);
}


void DnpInfo_Parse(unsigned char *pData)
{
	int nType = 0;
	int nPno = 0;
	DNP_DIO_Point dio_p;
	DNP_AIO_Point aio_p;

	nType = pData[1];
	nPno =  (pData[2] * 0x100) + pData[3];

	//printf("pData[2] = %d, pData[3] = %dd \n", pData[2], pData[3]);

	switch(nType)
	{
		case 1:
			memcpy(&dio_p, &pData[4], sizeof(DNP_DIO_Point));
            printf("DI PNO : [%d]    VAL : [%d]    sADDR : [%d]    ONADDR : [%d]   OFFADDR : [%d] \n",
                    nPno, dio_p.val, dio_p.PLCaddr, dio_p.ONaddr, dio_p.OFFaddr);
			break;

		case 2:
			memcpy(&dio_p, &pData[4], sizeof(DNP_DIO_Point));
            printf("DO PNO : [%d]    VAL : [%d]    sADDR : [%d]    ONADDR : [%d]   OFFADDR : [%d] \n",
                    nPno, dio_p.val, dio_p.PLCaddr, dio_p.ONaddr, dio_p.OFFaddr);
			break;

		case 3:
			memcpy(&aio_p, &pData[4], sizeof(DNP_AIO_Point));
            printf("AI PNO : [%d]    VAL : [%10.3f]    sADDR : [%d]    SCALE : [%10.3f]    MIN : [%d]    MAX : [%d] \n",
                    nPno, aio_p.val, aio_p.PLCaddr, aio_p.scale, aio_p.MinVal, aio_p.MaxVal);
			break;

		case 4:
			memcpy(&aio_p, &pData[4], sizeof(DNP_AIO_Point));
            printf("AO PNO : [%d]    VAL : [%10.3f]    sADDR : [%d]    SCALE : [%10.3f]    MIN : [%d]    MAX : [%d] \n",
                    nPno, aio_p.val, aio_p.PLCaddr, aio_p.scale, aio_p.MinVal, aio_p.MaxVal);
			break;

		default:
			break;
	}

	return;
}


