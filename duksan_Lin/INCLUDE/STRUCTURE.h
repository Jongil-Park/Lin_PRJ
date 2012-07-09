#ifndef STRUCTURE_H
#define STRUCTURE_H	


////////////////////////////////////////////////////////////////////////////////
// Net32 Interface (NET32 protocol)
// net32 token message type
typedef struct {
	unsigned char cHeader;
	unsigned char cCmd;
	unsigned char cLength;
	unsigned char cDbgNoq;
	unsigned char cDbgNiq;
	unsigned char cChkSum;
} __attribute__ ((packed)) NET32_TOKEN_T;

// net32 basic message type
typedef struct {
	unsigned char cHeader;
	unsigned char cCmd;
	unsigned char cLength;
	unsigned char cDbgNoq;
	unsigned char cDbgNiq;
	unsigned char cPcm;
	unsigned char cPno;
	unsigned char cVal[4];
	unsigned char cChkSum;
} __attribute__ ((packed)) NET32_MSG_T;

// net32 thread
typedef struct {
	int 				nFd;
	unsigned char 		*pRxBuf;
	unsigned char 		*pTxBuf;
	int 				nRecvByte;
} __attribute__ ((packed)) NET32_T;

// net32 status
typedef struct {
	unsigned int nCountNode;
	unsigned int nCountNiq;
	unsigned int nCountNoq;
} NET32_STATUS_T;


////////////////////////////////////////////////////////////////////////////////
// 3iStation Interface (Elba protocol)
//elba thread
typedef struct {
	int nFd;
	unsigned char *bTxBuf;
	unsigned char *bRxBuf;
	unsigned char *bPrevBuf;
	int nRecvLength;
	int nTempWp;
	int nIndexPrev;
	int nTimeRequest;	
	char chServerIp[CFG_VALUE_SIZE];
}ELBA_T;

//elba status
typedef struct {
	unsigned int nStatus;
	unsigned int nNtx;
	unsigned int nNrx;
} _ELBA_STATUS_T;


////////////////////////////////////////////////////////////////////////////////
// Point Drvier Interface 
typedef struct
{
	unsigned char c_pno;
	unsigned char c_factoryReset;	
	unsigned char c_type;
	unsigned char c_length;
	unsigned int  n_val;
	unsigned char c_chksum;
}__attribute__((packed)) PNT_DEV_MSG_T;

// point thread
typedef struct
{
	int n_dev;
	unsigned char *p_rx;
	unsigned char *p_tx;
	unsigned short w_pno;
	unsigned short w_type;
}__attribute__((packed)) PNT_HANDLER_T;


////////////////////////////////////////////////////////////////////////////////
// DBSVR Interface
// DBSVR data  structure
typedef struct {
	BYTE stx;
	WORD func;
	WORD length;
	WORD pcm;
	//WORD value[256];
	float fValue[256];
	BYTE etx;
} __attribute__ ((packed)) DBSVR_DATA_MSG_T;


// DBSVR data structure
typedef struct {
	BYTE stx;
	WORD pcm;
	WORD pno;
	WORD value;
	BYTE etx;
} __attribute__ ((packed)) DBSVR_ACK_MSG_T;


// DBSVR thread
typedef struct {
	int 			nFd;
	unsigned char 	*txbuf;
	unsigned char 	*rxbuf;
	int 			nRecvByte;
	char 			targetIp[32];
} __attribute__ ((packed)) DBSVR_T;


////////////////////////////////////////////////////////////////////////////////
// MODBUS Interface




////////////////////////////////////////////////////////////////////////////////
// APG Interface
typedef struct {
	int nFd;
	unsigned char *chTxBuf;
	unsigned char *chRxBuf;
	unsigned char *chPrevBuf;
	int nRecvByte;
	int nTempWp;
	int nIndexPrev;
} __attribute__ ((packed)) APG_T;


////////////////////////////////////////////////////////////////////////////////
// Library Interface
typedef struct {
	int nFd;
	unsigned char *chTxBuf;
	unsigned char *chRxBuf;
	unsigned char *chPrevBuf;
	int nRecvByte;
	int nTempWp;
	int nIndexPrev;
} __attribute__ ((packed)) LIB_T;

typedef struct {
	int nFd;							// uart2 fd
	struct pollfd sPollEvents;			// 체크할 event 정보를 갖는 struct
	int nPollState;						// poll의 상태
	//unsigned char *pchTxBuf;			// tx buffer
	//unsigned char *pchRxBuf;			// rx buffer
} __attribute__ ((packed)) LIB_UART2_T;

typedef struct {
	unsigned char 		chStx;
	unsigned short		wLength;
	unsigned short		wCmd;
	unsigned short		wPcm;
	unsigned short		wPno;
	float				fValue;
	unsigned short		wChkSum;
	unsigned char 		chEtx;
} __attribute__ ((packed)) LIB_PCM_SET_MSG_T;

