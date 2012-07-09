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
//
//
#include "define.h"
#include "plc_observer.h"
#include "queue_handler.h"				// jogn2ry plc
#include "FUNCTION.h"
#include "message_handler.h"	// jogn2ry plc



extern int g_nMyPcm;																// My Pcm Number

//static int debug_prototype_modbus_eth_client = 1;

int g_nflag = 0;

static unsigned char peak_rx_msg[1024];

static unsigned char auchCRCHi[] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
	0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
	0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
	0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
	0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
	0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
	0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
}; 

// Low Order Byte Table 
// Table of CRC values for low-order byte
static unsigned char auchCRCLo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 
	0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 
	0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 
	0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 
	0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 
	0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 
	0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 
	0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 
	0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 
	0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 
	0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 
	0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 
	0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 
	0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};


static unsigned char CRC16_hi(unsigned char *puchMsg, unsigned short usDataLen)
{
	unsigned char uchCRCHi = 0xFF ; /* high CRC byte initialized  */
	unsigned char uchCRCLo = 0xFF ;	/* low CRC byte  initialized  */
	unsigned uIndex ;               /* will index into CRC lookup */
									/* table                      */

	while (usDataLen--)				/* pass through message buffer */
    {
		uIndex = uchCRCHi ^ *puchMsg++ ;	/* calculate the CRC   */
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
		uchCRCLo = auchCRCLo[uIndex] ;
    }

    return uchCRCHi;
}

static unsigned char CRC16_lo(unsigned char *puchMsg, unsigned short usDataLen)
{
	unsigned char uchCRCHi = 0xFF ; /* high CRC byte initialized */
	unsigned char uchCRCLo = 0xFF ;	/* low CRC byte  initialized  */
   	unsigned uIndex ;               /* will index into CRC lookup*/
									/* table                     */

	while (usDataLen--)		/* pass through message buffer    */
    {
		uIndex = uchCRCHi ^ *puchMsg++ ;	/* calculate the CRC   */
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
		uchCRCLo = auchCRCLo[uIndex] ;
    }

    return uchCRCLo;
}



void iface_peak_sleep(int sec, int usec) 
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = usec;
    select( 0, NULL, NULL, NULL, &tv );
	return;
}

      

void iface_peak_log(int type)
{
	FILE *fp = NULL;
	time_t     tm_nd;
	struct tm *tm_ptr;
	
	fp = fopen("/duksan/FILE/peak_log.txt", "a+");
	if( fp == NULL ) {
		printf("[ERROR] File Open with Option 'r'\n");
		system("reboot");
		return;		
	}

	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);

	// write Communication Error
	if ( type == 0 ) {
		fprintf(fp, "[Com Error]  %d/%d %d:%d\n", tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min);
	}
	// write reboot
	else if ( type == 1 ) {
		fprintf(fp, "[Reboot]  %d/%d %d:%d\n", tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min);
	}
	// write Warnning class 1
	else if ( type == 2 ) {
		fprintf(fp, "[Warnning Class 1]  %d/%d %d:%d\n", tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min);
	}
	// write Warnning class 2
	else if ( type == 3 ) {
		fprintf(fp, "[Warnning Class 2]  %d/%d %d:%d\n", tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min);
	}
	// write Warnning class 3
	else if ( type == 4 ) {
		fprintf(fp, "[Warnning Class 3]  %d/%d %d:%d\n", tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min);
	}
	fclose(fp);
}

static int iface_peak_send_msg(int fd)
{
	int i = 0;
	unsigned char tx_msg[12];
	
	memset(tx_msg, 0x00, sizeof(tx_msg));
	
	tx_msg[0] = 0x01;
	tx_msg[1] = 0x04;
	tx_msg[2] = 0x00;
	tx_msg[3] = 0x11;
	tx_msg[4] = 0x00;
	tx_msg[5] = 0x01;
	tx_msg[7] = 0x00FF & CRC16_lo(tx_msg, 6);
	tx_msg[6] = 0x00FF & CRC16_hi(tx_msg, 6);

	fprintf(stdout, "TX = ");
	for ( i = 0; i < 8; i++ ){
		fprintf(stdout, "%02x ", tx_msg[i]);	
	}     	
	fprintf(stdout, "\n");
	fflush(stdout);	
	
    if(send(fd, tx_msg, 8, 0) > 0)
    {
        printf("TX_msg Sending Success...\n\n");
        return 1;
    }
    else
    {
        printf("TX_msg Sending Failed...\n\n");        
        return -1;
    }    
    //temp_crc = (0x00FF & CRC16_hi(tx_msg, 6)) | (0x00FF & CRC16_lo(tx_msg, 6)) << 8;
}


