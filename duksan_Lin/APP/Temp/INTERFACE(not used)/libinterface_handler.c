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
//
#include "function.h"
//
#include "define.h"
#include "queue.h"
#include "point_manager.h"
#include "libinterface_handler.h"

extern pthread_mutex_t interfaceMessageQ_mutex;
extern pthread_mutex_t pointTable_mutex;
//
extern point_queue interface_message_queue;
//
extern float point_table[MAX_NET32_NUMBER][MAX_POINT_NUMBER];
//
static unsigned char iface_tx_msg[MAX_BUFFER_SIZE];
static unsigned char iface_rx_msg[MAX_BUFFER_SIZE];
static unsigned char iface_message[MAX_BUFFER_SIZE];
int iface_msg_ptr = 0;

static int libinterface_dbg = 0;

static void DoWriteCommand(int fd, unsigned char *pData);
static void DoReadCommand(int fd, unsigned char *pData);
static unsigned short MakeChksum (unsigned char *p, int length);
static int DoLibInterfaceHandler(int fd, fd_set* reads);
void *iface_handler_main(void* arg);

extern void put_bacnet_message_queue(int pcm, int pno, float value);
/*******************************************************************/
void *iface_handler_main(void* arg)
/*******************************************************************/
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
	memset(iface_rx_msg, 0x00, sizeof(iface_rx_msg));
	memset(iface_tx_msg, 0x00, sizeof(iface_tx_msg));
	//
	fd = 0;
	sin_size = sizeof(client_addr);
	server_port = IFACE_SERVER_PORT;
	status = INIT_PROCESS;
	
	//Ignore broken_pipe signal
	signal(SIGPIPE, SIG_IGN);
	sleep(1); 
	
	// Super Loop
	while(1)
	{
		// State machine
		switch(status)
		{
			case INIT_PROCESS:
			{
				server_socket = socket(PF_INET, SOCK_STREAM, 0);
				printf("Interface Server Started, Port [%d]\n", server_port);
				//
				server_addr.sin_family = AF_INET;
				server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
				server_addr.sin_port = htons(server_port);
				//
				setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
				//
				if(bind(server_socket, (struct sockaddr *)&server_addr,
							sizeof(server_addr)) < 0)
				{
					printf("[ERROR] In Bind of Interface Server\n");
					close(server_socket);
					status = INIT_PROCESS;
					sleep(10);
					break;
				}
				//
				if(listen(server_socket, 10) < 0)
				{
					printf("[ERROR] In Listen of Interface Server\n");
					close(server_socket);
					status = INIT_PROCESS;
					sleep(10);
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
			
			case SELECT_PROCESS:
			{
				//
				temps = reads;
				//
				if(select(fd_max+1, &temps, 0, 0, &tv) == -1)
				{
					printf("[ERROR] In Select of Interface Server\n");
					status = INIT_PROCESS;
					sleep(10);
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
			
			case CONNECTION_REQUESTED:
			{
				client_socket = accept(server_socket, (struct sockaddr *)&client_addr,
						               &sin_size);
				//
				if (client_socket == -1)
				{
					printf("[Error] In Accept of Interface Server\n");
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
			
			case HANDLE_COMMAND:
			{
				DoLibInterfaceHandler(fd, &reads);
				status = SELECT_PROCESS;
				break;
			}
		}			
	}

}

static int DoLibInterfaceHandler(int fd, fd_set* reads)
{
	//Variables
	int i = 0;
	int recv_length = 0;
	_RES_GET_LENGTH_PACKET *p;	
	
	//Initialize
	//status = GET_COMMAND_TYPE;
	memset(iface_rx_msg, 0, sizeof(iface_rx_msg));

	// get rx-data from client.
	recv_length = recv(fd, iface_rx_msg, sizeof(iface_rx_msg), 0);
    
    //if (libinterface_dbg)
    //{
		printf("Interface Handler RX msg : \n");
		for(i = 0; i < recv_length; i++)
		{
			printf("%02x ", iface_rx_msg[i]);
		}
		printf("\n");
	//}

	// empty rx-data. socket close.
	if(recv_length == 0)
	{
		FD_CLR(fd, reads);
		close(fd);
		return SUCCESS;
	}

	memcpy(&iface_message[iface_msg_ptr], iface_rx_msg, recv_length);
	iface_msg_ptr += recv_length;

	for (;;)
	{
		p = (_RES_GET_LENGTH_PACKET *)iface_message;
		
		if (p->stx == LIB_CMD_STX && iface_msg_ptr >= p->length)
		{		
			switch(p->cmd)
			{
				case LIB_CMD_READ:
					//if (libinterface_dbg) printf("Read message\n");
					DoReadCommand(fd, iface_rx_msg);
					iface_msg_ptr = iface_msg_ptr - p->length;					
					break;
					
				case LIB_CMD_WRITE:
					//if (libinterface_dbg) printf("Write message\n");
					printf("Write message\n");
					DoWriteCommand(fd, iface_rx_msg);
					iface_msg_ptr = iface_msg_ptr - p->length;
					break;
					
				default:
					printf("Message Error\n");
					if(iface_msg_ptr > 0)	
						iface_msg_ptr--;		
					else					
						iface_msg_ptr = 0;
					break;
			}
		}
		else
		{
			if(iface_msg_ptr > 0)	
				iface_msg_ptr--;		
			else					
				iface_msg_ptr = 0;
		}
		
		if(iface_msg_ptr == 0)
			return SUCCESS;
	}
////////////////////////////////////////////////////////////////////



	// get data-length from rx-data
	p = (_RES_GET_LENGTH_PACKET *)iface_rx_msg;
	if (p->stx == LIB_CMD_STX)
	{
		// compare length.
		if (p->length == recv_length)
		{	
			switch(p->cmd)
			{
				case LIB_CMD_READ:
					//if (libinterface_dbg) printf("Read message\n");
					DoReadCommand(fd, iface_rx_msg);
					break;
					
				case LIB_CMD_WRITE:
					//if (libinterface_dbg) printf("Write message\n");
					printf("Write message\n");
					DoWriteCommand(fd, iface_rx_msg);
					break;
					
				default:
					printf("Message Error\n");
					return ERROR;
			}
			return SUCCESS;
		}
	}
	return ERROR;
}

static unsigned short MakeChksum (unsigned char *p, int length)
{
	int i = 0;
	unsigned short chksum = 0;

	for (i = 1; i < length; i++)
	{
		chksum += *(p + i);
	}

	return chksum;
}


static void DoReadCommand(int fd, unsigned char *pData)
{
	int i = 0;
	unsigned short chksum = 0;		// chksum 
	unsigned int value_size = 0;	// read value size. (float * 128)
	_REQ_READ_POINT_PACKET *pRx;	// Rx Struct Pointer
	_RES_READ_POINT_PACKET *pTx;	// Tx Struct Pointer
	
	// initialize
	memset(iface_tx_msg, 0x00, sizeof(iface_tx_msg));
	value_size = sizeof(float) * 128;
	pRx = (_REQ_READ_POINT_PACKET *)pData;
	pTx = (_RES_READ_POINT_PACKET *)iface_tx_msg;
	
	// get chksum from rx-data.
	chksum = MakeChksum(pData, sizeof(_REQ_READ_POINT_PACKET) - 3);
	
	// compare chksum.
	if (chksum == pRx->chksum)
	{
		if (libinterface_dbg) printf("Read message, Rxpcm = %d\n", pRx->pcm);
		
		// Make tx-data.
		pTx->stx = LIB_CMD_STX;
		pTx->length = sizeof(_RES_READ_POINT_PACKET);
	    pTx->cmd = LIB_CMD_READ;
		pTx->pcm = pRx->pcm;	
		pTx->part = pRx->part;	
		
		if (pRx->part == 1)
			memcpy(&pTx->val[0], &point_table[pRx->pcm][128], value_size);
		else
			memcpy(&pTx->val[0], &point_table[pRx->pcm][0], value_size);
		
		pTx->chksum = MakeChksum((unsigned char *)pTx, pTx->length - 3);
		pTx->etx = LIB_CMD_ETX;	

		if (libinterface_dbg)
		{
			printf("TX = ");
			for(i = 0; i < pTx->length; i++)
				printf(" %x", iface_tx_msg[i]);
			printf("\n");
		}
		
		// server send to client tx-data 
		send(fd, iface_tx_msg, pTx->length, 0);
	}
	else
	{
		printf ("Read Message Error chksum = %x, pRx->chksum = %x\n", chksum, pRx->chksum);		
	}
}

static void DoWriteCommand(int fd, unsigned char *pData)
{
	int i = 0;
	unsigned short chksum = 0;		// chksum 
	unsigned int value_size = 0;	// read value size. (float * 128)
	_REQ_WRITE_POINT_PACKET *pRx;	// Rx Struct Pointer
	_RES_WRITE_POINT_PACKET *pTx;	// Tx Struct Pointer
	
	// initialize
	memset(iface_tx_msg, 0x00, sizeof(iface_tx_msg));
	value_size = sizeof(float) * 128;
	pRx = (_REQ_WRITE_POINT_PACKET *)pData;
	pTx = (_RES_WRITE_POINT_PACKET *)iface_tx_msg;
	
	// get chksum from rx-data.
	chksum = MakeChksum(pData, sizeof(_REQ_WRITE_POINT_PACKET) - 3);
	
	// compare chksum.
	if (chksum == pRx->chksum)
	{
		if (libinterface_dbg) 
			printf("Write message, Rxpcm = %d, RxPno = %d, RxVal = %f\n", 
				pRx->pcm, pRx->pno, pRx->val);
		
		// Make tx-data. (tx-data same rx-data)
		memcpy ((unsigned char *)pTx, (unsigned char *)pRx, sizeof(_RES_WRITE_POINT_PACKET));
		
		if (libinterface_dbg)
		{
			printf("TX = ");
			for(i = 0; i < pTx->length; i++)
				printf(" %x", iface_tx_msg[i]);
			printf("\n");
		}	
		
		// server send to client tx-data 
		send(fd, iface_tx_msg, pTx->length, 0);
		
		// do pset
		if (point_table[pRx->pcm][pRx->pno] != pRx->val)
		{
			put_bacnet_message_queue(pRx->pcm, pRx->pno, pRx->val);
			pSet(pRx->pcm, pRx->pno, pRx->val);
		}
	}
	else
	{
		printf ("Write Message Error chksum = %x, pRx->chksum = %x\n", chksum, pRx->chksum);		
	}	
}



