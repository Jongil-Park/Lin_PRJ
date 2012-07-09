/****************************************************************************** 
 * file    : elba.c
 * author  : tykim
 * 
 * 
 * TCP/IP 를 통한 Application과의 data 통신 
 *  	1. elba protocol에 의한 data 처리.
 *  	2. 3istation 및 3imini에 대해 호환성 유지.
 * 
 * version :
 *		0.0.1  tykim working.
 *		0.0.2  jong2ry code clean.
 * 
******************************************************************************/

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

#include "define.h"
#include "queue.h"
#include "elba.h"
#include "net32.h"

const char gc_elba_head[5] = "{HEAD";
const char gc_elba_tail[5] = "TAIL}";
unsigned char gc_elba_tx_msg[MAX_BUFFER_SIZE];
unsigned char gc_elba_rx_msg[MAX_BUFFER_SIZE];
unsigned char gc_temp_rx_msg[MAX_BUFFER_SIZE];

// debug variable
int gc_dbg_txmsg = 0;	
int gc_dbg_rxmsg = 0;
int gc_view_elba = 0;

extern pthread_mutex_t elbaQ_mutex;
extern pthread_mutex_t elbaMessageQ_mutex;
extern pthread_mutex_t plcQ_mutex;

extern point_queue elba_queue;
extern point_queue elba_message_queue;
extern point_queue plc_message_queue;

//extern queue_status status;
extern int multi_ddc;
extern float point_table[MAX_NET32_NUMBER][MAX_POINT_NUMBER];
extern unsigned int g_iChkControl;
extern CHK_STATUS_T *gp_status;										// gp_status


/*
********************************************************************* 
* START A WAIT_TIMER
* 
* Description	: This function is called by Elba_Thread to create wait-timer.
* 
* 
* Arguments		: sec			Is a second data.
* 
* 				  usec			Is a micro-second data. 
* 
* 
* Returns		: none
* 
********************************************************************* 
*/
void elba_sleep(int sec, int usec) 
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = usec;
    select( 0, NULL, NULL, NULL, &tv );
	return;
}


void elba_put_queue(point_info *p)
{
	STATUS_QUEUE_T *p_elba_q = (STATUS_QUEUE_T *) &gp_status->st_elba;

	pthread_mutex_lock( &elbaQ_mutex );

	putq( &elba_queue, p );

	// status change
	p_elba_q->n_indata++;

	pthread_mutex_unlock( &elbaQ_mutex );	
}


void elba_put_msgqueue(point_info *p)
{
	STATUS_QUEUE_T *p_elbamsg_q = (STATUS_QUEUE_T *) &gp_status->st_elba;

	pthread_mutex_lock( &elbaMessageQ_mutex );

	putq( &elba_message_queue, p );

	// status change
	p_elbamsg_q->n_indata++;

	pthread_mutex_unlock( &elbaMessageQ_mutex );	
}



int elba_get_queue(point_info *p)
{
	int n_ret;
	STATUS_QUEUE_T *p_elba_q = (STATUS_QUEUE_T *) &gp_status->st_elba;

	memset( p, 0, sizeof(point_info) );
	pthread_mutex_lock( &elbaQ_mutex );

	n_ret = getq( &elba_queue, p );

	// status change
	if ( n_ret == QUEUE_FULL ) {
		p_elba_q->n_full++;
	}
	else if ( n_ret == QUEUE_EMPTY ) {
		p_elba_q->n_empty++;
	}
	else if ( n_ret == SUCCESS ) {
		p_elba_q->n_outdata++;
	}

	pthread_mutex_unlock( &elbaQ_mutex );
	
	return n_ret;
}