typedef struct {
	unsigned char 		chStx;
	unsigned short		wLength;
	unsigned short		wCmd;
	unsigned short		wPcm;
	unsigned short		wPno;
	unsigned short		wType;
	unsigned short		wChkSum;
	unsigned char 		chEtx;
} __attribute__ ((packed)) LIB_PCM_DEF_MSG_T;

typedef struct {
	unsigned char 		chStx;
	unsigned short		wLength;
	unsigned short		wCmd;
	unsigned short		wPcm;
	unsigned short		wChkSum;
	unsigned char 		chEtx;
} __attribute__ ((packed)) LIB_INFO_MSG_T;

typedef struct {
	unsigned char 		chStx;
	unsigned short		wLength;
	unsigned short		wCmd;
	unsigned short		wChkSum;
	unsigned char 		chEtx;
} __attribute__ ((packed)) LIB_ACK_MSG_T;

typedef struct {
	unsigned char 		chStx;
	unsigned short		wLength;
	unsigned short		wCmd;
	unsigned short		wChkSum;
	unsigned char 		chEtx;
} __attribute__ ((packed)) LIB_FAIL_MSG_T;


////////////////////////////////////////////////////////////////////////////////
// Hantec Light Interface
typedef struct {
	unsigned char id;
	unsigned char cmd;
	unsigned char addrHi;
	unsigned char addrLo;
	unsigned char dataHi;
	unsigned char dataLo;
	unsigned char crcHi;	
	unsigned char crcLo;	
} __attribute__ ((packed)) IfaceHT_ReadMsg_T;

typedef struct {
	unsigned char id;
	unsigned char cmd;
	unsigned char addrHi;
	unsigned char addrLo;
	unsigned char dataHi;
	unsigned char dataLo;
	unsigned char crcHi;	
	unsigned char crcLo;	
} __attribute__ ((packed)) IfaceHT_WriteMsg_T;

typedef struct {
	unsigned char id;
	unsigned char cmd;
	unsigned char crcHi;	
	unsigned char crcLo;	
} __attribute__ ((packed)) IfaceHT_InfoMsg_T;

typedef struct {
	int fd;							// uart2 fd
	struct pollfd pollEvents;		// 체크할 event 정보를 갖는 struct
	int pollState;					// poll의 상태
	unsigned char *txbuf;			// tx buffer
	unsigned char *rxbuf;			// rx buffer
	unsigned int recvLength;		// recevie data length
	unsigned int sendLength;		// send data length

	unsigned int readId;

	unsigned int ctrl;				// check user control. (ctrl = pcm and pno)
	float ctrlVal;					// value of user-control
	int ctrlType;					// control type. only hantec interface.
	
	float *nowPntTable;				// point-table
	float *prePntTable;				// pre-point-table
} __attribute__ ((packed)) IfaceHT_T;


////////////////////////////////////////////////////////////////////////////////
// NET32/IP Interface
typedef struct {
	unsigned char 	stx;
	unsigned char 	length;
	unsigned char 	cmd;	
	unsigned char 	etx;	
} __attribute__ ((packed)) UCLIENT_GET;

typedef struct {
	unsigned char 	stx;
	unsigned char 	length;
	unsigned char 	cmd;	
	unsigned short  addr;	
	unsigned short  pno;	
	float  			value;	
	unsigned char 	etx;	
} __attribute__ ((packed)) UCLIENT_MSG;


typedef struct {
	unsigned short 	wSeq;
	unsigned char 	chLength;
	unsigned short  wSrc;	
	unsigned short  wDest;	
	unsigned char 	chFunc;
	unsigned short  wPcm;	
	unsigned short  wPno;	
	float  			fValue;	
	unsigned char 	chChksum;	
} __attribute__ ((packed)) NET32_UDP_MSG;



////////////////////////////////////////////////////////////////////////////////
// WATCHDOG LIST STRUCTURE
typedef struct {
	int		nLoogCnt[32];
} __attribute__ ((packed)) WATCHDOG_LIST;



// APG data format
typedef struct {
	unsigned char		c_cmd;								// command 
	unsigned char		c_pcm;								// pcm
	unsigned short		w_pno;								// pno
	float				f_val;								// value
} __attribute__ ((packed)) APG_DATA_FORMAT_T;

// APG pdef data format
typedef struct {
	unsigned char		c_cmd;								// command
	unsigned char		c_pcm;								// pcm
	unsigned short		w_pno;								// pno
	float				f_type;								// type
	float				f_hyst;								// hyst
	float				f_scale;							// scale
	float				f_offset;							// offset
	float				f_min;								// min
	float				f_max;								// max
} __attribute__ ((packed)) APG_PDEF_FORMAT_T;





typedef struct 
{
	char name[CFG_COMMAND_SIZE];
	char value[CFG_COMMAND_SIZE];	
}cmdinfo;

// point-table에 대한 모든 정보을 저장한다.
typedef struct __attribute__ ( (__packed__))
{
	unsigned int n_type;
	unsigned int n_adc;
	float f_hyst;
	float f_scale;
	float f_offset;
	float f_min;
	float f_max;
	float f_val;
	float f_preval;
}PTBL_INFO_T;