static int iface_peak_recv_msg(int fd)
{
	int i = 0;
	//unsigned char tx_msg[12];
	int rxmsg_length = 0;
	unsigned char temp_crc_hi = 0;
	unsigned char temp_crc_lo = 0;
    	
	memset(peak_rx_msg, 0x00, sizeof(peak_rx_msg));

	rxmsg_length = recv(fd, peak_rx_msg, sizeof(peak_rx_msg), 0);

	if ( rxmsg_length == 0 ) 
		return 0;
	else if ( rxmsg_length < 0 )
		return -1;

	fprintf(stdout, "RX(%d) = ", rxmsg_length);
	for ( i = 0; i < rxmsg_length; i++ ){
		fprintf(stdout, "%02x ", peak_rx_msg[i]);	
	}     	
	fprintf(stdout, "\n");
	fflush(stdout);	
	
	temp_crc_hi = 0x00FF & CRC16_hi(peak_rx_msg, rxmsg_length - 2);
	temp_crc_lo = 0x00FF & CRC16_lo(peak_rx_msg, rxmsg_length - 2);

	printf("crc %x %x\n", temp_crc_hi, temp_crc_lo);

	if ( temp_crc_hi == peak_rx_msg[rxmsg_length - 2] 
			&& temp_crc_lo == peak_rx_msg[rxmsg_length - 1] ) {
		
		fprintf(stdout, "Recv data %d ...  Val[%d]\n", peak_rx_msg[4], peak_rx_msg[4]);
		fflush(stdout);

		if ( peak_rx_msg[4] == 2 ) {
			pSet(g_nMyPcm, 0, 1);	
			pSet(g_nMyPcm, 1, 0);	
			pSet(g_nMyPcm, 2, 0);	

			if (g_nflag != 2) {
				 g_nflag = 2;
				 iface_peak_log(2);
			}
		}
		else if ( peak_rx_msg[4] == 3 ) {
			pSet(g_nMyPcm, 0, 1);	
			pSet(g_nMyPcm, 1, 1);	
			pSet(g_nMyPcm, 2, 0);	

			if (g_nflag != 3) {
				 g_nflag = 3;
				 iface_peak_log(3);
			}
		}
		else if ( peak_rx_msg[4] == 4 ) {
			pSet(g_nMyPcm, 0, 1);	
			pSet(g_nMyPcm, 1, 1);	
			pSet(g_nMyPcm, 2, 1);	

			if (g_nflag != 4) {
				 g_nflag = 4;
				 iface_peak_log(4);
			}
		}
		else {
			g_nflag = 0;
			pSet(g_nMyPcm, 0, 0);	
			pSet(g_nMyPcm, 1, 0);	
			pSet(g_nMyPcm, 2, 0);	
		}
	}

	return 1;
}


#if 0
static int Set_Point_04(int client_socket)
{
    //Variables
    int i, j;
    unsigned char device_id;
    unsigned char func;
    short addr;
    short num_of_points;
    short temp_value; 
	int ret = 0;

    //Initialize
    i = j = 0;
    device_id = 1;
    func = 4;                      
    addr = 0;
    num_of_points = 1;            
    temp_value = 0;  
    
    if(debug_prototype_modbus_eth_client == 1)
        printf("\nFunction Code : 0x04 Reading...\n");
        
    addr = 17;
    memset(&temp_value, 0, sizeof(temp_value));

	ret = Ethernet_Modbus_Request_0304(client_socket, 
										device_id, 
										func, 
										addr, 
										num_of_points, 
										&temp_value);
	if( ret < 0 ){
        return -1;
    }
	else if (ret == 0) {
		return 0;
	}
    else {
        printf("Read Modbus Data, Device ID [%hd] Func [%hd] Addr [%hd] Value 0x[%04X]\n", device_id, func, addr, temp_value);
        //iPset(LSU_ID, ??, temp_value);
        
        if (temp_value == 2)
        {
            pSet(g_nMyPcm, 0, 1);
            pSet(g_nMyPcm, 1, 0);
            pSet(g_nMyPcm, 2, 0);
			
			if ( g_nflag != 2 ) {
				g_nflag = 2;
				iface_peak_log(2);
			}
			
        }
        else if (temp_value == 3)
        {
            pSet(g_nMyPcm, 0, 1);
            pSet(g_nMyPcm, 1, 1);
            pSet(g_nMyPcm, 2, 0);

			if ( g_nflag != 3 ) {
				g_nflag = 3;
				iface_peak_log(3);
			}
        }
        else if (temp_value == 4)
        {
            pSet(g_nMyPcm, 0, 1);
            pSet(g_nMyPcm, 1, 1);
            pSet(g_nMyPcm, 2, 1);        

			if ( g_nflag != 4 ) {
				g_nflag = 4;
				iface_peak_log(4);
			}
        }
        else
        {
            pSet(g_nMyPcm, 0, 0);
            pSet(g_nMyPcm, 1, 0);
            pSet(g_nMyPcm, 2, 0);
        } 
    } 

    return 1;                                
} 
#endif



