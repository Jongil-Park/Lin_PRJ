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
//

/*
int client_socket;
unsigned char tx_msg[32];
unsigned char rx_msg[32];	

time_t start_sec = 0;
time_t end_sec = 0
*/
/*****************************************************/
int 
main(int argc, char* argv[])
/*****************************************************/
{
	//Variables
	int client_socket;
	unsigned char tx_msg[32];
	struct sockaddr_in server_addr_in;
	struct timeval tv;
	char server_ip[15];
	int server_port;
	
	//Initialize
	client_socket = -1;
	memset(&server_addr_in, 0, sizeof(server_addr_in));
	tv.tv_sec = 0;
	tv.tv_usec = 100000; //0.1sec
	strcpy(server_ip, "127.0.0.1");
	server_port = 9004;


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
		printf("Connected to Interface Server : %s, Port : %d\n", server_ip, server_port);
	}
	//
	else
	{
		printf("Connecting to Interface Server Failed\n");
		close(client_socket);
		return -1;
	}

	if(send(client_socket, tx_msg, 9, 0) <= 0)
	{
		printf("[ERROR] Send in handleCmdPset()\n");
		return;
	}
	
	close(client_socket);
}


#if 0
/*****************************************************/
void 
handleCmdPset(int argc, char* argv[])
/*****************************************************/
{
	//Variables
	short pcm;
	short pno;
	float value;
	int recv_index;
	int recv_length;
	int retry_count;
	struct timespec ts;
	point_info point;
	
	//Initialize
	pcm = 0;
	pno = 0;
	value = 0;
	recv_index = 0;
	recv_length = 0;
	retry_count = 0;
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; //0.1sec
	memset(&point, 0, sizeof(point));
	//
	memset(tx_msg, 0, sizeof(tx_msg));
	memset(rx_msg, 0, sizeof(rx_msg));
	
	
	if(argc != 5)
	{
		printf("Incorrect arguments\n");
		printf("./cmd pset [pcm] [pno] [value]\n");
		return;
	}
	//
	pcm = atoi(argv[2]);
	pno = atoi(argv[3]);
	value = atoi(argv[4]);
	//
	//Check Validation of Arguments
    if(pcm < 0 || pcm >= MAX_NET32_NUMBER)
	{
		printf("Invalid PCM [%d] in handleCmdPset()\n", pcm);
		return;
	}
	//
	if(pno < 0 || pno >= MAX_POINT_NUMBER)
	{
		printf("Invalid PNO [%d] in handleCmdPset()\n", pno);
		return;
	}
	//
	//Make Send Message
	tx_msg[0] = COMMAND_PSET;
	memcpy(&tx_msg[1], &pcm, sizeof(pcm));
	memcpy(&tx_msg[3], &pno, sizeof(pno));
	memcpy(&tx_msg[5], &value, sizeof(value));
	//
	//Send Message
	if(send(client_socket, tx_msg, 9, 0) <= 0)
	{
		printf("[ERROR] Send in handleCmdPset()\n");
		return;
	}
	//
	//Get Message from Command Server
	while(1)
	{
		//printf("Waiting For response\n");
		while((recv_length =
			recv(client_socket, rx_msg + recv_index, sizeof(rx_msg) - recv_index, 0)) > 0)
		{
			recv_index = recv_index + recv_length;
		}
		//
		if(recv_index != 0)
			break;
		else
		{
			nanosleep(&ts, NULL);
			if(++retry_count > MAX_RETRY_COUNT)
			{
				printf("pset failed. something's wrong in Command Server\n");
				return;
			}
		}
	}
	//
	//Parse recv message
	while(getPointInfo(&point, &recv_index) == SUCCESS)
	{
		if(point.pcm == pcm && point.pno == pno && 
				point.message_type == COMMAND_PSET_SUCCESS)
		{
			printf("pSet Success, pcm [%d] pno [%d] value[%f]\n", pcm, pno, point.value);
			return;
		}
	}
	//
	return ;
}


