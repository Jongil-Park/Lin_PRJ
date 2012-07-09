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
#include <sys/poll.h>

#include "libqueue.h"
#include "libinterface.h"

// GPIO Module ==========================================================
#define		GPIO_MAJOR_DEF		218
#define		DEV_NAME			"S3C2410 GPIO Module ver 1.00"
#define		GPIO_IOCTL_MAGIC	'h'

// variable ==========================================================
static int show_dbg = 0;					// debug text
static int libinterface_socket = -1;		// client socket
static unsigned char tx_msg[MAX_BUFFER];	// tx buffer
static unsigned char rx_msg[MAX_BUFFER];	// rx buffer
float ptbl[MAX_PCM][MAX_POINT];		// point table
//int multi_pcm = 0;

//time_t start_sec = 0;
//time_t end_sec = 0;

static int uart2fd = -1;					// uart2 fd
static int gpiofd = -1;						// gpio fd
#define UART2DEVICE "/dev/s3c2410_serial2"
//#define UART2DEVICE "/dev/s3c2410_serial1"
 
// queue =============================================================
point_queue lib_queue;				// library queue

// mutex =============================================================
//pthread_mutex_t pointAccess_mutex = PTHREAD_MUTEX_INITIALIZER;	// point table access mutex
pthread_mutex_t queueAccess_mutex = PTHREAD_MUTEX_INITIALIZER;	// queue access mutex

// extern function ===================================================
extern void do_interface(void);

#define UART1_TX_EN 	ioctl( gpiofd, _IOW( GPIO_IOCTL_MAGIC, 2, int ), 5 );
#define UART2_TX_EN 	ioctl( gpiofd, _IOW( GPIO_IOCTL_MAGIC, 2, int ), 6 );
#define UART1_TX_DIS 	ioctl( gpiofd, _IOW( GPIO_IOCTL_MAGIC, 10, int ), 5 );
#define UART2_TX_DIS 	ioctl( gpiofd, _IOW( GPIO_IOCTL_MAGIC, 10, int ), 6 );

// functions =========================================================
/****************************************************************/
int	dev_open( char *fname, unsigned char major, unsigned char minor )
/****************************************************************/
{
	int	dev;
	
	dev = open( fname, O_RDWR|O_NDELAY );
	if (dev == -1 )
	{
		mknod( fname, (S_IRWXU|S_IRWXG|S_IFCHR), (major<<8)|(minor) );
		
		dev = open( fname, O_RDWR|O_NDELAY );
		if (dev == -1 )
		{
			printf( " Device OPEN FAIL %s\n", fname );
			return -1;
		}
	}
        
	return dev;
}

/****************************************************************/
// close libinterface_socket
static void ReleaseLibInterfaceSocket(void)
/****************************************************************/
{
	if (libinterface_socket != -1)
		close(libinterface_socket);
	libinterface_socket = -1;		
	if (show_dbg) printf("Close Socket\n");
}