int elba_get_msgqueue(point_info *p)
{
	int n_ret;
	STATUS_QUEUE_T *p_elbamsg_q = (STATUS_QUEUE_T *) &gp_status->st_elba;

	memset( p, 0, sizeof(point_info) );
	pthread_mutex_lock( &elbaMessageQ_mutex );

	n_ret = getq( &elba_message_queue, p );

	// status change
	if ( n_ret == QUEUE_FULL ) {
		p_elbamsg_q->n_full++;
	}
	else if ( n_ret == QUEUE_EMPTY ) {
		p_elbamsg_q->n_empty++;
	}
	else if ( n_ret == SUCCESS ) {
		p_elbamsg_q->n_outdata++;
	}

	pthread_mutex_unlock( &elbaMessageQ_mutex );
	
	return n_ret;
}


/*
********************************************************************* 
* GET A 3ISTATION IP ADDRESS
* 
* Description	: This function is called by new_elba() to get 3iStation ipaddress.
* 
* 
* Arguments		: stationip		Is a pointer to get 3iStation ipaddress
* 
* 
* Returns		: SUCCESS		If the call was successful
* 
* 				  ERROR			If not
* 
********************************************************************* 
*/
int get_stationIP(char* stationip)
{
	FILE* fp = NULL;
	int cnt = 0;
	cmdinfo gc_argument[CFG_COMMAND_COUNT];

	memset( gc_argument, 0, sizeof(gc_argument) );
	
	// search file and copy StationIP.
	if( (fp = fopen( "/duksan/CONFIG/config.dat", "r" )) == NULL ) {
		fclose( fp );
		fprintf( stderr, "[ELBA ERROR1] /duksan/CONFIG/config.dat not opened \n" );
		fflush( stderr );
		return ERROR;
	} 
	else {
		while( !feof(fp) ) {
			fscanf( fp, 
					"%s %s\n", 
					(char *)&gc_argument[cnt].name, (char *)&gc_argument[cnt].value );
				
			if ( strncmp( (char *)&gc_argument[cnt].name, "stationip", 5 ) == 0 ) {
				memcpy( stationip, (char *)&gc_argument[cnt].value, CFG_VALUE_SIZE );
				fclose( fp );		
				return SUCCESS;	
			}
			else
				cnt++;
		}				
	}

	fprintf( stderr, "[ELBA ERROR2] StationIP not found \n" );
	fflush( stderr );
	fclose( fp );
	return ERROR;		
}


/*
********************************************************************* 
* MAKE MESSAGE THAT SEND TO 3ISTATION
* 
* Description	: This function is called by new_elba() to get 3iStation ipaddress.
* 
* 
* Arguments		: p				Is a pointer to send transmit message
* 
* 				  point			Is a structure to send value of point table
* 
*				   
* Returns		: none
* 
********************************************************************* 
*/
void make_elba_tx_msg(unsigned char *p, point_info point)
{
	unsigned char total_length;
	float value;

	total_length = 0x19;
	value = change_byte_order(point.value);

	memcpy(p + 0,  gc_elba_head,			 5);
	memset(p + 5,  0x0D,     		 1);
	memset(p + 6,  0xfe,          	 1);
	memset(p + 7,  0xfe,          	 1);
	memset(p + 8,  0x20,          	 1);
	memset(p + 9,  0x0C,          	 1);
	memset(p + 10, 0xfe,          	 1);
	memset(p + 11, 0xfe,          	 1);
	memcpy(p + 12, &point.pcm,    	 1);
	memcpy(p + 13, &point.pno,    	 1);
	memcpy(p + 14, &value,		 	 4);
	memcpy(p + 19, &total_length, 	 1);
	memcpy(p + 20, gc_elba_tail,		 	 5);
	return;
}

/****************************************************************/
float change_byte_order(float fvalue)
/****************************************************************/
{
	unsigned char* p1;
	unsigned char* p2;
	float return_val;
	
	p1 = (char*) &fvalue;
	p2 = (char*) &return_val;
	
	p2[0] = p1[3];
	p2[1] = p1[2];
	p2[2] = p1[1];
	p2[3] = p1[0];

	return return_val;
}
	

