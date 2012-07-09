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
#include "global.h"
#include "struct.h"
#include "cmd_server.h"

//extern pthread_mutex_t interfaceMessageQ_mutex;
//extern pthread_mutex_t pointTable_mutex;
//
//extern point_queue interface_message_queue;
//
//extern float point_table[MAX_NET32_NUMBER][MAX_POINT_NUMBER];
//
unsigned char cmd_tx_msg[MAX_BUFFER_SIZE];
unsigned char cmd_rx_msg[MAX_BUFFER_SIZE];
//
extern int debug_plc_tx;
extern int debug_plc_rx;
extern int debug_dnp_tx;
extern int debug_dnp_rx;

/*******************************************************************/
void* command_handler_main(void* arg)
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
	//
	fd = 0;
	sin_size = sizeof(client_addr);
	server_port = COMMAND_SERVER_PORT;
	status = INIT_PROCESS;
	
	
	//Ignore broken_pipe signal
	signal(SIGPIPE, SIG_IGN);
	//
	while(1)
	{
		switch(status)
		{
			case INIT_PROCESS:
			{
				server_socket = socket(PF_INET, SOCK_STREAM, 0);
				printf("Command Server Started, Port [%d]\n", server_port);
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
					printf("[ERROR] In Bind of Command Server\n");
					close(server_socket);
					status = INIT_PROCESS;
					sleep(10);
					break;
				}
				//
				if(listen(server_socket, 10) < 0)
				{
					printf("[ERROR] In Listen of Command Server\n");
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
			//
			case SELECT_PROCESS:
			{
				//
				temps = reads;
				//
				if(select(fd_max+1, &temps, 0, 0, &tv) == -1)
				{
					printf("[ERROR] In Select of Command Server\n");
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
			//
			case CONNECTION_REQUESTED:
			{
				client_socket = accept(server_socket, (struct sockaddr *)&client_addr,
						               &sin_size);
				//
				if (client_socket == -1)
				{
					printf("[Error] In Accept\n");
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
				do_handle_command(fd, &reads);
				status = SELECT_PROCESS;
				break;
			}
		}			
	}
	//
	return;
}

/************************************************************/
int
do_handle_command(int fd, fd_set* reads)
/************************************************************/
{
	//Variables
	int i;
	int rxmsg_length;
	int status;
	float fVal = 0;
	
	//Initialize
	i = 0;
	rxmsg_length = 0;
	//status = GET_COMMAND_TYPE;
	
	memset(cmd_rx_msg, 0, sizeof(cmd_rx_msg));

	rxmsg_length = recv(fd, cmd_rx_msg, sizeof(cmd_rx_msg), 0);
    //
#if 0
	printf("Command Handler RX msg : \n");
	for(i = 0; i < rxmsg_length; i++)
	{
		printf("%02x ", cmd_rx_msg[i]);
	}
	printf("\n");
#endif
	//
	if(rxmsg_length == 0)
	{
		FD_CLR(fd, reads);
		close(fd);
		//printf("Command client closed, file descriptor [%d]\n", fd);
		return SUCCESS;
	}

	if(cmd_rx_msg[0] == 0x08)
	{
		if(cmd_rx_msg[1] == 'P' && cmd_rx_msg[2] == 'L' && cmd_rx_msg[3] == 'C')
		{
			debug_plc_tx = cmd_rx_msg[6];
			debug_plc_rx = cmd_rx_msg[6];
		}
		else if(cmd_rx_msg[1] == 'D' && cmd_rx_msg[2] == 'N' && cmd_rx_msg[3] == 'P')
		{
			debug_dnp_tx = cmd_rx_msg[6];
			debug_dnp_rx = cmd_rx_msg[6];
		}
		else
		{
			FD_CLR(fd, reads);
			close(fd);
			return ERROR;
		}
	}
	else
	{
		if(cmd_rx_msg[1] == 'D' && cmd_rx_msg[2] == 'S')
		{
			DnpSet_Proc(cmd_rx_msg[3],(cmd_rx_msg[4] * 0x100) + cmd_rx_msg[5], cmd_rx_msg[6]);
		}
		else if(cmd_rx_msg[1] == 'D' && cmd_rx_msg[2] == 'I')
		{
			while(1)
			{
				if(rxmsg_length == 0)
					break;
				else if (rxmsg_length > 0)
				{
					if(DnpInfo_Proc(fd, cmd_rx_msg[4], (cmd_rx_msg[5] * 0x100) + cmd_rx_msg[6]) == ERROR)
						break;
					rxmsg_length = -1;
				}
				else
				{
					rxmsg_length = recv(fd, cmd_rx_msg, sizeof(cmd_rx_msg), 0);
					
					#if 0
					printf("Command Handler RX msg : \n");
					for(i = 0; i < rxmsg_length; i++)
					{
						printf("%02x ", cmd_rx_msg[i]);
					}
					printf("\n");
					#endif
				}
			}
		}
		else
		{
			FD_CLR(fd, reads);
			close(fd);
			return ERROR;
		}
	}
	//
	FD_CLR(fd, reads);
	close(fd);
	return SUCCESS;
}

int DnpSet_Proc(int nType, int nPno, int nValue)
{
	float fVal = 0;
	DNP_DIO_Point *dio_p;
	DNP_AIO_Point *aio_p;

	if (nType == 2)		// DO
	{
		dio_p = &DNP_do_point[nPno];
		printf("DnpSet_Proc :: DO  pno %d, dio_val = %d \n", nPno, nValue);
		if (nValue)
		{
            dio_p->WrFlag = 1;
            dio_p->WrVal = 1;
            dio_p->PrevVal = 0;
		}
		else
		{
            dio_p->WrFlag = 1;
            dio_p->WrVal = 1;
            dio_p->PrevVal = 0;
		}
		return SUCCESS;
	}
	else if (nType == 4)	// AO
	{
		fVal = nValue;
		aio_p = &DNP_ao_point[nPno];
		printf("DnpSet_Proc :: AO pno %d, aio_val = %f \n", nPno, fVal);

		if(fVal > aio_p->MaxVal)
            fVal = aio_p->MaxVal;
        else if(fVal < aio_p->MinVal)
            fVal = aio_p->MinVal;
        
        aio_p->PrevVal = fVal;
        printf("DnpSet_Proc :: ao_point_val = %10.3f \n", aio_p->PrevVal);
        aio_p->WrFlag = 1;

		return SUCCESS;
	}
	else						// Wrong..
	{
		return ERROR;
	}
}

int DnpInfo_Proc(int fd, int nType, int nPno)
{
	int i = 0;
	int j = 0;
	float fVal = 0;
	unsigned char chVal = 0;
	DNP_DIO_Point *dio_p;
	DNP_AIO_Point *aio_p;
	unsigned char chChkSum = 0;
	unsigned char chPnoHi = 0x00;
	unsigned char chPnoLo = 0x00;

	if (nPno > 0xff)
	{
		chPnoHi = (unsigned char)(nPno >> 8);
		chPnoLo = (unsigned char)(nPno -(chPnoHi * 0x100));
	}
	else
	{
		chPnoLo = (unsigned char)nPno;
	}

	//printf("nPno = %d, chPnoHi = %d, chPnoLo = %d\n", nPno, chPnoHi, chPnoLo);

	if (nType == 1)		// DI
	{
		dio_p = &DNP_di_point[nPno];
		chVal = dio_p->val;
		//printf("%s :: DI  pno %d, dio_val = %d \n", __FUNCTION__, nPno, chVal);

		cmd_tx_msg[i++] = sizeof(DNP_DIO_Point) + 5; //Length
		cmd_tx_msg[i++] = nType;
		cmd_tx_msg[i++] = chPnoHi;
		cmd_tx_msg[i++] = chPnoLo;
		memcpy(&cmd_tx_msg[i],  &DNP_di_point[nPno], sizeof(DNP_DIO_Point));

		for (j = 0; j < sizeof(DNP_DIO_Point) + 4; j++)
			chChkSum += cmd_tx_msg[j];
		cmd_tx_msg[sizeof(DNP_DIO_Point) + 4] = chChkSum;
	}
	else if (nType == 2)		// DO
	{
		dio_p = &DNP_do_point[nPno];
		chVal = dio_p->val;
		//printf("%s :: DO  pno %d, dio_val = %d \n", __FUNCTION__, nPno, chVal);

		cmd_tx_msg[i++] = sizeof(DNP_DIO_Point) + 5; //Length
		cmd_tx_msg[i++] = nType;
		cmd_tx_msg[i++] = chPnoHi;
		cmd_tx_msg[i++] = chPnoLo;
		memcpy(&cmd_tx_msg[i],  &DNP_do_point[nPno], sizeof(DNP_DIO_Point));

		for (j = 0; j < sizeof(DNP_DIO_Point) + 4; j++)
			chChkSum += cmd_tx_msg[j];
		cmd_tx_msg[sizeof(DNP_DIO_Point) + 4] = chChkSum;
	}
	else if (nType == 3)	// AI
	{
		aio_p = &DNP_ai_point[nPno];
		fVal = aio_p->val;
		//printf("%s :: AI  pno %d, aio_p = %f \n", __FUNCTION__, nPno, fVal);
		
		cmd_tx_msg[i++] = sizeof(DNP_AIO_Point) + 5; //Length
		cmd_tx_msg[i++] = nType;
		cmd_tx_msg[i++] = chPnoHi;
		cmd_tx_msg[i++] = chPnoLo;
		memcpy(&cmd_tx_msg[i],  &DNP_ai_point[nPno], sizeof(DNP_AIO_Point));

		for (j = 0; j < sizeof(DNP_AIO_Point) + 4; j++)
			chChkSum += cmd_tx_msg[j];
		cmd_tx_msg[sizeof(DNP_AIO_Point) + 4] = chChkSum;
	}
	else if (nType == 4)	// AO
	{
		aio_p = &DNP_ao_point[nPno];
		fVal = aio_p->val;
		//printf("%s :: AO  pno %d, aio_p = %f \n", __FUNCTION__, nPno, fVal);

		cmd_tx_msg[i++] = sizeof(DNP_AIO_Point) + 5; //Length
		cmd_tx_msg[i++] = nType;
		cmd_tx_msg[i++] = chPnoHi;
		cmd_tx_msg[i++] = chPnoLo;
		memcpy(&cmd_tx_msg[i],  &DNP_ao_point[nPno], sizeof(DNP_AIO_Point));

		for (j = 0; j < sizeof(DNP_AIO_Point) + 4; j++)
			chChkSum += cmd_tx_msg[j];
		cmd_tx_msg[sizeof(DNP_AIO_Point) + 4] = chChkSum;
	}

#if 0
	printf("Tx :: ");
	for(j =0; j < cmd_tx_msg[0]; j++)
	{
		printf("%x ", cmd_tx_msg[j]);		
	}
	printf("\n");
#endif

	//Send Message
	if(send(fd, cmd_tx_msg, cmd_tx_msg[0], 0) <= 0)
	{
		printf("[ERROR] Send in %s\n", __FUNCTION__);
		return ERROR;
	}
	else
		return SUCCESS;
}