typedef struct __attribute__ ( (__packed__))
{
	float f_conv[2];
	float f_sig[2]; 
	float f_a;
	float f_b;
	float f_min;
	float f_max;
}CAL_VAL_T;


// calibration에 대한 정보를 저장한다. 
typedef struct __attribute__ ( (__packed__))
{
	//CAL_VAL_T cal_do; 
	float f_zero;  
	CAL_VAL_T cal_vo;
	CAL_VAL_T cal_vi;
	CAL_VAL_T cal_ci;
	CAL_VAL_T cal_jpt;
}CAL_INFO_T;


// Queue의 상태를 모니터링 한다.
typedef struct __attribute__ ( (__packed__))
{
	unsigned int n_indata;
	unsigned int n_outdata;
	unsigned int n_full;
	unsigned int n_empty;	 
}STATUS_QUEUE_T;


// Thread의 상태를 모니터링 한다.
typedef struct __attribute__ ( (__packed__))
{
	unsigned int n_loopcnt;
	unsigned int n_connetion;
	unsigned int n_chkqueue;
	unsigned int n_tx;
	unsigned int n_rx;
}STATUS_THREAD_T;

// DDC의 상태를 모니터링 한다.
typedef struct __attribute__ ( (__packed__))
{
	STATUS_QUEUE_T 		st_elba;
	STATUS_QUEUE_T 		st_elbamsg;
	STATUS_QUEUE_T 		st_net32;
	STATUS_QUEUE_T 		st_net32msg;
	STATUS_THREAD_T		st_elba_t;
	STATUS_THREAD_T		st_net32_t;
	STATUS_THREAD_T		st_pnt_handler_t;
	STATUS_THREAD_T		st_cmd_handler_t;
	STATUS_THREAD_T		st_msg_handler_t;
}CHK_STATUS_T;


//SUBIO DO REQUEST MESSAGE
typedef struct __attribute__ ( (__packed__))
{
    unsigned char 	c_id;
    unsigned char 	c_cmd;
    unsigned short 	w_addr;			
	unsigned short 	w_datacnt;		
    unsigned char 	c_bytecnt;
    unsigned short 	w_data;			
    unsigned char 	c_crchi;
    unsigned char 	c_crclo; 
}SUBIO_DO_MSG_T;

//SUBIO VO REQUEST MESSAGE
typedef struct __attribute__ ( (__packed__))
{
    unsigned char 	c_id;
    unsigned char 	c_cmd;
    unsigned short 	w_addr;			
    unsigned short 	w_datacnt;		
    unsigned char 	c_bytecnt;
    unsigned short 	w_data[8];		
    unsigned char 	c_crchi;
    unsigned char 	c_crclo; 
}SUBIO_VO_MSG_T;

//SUBIO ADI REQUEST MESSAGE
typedef struct __attribute__ ( (__packed__))
{
	unsigned char 	c_id;
    unsigned char 	c_cmd;
    unsigned short 	w_addr;			
    unsigned char 	c_data[8];		
    unsigned char 	c_crchi;
    unsigned char 	c_crclo; 
}SUBIO_ADI_MSG_T;

//SUBIO ADI REQUEST MESSAGE
typedef struct __attribute__ ( (__packed__))
{
	unsigned char 	c_id;
    unsigned char 	c_cmd;
    unsigned char 	w_addr;			
    unsigned short 	w_data[8];		
    unsigned char 	c_crchi;
    unsigned char 	c_crclo; 
}SUBIO_ADI_MSG_RES_T;


// SUBIO THREAD
typedef struct __attribute__ ( (__packed__))
{
	int n_fd;
	unsigned char *p_tx;
	unsigned char *p_rx;
	struct pollfd pollevents;		// 체크할 event 정보를 갖는 struct
	int n_pollstate;				// poll의 상태
	int n_type;
	int n_grp;
	int n_pno;
}IFACE_SUBIO_T;


typedef struct
{
	unsigned int net32Cnt;
	unsigned int elbaCnt;
	unsigned int elbaMessageCnt;
	unsigned int net32MessageCnt;
	unsigned int interfaceMessageCnt;
	unsigned int pointObserverCnt;
	unsigned int commandHandlerCnt;
	unsigned int libInterfaceHandlerCnt;
	unsigned int bacnetCnt;
}cntInfo;

typedef struct 
{
	float elba_in;
	float elba_cmd_in;
	float elba_message_in;
	float interface_message_in;
	float net32_in;
	float net32_message_in;
	float bacnet_message_in;

	float elba_out;
	float elba_cmd_out;
	float elba_message_out;
	float interface_message_out;
	float net32_out;
	float net32_message_out;
	float bacnet_message_out;

	float elba_full;
	float elba_cmd_full;
	float elba_message_full;
	float interface_message_full;
	float net32_full;
	float net32_message_full;
	float bacnet_message_full;

	float tcp_total_tx;
	float tcp_total_rx;
}queue_status;

//
// jong2ry plc
typedef struct
{
	unsigned int type;
	unsigned int addr;
	unsigned int pcm;
	unsigned int pno;
}plcInfo;