/*
********************************************************************* 
* INSERT A DATA INTO THE ELBA_QUEUE
* 
* Description	: This function is called to insert the data into the Elba_Queue.
* 
* 
* Arguments		: pPoint			Is a pointer to the data to insert. 
* 
* 
* Returns		: none
* 
********************************************************************* 
*/
/*
void putElbaQ(point_info *pPoint)
{
	pthread_mutex_lock(&elbaQ_mutex);
	putq(&elba_queue, *pPoint);
	pthread_mutex_unlock(&elbaQ_mutex);
}

*/

/*
********************************************************************* 
* SEND A DATA TO THE 3ISTATION
* 
* Description	: This function is called to send the data to 3iStation.
* 
* 
* Arguments		: pElba			Is a pointer to the ELBA_T structure. 
* 
* 
* Returns		: tx_msg_ptr	Is value to return send() function.
* 
********************************************************************* 
*/
int elba_send(void *pElba)
{
	int i = 0;
	point_info point;
	int result = 0;
	int tx_msg_ptr = 0;
	unsigned char *pBuf;
	int iTempBufSize = 0;
	unsigned char temp_msg[25];
	
	ELBA_T *p = (ELBA_T *) pElba;
	STATUS_THREAD_T *p_elba_t = (STATUS_THREAD_T *) &gp_status->st_elba_t;
	
	pBuf = p->txbuf;
	iTempBufSize = sizeof(temp_msg);
	memset(p->txbuf, 0, MAX_BUFFER_SIZE);

	for(i = 0; i < ELBA_TX_MESSAGE_COUNT; i++) {
		result = elba_get_queue( &point );

		// check status
		p_elba_t->n_chkqueue++;

		if(result != SUCCESS) 
			break;
	
		//status.elba_out++;
		
		make_elba_tx_msg(temp_msg, point);
		memcpy( (pBuf + tx_msg_ptr) , temp_msg, iTempBufSize);
		tx_msg_ptr = tx_msg_ptr + iTempBufSize;		
	}
	
	//if there's no data to send
	if(tx_msg_ptr == 0)
		return tx_msg_ptr;
	
	if(send(p->fd, pBuf, tx_msg_ptr, 0) < 0) 
		return -1;
	
	// status change
	p_elba_t->n_tx += tx_msg_ptr;
	
	if( gc_dbg_txmsg ) {
		fprintf(stderr, "TxD(%d) : \n", tx_msg_ptr);
		for(i = 1; i < tx_msg_ptr + 1; i++) {
			fprintf(stderr, "0x%02x ", pBuf[i-1]);
			if((i % 25) == 0) {
				fprintf(stderr, "\n");
				fflush(stderr);
			}
		}
		fprintf(stderr, "\n");
		fflush(stderr);
	}
	//status.tcp_total_tx += tx_msg_ptr;
	
	return tx_msg_ptr;
}


unsigned char chksum8(void *p, size_t len)
{
	unsigned char sum, *bp = p;

	sum = 0;
	while ( len-- )
		sum -= *bp++;
	return	sum;
}


