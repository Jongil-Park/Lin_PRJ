#ifndef  _XGT_PLC_
#define  _XGT_PLC_ 

// share memory
#define  	KEY_NUM    	 			9527			// share memory key
#define  	SHARE_MEM_SIZE    		32786			// point table size

#define 	MAX_NET32_NUMBER		32				// NET32 MAX NODE
#define		MAX_POINT_NUMBER		256				// MAX POINT TABLE

#define		XGT_PLC_1				1
#define		XGT_PLC_2				2
#define		XGT_PLC_3				3
#define		XGT_PLC_M				4

#define		XGT_AIO_TYPE			1
#define		XGT_DIO_TYPE			2

#define		XGT_PLC_PORT			4200			// test port

#define 	ACCEPT_NONBLOCKING		1

#define 	XGT_BUFFER_SIZE			2048

struct XGT_HEADER {
	char company[8];
	unsigned short reserved;
	unsigned short info;
	unsigned char cpu;
	unsigned char farme;
	unsigned short invoke;
	unsigned short length;
	unsigned char postion;
	unsigned char bcc;
}__attribute__ ((packed)) ;


struct XGT_PLC_COUNT {
	int type;
	unsigned int addr;
}__attribute__ ((packed)) ;


struct XGT_PLC_POINT {
	int type;		
	int paddr;
	int laddr;
	int min;	
	int max;	
	int scale;	
	int offset;	
	int pcm;	
	int pno;		
};


void xgt_copy_ptbl(void);
int xgt_init_sharememory(void);
int xgt_connect_plc(char *pIp);
void xgt_make_header(struct XGT_HEADER *pHeader);
void xgt_make_message(void);
int xgt_read_aio(int nPlcSelect, unsigned int nAddr);
int xgt_read_dio(int nPlcSelect, unsigned int nAddr);
void xgt_close_plc(int nSocket);
int main(int argc, char *argv[]); 


#endif

