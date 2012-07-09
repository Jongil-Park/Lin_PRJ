#ifndef   _IFACE_MDTD_SVR_
#define   _IFACE_MDTD_SVR_

/*
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
*/

typedef struct {
	int 			fd;				// uart2 fd
	struct pollfd 	pollEvents;		// üũ�� event ������ ���� struct
	int 			pollState;		// poll�� ����
	unsigned char 	*pTxbuf;		// tx buffer
	unsigned char 	*pRxbuf;		// rx buffer
	unsigned int 	recvLength;		// recevie data length
	unsigned int 	sendLength;		// send data length
} __attribute__ ((packed)) IfaceMsvr_T;





void *Iface_Msvr_Main(void* arg);

#endif



