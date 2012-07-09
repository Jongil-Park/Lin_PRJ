#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define BUFFSIZE 32
void Die(char *mess) { perror(mess); exit(1); }
          
int    udp_client_socket;
struct sockaddr_in udp_client_sockaddr, udp_client_sockaddr;                               // 채팅 서버와 통신하기 위한 소켓 구조체...

int    ds_client_socket;
struct sockaddr_in ds_client, client;                               // 채팅 서버와 통신하기 위한 소켓 구조체...
int 	g_nIndex = 0;
int 	g_nPointCount = 0;

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


typedef struct 
{
	int nPcm;
	int nPno;
} NET32_POINT;
static NET32_POINT g_sPointList[8192];




void init_udp_socket(void)
{
	int nSockOpt = 1; 
	
	udp_client_socket = socket(PF_INET, SOCK_DGRAM, 0);
	if (udp_client_socket < 0) {
		printf("Can't create a client socket\n");
		exit(-1);
	}

	udp_client_sockaddr.sin_family      = PF_INET;
	udp_client_sockaddr.sin_addr.s_addr = INADDR_BROADCAST;
	udp_client_sockaddr.sin_port        = htons(9998);
	
	// prevent bind error
	if ( setsockopt(udp_client_socket, SOL_SOCKET, SO_BROADCAST, &nSockOpt, sizeof(nSockOpt)) != 0 )
		perror("set2 SO_BROADCAST failed");
	
	if ( setsockopt(udp_client_socket, SOL_SOCKET, SO_REUSEADDR, &nSockOpt, sizeof(nSockOpt)) != 0 )
		perror("set SO_REUSEADDR failed");	
}


