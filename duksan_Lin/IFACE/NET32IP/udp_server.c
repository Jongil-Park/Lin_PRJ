#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>


int    		udp_server_socket;
char   		message[512];                                    // message전송을 하기위한 버퍼
struct    	sockaddr_in udp_server_sockaddr;
struct 		sockaddr_in udp_client_sockaddr;


#define UCLIENT_CMD_ACK			'a'
#define UCLIENT_CMD_MSG			'm'

#define UCLIENT_CMD_GET			'G'
#define UCLIENT_CMD_SET			'S'

#define UCLIENT_DEST_UNKNOWN	0x0000

typedef struct {
	unsigned char stx;
	unsigned char length;
	unsigned char cmd;	
	unsigned char etx;	
} __attribute__ ((packed)) UCLIENT_GET;


typedef struct {
	unsigned char 	stx;
	unsigned char 	length;
	unsigned char 	cmd;	
	unsigned short  	addr;	
	unsigned short  	pno;	
	float  			value;	
	unsigned char 	etx;	
} __attribute__ ((packed)) UCLIENT_MSG;


typedef struct {
	unsigned short 	wSeq;
	unsigned char 	chLength;
	unsigned short  wSrc;	
	unsigned short  wDest;	
	unsigned char 	chFunc;
	unsigned short  wPcm;	
	unsigned short  wPno;	
	float  			fValue;	
	unsigned char 	chChksum;	
} __attribute__ ((packed)) NET32_UDP_MSG;


void init_udp_server_socket(void)
{
	int nSockOpt = 0;
	
	if ((udp_server_socket = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("socket create error\n");
		exit(-1);
	}

	udp_server_sockaddr.sin_family      = AF_INET;
	udp_server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	udp_server_sockaddr.sin_port        = htons(9998);

	nSockOpt = 1;

	// set broadcast
	if ( setsockopt(udp_server_socket, SOL_SOCKET, SO_BROADCAST, &nSockOpt, sizeof(nSockOpt)) != 0 )
		perror("set2 SO_BROADCAST failed");
	
	// prevent bind error
	if ( setsockopt(udp_server_socket, SOL_SOCKET, SO_REUSEADDR, &nSockOpt, sizeof(nSockOpt)) != 0 )
		perror("set SO_REUSEADDR failed");	
			
	// socket bind
	if (bind(udp_server_socket, (struct sockaddr *)&udp_server_sockaddr, sizeof(udp_server_sockaddr)) < 0) {
		printf("socket bind error\n");
		exit(-1);
	}	
}


int main(int argc, char *argv[]) 
{
	int       length, socket_size, index, number;
	fd_set    read_fds;
	
	init_udp_server_socket();
	
	socket_size = sizeof(struct sockaddr_in);
	
	while(1) {
		FD_ZERO(&read_fds);
		FD_SET(0, &read_fds);
		FD_SET(udp_server_socket, &read_fds);

		if (select(udp_server_socket+1, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)NULL) < 0) {
			printf("select open fail\n");
			exit(-1);
		}		
		
		// 클라이언트로부터 데이타가 도착했다....
		if (FD_ISSET(udp_server_socket, &read_fds)) {
			printf("receive message\n");
			
			memset(message, 0, 512);
			
			if ((length = recvfrom(udp_server_socket, (void *)message, 512, 0, (struct sockaddr *)&udp_client_sockaddr, &length)) < 0) {
				printf("클라이언트의 접속을 받아들일수가 없습니다\n");
				exit(-1);
			}
						
			printf("Received Message : %d\n", length);
			
			if ( sendto(udp_server_socket, message, length, 0, (struct sockaddr *)&udp_client_sockaddr, sizeof(udp_client_sockaddr)) == -1 ) {
				printf("Can't send to server\n");
			}		
		}		
	}
}




// UDP Server Test Code
#if 0
void init_udp_server_socket(void)
{
	int nSockOpt = 0;
	
	if ((udp_server_socket = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("socket create error\n");
		exit(-1);
	}

	udp_server_sockaddr.sin_family      = AF_INET;
	udp_server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	udp_server_sockaddr.sin_port        = htons(9998);

	nSockOpt = 1;

	// set broadcast
	if ( setsockopt(udp_server_socket, SOL_SOCKET, SO_BROADCAST, &nSockOpt, sizeof(nSockOpt)) != 0 )
		perror("set2 SO_BROADCAST failed");
	
	// prevent bind error
	if ( setsockopt(udp_server_socket, SOL_SOCKET, SO_REUSEADDR, &nSockOpt, sizeof(nSockOpt)) != 0 )
		perror("set SO_REUSEADDR failed");	
			
	// socket bind
	if (bind(udp_server_socket, (struct sockaddr *)&udp_server_sockaddr, sizeof(udp_server_sockaddr)) < 0) {
		printf("socket bind error\n");
		exit(-1);
	}	
}


int main(int argc, char *argv[]) 
{
	int       length, socket_size, index, number;
	fd_set    read_fds;
	
	init_udp_server_socket();
	
	socket_size = sizeof(struct sockaddr_in);
	
	while(1) {
		socket_size = sizeof(udp_client_sockaddr);
		memset(message, 0, 512);
		
		if ((length = recvfrom(udp_server_socket, (void *)message, 512, 0, (struct sockaddr *)&udp_client_sockaddr, &socket_size)) < 0) {
			printf("클라이언트의 접속을 받아들일수가 없습니다\n");
			exit(-1);
		}
		
		printf("\nReceived Message : %d\n", length);
		
		if ( sendto(udp_server_socket, "DEF", 3, 0, (struct sockaddr *)&udp_client_sockaddr, sizeof(udp_client_sockaddr)) == -1 ) {
			printf("Can't send to server\n");
		}		
	}
}
#endif