/****************************************************************/
// Connect 127.0.0.1 server
static int OpenLibInterfaceSocket(void)
/****************************************************************/
{
	struct sockaddr_in server_addr_in;
	struct timeval tv;
	char server_ip[15];
	int server_port;
	
	libinterface_socket = -1;
	memset(&server_addr_in, 0, sizeof(server_addr_in));
	tv.tv_sec = 0;
	tv.tv_usec = 100000; //0.1sec
	strcpy(server_ip, "127.0.0.1");
	server_port = LIBINTERFACE_SERVER_PORT;

    libinterface_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	setsockopt(libinterface_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	
	server_addr_in.sin_family = AF_INET;
	server_addr_in.sin_addr.s_addr = inet_addr(server_ip);
	server_addr_in.sin_port = htons(server_port);
	
	if(connect(libinterface_socket, (struct sockaddr *) &server_addr_in,
				sizeof(server_addr_in)) >= 0)
	{
		if (show_dbg) printf("Connected Socket : %s, %d\n", server_ip, server_port);
		return SUCCESS;
	}
	else
	{
		if (show_dbg) printf("Connecting to Interface Server Failed\n");
		return FAIL;
	}
}

// Send Message
/****************************************************************/
static void SendTxMsg(int length)
/****************************************************************/
{
	if(send(libinterface_socket, tx_msg, length, 0) <= 0)
	{
		printf("[ERROR] Send() in handleCmdPget()\n");
		return;
	}	
}

#if 0
// Receive handler
/****************************************************************/
static void DoRecvHandler(void)
/****************************************************************/
{
	int i = 0;
	int recv_length = 0;
	int recv_index = 0;
	int recv_retry = 0;
	int value_size = 0;
	_RES_GET_LENGTH_PACKET *p;
	_RES_READ_POINT_PACKET *pRx;
	struct timespec ts;
	
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000;		// 0.1 sec;
	value_size = sizeof(float) * 128;
	
	memset(rx_msg, 0x00, sizeof(rx_msg));

	while(1)
	{
		recv_length = recv(libinterface_socket, 
							rx_msg + recv_index, 
							sizeof(rx_msg) - recv_index, 
							0);
		
		// data가 한번에 전부 전송되지 못하는 경우를 생각해야 하기 때문에
		// recv_index를 사용해서 모두 전송받도록 처리하였다.
		if (recv_length > 0)
		{
			recv_index = recv_index + recv_length;
			
			p = (_RES_GET_LENGTH_PACKET *)rx_msg;
			if (p->stx == LIB_CMD_STX)
			{
				if (p->length == recv_index)
				{
					if (show_dbg)
					{
						printf("RX = ");
						for(i = 0; i < recv_index; i++)
							printf(" %x", rx_msg[i]);
						printf("\n");
					}
					
					// ignore write-command
					if (p->cmd == 'W')
						return;
						
					// pRx로 pointer 가 변경된다. 주의해야 한다.
					pRx = (_RES_READ_POINT_PACKET *)rx_msg;
					
					switch(pRx->part)
					{
						// 0 ~ 127번 Point value를 복사한다.
						case 0:
							if (show_dbg) printf("copy float value (0 ~ 127) from pcm(%d)\n", pRx->pcm);
							memcpy(&ptbl[pRx->pcm][0], &pRx->val[0], value_size);
							break;

						// 128 ~ 255번 Point value를 복사한다.
						case 1:
							if (show_dbg) printf("copy float value (128 ~ 255) from pcm(%d)\n", pRx->pcm);
							memcpy(&ptbl[pRx->pcm][128], &pRx->val[0], value_size);
							break;
						
						default:
							break;
						
					}
					return;
				}
				else
					if (show_dbg) printf("p->length = %d, recv_index = %d\n", 
							p->length, recv_index);
			}
		}
		else if (recv_length < 0)
		{
			ReleaseLibInterfaceSocket();
			return;
		}
		else
		{
			nanosleep(&ts, NULL);	
			recv_retry++;
			if (recv_retry > 10)
				return;
		}					
	}	
}
#endif

#if 0
// test function.
static void lib_test(void)
{
	printf("hello world\n");
	OpenLibInterfaceSocket();
	ReleaseLibInterfaceSocket();	
}
#endif

/****************************************************************/
static unsigned short MakeChksum (unsigned char *p, int length)
/****************************************************************/
{
	int i = 0;
	unsigned short chksum = 0;

	for (i = 1; i < length; i++)
		chksum += *(p + i);

	return chksum;
}

/****************************************************************/
static void MakePsetMsg(int pcm, int pno, float val)
/****************************************************************/
{
	int i = 0;
	unsigned char *pData;
	_REQ_WRITE_POINT_PACKET *pWrite;

	memset(tx_msg, 0x00, sizeof(tx_msg));
	pData = (unsigned char *)tx_msg;
	pWrite = (_REQ_WRITE_POINT_PACKET *) pData;

	if (show_dbg) printf("LIB : %s() pcm = %d, pno = %d, val = %f\n", 
		__FUNCTION__, pcm, pno, val);
	
	pWrite->stx = LIB_CMD_STX;
	pWrite->length = sizeof(_REQ_WRITE_POINT_PACKET);
    pWrite->cmd = LIB_CMD_WRITE;
	pWrite->pcm = pcm;
	pWrite->pno = pno;
	pWrite->val = val;
	pWrite->chksum = MakeChksum(pData, sizeof(_REQ_WRITE_POINT_PACKET) - 3);
	pWrite->etx = LIB_CMD_ETX;
			
	if (show_dbg)
	{
		printf("TX = ");
		for(i = 0; i < pWrite->length; i++)
			printf(" %x", pData[i]);
		printf("\n");
	}
}

/****************************************************************/
static void MakePgetMsg(unsigned int pcm, int type)
/****************************************************************/
{
	int i = 0;
	unsigned char *pData;
	_REQ_READ_POINT_PACKET *pRead;

	memset(tx_msg, 0x00, sizeof(tx_msg));
	pData = (unsigned char *)tx_msg;
	pRead = (_REQ_READ_POINT_PACKET *) pData;

	if (show_dbg) printf("LIB : %s() LIB_CMD_READPOINT\n", __FUNCTION__);

	switch(type)
	{
		case LIB_CMD_READPOINT_1:
			pRead->part = LIB_READ_PART_1_INDEX;
			break;
		case LIB_CMD_READPOINT_2:
			pRead->part = LIB_READ_PART_2_INDEX;
			break;
		default:
			pRead->part = LIB_READ_PART_1_INDEX;
			break;
	}
	
	pRead = (_REQ_READ_POINT_PACKET *) pData;
	pRead->stx = LIB_CMD_STX;
	pRead->length = sizeof(_REQ_READ_POINT_PACKET);
    pRead->cmd = LIB_CMD_READ;
	pRead->pcm = pcm;	
	pRead->chksum = MakeChksum(pData, sizeof(_REQ_READ_POINT_PACKET) - 3);
	pRead->etx = LIB_CMD_ETX;

	if (show_dbg)
	{
		printf("size = 0x%x\n", sizeof(_REQ_READ_POINT_PACKET));
		
		printf("TX = ");
		for(i = 0; i < pRead->length; i++)
			printf(" %x", pData[i]);
		printf("\n");
	}
}

/****************************************************************/
float pget(int pcm, int pno)
/****************************************************************/
{
	float val = 0;
	
	if (show_dbg) printf("LIB : pget (%d, %d)\n", pcm, pno);
    val = ptbl[pcm][pno];
	if (show_dbg) printf("LIB : pget return value = %f\n", val);
	return val;
}

/****************************************************************/
void pset(int pcm, int pno, float val)
/****************************************************************/
{
	point_info point;

	//if (show_dbg) printf("LIB : pset (%d, %d, %f)\n", pcm, pno, val);
	printf("LIB : pset (%d, %d, %f)\n", pcm, pno, val);
	point.pcm = pcm;
	point.pno = pno;
	point.value = val;
	
	pthread_mutex_lock(&queueAccess_mutex);
	putq(&lib_queue, point);
	pthread_mutex_unlock(&queueAccess_mutex);
}

/****************************************************************/
static int GetLibQueue(point_info *pPoint)
/****************************************************************/
{
	int result = FALSE;

	pthread_mutex_lock(&queueAccess_mutex);
	result = getq(&lib_queue, pPoint);
	pthread_mutex_unlock(&queueAccess_mutex);

	if(result == SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/****************************************************************/
int point_file_init(void)
/****************************************************************/
{
	int i, j;
	int pno;
	float value;
	FILE *fp;
	char filename[32];
	char index[2];
	int init_status;
	
	//Initialize
	i = j   = 0;
	pno	 	= 0;
	value   = 0;
	fp      = NULL;
	strncpy(filename, "/duksan/DATA/point00.dat\0", sizeof(filename));
	index[0] = index[1] = 0;
	init_status = MAIN_GET_FILE_NAME;
	
	//mkdir
	mkdir("/duksan/DATA", 00777);
	
	while(1)
	{
		switch(init_status)
		{
			case MAIN_GET_FILE_NAME:
			{
				index[0] = (i / 10) + 48;
				index[1] = (i % 10) + 48;
				strncpy(&filename[18], index, sizeof(index));
				init_status = MAIN_FILE_READWRITE_OPEN;
				break;
			}

			case MAIN_FILE_READWRITE_OPEN:
			{
				if((fp = fopen(filename, "r")) == NULL)
					return ERROR;
				else
					init_status = MAIN_FILE_READ;
				break;
			}

			case MAIN_FILE_READ:
			{
				for(j = 0; j < MAX_POINT_NUMBER; j++)
				{
					if(fread(&value, sizeof(float), 1, fp) == 1)
						ptbl[i][j] = value;
					else 
					{
						//file may have been corrupted, so make new one.
						fclose(fp);
						init_status = MAIN_FILE_CREATE;
						break;
					}
				}
				//
				fclose(fp);
				init_status = MAIN_INCREASE_INDEX;
				break;
			}
			
			case MAIN_INCREASE_INDEX:
			{
				i = i + 1;
				//
				if(i < MAX_NET32_NUMBER)
					init_status = MAIN_GET_FILE_NAME;
				else
					init_status = MAIN_FINISH_INIT_PROCESS;
				break;
			}

			case MAIN_FINISH_INIT_PROCESS:
			{
				//printf("+ Success pcm-file-data initialize\n");
				return SUCCESS;
			}
		}
	}
	//
	//printf("Failed in %s()\n", __FUNCTION__);
	return ERROR;
}


int main(void)
{
	unsigned int pcm = 0;
	int sel_part = 0;
	int socket_info = 0;
	point_info point;
	pthread_t thread[2];
	pthread_attr_t thread_attr;
	struct sched_param thread_param;
	int thread_policy;	
	int th_id;
	int th_status;
	int rr_min_priority, rr_max_priority;
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000;		// 0.1 sec;

	initq(&lib_queue);
	memset(&ptbl, 0x00, sizeof(ptbl));

	if(pthread_attr_init(&thread_attr) != 0)
	{
		printf("Thread attr init error\n");
		return;
	}
	pthread_attr_getschedpolicy(&thread_attr, &thread_policy);
	pthread_attr_getschedparam(&thread_attr, &thread_param);
	
	printf("Default policy is %s, priority is %d\n", 
			(thread_policy == SCHED_FIFO ? "FIFO"
			 : (thread_policy == SCHED_RR ? "RR"
				: (thread_policy == SCHED_OTHER ? "OTHER"
					: "unknown"))), thread_param.sched_priority);
	
	
	rr_min_priority = sched_get_priority_min(SCHED_RR);
	rr_max_priority = sched_get_priority_max(SCHED_RR);
	printf("[Round Robin] Min : %d, Max : %d\n", rr_min_priority, rr_max_priority);

	th_id = pthread_create(&thread[0], NULL, (void*(*)(void*)) do_interface, 
						   (void*) &thread[0]);
	printf("Interface thread created\n");
	sleep(1);

	gpiofd = dev_open("/dev/gpio", GPIO_MAJOR_DEF, 0 );
	point_file_init();
	
	while(1)
	{
		// If libinterface_socket is -1, not connected.
		// try connect to server
		if (libinterface_socket == -1)
		{
			if(OpenLibInterfaceSocket() != SUCCESS)
			{
				if (show_dbg) printf("Retry connect\n");
				sleep(3);
				ReleaseLibInterfaceSocket();
				continue;		
			}
		}

		// get library queue.
		if (GetLibQueue(&point) == TRUE) 
		{
			///*
			printf("GetLibQueue pcm = %d, pno = %d, value = %f\n", 
				point.pcm, point.pno, point.value);		
			//*/
			MakePsetMsg(point.pcm, point.pno, point.value);		
			SendTxMsg(sizeof(_REQ_WRITE_POINT_PACKET));
			//DoRecvHandler();		
		}
	
		point_file_init();
		
		ts.tv_sec = 0;
		ts.tv_nsec = 10000000;		// 0.01 sec;
		nanosleep(&ts, NULL);
	}
	
	pthread_join(thread[0], (void **)th_status);		// interface
	return;
}

int RecvUart2(unsigned char *p, int maxlength)
{
	int i = 0;
	int res;
	
	res = read(uart2fd, p, maxlength);
	return res;
}

int SendUart2(unsigned char *p, int length)
{
	int i = 0;
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000;		// 0.1 sec;

	UART2_TX_EN
	write(uart2fd, p, length);
	UART2_TX_DIS
	//nanosleep(&ts, NULL);
	//UART2_TX_DIS
	return length;
}

void InitUart2(int baudrate)
{
	int i, c, res;
	struct termios oldtio, newtio;
	//char buf[1024];

	uart2fd = open(UART2DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (uart2fd < 0) { 
		printf("Serial uart2fd Open fail\n");
		perror(UART2DEVICE); exit(-1); 
	}
	else
		printf("Serial uart2fd Open success\n");
	
	if (tcgetattr(uart2fd, &oldtio) < 0)
	{
		perror("error in tcgetattr");
		return; 
	}
	
	bzero(&newtio, sizeof(newtio));
	//newtio.c_cflag = CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	switch(baudrate)
	{
		case 4800:	
			printf("baudrate = 4800\n");
			//cfsetispeed(&newtio, B9600);
			//cfsetospeed(&newtio, B9600);
			newtio.c_cflag = B4800 | CS7 | CLOCAL | CREAD;
			break;

		case 9600:	
			printf("baudrate = 9600\n");
			//cfsetispeed(&newtio, B9600);
			//cfsetospeed(&newtio, B9600);
			newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
			// Hong Univ GHP Interface.
			/*
			newtio.c_cflag = B9600 | CS7 | CLOCAL | CREAD | PARENB;
			newtio.c_cflag &= ~CSTOPB;
			newtio.c_cflag &= ~CRTSCTS;
			*/
			printf("Test...9600\n");
			break;
			
		case 19200:	
			printf("baudrate = 19200\n");
			//cfsetispeed(&newtio, B19200);
			//cfsetospeed(&newtio, B19200);
			newtio.c_cflag = B19200 | CS8 | CLOCAL | CREAD;
			break;			

		case 115200:
			printf("baudrate = 115200\n");
			//cfsetispeed(&newtio, B115200);
			//cfsetospeed(&newtio, B115200);
			newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
			break;			
	}
	
	newtio.c_lflag = 0;
	
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN]  = 1;
	
	tcflush(uart2fd, TCIFLUSH);
	tcsetattr(uart2fd,TCSANOW,&newtio);	
	printf ("uart2fd = %x\n", uart2fd);
}