void elba_send_time(void *pElba)
{
	int i = 0;
	unsigned char *pBuf;
	unsigned char ctotal_length;
	time_t the_time;
	struct tm *pTm;
	STATUS_THREAD_T *p_elba_t = (STATUS_THREAD_T *) &gp_status->st_elba_t;

	time(&the_time);
	pTm = localtime(&the_time);

	/*
    fprintf(stderr, "%d/%d/%d %d:%d:%d\n",
                    pTm->tm_year+1900,
                    pTm->tm_mon+1,
                    pTm->tm_mday,
                    pTm->tm_hour,
                    pTm->tm_min,
                    pTm->tm_sec);
	fflush(stderr);
	*/

	ELBA_T *p = (ELBA_T *) pElba;
	pBuf = p->txbuf;
	memset(p->txbuf, 0, MAX_BUFFER_SIZE);
	
	ctotal_length = 0x19;

	memcpy(pBuf + 0,  gc_elba_head,			 5);
	memset(pBuf + 5,  0x0f,    		 1);
	memset(pBuf + 6,  0xfe,        	 1);
	memset(pBuf + 7,  0xfe,        	 1);
	memset(pBuf + 8,  0x20,        	 1);
	memset(pBuf + 9,  0x0C,        	 1);
	memset(pBuf + 10, 0xfe,        	 1);
	memset(pBuf + 11, 0xfe,        	 1);
	pBuf[12] = 2000 - (pTm->tm_year+1900);
	pBuf[13] = pTm->tm_mon+1;
	pBuf[14] = pTm->tm_mday;
	pBuf[15] = pTm->tm_hour;
	pBuf[16] = pTm->tm_min;
	pBuf[17] = pTm->tm_wday;
	pBuf[18] = chksum8(&pBuf[4], pBuf[4]);
	memcpy(pBuf + 19, &ctotal_length, 	 1);
	memcpy(pBuf + 20, gc_elba_tail,		 	 5);

	if( send(p->fd, pBuf, ctotal_length, 0) < 0 ) 
		return;

	// status change
	p_elba_t->n_tx += ctotal_length;
	
	if( gc_dbg_txmsg ) {
		fprintf(stderr, "TxD Time (%d) : \n", ctotal_length);
		for( i = 0; i < ctotal_length; i++ ) 
			fprintf(stderr, "0x%02x ", pBuf[i]);
		fprintf(stderr, "\n");
		fflush(stderr);
	}
	//status.tcp_total_tx += ctotal_length;
}

/*
********************************************************************* 
* RECEIVE A DATA FROM 3ISTATION
* 
* Description	: This function is called to receive the data from 3iStation.
* 
* 
* Arguments		: pElba			Is a pointer to the ELBA_T structure. 
* 
* 
* Returns		: recv_length	Is length to receive the data from 3iStation
* 
********************************************************************* 
*/
int elba_recv(void *pElba)
{
	int i = 0;	
	int recv_length = 0;
	int temp_length = 0;
	unsigned char *pBuf;
	
	ELBA_T *p = (ELBA_T *)pElba;
	STATUS_THREAD_T *p_elba_t = (STATUS_THREAD_T *) &gp_status->st_elba_t;

	pBuf = p->rxbuf;

	/*
	3iS로부터 data를 받는다.
	*/	
	recv_length = recv(p->fd, pBuf, sizeof(gc_elba_rx_msg), 0);
	
	/*
	data의 리턴값이 0이나 0보다 작으면 
	socket이 끊어지거나 data가 없는 것이므로
	recv_length을 리턴한다.
	*/
	if (recv_length <= 0)
		return recv_length;

	// status change
	p_elba_t->n_rx += recv_length;

	for (;;) {
		/*
		임시로 data를 더 받는다.
		*/
		temp_length = recv(p->fd, 
							pBuf + recv_length, 
							MAX_BUFFER_SIZE - recv_length, 
							0);

		/*
		임시로 받은 data의 리턴값를 확인한다. 
		만약 0이나 0보다 작으면 socket이 끊어지거나 data가 없는 것이므로
		그동안 받은  모든 data의 길이를 리턴한다.
		*/		
		if (temp_length <= 0) {
			if ( gc_dbg_rxmsg ) {
				printf("RxD(%d) : \n", recv_length);
				for(i = 0; i < recv_length; i++)
				{
					printf("0x%02x ", pBuf[i]);
					if (gc_elba_rx_msg[i-1] == 0x4c && pBuf[i] == 0x7d)
						printf("\n");
				}
			}	
			
			//status.tcp_total_rx += recv_length;
			return recv_length;	
		}
		
		/*
		받은 모든 data의 길이를 계산한다.
		*/	
		recv_length = recv_length + temp_length;					

		/*
		한번에 TCP로 받을 수 있는 최대 byte 수는 1500 Byte이다.  
		그러므로 1500 byte의 buffer의 공간을 확보한 후에 
		receive를 하도록 해야 한다.
		*/			
		if ( (MAX_BUFFER_SIZE - recv_length) < 1500 ) {
			return recv_length;	
		}
		else
			continue;
	}
}

