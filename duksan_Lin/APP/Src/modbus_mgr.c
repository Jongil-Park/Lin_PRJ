/*
 * file 		: modbus_mgr.c
 * creator   	: jong2ry
 *
 *
 * TCP/IP����� MODBUS Server ����
 *  	1. modbus protocol�� ���� data ó��.
 *		2. mdsvr_log.c�� ����Ѵ�. 
 *
 *
 * version :
 *		0.0.1  jong2ry working.
 *		0.0.2  jong2ry code clean.
 *
 *
 * code �ۼ��� ���ǻ���
 *		1. global ������ 'g_' ���λ縦 ����.
 *		2. ������ ������ ������ �Ʒ��� ���� ��Ģ���� �����Ѵ�. 
 *			int					n
 *			short				w
 *			long				l
 *			float				f
 *			char				ch
 *			unsigned char       b
 *			unsignnd int		un
 *			unsigned short 		uw 
 *			pointer				p
 *			structure			s or nothing
 *		3. �������� ù���ڴ� ����ڷ� ����Ѵ�. 
 *		4. ���������� �� ������ ������ ������� ����ڷ� �Ѵ�. 
 *			ex > g_nTest, g_bFlag, g_fPointTable
 *		5. �Լ����� �ҹ��ڷ� '_' ��ȣ�� ����ؼ� �����Ѵ�. 
 *		6. �Լ��� �Լ� ������ ������ 2�ٷ� �Ѵ�. 
 */
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
#include <sys/time.h>
#include <time.h>
#include <sys/poll.h>		// use poll event

////////////////////////////////////////////////////////////////////////////////
//define
#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

#include "modbus_mgr.h"										// elba manager


////////////////////////////////////////////////////////////////////////////////
// extern variable


////////////////////////////////////////////////////////////////////////////////
// global variable
unsigned char modbusSvr_tx_msg[4096];
unsigned char modbusSvr_rx_msg[4096];
float g_fVal[MAX_POINT_NUMBER];													// 
int nVal[MAX_POINT_NUMBER];    													//


static void modsvr_sleep(int sec, int msec) 
// ----------------------------------------------------------------------------
// WAIT TIMER
// Description : use select function for timer.
// Arguments   : sec		Is a second value.
//				 msec		Is a milli-second value. 
// Returns     : none
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = msec * 1000;
    select( 0, NULL, NULL, NULL, &tv );
	return;
}


