#ifndef LIB_QUEUE_H
#define LIB_QUEUE_H	

// define ============================================================
//Queue Message
#define QUEUE_FULL				2
#define QUEUE_EMPTY				3
//
//Message Type In Queue
#define ELBA_REQUIRE			1
#define ELBA_REPORT				2
#define ELBA_COMMAND			3
//
#define OBSERVER_CHANGED		10
//
#define NET32_REQUIRE			20
#define NET32_COMMAND			21
#define NET32_BROADCAST			22
#define NET32_IF_PGET_REQUIRE 	23
//
#define UNKNOWN             	30
#define IF_PSET             	31
#define IF_PGET             	32
#define IF_PGET_RESPONSE    	33
#define IF_PGET_PLURAL			34
#define IF_ELBA_CMD_GET			35
//
#define CMD_PSET				41
#define CMD_PGET				42
//
#define LIB_IF_PSET				50
//
#define MAX_QUEUE_SIZE			2048
//
#define SUCCESS					1
#define ERROR				   -1
//
#define TRUE					1
#define FALSE					0
//

// structs =========================================================
typedef struct 
{
	short	pcm;
	short	pno;
	float	value;
	float	pre_value;			// add jong2ry.
	int	message_type;
} point_info ;

typedef struct
{
	int front;  //front index of queue
	int rear;	//rear index of queue
	point_info data[MAX_QUEUE_SIZE];
} point_queue;


// functions =========================================================
void initq(point_queue* queue);
int putq(point_queue* queue, point_info point);
int getq(point_queue* queue, point_info* point);
int queue_full();
int queue_empty();

#endif