/****************************************************************/
void elba_handler(void *pElba) 
/****************************************************************/
{
	int iWp = 0;
	int index = 0;
	int iLengthChk;
	point_info point;
	int recv_status = 0;
	unsigned char *pBuf;
    //struct tm tm_ptr;
    //time_t m_time;
	struct timeval set_time;
	time_t     tm_st;
	time_t     tm_nd;
	//int        tm_day, tm_hour, tm_min, tm_sec;
	double     d_diff;
	struct tm  user_stime;
	
	ELBA_T *p = (ELBA_T *)pElba;
	
	iWp = p->tempWp;
	pBuf = p->tempbuf;
	recv_status = IN_LENGTH_CHECK;

	for (;;) {

		switch(recv_status) {
			
			case IN_LENGTH_CHECK:
				//printf("DEBUG : In Length check\n");
				//p->tempWp = iWp;
				iLengthChk = iWp - index;
				if ( iLengthChk >= 21 ) {
					recv_status = IN_HEAD;	
					//printf("1 iWp = %d, index = %d\n", iWp, index);
					break;
				}
				else if ( iLengthChk <= 0 ) {
					p->tempWp = 0;
					memset(pBuf, 0, MAX_BUFFER_SIZE);
					//printf("2 iWp = %d, index = %d\n", iWp, index);
					return;
				}
				else {
					memcpy (pBuf, (pBuf + index), iLengthChk);	
					p->tempWp = iLengthChk;
					//printf("4 iWp = %d, index = %d\n", iWp, index);
					return;
				}
				break;
				
			case IN_HEAD:
				if( memcmp(gc_elba_head, pBuf + index , sizeof(gc_elba_head)) == 0 ) {
					recv_status = IN_DATA_TYPE;
					index = index + 8;
				}
				else {
					printf("[ERROR3] Unidentified Header In Elba\n");
					index++;
					continue;
				}			
				break;
			

			case IN_DATA_TYPE:
				// it is GCU timeSync protocol.
				if( pBuf[5] == 0x0F ) {
					index = index + 12;
					recv_status = IN_TAIL;
					
					if ( pBuf[8] == 0x00 ) {
						p->ntime_request = ELBA_TIME_REQUEST;
						//fprintf( stderr, "Time Sync \n" );
						//fflush( stderr);
					}
					else if ( pBuf[8] == 0x40 ) {

						// 기준이 되는 시간. 1970-01-01 00:00
						user_stime.tm_year   = 1970 - 1900;   // 주의 :년도는 1900년부터 시작
						user_stime.tm_mon    = 1      -1;      // 주의 :월은 0부터 시작
						user_stime.tm_mday   = 1;
						user_stime.tm_hour   = 0;
						user_stime.tm_min    = 0;
						user_stime.tm_sec    = 0;
						user_stime.tm_isdst  = 0;              // 썸머 타임 사용 안함
						tm_st = mktime( &user_stime );
						
						user_stime.tm_year   = (2000 + pBuf[12])  -1900;   // 주의 :년도는 1900년부터 시작
						user_stime.tm_mon    = (int)pBuf[13] - 1;      // 주의 :월은 0부터 시작
						user_stime.tm_mday   = (int)pBuf[14];
						user_stime.tm_hour   = (int)pBuf[15];
						user_stime.tm_min    = (int)pBuf[16];
						user_stime.tm_sec    = 0;
						user_stime.tm_isdst  = 0;              // 썸머 타임 사용 안함
						tm_nd = mktime( &user_stime );	
						
						d_diff   = difftime( tm_nd, tm_st );
						fprintf( stderr, "[Time Set] d_diff %f \n",	d_diff );
						gettimeofday( &set_time, NULL );
						set_time.tv_sec = (long)d_diff;
						fprintf( stderr, "[Time Set] set_time.tv_sec %ld \n", set_time.tv_sec );
						settimeofday( &set_time, NULL );
						fprintf( stderr, "[Time Set] %d-%d-%d %d:%d \n", 
								 pBuf[12], pBuf[13], pBuf[14], pBuf[15], pBuf[16] );
						fflush( stderr);

						system( "hwclock --systohc" );
					}
					break;
				}
				// it is request protocol in elba.
				else if( *(pBuf+index) == 0x00) {
					recv_status = IN_REQUIRE;
					index = index + 4;
					break;
				}
				// it is command protocol in elba.
				else if( *(pBuf+index) == 0x40) {
					recv_status = IN_COMMAND;
					index = index + 4;
					break;
				}
				// Error	
				else {
					printf("[ERROR4] Unidentified Data type In Elba\n");
					p->tempWp = 0;
					memset(pBuf, 0, MAX_BUFFER_SIZE);
					return;
				}
				break;
			
			case IN_REQUIRE:
				//printf("DEBUG : In IN_REQUIRE\n");
				//memcpy(&point.pcm, (pBuf+index), sizeof(char));
				point.pcm = *(pBuf+index);
				index++;
				
				//memcpy(&point.pno, (pBuf+index), sizeof(char));
				point.pno = *(pBuf+index);
				index = index + 3;

				point.value = point_table[point.pcm][point.pno];	

				//printf("iWp = %d, index = %d\n", iWp, index);
				//printf("DEBUG : putQ to elba_message_queue(ELBA_REQUIRE) pcm = %d pno = %d\n", point.pcm, point.pno);

				/*
				point_table의 값은 항상 update된 값을 가지고 있는 DB이기 때문에
				point_table의 값을 바로 올려보내줘도 괜찮을 것이라고 생각한다.
				*/
				/*
				만약 net32가 연결되어 있다면, 
				Net32는 느린 Device이기 때문에 먼저 3iS에서 요청을 처리하고
				Queue를 사용해서 새로운 값을 갱신하도록 한다.
				그러므로 Net32가 연결되어 있다면, 
				3iS의 요청에 의해 2번의 응답을 보내게 되는 것이다.
				*/
				elba_put_queue(&point);
				if ( !multi_ddc) {
					point.message_type = NET32_TYPE_REQUIRE;
					net32_put_msgqueue(&point);		// send to net32_message_queue

					if ( gc_view_elba ) {
						fprintf(stderr, "\nRequest %d, %d  ", point.pcm, point.pno);
						fflush(stderr);
					}
				}
				recv_status = IN_TAIL;
				break;

			case IN_COMMAND:
				//printf("DEBUG : In IN_COMMAND\n");
				//memcpy(&point.pcm, (pBuf + index), sizeof(char));
				point.pcm = *(pBuf+index);
				index++;

				//memcpy(&point.pno, (pBuf + index), sizeof(char));
				point.pno = *(pBuf+index);
				index++;

				memcpy(&point.value, (pBuf + index), sizeof(float));
				index = index + 6;
				point.value = change_byte_order(point.value);

				// Interface for PLC
				pthread_mutex_lock( &plcQ_mutex );
				putq( &plc_message_queue, &point );
				pthread_mutex_unlock( &plcQ_mutex );	
				g_iChkControl = 1;
				//printf("Elba g_iChkControl = %d\n", g_iChkControl);				
				//get_3iStoPlcData();

				/*
				만약 Net32를 사용한다면, 
				point_table의 값을 갱신하지 않고 Net32로 data를 보낸다.
				하지만 Net32를 사용하지 않는 경우에는, 
				point_table의 값을 갱신하고 그 값을 3iS로 보낸다.
				*/				
				if ( multi_ddc )	{
					point_table[point.pcm][point.pno] = point.value;
					elba_put_queue(&point);								
				}
				else {
					point.message_type = NET32_TYPE_COMMAND;
					net32_put_msgqueue(&point);		// send to net32_message_queue

					if ( gc_view_elba ) {
						fprintf(stderr, "\nCommand %d, %d  ", point.pcm, point.pno);
						fflush(stderr);
					}
				}
				//printf("DEBUG : putQ to elba_message_queue(ELBA_COMMAND) pcm = %d pno = %d\n", point.pcm, point.pno);
				recv_status = IN_TAIL;
				break;
		
			case IN_TAIL:
				//fprintf( stderr, "In Tail index = %d\n", index );
				//fflush( stderr);
				elba_sleep( 0, 100 );

				if(memcmp(gc_elba_tail, (pBuf + index), sizeof(gc_elba_tail)) == 0) {
					index = index + 5;
					//printf("iWp = %d, index = %d\n", iWp, index);
					recv_status = IN_LENGTH_CHECK;
					break;
				}
				else {
					//fprintf( stderr, "%x " , pBuf[index] );
					//fprintf( stderr, "%x " , pBuf[index+1] );
					//fprintf( stderr, "%x " , pBuf[index+2] );
					//fprintf( stderr, "%x " , pBuf[index+3] );
					//fprintf( stderr, "%x " , pBuf[index+4] );
					//fprintf( stderr, "[ERROR5] Unidentified Tail in Elba\n"  );
					//fflush( stderr);
					p->tempWp = 0;
					memset(pBuf, 0, MAX_BUFFER_SIZE);
					return;
				}				
			break;			
		}
	}
}