void init_ds_socket(void)
{
	/* Create the TCP socket */
	if ((ds_client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		Die("Failed to create socket");
	}

	/* Construct the server sockaddr_in structure */
	memset(&ds_client, 0, sizeof(ds_client));       /* Clear struct */
	ds_client.sin_family = AF_INET;                  /* Internet/IP */
	ds_client.sin_addr.s_addr = inet_addr("127.0.0.1");  /* IP address */
	ds_client.sin_port = htons(9991);       /* server port */
	
	/* Establish connection */
	if (connect(ds_client_socket, (struct sockaddr *) &ds_client, sizeof(ds_client)) < 0) {
		Die("Failed to connect with server");
	}
}

int get_pointindex(void) 
{
	if ( g_nIndex < g_nPointCount ) 
		g_nIndex++;

	if ( g_nIndex >= g_nPointCount ) 	
		g_nIndex = 0;

	return g_nIndex;
}



int get_message(unsigned short wSeq, unsigned char *pBuf) 
{
	int i = 0;
	int nPointIndex = 0;
	NET32_UDP_MSG *pMsg;	
	
	pMsg = (NET32_UDP_MSG *)pBuf;
	
	nPointIndex = get_pointindex();

	pMsg->wSeq = wSeq;
	pMsg->chLength = sizeof(NET32_UDP_MSG);
	pMsg->wSrc = 0x0000;			// test code
	pMsg->wDest = UCLIENT_DEST_UNKNOWN;
	pMsg->chFunc = UCLIENT_CMD_GET;
	pMsg->wPcm = g_sPointList[nPointIndex].nPcm;
	pMsg-> wPno = g_sPointList[nPointIndex].nPno;	
	pMsg->fValue;	
	pMsg->chChksum = 0;
	for ( i = 0; i < pMsg->chLength ; i++ ) {
		pMsg->chChksum -= pBuf[i];	
	}
	
	return sizeof(NET32_UDP_MSG);
}


int get_file(void)
// ----------------------------------------------------------------------------
// CHECK THREAD OPTION
{
	int i = 0;
	char seps[]   = " ,\t\n";
	char *token;
	int buf_val[8];
	char chTemp[32];
	//int cnt = 0;
	FILE* fp;

	//Initialize
	fp = NULL;
	g_nPointCount = 0;
	memset(g_sPointList, 0, sizeof(g_sPointList));
	
	// file open
	if((fp = fopen("/duksan/DATA/PointList_UDP", "r")) == NULL) {
		if (fp != NULL)  
			fclose(fp);
		printf("[ERROR] File Open with Option 'r'\n");		

		return -1;
	}
	else {
		// search file.
		while( !feof(fp) ) {
			memset(chTemp, 0x00, sizeof(chTemp));
			fscanf(fp, "%s", 	(char *)&chTemp);	
			
			//printf(">%s\n", &chTemp);
			
			i = 0;
			token = strtok( chTemp, seps );  
			while( token != NULL ) {
				buf_val[i++] = atoi(token);
				token = strtok( NULL, seps );		//Get next token;
			}			
			
			g_sPointList[g_nPointCount].nPcm = buf_val[0]; 
			g_sPointList[g_nPointCount].nPno = buf_val[1];

			printf("-Read : %d, %d\n", g_sPointList[g_nPointCount].nPcm , g_sPointList[g_nPointCount].nPno);
				
			//cnt++;
			g_nPointCount++;
		}				
	}
		
	if (fp != NULL)  
		fclose(fp);

	return 1;		
}


int main(int argc, char *argv[]) 
{
	int i = 0;
	int    length, IsAlive = 1;
	int 	received = 0;
	char   message[512], buffer[512];                              // 통신에 사용될 메시지 버퍼
	UCLIENT_GET *pGet;
	unsigned short wSeq = 0;
	fd_set    read_fds;
	
	struct timespec ts;
	struct timeval	wait;
	int bytes = 0;
	int socket_size = 0;

	init_udp_socket();
	//init_ds_socket();
	
	get_file();
	
	g_nIndex = 0;
	pGet = (UCLIENT_GET *)&message[0];
	
	
	// test code
	for(;;) {
		// wait 3msec
		ts.tv_sec = 0;
		ts.tv_nsec = 5 * 1000000;				
		nanosleep(&ts, NULL);	
		
		memset(message, 0x00, sizeof(message));
		memset(buffer, 0x00, sizeof(buffer));
		
		printf(">\n");
		
		wSeq++;
		length = get_message(wSeq, message);	
				
		if ( sendto(udp_client_socket, message, length, 0, (struct sockaddr *)&udp_client_sockaddr, sizeof(udp_client_sockaddr)) == -1 ) {
			printf("Can't send to server\n");
		}		
		
		printf(".\n");
		
		FD_ZERO(&read_fds);
		FD_SET(0, &read_fds);
		FD_SET(udp_client_socket, &read_fds);

		wait.tv_sec = 1;
		wait.tv_usec = 0;

		//if (select(udp_client_socket+1, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)NULL) < 0) {
		if (select(udp_client_socket+1, &read_fds, (fd_set *)0, (fd_set *)0, ts) < 0) {
			printf("select open fail\n");
			exit(-1);
		}				
				
		if (FD_ISSET(udp_client_socket, &read_fds)) {				
			socket_size = sizeof(udp_client_sockaddr);
			if ((received = recvfrom(udp_client_socket, (void *)buffer, 512, 0, (struct sockaddr *)&udp_client_sockaddr, &socket_size)) < 0) {
				printf("클라이언트의 접속을 받아들일수가 없습니다\n");
				exit(-1);
			}
			
			printf("#\n");
			printf("Rx(%d) = ", received);
			for ( i = 0; i < received; i++ ) {
				printf("%x ", buffer[i] );
			}
			printf("\n");			
		}

	}
	                   
	
	while (IsAlive) {
		
		/* Send the word to the server */
		pGet->stx = '<';
		pGet->length = 4;
		pGet->cmd = UCLIENT_CMD_ACK;
		pGet->etx = '>';
		if (send(ds_client_socket, pGet, sizeof(UCLIENT_GET), 0) != sizeof(UCLIENT_GET)) {
			Die("Mismatch in number of sent bytes");
		}
	
		if ((received = recv(ds_client_socket, buffer, BUFFSIZE-1, 0)) < 1) {
			Die("Failed to receive bytes from server");
		}

		if ( received == sizeof(UCLIENT_GET) ) {
			continue;
		}
		else if ( received == sizeof(UCLIENT_MSG) ) {
			/*
			for ( i = 0; i < received; i++ ) {
				printf("%02X ", buffer[i]);				
			}
			printf("\n");
			*/

			if ( sendto(udp_client_socket, buffer, received, 0, (struct sockaddr *)&udp_client_sockaddr, sizeof(udp_client_sockaddr)) == -1 ) {
				printf("Can't send to server\n");
			}
		}
		continue;
	}
}
