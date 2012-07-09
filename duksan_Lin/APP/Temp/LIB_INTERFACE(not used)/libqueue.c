#include <stdio.h>
#include <stdlib.h>

#include "libqueue.h"

void initq(point_queue* queue)
{
	queue->front = 0;
	queue->rear = 0;
	memset(queue->data, 0, sizeof(queue->data));
	return;
}

int putq(point_queue* queue, point_info point) 
{ 
	int prev_rear;

	prev_rear = queue->rear;
	
	queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE; 
	
	if (queue->front == queue->rear) 
	{ 
		queue->rear = prev_rear;
		return queue_full();        
	} 
	
	memcpy(&queue->data[queue->rear], &point, sizeof(point_info));
	return SUCCESS;
}

int getq(point_queue* queue, point_info* point) 
{ 	

	if (queue->front == queue->rear) 
	{		
		return queue_empty(); 
	}
	queue->front = (queue->front + 1) % MAX_QUEUE_SIZE; 

	memcpy(point, &queue->data[queue->front], sizeof(point_info)); 
	//printf("in getq, pcm = %d, pno = %d\n", queue->data[queue->front].pcm, queue->data[queue->front].pno);
	return SUCCESS;
} 

int queue_full()
{
	//printf("Queue Full...\n");
	return QUEUE_FULL;
}

int queue_empty()
{
	//printf("Queue Empty...\n");
	return QUEUE_EMPTY;
}