/****************************************************************/
void elba_relocate_data(void *pElba) 
/****************************************************************/
{
	int iWp = 0;
	int iCopySize = 0;
	int iRecvLength = 0;
	unsigned char *pBuf;
	unsigned char *pRecvBuf;
	ELBA_T *p = (ELBA_T *)pElba;
	
	pBuf = p->tempbuf;
	pRecvBuf = p->rxbuf;
	iWp = p->tempWp;
	iRecvLength = p->recvLength;
	iCopySize = p->bufSize - p->tempWp;
	
	memcpy ( (pBuf + iWp), pRecvBuf, iCopySize );
	p->tempWp = p->tempWp + p->recvLength;
}


/*
********************************************************************* 
* CREATE THE ELBA_T STRUTURE 
* 
* Description	: This function is called by elba_main().
* 					- create ELBA_T structure
* 					- initialize variable of ELBA_T structure
* 
* 
* Arguments		: none
* 
*  
* Returns		: sock		Is pointer of ELBA_T structure
* 
********************************************************************* 
*/
ELBA_T *new_elba(void)
{
	ELBA_T *sock;
	
	sock = (ELBA_T *)malloc( sizeof(ELBA_T) );
	
	sock->fd = -1;
	sock->rxbuf = (unsigned char *)&gc_elba_rx_msg;
	sock->txbuf = (unsigned char *)&gc_elba_tx_msg;
	sock->tempbuf = (unsigned char *)&gc_temp_rx_msg;
	sock->tempWp = 0;
	sock->recvLength = 0;
	sock->ntime_request = ELBA_TIME_WAIT;
	sock->bufSize = MAX_BUFFER_SIZE;
	
	if ( get_stationIP(sock->targetIp) == ERROR ) {
		fprintf(stderr, "Can't get stationIP from config.dat file.\nPlease check config.dat file\n");
		fflush(stderr);
		return NULL;
	}

	return sock;
}

