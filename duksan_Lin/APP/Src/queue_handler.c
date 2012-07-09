/****************************************************************************** 
 * file : queue.c
 * author :	tykim
 * 
 * 
 * main.c에서 생성한 queue를 control 하는 funcions.
 * 
 * version :
 *		0.0.1 - tykim working.
 *  	0.0.2 - jong2ry code clean and change putq, getq argument2 type.
 *  	0.0.3 - file name change (queue.c -> queue_handler.c)
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
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/poll.h>										// use poll event

#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

#include "queue_handler.h"									// queue handler


/*
void initBacnetq(bacnet_queue* queue)
{
	queue->front = 0;
	queue->rear = 0;
	memset(queue->data, 0, sizeof(queue->data));
	return;
}
*/


void initq(point_queue *p)
// ----------------------------------------------------------------------------
// INITIALIZE QUEUE
// Description : It initialize queue.
// Arguments   : p				Is a queue pointer.
// Returns     : none
{
	p->front = 0;
	p->rear = 0;
	memset( p->data, 0, sizeof(p->data) );
	return;
}


int putq(point_queue* p1, point_info *p2) 
// ----------------------------------------------------------------------------
// DATA PUT QUEUE
// Description : It push data in queue
// Arguments   : p1				Is a queue pointer.
// 				 p2				Is a point-value pointer.
// Returns     : none
{ 
	int prev_rear;

	prev_rear = p1->rear;
	
	p1->rear = (p1->rear + 1) % MAX_QUEUE_SIZE; 
	
	if ( p1->front == p1->rear ) { 
		p1->rear = prev_rear;
		return queue_full();        
	} 
	
	memcpy( &p1->data[p1->rear], p2, sizeof(point_info) );
	return SUCCESS;
}


int getq(point_queue *p1, point_info *p2) 
// ----------------------------------------------------------------------------
// DATA GET QUEUE
// Description : It pop data in queue
// Arguments   : p1				Is a queue pointer.
// 				 p2				Is a point-value pointer.
// Returns     : none
{ 	
	if ( p1->front == p1->rear ) {		
		return queue_empty(); 
	}

	p1->front = (p1->front + 1) % MAX_QUEUE_SIZE ; 

	memcpy( p2, &p1->data[p1->front], sizeof(point_info) ); 
	return SUCCESS;
} 


int queue_full(void)
// ----------------------------------------------------------------------------
// RETUREN QUEUE FULL 
// Description : return QUEUE_FULL
// Arguments   : none
// Returns     : QUEUE_FULL
{
	return QUEUE_FULL;
}


int queue_empty(void)
// ----------------------------------------------------------------------------
// RETUREN QUEUE EMPTY 
// Description : return QUEUE_EMPTY
// Arguments   : none
// Returns     : QUEUE_EMPTY
{
	return QUEUE_EMPTY;
}