/*****************************************************/
void 
handleCmdPget(int argc, char* argv[])
/*****************************************************/
{	
	//Variables
	int i;
	short pcm;
	short pno;
	int number;
	int recv_index;
	int recv_length;
	int retry_count;
	int count;
	struct timespec ts;
	point_info point;
	
	//Initialize
	i = 0;
	pcm = 0;
	pno = 0;
	number = 0;
	recv_index = 0;
	recv_length = 0;
	retry_count = 0;
	count = 0;
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; //0.1sec
	memset(&point, 0, sizeof(point));
	//
	memset(tx_msg, 0, sizeof(tx_msg));
	memset(rx_msg, 0, sizeof(rx_msg));
	
	
	if(argc != 5)
	{
		printf("Incorrect Arguments\n");
		printf("./cmd pget [pcm] [pno] [number]\n");
		return;
	}
	//
	pcm = atoi(argv[2]);
	pno = atoi(argv[3]);
	number = atoi(argv[4]);
	//
	//Check Validation of Arguments
    if(pcm < 0 || pcm >= MAX_NET32_NUMBER)
	{
		printf("Invalid PCM [%d] in handleCmdPget()\n", pcm);
		return;
	}
	//
	if(pno < 0 || pno >= MAX_POINT_NUMBER)
	{
		printf("Invalid PNO [%d] in handleCmdPget()\n", pno);
		return;
	}
	//
	if(number < 1 || (pno + number) > MAX_POINT_NUMBER)
	{
		printf("Invalid Number [%d] in handleCmdPget()\n", number);
		return;
	}
	//
	//Make Send Message
	tx_msg[0] = COMMAND_PGET;
	memcpy(&tx_msg[1], &pcm, sizeof(pcm));
	memcpy(&tx_msg[3], &pno, sizeof(pno));
	memcpy(&tx_msg[5], &number, sizeof(number));

	start_sec = time(NULL);

	//
	// Send Message
	// 
	if(send(client_socket, tx_msg, 9, 0) <= 0)
	{
		printf("[ERROR] Send() in handleCmdPget()\n");
		return;
	}
	//
	//Recv Message
	while(1)
	{
		while((recv_length =
			recv(client_socket, rx_msg + recv_index, sizeof(rx_msg) - recv_index, 0)) > 0)
		{
			recv_index = recv_index + recv_length;
		}
		//
		if(recv_index != 0)
			break;
		else
		{
			nanosleep(&ts, NULL);
			if(++retry_count > MAX_RETRY_COUNT)
			{
				printf("pset failed. something's wrong in Command Server\n");
				return;
			}
		}
	}

	end_sec = time(NULL);

	//
#if 0
    printf("Command Client RX msg : \n");
	for(i = 0; i < recv_index; i++)
	{
		printf("%02x ", rx_msg[i]);
	}
	printf("\n");
#endif
	//
	//Parsing Recved Message
	while(getPointInfo(&point, &recv_index) == SUCCESS)
	{
		if(point.pcm != pcm || point.pno != (pno + count) || 
				point.message_type != COMMAND_PGET_SUCCESS)
			break;
		//else
		//{
			printf("PCM [%d] PNO [%d] VALUE [%f]\n", 
				    pcm, pno + count, point.value);
		//}
		count++;
	}

	printf("start_sec %d, end_sec %d \n", start_sec, end_sec);

	//
	if(count == number)
	{
		printf("pget [%d] success\n", count);
		return;
	}
	//
	printf("pget [%d] failed\n", count);
	return;
}


/*****************************************************/
int
getPointInfo(point_info* point, int* recv_index)
/*****************************************************/
{
	if(*recv_index >= sizeof(point_info))
	{
		memcpy(point, rx_msg, sizeof(point_info));
		//
		*recv_index = *recv_index - sizeof(point_info);
		//
		memcpy(rx_msg, rx_msg + sizeof(point_info), *recv_index);
		//
		return SUCCESS;
	}
	else
		return FAIL;
}

#endif