/*
********************************************************************* 
* CLOSE THE ELBA_T STRUTURE 
* 
* Description	: This function is called by elba_main().
* 					- close socket and initialize variable of ELBA_T structure
* 
* 
* Arguments		: p			Is a pointer to the ELBA_T structure.
* 
*  
* Returns		: 1			If the call was successful
*              	  -1		If not
* 
********************************************************************* 
*/
void elba_close(ELBA_T *p)
{
	STATUS_THREAD_T *p_elba_t = (STATUS_THREAD_T *) &gp_status->st_elba_t;
	
	// status change
	p_elba_t->n_connetion = 0;

	close(p->fd);

	p->fd = -1;
	p->tempWp = 0;
	p->recvLength = 0;

	memset( p->tempbuf, 0, MAX_BUFFER_SIZE );
	memset( p->rxbuf, 0, MAX_BUFFER_SIZE );
	memset( p->txbuf, 0, MAX_BUFFER_SIZE );
}


/*
********************************************************************* 
* OPEN THE ELBA_T STRUTURE 
* 
* Description	: This function is called by elba_main().
* 					- create socket and connect 3iStation
* 
* 
* Arguments		: p			Is a pointer to the ELBA_T structure.
* 
*  
* Returns		: 1			If the call was successful
*              	  -1		If not
* 
********************************************************************* 
*/
int elba_open(ELBA_T *p)
{
	struct sockaddr_in server_addr;
	struct timeval timeo;
	STATUS_THREAD_T *p_elba_t = (STATUS_THREAD_T *) &gp_status->st_elba_t;
		
	memset( &server_addr, 0, sizeof(server_addr) );
	timeo.tv_sec = 0;
	timeo.tv_usec = 10000;
	
	p->fd = socket( AF_INET, SOCK_STREAM, 0 );
	if( p->fd < 0 ) {
		fprintf( stderr, "+ Socket creation error\n" );
		fflush( stderr );
		close( p->fd );
		return -1;					
	}

	setsockopt( p->fd, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo) );

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr( p->targetIp );
	server_addr.sin_port = htons( SERVER_PORT );

	if ( connect(p->fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 ) {
		return -1;
	}
	else {
		// status change
		p_elba_t->n_connetion = 1;

		fprintf( stderr, "+ Connect to 3iStation %s\n", p->targetIp );
		fflush( stderr );
		return 1;
	}	
}

