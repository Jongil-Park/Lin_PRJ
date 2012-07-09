#ifndef IFACE_HANDLER_H
#define IFACE_HANDLER_H

// define ============================================================
#define INIT_PROCESS					1
#define SELECT_PROCESS					2
#define CONNECTION_REQUESTED			3
#define HANDLE_COMMAND					4

//#define GET_COMMAND_TYPE				11
//#define COMMAND_PSET					12
//#define COMMAND_PSET_SUCCESS			13
//#define COMMAND_PGET_PLURAL				14
//#define COMMAND_PGET_SUCCESS			15

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
/*
void *iface_handler_main(void* arg);
static int DoLibInterfaceHandler(int fd, fd_set* reads);
static unsigned short MakeChksum (unsigned char *p, int length);
static void DoReadCommand(int fd, unsigned char *pData);
static void DoWriteCommand(int fd, unsigned char *pData);
*/
#endif