//
typedef struct	{
    unsigned char pno;				// n-th point number
    unsigned char Unsol;			// Point status (ERR, OK, ...)
    unsigned char WrFlag;
    float	val;					// Current point value
    float	PrevVal;				// Previous point value
    //
    short   PLCaddr;
    float   scale;
    float   offset;
    //
    int     MinVal;					// minimum point value
    int     MaxVal;         		// maximum point value
}	DNP_AIO_Point;
//
typedef struct	{
    unsigned char	pno;			// n-th point number
    unsigned char	Unsol;			// Point status (ERR, OK, ...)
    unsigned char	EnUnsol;		// Point status (ERR, OK, ...)
    unsigned char  	WrFlag;
    unsigned char	val;			// Current point value
    unsigned char	PrevVal;		// Previous point value
    unsigned char	WrVal;			// Write point value
    //
    short   PLCaddr;
    short   ONaddr;
    short   OFFaddr;
}	DNP_DIO_Point;




// Queue 
typedef struct 
{
	short	pcm;
	short	pno;
	float	value;
	int		message_type;
} point_info ;

// Queue 
typedef struct
{
	int front;  //front index of queue
	int rear;	//rear index of queue
	point_info data[MAX_QUEUE_SIZE];
} point_queue;

// Queue
typedef struct 
{
	unsigned int insNumber;
	unsigned int device;
	unsigned int objType;
	unsigned int objInstance;
	unsigned int objProperty;
	unsigned int val;
} bacnet_info ;

// Queue
typedef struct
{
	int front;  //front index of queue
	int rear;	//rear index of queue
	bacnet_info data[MAX_QUEUE_SIZE];
} bacnet_queue;









/*
typedef struct 
{
	char name[CFG_COMMAND_SIZE];
	char value[CFG_COMMAND_SIZE];	
}cmdinfo;
*/