/*
********************************************************************* 
* ELBA_THREAD MAIN LOOP 
* 
* Description	: This function is called by main().
* 					- create ELBA_T structure
* 					- call elba_open() and create socket
* 					- receive data from 3iStation and parsing data.
* 					- data send to 3iStation.
* 
* 
* Arguments		: none.
* 
*  
* Returns		: none.
* 
********************************************************************* 
*/
void *elba_main(void* arg)
{
	ELBA_T *elba;
	STATUS_THREAD_T *p_elba_t = (STATUS_THREAD_T *) &gp_status->st_elba_t;

	elba = new_elba();
	if ( elba == NULL ) {
		fprintf( stderr, "Can't create ELBA.\n" );
		fflush( stderr );
		exit(1);
	}

	while (1) {
		if ( elba_open( elba ) < 0 ) {
			elba_sleep( 3, 0 );
			elba_close( elba );
			continue;	
		}
		
		for(;;) {
			// status change
			p_elba_t->n_loopcnt++;

			elba_send( elba );
	
			if ( elba->ntime_request == ELBA_TIME_REQUEST ) {
				elba_send_time( elba );
				elba->ntime_request = ELBA_TIME_RESPONSE;
			}

			elba->recvLength = elba_recv( elba );
			if ( elba->recvLength == SOCKET_CLOSE ) {
				fprintf( stderr, "lost host server\n" );
				fflush( stderr );
				elba_sleep( 3, 0 );
				elba_close( elba );				
				break;				
			}
			else if ( elba->recvLength > 0 ) {
				elba_relocate_data( elba );	
				elba_sleep( 0, 500 );	// must have delay.
				elba_handler( elba );
				elba_sleep( 0, 500 );	// must have delay.
			}
			else {
				continue;
			}
			continue;
		}
	}
	exit(1);
}