void *iface_peak_main(void *arg)
{
	//int org_flags = 0;
	//int flags = 0;
	int nRetryCnt = 0;
    int nReturnFailCnt = 0;
   	int ret = 0;

   printf("\nTrail_PeakControl_Modbus_Eth_Client_Thread Started\n\n");
    
    int client_socket = -1;
    struct sockaddr_in server_addr_in;
    int server_connect = 0;

	time_t     tm_nd;
	struct tm *tm_ptr;

    
    //int i;
    //struct timespec ts;
    struct timeval tv;
    char server_ip[15] = "172.22.48.189";   //방화변전소 MOXA (Ethernet-485 Converter) IP
    //char server_ip[15] = "172.22.41.225"; //도봉변전소 MOXA (Ethernet-485 Converter) IP
    //char server_ip[15] = "192.168.2.3"; //Test Server
    int server_port = 4001;

    int nPrintCnt = 0; 

	int now_val = 0;
	int prev_val = 0;

    pSet(g_nMyPcm, 0, 0);
    pSet(g_nMyPcm, 1, 0);
    pSet(g_nMyPcm, 2, 0);
    
    if ( pGet(g_nMyPcm, 3) )
        pSet(g_nMyPcm, 3, 1);      // 통신상태 접점. (평상시 Off / 통신두절시 On)
    else 
        pSet(g_nMyPcm, 3, 0);      // 통신상태 접점. (평상시 Off / 통신두절시 On) 


	while( 1 ) {

		iface_peak_sleep(3, 0);
			
		now_val = pGet(g_nMyPcm, 3);
		if ( prev_val != now_val && now_val > 0 ) {
			prev_val = now_val;
			iface_peak_log(0);
		}
		else {
			prev_val = now_val;
		}

		time(&tm_nd);
		tm_ptr = localtime(&tm_nd);
		fprintf(stdout, " >> TIME  %d/%d %d:%d \n", tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min);
		fflush(stdout);	

		if (server_connect == 0) {   
            nPrintCnt = 0;
			
			pSet(g_nMyPcm, 3, 1);	
            
            client_socket = socket(AF_INET, SOCK_STREAM, 0);
            printf("Socket Num : [%d] \n", client_socket);
             
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &tv, sizeof(tv));
    
            memset(&server_addr_in, 0, sizeof(server_addr_in));
            server_addr_in.sin_family = AF_INET;
            server_addr_in.sin_addr.s_addr = inet_addr(server_ip);
            server_addr_in.sin_port = htons(server_port); 
    
            printf("Connecting to Modbus Server: %s, Port: %d \n", server_ip, server_port);
            
			//org_flags = fcntl(client_socket, F_GETFL, 0);
			//flags = org_flags | O_NONBLOCK;
			//fcntl(client_socket, F_SETFL, flags);			
			            
            if (connect(client_socket, (struct sockaddr *)&server_addr_in, sizeof(server_addr_in)) >= 0) {
                printf("Connected to Modbus Server: %s, Port: %d \n", server_ip, server_port);
                server_connect = 1;
                nRetryCnt = 0;
				
				pSet(g_nMyPcm, 3, 0);	

				//fcntl(client_socket, F_SETFL, org_flags);

				continue;
            }
            else {
				
				//fcntl(client_socket, F_SETFL, org_flags);

				printf("Connecting to Server is failed!!! \n");
                server_connect = 0;
                close(client_socket);

				//iface_peak_sleep(3, 0);

                nRetryCnt++;
                nReturnFailCnt = pGet(g_nMyPcm, 251);
                pSet(g_nMyPcm, 251, nReturnFailCnt + 1);
            }
        }
        else
        {   
            /*
            if(Set_Point_04(client_socket) == 0) {
                server_connect = 0;
                close(client_socket);
                //iPset(g_nMyPcm, 3, 1);      // 090529 Jong2ry Add. 통신상태 접점. (평상시 Off / 통신두절시 On)
                nRetryCnt++;
                nReturnFailCnt = pGet(g_nMyPcm, 250);
                pSet(g_nMyPcm, 250, nReturnFailCnt + 1);
            }
            else {
                printf("Keep Modbus Communication...\n");
                nRetryCnt = 0;
                pSet(g_nMyPcm, 3, 0);      // 090529 Jong2ry Add. 통신상태 접점. (평상시 Off / 통신두절시 On)
            } 
            */
            if ( iface_peak_send_msg(client_socket) < 0 ) {
                server_connect = 0;
                close(client_socket);
                nRetryCnt++;            	
            }
            else {
     			ret = iface_peak_recv_msg(client_socket);
     			if ( ret > 0 ){
	                printf("Keep Modbus Communication...\n");
	                nRetryCnt = 0;
	                pSet(g_nMyPcm, 3, 0);      // 090529 Jong2ry Add. 통신상태 접점. (평상시 Off / 통신두절시 On)     				
     				;	
     			}
     			else {
	                server_connect = 0;
	                close(client_socket);
	                nRetryCnt++;            	     				
     			}
            }	
        }

        // Retry가 6번이면 경보발생.
        if ( nRetryCnt > 20 ) {
			iface_peak_log(1);
			printf("System rebooting..\n");
			system( "reboot" );
        }
        else if( nRetryCnt > 5 ) { 
            pSet(g_nMyPcm, 3, 1);          // 090529 Jong2ry Add. 통신상태 접점. (평상시 Off / 통신두절시 On)
        }
        else if (nRetryCnt == 0) {
            pSet(g_nMyPcm, 3, 0);          // 090529 Jong2ry Add. 통신상태 접점. (평상시 Off / 통신두절시 On)
        }


        printf("nRetryCnt = %d\n", nRetryCnt);
	}
        
}