/*
// point-table에 대한 모든 정보을 저장한다.
typedef struct __attribute__ ( (__packed__))
{
	unsigned int n_type;
	unsigned int n_adc;
	float f_hyst;
	float f_scale;
	float f_offset;
	float f_min;
	float f_max;
	float f_val;
	float f_preval;
}PTBL_INFO_T;


typedef struct __attribute__ ( (__packed__))
{
	float f_conv[2];
	float f_sig[2]; 
	float f_a;
	float f_b;
	float f_min;
	float f_max;
}CAL_VAL_T;


// calibration에 대한 정보를 저장한다. 
typedef struct __attribute__ ( (__packed__))
{
	//CAL_VAL_T cal_do; 
	float f_zero;  
	CAL_VAL_T cal_vo;
	CAL_VAL_T cal_vi;
	CAL_VAL_T cal_ci;
	CAL_VAL_T cal_jpt;
}CAL_INFO_T;


// Queue의 상태를 모니터링 한다.
typedef struct __attribute__ ( (__packed__))
{
	unsigned int n_indata;
	unsigned int n_outdata;
	unsigned int n_full;
	unsigned int n_empty;	 
}STATUS_QUEUE_T;


// Thread의 상태를 모니터링 한다.
typedef struct __attribute__ ( (__packed__))
{
	unsigned int n_loopcnt;
	unsigned int n_connetion;
	unsigned int n_chkqueue;
	unsigned int n_tx;
	unsigned int n_rx;
}STATUS_THREAD_T;

// DDC의 상태를 모니터링 한다.
typedef struct __attribute__ ( (__packed__))
{
	STATUS_QUEUE_T 		st_elba;
	STATUS_QUEUE_T 		st_elbamsg;
	STATUS_QUEUE_T 		st_net32;
	STATUS_QUEUE_T 		st_net32msg;
	STATUS_THREAD_T		st_elba_t;
	STATUS_THREAD_T		st_net32_t;
	STATUS_THREAD_T		st_pnt_handler_t;
	STATUS_THREAD_T		st_cmd_handler_t;
	STATUS_THREAD_T		st_msg_handler_t;
}CHK_STATUS_T;


//SUBIO DO REQUEST MESSAGE
typedef struct __attribute__ ( (__packed__))
{
    unsigned char 	c_id;
    unsigned char 	c_cmd;
    unsigned short 	w_addr;			
	unsigned short 	w_datacnt;		
    unsigned char 	c_bytecnt;
    unsigned short 	w_data;			
    unsigned char 	c_crchi;
    unsigned char 	c_crclo; 
}SUBIO_DO_MSG_T;

//SUBIO VO REQUEST MESSAGE
typedef struct __attribute__ ( (__packed__))
{
    unsigned char 	c_id;
    unsigned char 	c_cmd;
    unsigned short 	w_addr;			
    unsigned short 	w_datacnt;		
    unsigned char 	c_bytecnt;
    unsigned short 	w_data[8];		
    unsigned char 	c_crchi;
    unsigned char 	c_crclo; 
}SUBIO_VO_MSG_T;

//SUBIO ADI REQUEST MESSAGE
typedef struct __attribute__ ( (__packed__))
{
	unsigned char 	c_id;
    unsigned char 	c_cmd;
    unsigned short 	w_addr;			
    unsigned char 	c_data[8];		
    unsigned char 	c_crchi;
    unsigned char 	c_crclo; 
}SUBIO_ADI_MSG_T;

//SUBIO ADI REQUEST MESSAGE
typedef struct __attribute__ ( (__packed__))
{
	unsigned char 	c_id;
    unsigned char 	c_cmd;
    unsigned char 	w_addr;			
    unsigned short 	w_data[8];		
    unsigned char 	c_crchi;
    unsigned char 	c_crclo; 
}SUBIO_ADI_MSG_RES_T;


// SUBIO THREAD
typedef struct __attribute__ ( (__packed__))
{
	int n_fd;
	unsigned char *p_tx;
	unsigned char *p_rx;
	struct pollfd pollevents;		// 체크할 event 정보를 갖는 struct
	int n_pollstate;				// poll의 상태
	int n_type;
	int n_grp;
	int n_pno;
}IFACE_SUBIO_T;


typedef struct
{
	unsigned int net32Cnt;
	unsigned int elbaCnt;
	unsigned int elbaMessageCnt;
	unsigned int net32MessageCnt;
	unsigned int interfaceMessageCnt;
	unsigned int pointObserverCnt;
	unsigned int commandHandlerCnt;
	unsigned int libInterfaceHandlerCnt;
	unsigned int bacnetCnt;
}cntInfo;

typedef struct 
{
	float elba_in;
	float elba_cmd_in;
	float elba_message_in;
	float interface_message_in;
	float net32_in;
	float net32_message_in;
	float bacnet_message_in;

	float elba_out;
	float elba_cmd_out;
	float elba_message_out;
	float interface_message_out;
	float net32_out;
	float net32_message_out;
	float bacnet_message_out;

	float elba_full;
	float elba_cmd_full;
	float elba_message_full;
	float interface_message_full;
	float net32_full;
	float net32_message_full;
	float bacnet_message_full;

	float tcp_total_tx;
	float tcp_total_rx;
}queue_status;

//
// jong2ry plc
typedef struct
{
	unsigned int type;
	unsigned int addr;
	unsigned int pcm;
	unsigned int pno;
}plcInfo;

//
typedef struct	{
    unsigned char pno;				// n-th point number
    unsigned char Unsol;			// Point status (ERR, OK, ...)
    unsigned char WrFlag;
    float	val;					// Current point value
    float	PrevVal;				// Previous point value
    //
    short   PLCaddr;
    float   scale;
    float   offset;
    //
    int     MinVal;					// minimum point value
    int     MaxVal;         		// maximum point value
}	DNP_AIO_Point;
//
typedef struct	{
    unsigned char	pno;			// n-th point number
    unsigned char	Unsol;			// Point status (ERR, OK, ...)
    unsigned char	EnUnsol;		// Point status (ERR, OK, ...)
    unsigned char  	WrFlag;
    unsigned char	val;			// Current point value
    unsigned char	PrevVal;		// Previous point value
    unsigned char	WrVal;			// Write point value
    //
    short   PLCaddr;
    short   ONaddr;
    short   OFFaddr;
}	DNP_DIO_Point;
*/
//
//
/*
DNP_DIO_Point DNP_di_point[512];
DNP_DIO_Point DNP_do_point[512];
DNP_AIO_Point DNP_ai_point[512];
DNP_AIO_Point DNP_ao_point[512];
//
int debug_plc_tx;
int debug_plc_rx;
*/

