#ifndef LIBINTERFACE_HANDLER_H
#define LIBINTERFACE_HANDLER_H

// define ============================================================
#define INIT_PROCESS					1
#define SELECT_PROCESS					2
#define CONNECTION_REQUESTED			3
#define HANDLE_COMMAND					4

//#define LIB_CMD_PGET					20
//#define LIB_CMD_PSET					21

// library command
#define LIB_CMD_READPOINT_1				30
#define LIB_CMD_READPOINT_2				31
#define LIB_CMD_SETPOINT				32

// library packet data define
#define LIB_CMD_STX						'<'
#define LIB_CMD_READ					'R'
#define LIB_CMD_WRITE					'W'
#define LIB_CMD_ETX						'>'

// part number for read-command
#define LIB_READ_PART_1_INDEX			0
#define LIB_READ_PART_2_INDEX			1

// return value
#define SUCCESS							1
#define FAIL							-1

// server port
#define LIBINTERFACE_SERVER_PORT		2070

// buffer size
#define MAX_BUFFER						4096

// pcm, pno maximun count
#define MAX_PCM							32
#define MAX_POINT						256

//
#define MAIN_GET_FILE_NAME              1
#define MAIN_FILE_READWRITE_OPEN        2
#define MAIN_FILE_CREATE                3
#define MAIN_FILE_READ                  4
#define MAIN_INCREASE_INDEX             5
#define MAIN_FINISH_INIT_PROCESS        6

//
#define MAX_NET32_NUMBER				32
#define MAX_POINT_NUMBER				256

// structs =========================================================
typedef struct _interface_tag_req_read
{
	unsigned char stx;
	unsigned short length;
	unsigned short cmd;
	unsigned short pcm;
	unsigned short part;
	unsigned short chksum;
	unsigned char etx;
}__attribute__((packed)) _REQ_READ_POINT_PACKET;

typedef struct _interface_tag_res_read
{
	unsigned char stx;
	unsigned short length;
	unsigned short cmd;
	unsigned short pcm;
	unsigned short part;
	float val[128];	
	unsigned short chksum;
	unsigned char etx;
}__attribute__((packed)) _RES_READ_POINT_PACKET;

typedef struct _interface_tag_req_write{
	unsigned char stx;
	unsigned short length;
	unsigned short cmd;
	unsigned short pcm;
	unsigned short pno;
	float val;	
	unsigned short chksum;
	unsigned char etx;
}__attribute__((packed)) _REQ_WRITE_POINT_PACKET;

typedef struct _interface_tag_res_write{
	unsigned char stx;
	unsigned short length;
	unsigned short cmd;
	unsigned short pcm;
	unsigned short pno;
	float val;	
	unsigned short chksum;
	unsigned char etx;
}__attribute__((packed)) _RES_WRITE_POINT_PACKET;

typedef struct _interface_tag_get_length{
	unsigned char stx;
	unsigned short length;
	unsigned short cmd;
}__attribute__((packed)) _RES_GET_LENGTH_PACKET;


// functions =========================================================
static int OpenLibInterfaceSocket(void);
static void ReleaseLibInterfaceSocket(void);
static void SendTxMsg(int length);
static void DoRecvHandler(void);
static void lib_test(void);
static unsigned short MakeChksum (unsigned char *p, int length);
static void MakePsetMsg(int pcm, int pno, float val);
static void MakePgetMsg(unsigned int pcm, int type);
float pget(int pcm, int pno);
void pset(int pcm, int pno, float val);
static int GetLibQueue(point_info *pPoint);
void InitUart2(int baudrate);
#endif
