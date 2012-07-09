#ifndef QUEUE_H
#define QUEUE_H	

//#include "define.h"

//Queue Message
//#define QUEUE_FULL				2
//#define QUEUE_EMPTY				3

//Message Type In Queue
#define ELBA_REQUIRE			1
#define ELBA_REPORT				2
#define ELBA_COMMAND			3

#define OBSERVER_CHANGED		10

#define NET32_REQUIRE			20
#define NET32_COMMAND			21
#define NET32_BROADCAST			22
#define NET32_IF_PGET_REQUIRE 	23

#define UNKNOWN             	30
#define IF_PSET             	31
#define IF_PGET             	32
#define IF_PGET_RESPONSE    	33
#define IF_PGET_PLURAL			34
#define IF_ELBA_CMD_GET			35

#define CMD_PSET				41
#define CMD_PGET				42

#define BACNET_COMMAND			50
#define BACNET_REPORT			51


#endif