/*
// APG handler 
typedef struct {
	int   				svr_sock;							// server socket
	int   				cli_sock;							// client socket
	struct sockaddr_un 	server_addr;						// server addr
	struct sockaddr_un 	client_addr;						// client addr
	unsigned char 		*txbuf;								// is pointer of tx buffer
	unsigned char 		*rxbuf;								// is pointer of rx buffer
	unsigned char 		*p_msgbuf;							// is pointer of message buffer
	unsigned int 		n_recvlength;						// length of receive message
	int 				n_fdmax;
	int 				cli_len;
	fd_set 				reads, temps;
	int 				n_sendtype;							// send type.
} __attribute__ ((packed)) APG_T;

// APG data format
typedef struct {
	unsigned char		c_cmd;								// command 
	unsigned char		c_pcm;								// pcm
	unsigned short		w_pno;								// pno
	float				f_val;								// value
} __attribute__ ((packed)) APG_DATA_FORMAT_T;

// APG pdef data format
typedef struct {
	unsigned char		c_cmd;								// command
	unsigned char		c_pcm;								// pcm
	unsigned short		w_pno;								// pno
	float				f_type;								// type
	float				f_hyst;								// hyst
	float				f_scale;							// scale
	float				f_offset;							// offset
	float				f_min;								// min
	float				f_max;								// max
} __attribute__ ((packed)) APG_PDEF_FORMAT_T;


// Queue 
typedef struct 
{
	short	pcm;
	short	pno;
	float	value;
	int		message_type;
} point_info ;

// Queue 
typedef struct
{
	int front;  //front index of queue
	int rear;	//rear index of queue
	point_info data[MAX_QUEUE_SIZE];
} point_queue;

// Queue
typedef struct 
{
	unsigned int insNumber;
	unsigned int device;
	unsigned int objType;
	unsigned int objInstance;
	unsigned int objProperty;
	unsigned int val;
} bacnet_info ;

// Queue
typedef struct
{
	int front;  //front index of queue
	int rear;	//rear index of queue
	bacnet_info data[MAX_QUEUE_SIZE];
} bacnet_queue;


// DBSVR data  structure
typedef struct {
	BYTE stx;
	WORD func;
	WORD length;
	WORD pcm;
	WORD value[256];
	BYTE etx;
} __attribute__ ((packed)) DBSVR_DATA_MSG_T;


// DBSVR data structure
typedef struct {
	BYTE stx;
	WORD pcm;
	WORD pno;
	WORD value;
	BYTE etx;
} __attribute__ ((packed)) DBSVR_ACK_MSG_T;


// DBSVR에서 사용하는 structure
typedef struct {
	int fd;
	unsigned char *txbuf;
	unsigned char *rxbuf;
	unsigned char *tempbuf;
	int recvLength;
	int tempWp;
	int bufSize;
	int ntime_request;
	char targetIp[32];
} __attribute__ ((packed)) DBSVR_T;



///////////////////////////////////////////////////////////////////////////////
// Net32 Interface (NET32 protocol)
// NET32 TOKEN MESSAGE
typedef struct {
	unsigned char cHeader;
	unsigned char cCmd;
	unsigned char cLength;
	unsigned char cDbgNoq;
	unsigned char cDbgNiq;
	unsigned char cChkSum;
} __attribute__ ((packed)) NET32_TOKEN_T;

// NET32 MESSAGE
typedef struct {
	unsigned char cHeader;
	unsigned char cCmd;
	unsigned char cLength;
	unsigned char cDbgNoq;
	unsigned char cDbgNiq;
	unsigned char cPcm;
	unsigned char cPno;
	unsigned char cVal[4];
	unsigned char cChkSum;
} __attribute__ ((packed)) NET32_MSG_T;

// NET32 NODE MESSAGE
typedef struct {
	unsigned char cHeader;
	unsigned char cCmd;
	unsigned char cLength;
	unsigned char cNode[32];
	unsigned char cChkSum;
} __attribute__ ((packed)) NET32_NODE_T;

// NET32 THREAD
typedef struct {
	int 				iNet32Fd;
	int 				iSendSock;
	int 				iRcvSock;
	struct 				sockaddr_in addr_send;
	struct 				sockaddr_in addr_rcv;
	unsigned char 		*rxBuf;
	unsigned char 		*txBuf;
	unsigned int 		nRecvByte;
	
	int 				nFd;
	fd_set				reads;
	fd_set				temps;
	int 				nFdMax;
	
	int 				n_svrsock;
	int 				n_clisock;
	struct sockaddr_in 	svr_addr;	
	struct sockaddr_in 	cli_addr;	
} __attribute__ ((packed)) NET32_T;
*/

///////////////////////////////////////////////////////////////////////////////
// 3iStation Interface (ELBA protocol)
//#define ELBA_SERVER_PORT         		  	4200
/*
typedef struct {
	int nFd;
	unsigned char *bTxBuf;
	unsigned char *bRxBuf;
	unsigned char *bPrevBuf;
	int nRecvLength;
	int nTempWp;
	int nIndexPrev;
	int nTimeRequest;	
	char chServerIp[CFG_VALUE_SIZE];
}ELBA_T;
*/