void mdsvr_log(int type)
// ----------------------------------------------------------------------------
// WRITE LOG TEXT
// Description : log�� ����Ͽ� Debugging�� ����Ѵ�. 
// Arguments   : type		log�� ��ϵǴ� text�� type
// Returns     : none
{
	FILE *fp = NULL;
	FILE *fp_bak = NULL;
	FILE *fp_log = NULL;
	time_t     tm_nd;
	struct tm *tm_ptr;
	int filesize = 0;
	int size = 0;
	unsigned char buff[1028];
	
	fp = fopen("/duksan/FILE/mdsvr_log.txt", "a+");
	if( fp == NULL ) {
		printf("[ERROR] Mdsvr File open Error\n");
		modsvr_sleep(5, 0);
		//system("reboot");
		//exit(1);
		system("killall duksan");
		return;		
	}
	
	fseek(fp, 0L, SEEK_END); 
	filesize = ftell( fp );
	printf("file size = %d\n", filesize);

	if ( filesize > 4096 ) {

		fseek(fp, 0L, SEEK_SET); 

		fp_bak = fopen("/duksan/FILE/mdsvr_log.bak", "w");
		if( fp_bak == NULL ) {
			printf("[ERROR] Mdsvr Bakcup File open Error\n");
			modsvr_sleep(5, 0);
			//system("reboot");
			//exit(1);
			system("killall duksan");
			return;		
		}
		
		memset( buff, 0x00, sizeof(buff) );
		while( 0 < (size = fread( buff, 1, 1024, fp))) {
			fwrite( buff, 1, size, fp_bak);
			memset( buff, 0x00, sizeof(buff) );
		}  

		fclose(fp);
		fclose(fp_bak);
	
		fp = fopen("/duksan/FILE/mdsvr_log.txt", "w");
		if( fp == NULL ) {
			printf("[ERROR] File Open with Option 'r'\n");
			//system("reboot");
			//exit(1);
			system("killall duksan");
			return;		
		}
	}

	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);

	// write Communication Error
	switch ( type ) {
	// BootOn Text
	case MDSVR_LOG_BOOTON:
		fprintf(fp,	"[BootOn]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;

	// Connection ok Text
	case MDSVR_LOG_CONNECTION_OK:
		fprintf(fp,	"[Connect On]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;

	// Connection error Text
	case MDSVR_LOG_CONNECTION_FAIL:
		fprintf(fp,	"[Connect Error]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;

	// Connection Close Text
	case MDSVR_LOG_CONNECTION_CLOSE:
		fprintf(fp,	"[Connect Close]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;

	// Connection Close Text
	case MDSVR_LOG_BIND_ERROR:
		fprintf(fp,	"[Bind Error]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;		
		
	case MDSVR_LOG_LISTEN_ERROR:
		fprintf(fp,	"[Listen Error]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;				
	}

	fclose(fp);

	fp_log = fopen("/httpd/mdsvr_log.txt", "w");
	if( fp_log == NULL ) {
		printf("[ERROR] Mdsvr Html Log File open Error\n");
		return;		
	}
	
	fp_bak = fopen("/duksan/FILE/mdsvr_log.bak", "r");
	if( fp_bak == NULL ) {
		printf("[ERROR] Mdsvr Bakcup File open Error\n");
	} 
	else {
		fseek(fp_bak, 0L, SEEK_SET); 
		memset( buff, 0x00, sizeof(buff) );
		while( 0 < (size = fread( buff, 1, 1024, fp_bak))) {
			fwrite( buff, 1, size, fp_log);
			memset( buff, 0x00, sizeof(buff) );
		}  
	
		fclose(fp_bak);	
	}
	
	fp = fopen("/duksan/FILE/mdsvr_log.txt", "r");
	if( fp == NULL ) {
		printf("[ERROR] Mdsvr File open Error\n");
	} 
	else {
		fseek(fp, 0L, SEEK_SET); 
		memset( buff, 0x00, sizeof(buff) );
		while( 0 < (size = fread( buff, 1, 1024, fp))) {
			fwrite( buff, 1, size, fp_log);
			memset( buff, 0x00, sizeof(buff) );
		}  
	
		fclose(fp);	
	}	
	
	fclose(fp_log);	
}


static void convert_float_to_4byte(unsigned char *pData, unsigned char *pTxMsg)
{
    int i = 0;
    unsigned char cData[4];
    
    for (i = 0; i < 4; i++)
        cData[i] = *(pData + i);

    *(pTxMsg + 0) = cData[1];  
    *(pTxMsg + 1) = cData[0];
    *(pTxMsg + 2) = cData[3];
    *(pTxMsg + 3) = cData[2];
} 


/*
#if 0
static void convert_int_to_2byte(unsigned char *pData, unsigned char *pTxMsg)
{
    int i = 0;
    unsigned char cData[4];
    
    for (i = 0; i < 4; i++)
        cData[i] = *(pData + i);
    
    *(pTxMsg + 0) = cData[1];  
    *(pTxMsg + 1) = cData[0];
} 
#endif
*/


/*
#if 0
static void convert_rxdata_to_float(float *pData, unsigned char *pRxMsg)
{
    unsigned char *pVal;
    
    pVal = (unsigned char *) pData;

    *(pVal + 3) = *(pRxMsg + 2);  
    *(pVal + 2) = *(pRxMsg + 3);
    *(pVal + 1) = *(pRxMsg + 0);
    *(pVal + 0) = *(pRxMsg + 1); 
} 
#endif 
*/


int modsvr_handler(int fd, int rxmsg_length)
{
	// Variables
	int i = 0;
	int pcm = 0;
	int pno = 0;
	int	message_type = 0;
	//int rxmsg_length = 0;
	int txmsg_length = 0;
    int modbus_id = 0;
	short modbus_addr = 0;
	short modbus_length = 0;
    //int modbus_byte = 0;
    unsigned char *pVal;
    int nCmdState = 0;
    //unsigned char cRxData[4];
    //float fRxData = 0;
    int nRxData = 0;

	// Initialize
	i = 0;
	message_type = 0;
	//rxmsg_length = 0;
	//memset(modbusSvr_rx_msg, 0x00, sizeof(modbusSvr_rx_msg));

	//rxmsg_length = recv(fd, modbusSvr_rx_msg, sizeof(modbusSvr_rx_msg), 0);
    //nRxTraffic += rxmsg_length;
    
    nCmdState = modbusSvr_rx_msg[7]; 
    switch(nCmdState)
    {
        case CMD_READ:
            modbus_id = modbusSvr_rx_msg[6];
        	modbus_addr = modbusSvr_rx_msg[8] << 8 | modbusSvr_rx_msg[9];
        	modbus_length = modbusSvr_rx_msg[10] << 8 | modbusSvr_rx_msg[11];
            
            pcm = modbus_id - 1;
        	pno = modbus_addr / 2;
            break;

        // Jong2ry 20081209 ��õ�ø������� SI ����
        //case CMD_WRITE:
        case CMD_SINGLE_WRITE:
            modbus_id = modbusSvr_rx_msg[6];
        	modbus_addr = modbusSvr_rx_msg[8] << 8 | modbusSvr_rx_msg[9];
        	//modbus_length = modbusSvr_rx_msg[10] << 8 | modbusSvr_rx_msg[11];
            //modbus_byte = modbusSvr_rx_msg[12];
            nRxData = modbusSvr_rx_msg[10] << 8 | modbusSvr_rx_msg[11];
            pcm = modbus_id - 1;
        	pno = modbus_addr / 2;
            //memcpy(cRxData, &modbusSvr_rx_msg[13], sizeof(cRxData));
            //convert_rxdata_to_float(&fRxData, cRxData);
            break;

        default :
            // dougwon 20081202
            // 
            printf("Mdsvr(Line %d) : Command Error(0x%x), rxmsg_length %d 0x%x \n", __LINE__, nCmdState, rxmsg_length, rxmsg_length);
            // dougwon 20081202 
            // return FAIL;
            
            // dougwon 20081202
            // It looks like socket is closed, so socket descriptor SHOULD be closed !!!
            break;
    }    
    
	printf("\nFD [%d] Interface Handler RX msg : \n", fd);
	for(i = 0; i < rxmsg_length; i++)
		printf("%02x ", modbusSvr_rx_msg[i]);
	printf("\n");	
	printf("modbus_addr %d, modbus_length %d, pcm %d, pno %d \n", modbus_addr, modbus_length, pcm, pno);
	
	    
    switch(nCmdState)
    {
        // Command function 0x03. 
        case CMD_READ:
            // 4byte (float) ������ ������ ��� ������.
        	// dougwon 20081202 ��õ�ø������� SI ����
            // txmsg_length = 9 + (4 * modbus_length);
        	txmsg_length = 9 + (2 * modbus_length);
        	
            // dougwon 20081202 ��õ�ø������� SI ����
            // modbusSvr_tx_msg[8] = 4 * modbus_length;
        	modbusSvr_tx_msg[8] = 2 * modbus_length;
            
            memcpy(modbusSvr_tx_msg, modbusSvr_rx_msg, 8); 
            printf("txmsg_length = %d, data_length = %d\n",txmsg_length, modbusSvr_tx_msg[8]);    
            
            // dougwon 20081202 ��õ�ø������� SI ����
            // for (i = 0; i < modbus_length; i++)
            for (i = 0; i < modbus_length / 2; i++)
            {
                //RequirePval(pcm, pno + i);
                g_fVal[i] = pGet(pcm, pno + i);
                pVal = (unsigned char *) &g_fVal[i];
                convert_float_to_4byte(pVal, &modbusSvr_tx_msg[9 + (4 * i)]);
                //if (pcm == 6)
                //    printf("(%d) Pcm = %d, Pno = %d\n", i, pcm, pno + i); 
            } 


            // CHECK_ME FIX_ME  
            // dougwon 20081202 ��õ�ø�������
            // ??? write �ÿ��� �� ������ �Ǿ��°�?
            // 
            modbusSvr_tx_msg[4] = ((modbus_length*2 + 3) & 0xff00) >> 4;
            modbusSvr_tx_msg[5] = (modbus_length*2 + 3) & 0xff;


            // 2byte (int) ������ ������ ��� ������.
        	/*
            txmsg_length = 9 + (2 * modbus_length);
        	modbusSvr_tx_msg[8] = 2 * modbus_length;
            memcpy(modbusSvr_tx_msg, modbusSvr_rx_msg, 8);
            if (g_nMdSvr_Dbg) 
                printf("txmsg_length = %d, data_length = %d\n",txmsg_length, modbusSvr_tx_msg[8]);  
             
            for (i = 0; i < modbus_length; i++)
            {
                nVal[i] = pGet(pcm, pno + i);
                pVal = &nVal[i];
                convert_int_to_2byte(pVal, &modbusSvr_tx_msg[9 + (2 * i)]);
            }
            */
            break; 

        // Command function 0x10.
        // Jong2ry 20081209 ��õ�ø������� SI ����
        //case CMD_WRITE:
        case CMD_SINGLE_WRITE:
            pSet(pcm, pno, nRxData);
            txmsg_length = rxmsg_length;
            memcpy(modbusSvr_tx_msg, modbusSvr_rx_msg, txmsg_length);             

			//printf("id= %d, addr = %d, length = %d, byte = %d, fVal = %f\n", 
			//    modbus_id, modbus_addr, modbus_length, modbus_byte, fRxData);
			printf("id= %d, addr = %d, pcm = %d, pno = %d, nVal = %d\n", 
				modbus_id, modbus_addr, pcm, pno, nRxData);
				
			printf("\nFD [%d] Interface Handler RX msg : \n", fd);
			for(i = 0; i < rxmsg_length; i++)
				printf("%02x ", modbusSvr_rx_msg[i]);
			printf("\nFD [%d] Interface Handler TX msg (%d) : \n", fd, txmsg_length);
			for(i = 0; i < txmsg_length; i++)
				printf("%02x ", modbusSvr_tx_msg[i]);
			printf("\n");	                

            break;
            
        default :
            printf("Mdsvr(Line %d) : Command Error(0x%x)\n", __LINE__, nCmdState);
            return FAIL;
            break;
    }
    
	send(fd, modbusSvr_tx_msg, txmsg_length, 0);
    //nTxTraffic += txmsg_length; 

	printf("FD [%d] Interface Handler TX msg (%d) : \n", fd, txmsg_length);
	for (i = 0; i < txmsg_length; i++)
	{
		printf("%02x ", modbusSvr_tx_msg[i]);
		if (!((i+1) % 16))
			printf("\n");
	}
	printf("\n");

    return SUCCESS;   
}

/*
int nonblock(int fd, int nblockFlag)
{
   int flags;
 
   flags = fcntl( fd, F_GETFL, 0);
   if ( nblockFlag == 1 )
      return fcntl( fd, F_SETFL, flags | O_NONBLOCK);
   else
      return fcntl( fd, F_SETFL, flags & (~O_NONBLOCK));
}
*/


void *modsvr_main(void* arg)
{
	//int i = 0, j = 0;
	struct timeval tv;
	int server_socket;
	int client_socket;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int sin_size = sizeof(client_addr);
	int server_port = MODBUS_SERVER_PORT;
	int status = MDSVR_INIT_SOCK;
	int rxmsg_length = 0;
	int log_flag = -1;

	tv.tv_sec = 0;
	tv.tv_usec = 10000; //0.01sec

	server_socket = 0;
	client_socket = 0;
	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));

	
	//Ignore broken_pipe signal
	signal(SIGPIPE, SIG_IGN);
	modsvr_sleep(3, 0);

	mdsvr_log(MDSVR_LOG_BOOTON);

	while(1)
	{
		modsvr_sleep(0, 50);

		server_socket = socket(PF_INET, SOCK_STREAM, 0);
		printf("MODBUS-Server Started, socket = [%d], Port [%d]\n", server_socket, server_port);
		
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons(server_port);
		
		setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &tv, sizeof(tv));
		
		if(bind(server_socket, (struct sockaddr *)&server_addr,
					sizeof(server_addr)) < 0) {
			printf("[ERROR] In Bind of MODBUS-Server\n");
			close(server_socket);
			status = MDSVR_INIT_SOCK;
			mdsvr_log(MDSVR_LOG_BIND_ERROR);
			system("killall duksan");
			continue;
		}
		
		if(listen(server_socket, 10) < 0) {
			printf("[ERROR] In Listen of MODBUS-Server\n");
			close(server_socket);
			status = MDSVR_INIT_SOCK;
			mdsvr_log(MDSVR_LOG_LISTEN_ERROR);
			system("killall duksan");
			
			continue;
		}
		
		status = MDSVR_ACCEPT_SOCK;
		
		for (;;) {			
			switch (status) {
			// socket accept �� ��ٸ���.
			case MDSVR_ACCEPT_SOCK:
				client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &sin_size);
				if (client_socket == -1) {
					printf("[Error] In Accept of MODBUS-Server\n");
					modsvr_sleep(3, 0);
					status = MDSVR_ACCEPT_SOCK;
					break;
				}
				else 
					status = MDSVR_HANDLER_SOCK;
					printf("MODBUS-Server Connection Ok.\n");

					if ( log_flag != MDSVR_LOG_CONNECTION_OK ) {
						log_flag = MDSVR_LOG_CONNECTION_OK;
						mdsvr_log(MDSVR_LOG_CONNECTION_OK);
					}
					
				break;
					
			case MDSVR_HANDLER_SOCK:
				memset(modbusSvr_rx_msg, 0x00, sizeof(modbusSvr_rx_msg));
				rxmsg_length = recv(client_socket, 
									modbusSvr_rx_msg, 
									sizeof(modbusSvr_rx_msg),
									0);
			
				if ( rxmsg_length == 0 ) {
					status = MDSVR_ACCEPT_SOCK;
					close(client_socket);
					printf("MODBUS-Server Close\n");

					if ( log_flag != MDSVR_LOG_CONNECTION_CLOSE ) {
						log_flag = MDSVR_LOG_CONNECTION_CLOSE;
						mdsvr_log(MDSVR_LOG_CONNECTION_CLOSE);
					}
					
					break;
				}
				else if ( rxmsg_length > 0 ) {
					modsvr_handler(client_socket, rxmsg_length);
					status = MDSVR_HANDLER_SOCK;
					break;					
				}
				else {
					status = MDSVR_HANDLER_SOCK;	
					break;
				}				
				break;					
			}
		}
	}	
	
	//exit(1);
	system("killall duksan");
}