/*
///////////////////////////////////////////////////////////////////////////////
// CCMS Server Interface 
#define CCMS_MAX_CLIENT			32
#define CCMS_MAX_POINT			2048
#define CCMS_BUFFER_SIZE		2048
#define CCMS_PKT_DATA_CNT		32

#define CCMS_SERVER				0
#define CCMS_CLIENT				1

#define CCMS_PORT				9103

typedef struct {
	unsigned short 		wFd;
	unsigned short 		wStatus;
	unsigned short 		wValue[CCMS_MAX_POINT];
} __attribute__ ((packed)) CCMS_PTBL_T;


typedef struct {
	CCMS_PTBL_T	ptbl[CCMS_MAX_CLIENT];
} __attribute__ ((packed)) CCMS_PTBL_LIST_T;


typedef struct {
	unsigned int 		nIndexPrev;
	unsigned char 		chRxBuf[CCMS_BUFFER_SIZE];
	unsigned char 		chPrevBuf[CCMS_BUFFER_SIZE];
} __attribute__ ((packed)) CCMS_BUFFER_T;


typedef struct {
	int 				nFd;
	fd_set				reads;
	fd_set				temps;
	int 				nFdMax;
	int					nClientStatus[CCMS_MAX_CLIENT];
	int					nClientId[CCMS_MAX_CLIENT];
	int					nAliveCnt[CCMS_MAX_CLIENT];
	unsigned int		nServerId;
	unsigned int 		nIndexPrev;
	unsigned char 		chRxBuf[CCMS_BUFFER_SIZE];
	unsigned char 		chPrevBuf[CCMS_BUFFER_SIZE];	
	CCMS_PTBL_LIST_T	*pPtbl;
} __attribute__ ((packed)) CCMS_T;


typedef struct {
	int 				nFd;
	unsigned int		nClientId;
	unsigned int 		nIndexPrev;
	unsigned int 		nIndexPno;
	unsigned int 		nIndexSend;
	unsigned char		chTxBuf[CCMS_BUFFER_SIZE];
	unsigned char		chRxBuf[CCMS_BUFFER_SIZE];
	unsigned char 		chPrevBuf[CCMS_BUFFER_SIZE];
	CCMS_PTBL_LIST_T	*pPtbl;
} __attribute__ ((packed)) CCMS_CLIENT_T;


typedef struct {
	unsigned char 	chStx;
	unsigned char 	chCmd;
	unsigned char 	chId;
	unsigned short	wIndex;
	float			fData[CCMS_PKT_DATA_CNT];
	unsigned short	wDummy;
	unsigned char 	chEtx;
} __attribute__ ((packed)) CCMS_PKT_T;


typedef struct {
	unsigned char 	chStx;
	unsigned char 	chCmd;
	unsigned char 	chId;
	unsigned char 	chPcm;
	unsigned char 	chPno;
	unsigned short	wData;
	unsigned char 	chEtx;
} __attribute__ ((packed)) CCMS_PKT_SET_T;


typedef struct {
	unsigned char 	chStx;
	unsigned char 	chCmd;
	unsigned char 	chId;
	unsigned char 	chEtx;
} __attribute__ ((packed)) CCMS_PKT_ACK_T;



///////////////////////////////////////////////////////////////////////////////
// CCMS MODEM Server Interface 
#define CCMS_MODEM_MAX_CLIENT			32
#define CCMS_MODEM_MAX_POINT			2048
#define CCMS_MODEM_BUFFER_SIZE			2048
#define CCMS_MODEM_PKT_DATA_CNT			32

#define CCMS_MODEM_SERVER				0
//#define CCMS_MODEM_CLIENT				1

#define CCMS_MODEM_PORT					9104


typedef struct {
	int 				nFd;
	fd_set				reads;
	fd_set				temps;
	int 				nFdMax;
	int					nClientStatus[CCMS_MAX_CLIENT];
	int					nClientId[CCMS_MAX_CLIENT];
	int					nAliveCnt[CCMS_MAX_CLIENT];
	unsigned int		nServerId;
	unsigned int 		nIndexPrev;
	unsigned char 		chRxBuf[CCMS_BUFFER_SIZE];
	unsigned char 		chPrevBuf[CCMS_BUFFER_SIZE];	
} __attribute__ ((packed)) CCMS_MODEM_T;


typedef struct {
	int 				nFd;
	unsigned int		nClientId;
	unsigned int 		nIndexPrev;
	unsigned int 		nIndexPno;
	unsigned int 		nIndexSend;
	unsigned char		chTxBuf[CCMS_BUFFER_SIZE];
	unsigned char		chRxBuf[CCMS_BUFFER_SIZE];
	unsigned char 		chPrevBuf[CCMS_BUFFER_SIZE];
	CCMS_PTBL_LIST_T	*pPtbl;
} __attribute__ ((packed)) CCMS_MODEM_CLIENT_T;


typedef struct {
	unsigned char 	chStx;
	unsigned char 	chCmd;
	unsigned char 	chId;
	unsigned short	wIndex;
	float			fData;
	unsigned short	wDummy;
	unsigned char 	chEtx;
} __attribute__ ((packed)) CCMS_MODEM_PKT_T;


typedef struct {
	unsigned char 	chStx;
	unsigned char 	chCmd;
	unsigned char 	chId;
	unsigned char 	chEtx;
} __attribute__ ((packed)) CCMS_MODEM_ACK_T;



///////////////////////////////////////////////////////////////////////////////
// Iface Hander
#define IFH_MAX_CLIENT			2
#define IFH_BUFFER_SIZE			2048

#define IFH_PORT				9004

#define	IFH_CODE_CMD			0x01
#define	IFH_CODE_HTTPD			0x02
#define	IFH_CODE_CCMS			0x03
#define	IFH_CODE_PCTRL			0x04


typedef struct {
	unsigned int 		nIndexPrev;
	unsigned char 		chRxBuf[CCMS_BUFFER_SIZE];
	unsigned char 		chPrevBuf[CCMS_BUFFER_SIZE];
} __attribute__ ((packed)) IFH_BUFFER_T;


typedef struct {
	int 				nFd;
	fd_set				reads;
	fd_set				temps;
	int 				nFdMax;
	int					nClientStatus[CCMS_MAX_CLIENT];
	int					nClientId[CCMS_MAX_CLIENT];
	IFH_BUFFER_T		RxBuf[CCMS_MAX_CLIENT];
} __attribute__ ((packed)) IFH_T;


typedef struct {
	unsigned char 	chStx;
	unsigned char 	chCode;
	unsigned char 	chRemoteId;
	unsigned char 	chPcm;
	unsigned char 	chPno;
	unsigned short	wData;
	unsigned char 	chEtx;
} __attribute__ ((packed)) IFH_PKT_CCMS_T;


typedef struct {
	unsigned char 	chStx;
	unsigned char 	chCode;
	unsigned char 	chDummy;
	unsigned char 	chPcm;
	unsigned char 	chPno;
	unsigned short	wData;
	unsigned char 	chEtx;
} __attribute__ ((packed)) IFH_PKT_PCTRL_T;


typedef struct {
	unsigned char 	chStx;
	unsigned char 	chCode;
	unsigned char 	chdummy;
	unsigned char 	chEtx;
} __attribute__ ((packed)) IFH_PKT_ACK_T;


///////////////////////////////////////////////////////////////////////////////
// CDMA MODMEM TCP Interface
///////////////////////////////////////////////////////////////////////////////
typedef struct {
	int fd;							// uart2 fd
	struct pollfd pollEvents;		// 체크할 event 정보를 갖는 struct
	int pollState;					// poll의 상태
	unsigned char *txbuf;			// tx buffer
	unsigned char *rxbuf;			// rx buffer
	unsigned int recvLength;		// recevie data length	 
	unsigned int sendLength;		// send data length	 
} __attribute__ ((packed)) IfaceModem_T;

// FSM status value
#define IFACE_MODEM_READY				0
#define IFACE_MODEM_CONNECT				1
#define IFACE_MODEM_RECV_DATA			2
#define IFACE_MODEM_SEND_DATA			3
#define IFACE_MODEM_CLOSE				4
#define IFACE_MODEM_SEND_ACK			5

// Receive return value
#define IFACE_RTN_OK					1
#define IFACE_RTN_CONNECT				2
#define	IFACE_RTN_TCPOPEN				3
#define	IFACE_RTN_ERROR					4
#define	IFACE_RTN_TCPSENDDONE			5
#define IFACE_RTN_TCPCLOSED				6
#define IFACE_RTN_TCPREAD				7


///////////////////////////////////////////////////////////////////////////////
// CDMA MODMEM SMS Interface
///////////////////////////////////////////////////////////////////////////////
typedef struct {
	int fd;							// uart2 fd
	struct pollfd pollEvents;		// 체크할 event 정보를 갖는 struct
	int pollState;					// poll의 상태
	unsigned char *txbuf;			// tx buffer
	unsigned char *rxbuf;			// rx buffer
	unsigned int recvLength;		// recevie data length	 
	unsigned int sendLength;		// send data length	 
} __attribute__ ((packed)) IfaceSms_T;

// FSM status value
#define IFACE_SMS_READY				0
#define IFACE_SMS_CLOSE				1
#define IFACE_SMS_CHECK_VALUE		2
#define IFACE_SMS_SEND_MSG			3

// Receive return value
#define IFACE_SMS_RTN_OK					1
#define IFACE_SMS_RTN_CONNECT				2
#define	IFACE_SMS_RTN_TCPOPEN				3
#define	IFACE_SMS_RTN_ERROR					4
#define	IFACE_SMS_RTN_TCPSENDDONE			5
#define IFACE_SMS_RTN_TCPCLOSED				6
#define IFACE_SMS_RTN_006					7


///////////////////////////////////////////////////////////////////////////////
// XGT Interface
///////////////////////////////////////////////////////////////////////////////
#define 	IFC_XGT_BUFFER_SIZE			1024

typedef struct {
	int 				nFd;
	unsigned short 		wInvokeId;
	unsigned int 		nIndexPrev;
	unsigned int 		nIndexPno;
	unsigned int 		nIndexSend;
	unsigned char		chTxBuf[IFC_XGT_BUFFER_SIZE];
	unsigned char		chRxBuf[IFC_XGT_BUFFER_SIZE];
	unsigned char 		chPrevBuf[IFC_XGT_BUFFER_SIZE];	
} __attribute__ ((packed)) IFC_XGT_T;


typedef struct {
	unsigned char 	chCompany[8];
	unsigned short 	chReserved;
	unsigned short 	chPLC_Info;
	unsigned char 	chCpu_Info;
	unsigned char 	chSource;
	unsigned short	chInvokeId;
	unsigned short	chLength;
	unsigned char 	chFnetPos;
	unsigned char 	chReserved2;
} __attribute__ ((packed)) XGT_HEADER_T;


typedef struct {
	unsigned short 	wCmd;
	unsigned short 	wType;
	unsigned short 	wReserved;
	unsigned short 	wCnt;
	unsigned short 	wNameLength;
	unsigned char	chName[8];
	unsigned short	wLength;
} __attribute__ ((packed)) XGT_BIT_APP_T;
*/




#endif






