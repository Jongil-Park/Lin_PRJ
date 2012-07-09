#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/un.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/poll.h>		// use poll event

#include "define.h"
#include "queue.h"
#include "cmd_handler.h"
#include "point_manager.h"
#include "point_handler.h"


// from main.c
extern int my_pcm;
extern char *p_dfn_type_name[];	// point-define type name
extern define_Info dfn_point[MAX_POINT_NUMBER];
extern float point_table[MAX_NET32_NUMBER][MAX_POINT_NUMBER];
extern PTBL_INFO_T *gp_ptbl;
CHK_STATUS_T *gp_status;														// gp_status

unsigned char cmd_tx_msg[MAX_BUFFER_SIZE];
unsigned char cmd_rx_msg[MAX_BUFFER_SIZE];

// from elba.c
extern int gc_dbg_txmsg;
extern int gc_dbg_rxmsg;
extern int gc_view_elba;

extern unsigned char dbg_mtr_niq;
extern unsigned char dbg_mtr_noq;
extern unsigned char g_cNode[32];
extern int g_nViewNet32;

// debugging net32 status
extern int g_nStatusNet32;

// from point_handler.c
extern int gn_adc_dbg;


extern queue_status status;

typedef struct {
	int   server_socket;
	int   client_socket;
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	unsigned char *txbuf;
	unsigned char *rxbuf;
	int recvLength;
	int  (*on_sock_recv)(void *);
	int  (*on_hadler)(void *);
	void (*on_sleep)(int *, int *);  
	int (*on_pset)(void *);  
	int (*on_pinfo)(void *);  
	int (*on_preq)(void *);  
	int (*on_dbgTcp)(void *);  
	int (*on_dbgNtx)(void *);  	
	int (*on_dbgNrx)(void *);  
	int (*on_mtr)(void *);  
	int (*on_node)(void *); 
	int (*on_view)(void *); 
} __attribute__ ((packed)) COMMAND_T;

COMMAND_T *New_Cmd(void);
int Cmd_Open(COMMAND_T *p);
void Cmd_Close(COMMAND_T *p);
int On_Cmd_Recv(void *pcmd);
int On_Cmd_Handler (void *pcmd);
int cmd_pset(void *pCmd);
int cmd_pinfo(void *pCmd);
int cmd_dbgTcp(void *pCmd);
int cmd_dbgNtx(void *pCmd);
int cmd_dbgNrx(void *pCmd);
int cmd_preq(void *pCmd);
int cmd_mtr(void *pCmd);
int cmd_node(void *pCmd);
int cmd_view(void *pCmd);
int cmd_apset(void *pCmd);
int cmd_pdef(void *pCmd);
int cmd_calibration(void *pCmd);
int cmd_status(void *pCmd);
void commandHandlerSelectSleep(int sec, int msec);



/*******************************************************************/
void commandHandlerSelectSleep(int sec,int msec) 
/*******************************************************************/
{
    struct timeval tv;
    tv.tv_sec=sec;
    tv.tv_usec=msec * 1000;                
    select(0,NULL,NULL,NULL,&tv);
    return;
}


/*******************************************************************/
COMMAND_T *New_Cmd(void)
/*******************************************************************/
{
	COMMAND_T *cmdSock;
	
	cmdSock = (COMMAND_T *)malloc( sizeof(COMMAND_T) );
	memset( (unsigned char *)cmdSock, 0, sizeof(COMMAND_T) );
	
	cmdSock->server_socket = -1;
	cmdSock->client_socket = -1;
	cmdSock->rxbuf = (unsigned char *)&cmd_rx_msg;
	cmdSock->txbuf = (unsigned char *)&cmd_tx_msg;
	cmdSock->recvLength = 0;
	memset( &cmdSock->server_addr, 0, sizeof( cmdSock->server_addr) );
	memset( &cmdSock->client_addr, 0, sizeof( cmdSock->client_addr) );
	cmdSock->on_sock_recv = On_Cmd_Recv; 
	cmdSock->on_hadler = On_Cmd_Handler;

	//cmdSock->on_sleep = commandHandlerSelectSleep;
	cmdSock->on_pset = cmd_pset; 
	cmdSock->on_pinfo = cmd_pinfo; 
	cmdSock->on_dbgTcp = cmd_dbgTcp; 
	cmdSock->on_dbgNtx = cmd_dbgNtx; 
	cmdSock->on_dbgNrx = cmd_dbgNrx; 
	cmdSock->on_preq = cmd_preq; 
	cmdSock->on_mtr = cmd_mtr; 
	cmdSock->on_node = cmd_node;
	cmdSock->on_view = cmd_view;
	
	return cmdSock;	
}


/*******************************************************************/
int Cmd_Open(COMMAND_T * p)
/*******************************************************************/
{
	if ( 0 == access( FILE_SERVER, F_OK))
	  unlink( FILE_SERVER);
	
	p->server_socket = socket( PF_FILE, SOCK_STREAM, 0);
	
	if( -1 == p->server_socket) {
	  printf( "Can't create server_socket in %s\n", __FUNCTION__);
	  return -1;
	}
	
	memset( &p->server_addr, 0, sizeof(p->server_addr));
	p->server_addr.sun_family  = AF_UNIX;
	strcpy( p->server_addr.sun_path, FILE_SERVER);
	
	if( -1 == bind( p->server_socket, (struct sockaddr*)&p->server_addr, sizeof(p->server_addr) ) ) {
	  printf( "bind error in %s\n", __FUNCTION__);
	  return -1;
	}	
	
	return p->server_socket;
}


/*******************************************************************/
int Cmd_Accept(COMMAND_T * p)
/*******************************************************************/
{
	int client_addr_size;
	
	if( -1 == listen(p->server_socket, 5)) {
		printf( "listen error in %s\n", __FUNCTION__);
		return -1;
	}
	
	client_addr_size = sizeof(p->client_addr);
	p->client_socket = accept(p->server_socket, (struct sockaddr*)&p->client_addr, &client_addr_size);
	
	if ( -1 == p->client_socket) {
		printf( "accept error in %s\n", __FUNCTION__);
		return -1;
	}
	
	return p->client_socket;
}


/*******************************************************************/
int On_Cmd_Recv(void *pcmd)
/*******************************************************************/
{
	//int i = 0;
	int length;
	COMMAND_T *p;
	unsigned char *pbuf;
	
	p = (COMMAND_T *)pcmd;
	pbuf = (unsigned char *)p->rxbuf;

	length = read ( p->client_socket, pbuf, MAX_BUFFER_SIZE);
	p->recvLength = length;
	
	/*	
	printf("Rx(%d) = ", length);
	for (i = 0; i < length; i++) {
		printf("%x ", pbuf[i]);	
	} 
	printf("\n");
	*/
	
	return length;
}


/*******************************************************************/
int On_Cmd_Handler (void *pcmd)
/*******************************************************************/
{
	//int i = 0;
	COMMAND_T *p;
	unsigned char *pbuf;
	
	p = (COMMAND_T *)pcmd;
	pbuf = (unsigned char *)p->rxbuf;
	
	/*
	printf("Rx(%d) = ", p->recvLength );
	for (i = 0; i < p->recvLength ; i++) {
		printf("%x ", pbuf[i]);	
	} 
	printf("\n");
	*/

	switch (pbuf[0]) {
		case COMMAND_PSET:				p->on_pset(p);		break;
		case COMMAND_PGET_MULTI_POINT:	p->on_pinfo(p);		break;
		case COMMAND_PGET_SUCCESS:		p->on_pinfo(p);		break;			
		case COMMAND_DEBUG_TCP:			p->on_dbgTcp(p);	break;
		case COMMAND_DEBUG_NTX:			p->on_dbgNtx(p);	break;
		case COMMAND_DEBUG_NRX:			p->on_dbgNrx(p);	break;
		case COMMAND_PREQ:				p->on_preq(p);		break;
		case COMMAND_MTR:				p->on_mtr(p);		break;
		case COMMAND_NODE:				p->on_node(p);		break;
		case COMMAND_VIEW:				p->on_view(p);		break;
		case COMMAND_APSET:				cmd_apset(p);		break;
		case COMMAND_PDEF:				cmd_pdef(p);		break;
		case COMMAND_CAL:				cmd_calibration(p);	break;
		case COMMAND_STATUS:			cmd_status(p);		break;
			
	}

	return 1;
}


/*******************************************************************/
void Cmd_Close(COMMAND_T *p)
/*******************************************************************/
{
	close (p->client_socket);
	p->client_socket = -1;
}


/*******************************************************************/
void *command_handler_main(void* arg)
/*******************************************************************/
{
	COMMAND_T *cmd;
	
	cmd = New_Cmd();
	if (cmd == NULL) {
		printf("Can't create COMMAND.\n");
		exit(1);
	}

	while (1) {
		if ( Cmd_Open(cmd) < 0 ) {
			sleep(1);
			continue;	
		}	
		
		for(;;) {
			if (Cmd_Accept(cmd) > 0 ) {
				commandHandlerSelectSleep(0, 100); 
				if ( cmd->on_sock_recv(cmd) > 0) {
					cmd->on_hadler(cmd);
				}
				Cmd_Close(cmd);
			}
			continue;
		}		
	}		
}

/*******************************************************************/
int cmd_pset(void *pCmd)
/*******************************************************************/
{
	//Variables
	//int i;
	point_info point;
	COMMAND_T *p;
	unsigned char *pBuf;
	
	//Initialize
	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;
	memset(&point, 0, sizeof(point));
	
	if(p->recvLength < 9) {
		printf("[ERROR] Incorrect pset message in command handler\n");
		return ERROR;
	}

	memcpy(&point.pcm,	 &pBuf[1], sizeof(short));
	memcpy(&point.pno,	 &pBuf[3], sizeof(short));
	memcpy(&point.value, &pBuf[5], sizeof(float));

	if(point.pcm < 0 || point.pcm >= MAX_NET32_NUMBER || 
	   point.pno < 0 || point.pno >= MAX_POINT_NUMBER) {
		printf("[ERROR] Incorrect pSet pcm [%hd] pno [%hd]\n", point.pcm, point.pno);
		return ERROR;
	}

	pSet(point.pcm, point.pno, point.value);
	return SUCCESS;
}


/*******************************************************************/
int cmd_pinfo(void *pCmd)
/*******************************************************************/
{
	//Variables
	int i;
	short pcm;
	short pno;
	int number;
	COMMAND_T *p;
	unsigned char *pBuf;
	
	//Initialze
	i       = 0;
	pcm     = 0;
	pno     = 0;
	number  = 0;
	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;

	if(p->recvLength < 8) {
		printf("[ERROR] Incorrect pinfo message in command handler\n");
		return ERROR;
	}

	memcpy(&pcm,   &pBuf[1], sizeof(short));
	memcpy(&pno,   &pBuf[3], sizeof(short));
	memcpy(&number, &pBuf[5], sizeof(short) );

	if(pcm < 0 || pcm >= MAX_NET32_NUMBER  || 
	   pno < 0 || pno >= MAX_POINT_NUMBER  ||
	   number < 0 || number > MAX_POINT_NUMBER) {
		fprintf( stderr, "[ERROR] Incorrect pGet pcm [%hd] pno [%hd] number [%hd]\n", 
				pcm, pno, number);
		return ERROR;
	}
	
	if ( pcm == my_pcm ) {
		fprintf( stderr, "\n  PCM       VALUE   TYPE   HYST   OFFSET   SCALE       MIN         MAX       ADC\n");
		fprintf( stderr, "====================================================================================\n");
		for (i = 0; (pno + i) <= number; i++) {
			fprintf( stderr, " %02d-%03d   %7.2f    %s   %0.2f     %0.2f    %0.2f   %6.2f   %6.2f   %4d\n", 
					 pcm, pno+i, point_table[pcm][pno+i],
					 p_dfn_type_name[gp_ptbl[pno+i].n_type], 
					 gp_ptbl[pno+i].f_hyst, 
					 gp_ptbl[pno+i].f_offset, 
					 gp_ptbl[pno+i].f_scale, 
					 gp_ptbl[pno+i].f_min, 
					 gp_ptbl[pno+i].f_max, 
					 gp_ptbl[pno+i].n_adc);
		}
	}
	else {
		fprintf( stderr, "\n  PCM       VALUE      INFORMATION\n");
		fprintf( stderr, "====================================\n");
		for (i = 0; (pno + i) <= number; i++) {
			fprintf( stderr, " %02d-%03d   %7.2f    No Information\n", 
				   pcm, pno+i, point_table[pcm][pno+i] );
		}
	}

	fflush(stderr);
	return SUCCESS;
}


/*******************************************************************/
int cmd_preq(void *pCmd)
/*******************************************************************/
{
	//Variables
	//int i;
	point_info point;
	COMMAND_T *p;
	unsigned char *pBuf;
	
	//Initialize
	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;
	memset(&point, 0, sizeof(point));
	
	if(p->recvLength < 5) {
		printf("[ERROR] Incorrect preq message in command handler\n");
		return ERROR;
	}

	memcpy(&point.pcm,	 &pBuf[1], sizeof(short));
	memcpy(&point.pno,	 &pBuf[3], sizeof(short));

	if(point.pcm < 0 || point.pcm >= MAX_NET32_NUMBER || 
	   point.pno < 0 || point.pno >= MAX_POINT_NUMBER) {
		printf("[ERROR] Incorrect preq pcm [%hd] pno [%hd]\n", point.pcm, point.pno);
		return ERROR;
	}

	pReq(point.pcm, point.pno);
	return SUCCESS;
}


/************************************************************/
int cmd_dbgTcp(void *pCmd)
/************************************************************/
{
	COMMAND_T *p;
	unsigned char *pBuf;
	
	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;

	if(p->recvLength < 5) {
		printf("[ERROR] Incorrect dbgTcp message in command handler\n");
		return ERROR;
	}

	if(pBuf[4] == 1) {
		printf("debug tcp on\n");	
		gc_dbg_txmsg = 1;
		gc_dbg_rxmsg = 1;
	}		
	else {
		gc_dbg_txmsg = 0;
		gc_dbg_rxmsg = 0;
		printf("debug tcp off\n");		
	}

	return SUCCESS;
}

/************************************************************/
int cmd_dbgNtx(void *pCmd)
/************************************************************/
{
	COMMAND_T *p;
	unsigned char *pBuf;
	
	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;

	if(p->recvLength < 5) {
		printf("[ERROR] Incorrect dbgNtx message in command handler\n");
		return ERROR;
	}

	if(pBuf[4] == 1) {
		printf("debug ntx on\n");	
		gc_dbg_txmsg = 1;
	}		
	else {
		gc_dbg_txmsg = 0;
		printf("debug ntx off\n");		
	}

	return SUCCESS;
}

/************************************************************/
int cmd_dbgNrx(void *pCmd)
/************************************************************/
{
	COMMAND_T *p;
	unsigned char *pBuf;
	
	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;

	if(p->recvLength < 5) {
		printf("[ERROR] Incorrect dbgNrx message in command handler\n");
		return ERROR;
	}

	if(pBuf[4] == 1) {
		printf("debug nrx on\n");	
		gc_dbg_rxmsg = 1;
	}		
	else {
		gc_dbg_rxmsg = 0;
		printf("debug nrx off\n");		
	}

	return SUCCESS;
}

/*******************************************************************/
/* message information
   | COMMAND_MTR | 'I' | value(0) | value(1) | 
   | COMMAND_MTR | 'O' | value(0) | value(1) |
*/
int cmd_mtr(void *pCmd)
/*******************************************************************/
{
	//int i;
	short value;
	COMMAND_T *p;
	unsigned char *pBuf;

	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;
	
	if( p->recvLength < 4 ) {
		printf("[ERROR] Incorrect mtr message in command handler\n");
		return ERROR;
	}

	memcpy( &value, &pBuf[2], sizeof(short) );
	
	if ( pBuf[1] == 'I') {
		printf("mtr niq ==>> %x\n", value);
		dbg_mtr_niq = value;
	}
	else {
		printf("mtr noq ==>> %x\n", value);
		dbg_mtr_noq = value;
	}
	return SUCCESS;
}

/*******************************************************************/
/* message information
   | COMMAND_NODE | 'N' |  
*/
int cmd_node(void *pCmd)
/*******************************************************************/
{
	int i;
	COMMAND_T *p;
	unsigned char *pBuf;

	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;
	
	if( p->recvLength < 2 ) {
		fprintf(stderr, "\n[ERROR] Incorrect node message in command handler\n");
		fflush(stderr);
		return ERROR;
	}

	fprintf(stderr, "\nMy Id  : %02d", my_pcm);

	if ( pBuf[1] == 'N') {
		for ( i = 0; i < 32; i++ ) {
			if ( g_cNode[i] ) {
				fprintf(stderr, "\nMaster : %02d\n", i);
				break;
			}
		}
		
		fprintf(stderr, "Node ( 0 ~ 31 ): ");
		for ( i = 0; i < 32; i++ ) {
			if ( !(i & 3) )		
				fprintf(stderr, " ");
			if ( g_cNode[i] )	
				fprintf(stderr, "O");
			else				
				fprintf(stderr, "-");
		}				
		fprintf(stderr, "\n\n");
		fflush(stderr);
	}
	return SUCCESS;
}



/*
********************************************************************* 
* VIEW COMMAND PROCESS
* 
* Description	: view command에 대한 처리를 수행한다. 
* 
* 
* Arguments		: rxmsg_length		Is a length of to receive data. 
* 
* 
* Returns		: SUCCESS      		If the call was successful
*              	  ERROR      		If not
* 
********************************************************************* 
*/
int cmd_view(void *pCmd)
{
	COMMAND_T *p;
	unsigned char *pBuf;
	
	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;

	if(p->recvLength < 5) {
		fprintf(stderr, "[ERROR] Incorrect view message in command handler\n");
		fflush(stderr);
		return ERROR;
	}

	// Message format : || COMMAND_VIEW || 'V' || '-' || 'T' || [1/0] ||
	// Message format : || COMMAND_VIEW || 'V' || '-' || 'N' || [1/0] ||
	if ( cmd_rx_msg[3] == 'T' ) {
		if ( cmd_rx_msg[4] == 1 ) {
			gc_view_elba = 1;
			fprintf(stderr, "CMD : view tcp on\n");
		}		
		else {
			gc_view_elba = 0;
			fprintf(stderr, "CMD : view tcp off\n");
		}
	}
	else if ( cmd_rx_msg[3] == 'N' ) {
		fprintf(stderr, "g_nStatusNet32 = %d\n", g_nStatusNet32);
		if ( cmd_rx_msg[4] == 1 ) {
			g_nViewNet32 = 1;
			fprintf(stderr, "CMD : view net32 on\n");
		}		
		else {
			g_nViewNet32 = 0;
			fprintf(stderr, "CMD : view net32 off\n");
		}
	}
	else if ( cmd_rx_msg[3] == 'A' ) {
		if ( cmd_rx_msg[4] == 1 ) {
			gn_adc_dbg = 1;
			fprintf(stderr, "CMD : view adc on\n");
		}		
		else {
			gn_adc_dbg = 0;
			fprintf(stderr, "CMD : view adc off\n");
		}
	}

	fflush(stderr);
	return SUCCESS;
}


int cmd_apset(void *pCmd)
{
	COMMAND_T *p;
	unsigned int n_pno;
	float f_val;
	unsigned char *p_buf;
	
	p = (COMMAND_T *)pCmd;
	p_buf = (unsigned char *)p->rxbuf;
	
	if(p->recvLength < 9) {
		printf("[ERROR] Incorrect apset message in command handler\n");
		return ERROR;
	}

	memcpy( &f_val, &p_buf[5], sizeof(float) );

	for (n_pno = 0; n_pno < MAX_POINT_NUMBER; n_pno++ ) {
		pSet( my_pcm, n_pno, f_val);
	}
	return SUCCESS;
}


int cmd_pdef(void *pCmd)
{
	COMMAND_T *p;
	unsigned char *p_buf;
	unsigned short w_pcm = 0;
	unsigned short w_pno = 0;
	unsigned short w_type = 0;
	float f_hyst = 0;
	float f_min = 0;
	float f_max = 0;
	float f_scale = 0;
	float f_offset = 0;
	
	p = (COMMAND_T *)pCmd;
	p_buf = (unsigned char *)p->rxbuf;
	
	if(p->recvLength == 5) {
		memcpy( &w_pcm, &p_buf[1], sizeof(short) );
		memcpy( &w_pno, &p_buf[3], sizeof(short) );
		
		if ( w_pcm != my_pcm ) {
			fprintf( stderr, "[ERR] pdef command only my_pcm\n" );
			fflush(stderr);
			return SUCCESS;
		}

		w_type = (unsigned short)gp_ptbl[w_pno].n_type;
		f_hyst = gp_ptbl[w_pno].f_hyst;
		f_scale = gp_ptbl[w_pno].f_scale;
		f_offset = gp_ptbl[w_pno].f_offset;
		f_min = gp_ptbl[w_pno].f_min;
		f_max = gp_ptbl[w_pno].f_max;
		
		fprintf( stderr, "pcm = %d, pno = %d, type = %s\n", w_pcm, w_pno, p_dfn_type_name[w_type] );
		fprintf( stderr, "hyst = %0.2f, scale = %0.2f, offset = %0.2f\n",
				 f_hyst, f_scale, f_offset );
		fprintf( stderr, "min = %0.2f, max = %0.2f\n", f_min, f_max );
		fflush(stderr);
		return SUCCESS;
	}	
	
	if(p->recvLength < 27) {
		fprintf( stderr, "[ERROR] Incorrect pdef message in command handler\n" );
		fflush(stderr);
		return ERROR;
	}

	memcpy( &w_pcm, &p_buf[1], sizeof(short) );
	memcpy( &w_pno, &p_buf[3], sizeof(short) );
	memcpy( &w_type, &p_buf[5], sizeof(short) );
	memcpy( &f_hyst, &p_buf[7], sizeof(float) );
	memcpy( &f_scale, &p_buf[11], sizeof(float) );
	memcpy( &f_offset, &p_buf[15], sizeof(float) );
	memcpy( &f_min, &p_buf[19], sizeof(float) );
	memcpy( &f_max, &p_buf[23], sizeof(float) );

	if ( w_pcm != my_pcm ) {
		fprintf( stderr, "[ERR] pdef command only my_pcm\n" );
		fflush(stderr);
		return SUCCESS;
	}

	fprintf( stderr, "pdef %d %d %s   %0.2f  %0.2f  %0.2f  %0.2f  %0.2f\n",
			 w_pcm, w_pno, p_dfn_type_name[w_type],
			 f_hyst, f_scale, f_offset, f_min, f_max);
	fflush(stderr);

	gp_ptbl[w_pno].n_type = w_type;
	gp_ptbl[w_pno].f_hyst = f_hyst;
	gp_ptbl[w_pno].f_scale = f_scale;
	gp_ptbl[w_pno].f_offset = f_offset;
	gp_ptbl[w_pno].f_min = f_min;
	gp_ptbl[w_pno].f_max = f_max;
	
	pDef();

	return SUCCESS;
}


int cmd_calibration(void *pCmd)
{
	COMMAND_T *p;
	unsigned char *p_buf;
	
	p = (COMMAND_T *)pCmd;
	p_buf = (unsigned char *)p->rxbuf;
	
	if ( p_buf[1] != 'C' 
		 && p_buf[2] != 'A'  
		 && p_buf[3] != 'L'	) {
		return ERROR;
	}

	if ( p->recvLength == 5 ) {

		if ( p_buf[4] == 'L' ) {
			pnt_cal_load();
		}
		else if ( p_buf[4] == 'I' ) {
			pnt_cal_init();
		}
		else if ( p_buf[4] == 'S' ) {
			pnt_cal_save();
		}
		else if ( p_buf[4] == 'D' ) {
			pnt_cal_display();
		}
		else if ( p_buf[4] == 'Z' ) {
			pnt_cal_zero();
		}

		return SUCCESS;
	}	
	else if ( p->recvLength == 6 ) {
		// 0 ~ 3그룹까지만을 지원한다.
		if ( p_buf[4] < 4 ) {
			pnt_cal_grp(p_buf[4], p_buf[5]);
		}
	}
	else if ( p->recvLength == 14 ) {
		// 0 ~ 3그룹까지만을 지원한다.
		if ( p_buf[4] < 4 ) {
			pnt_cal_setvalue( &p_buf[4] );
		}
	}

	return SUCCESS;
}


int cmd_status(void *pCmd)
// ----------------------------------------------------------------------------
// DISPLAY LINUX STAT
// Description : 현재 상태를 화면에 출력한다. 
// Arguments   : pCmd			Is a pointer of message
// Returns     : SUCCESS		always return success
{
	STATUS_QUEUE_T *pq;
	STATUS_THREAD_T *pt;
	unsigned int n_temp;

	fprintf ( stderr, "\n\n" );
	fprintf ( stderr, "Point Thread Testing ################ \n" );
	pt = (STATUS_THREAD_T *) &gp_status->st_pnt_handler_t;
	if ( pt->n_connetion == 1 ) 
		fprintf ( stderr, " - Connect           [OK] \n" );
	else
		fprintf ( stderr, " - Connect           [FAIL] \n" );
	
	n_temp = pt->n_loopcnt;
	commandHandlerSelectSleep (0, 500);
	if ( pt->n_loopcnt != n_temp ) 
		fprintf ( stderr, " - LoopCnt           [OK] \n" );
	else
		fprintf ( stderr, " - LoopCnt           [FAIL] (%d)\n", n_temp );

	n_temp = pt->n_tx;
	commandHandlerSelectSleep (0, 500);
	if ( pt->n_tx != n_temp ) 
		fprintf ( stderr, " - MsgTX             [OK] \n" );
	else
		fprintf ( stderr, " - MsgTX             [FAIL] (%d)\n", n_temp );

	n_temp = pt->n_rx;
	commandHandlerSelectSleep (0, 500);
	if ( pt->n_rx != n_temp ) 
		fprintf ( stderr, " - MsgRX             [OK] \n" );
	else
		fprintf ( stderr, " - MsgRX             [FAIL] (%d)\n", n_temp );

	fprintf ( stderr, "Point Thread Test Complete \n\n" );
	fflush(stderr);


	fprintf ( stderr, "Elba Thread Testing ################# \n" );
	pt = (STATUS_THREAD_T *) &gp_status->st_elba_t;
	if ( pt->n_connetion == 1 ) 
		fprintf ( stderr, " - Connect           [OK] \n" );
	else
		fprintf ( stderr, " - Connect           [FAIL] \n" );
	
	n_temp = pt->n_loopcnt;
	commandHandlerSelectSleep (0, 500);
	if ( pt->n_loopcnt != n_temp ) 
		fprintf ( stderr, " - LoopCnt           [OK] \n" );
	else
		fprintf ( stderr, " - LoopCnt           [FAIL] (%d)\n", n_temp );

	n_temp = pt->n_chkqueue;
	commandHandlerSelectSleep (0, 500);
	if ( pt->n_chkqueue != n_temp ) 
		fprintf ( stderr, " - ChkQueue          [OK] \n" );
	else
		fprintf ( stderr, " - ChkQueue          [FAIL] (%d)\n", n_temp );

	n_temp = pt->n_tx;
	commandHandlerSelectSleep (0, 500);
	if ( pt->n_tx != n_temp ) 
		fprintf ( stderr, " - MsgTX             [OK] \n" );
	else
		fprintf ( stderr, " - MsgTX             [FAIL] (%d)\n", n_temp );

	n_temp = pt->n_rx;
	commandHandlerSelectSleep (0, 500);
	if ( pt->n_rx != n_temp ) 
		fprintf ( stderr, " - MsgRX             [OK] \n" );
	else
		fprintf ( stderr, " - MsgRX             [FAIL] (%d)\n", n_temp );

	pq = (STATUS_QUEUE_T *) &gp_status->st_elba;
	n_temp = pq->n_indata;
	commandHandlerSelectSleep (0, 500);
	fprintf ( stderr, " :: Elba Queue \n" );
	if ( pq->n_indata != n_temp ) 
		fprintf ( stderr, " - QueueIn           [OK] \n" );
	else
		fprintf ( stderr, " - QueueIn           [FAIL] (%d)\n", n_temp );

	n_temp = pq->n_outdata;
	commandHandlerSelectSleep (0, 500);
	if ( pq->n_outdata != n_temp ) 
		fprintf ( stderr, " - QueueOut          [OK] \n" );
	else
		fprintf ( stderr, " - QueueOut          [FAIL] (%d)\n", n_temp );

	fprintf ( stderr, " - QueueFull         [%d] \n", pq->n_full );
	fprintf ( stderr, " - QueueEmpty        [%d] \n", pq->n_empty );

	pq = (STATUS_QUEUE_T *) &gp_status->st_elbamsg;
	n_temp = pq->n_indata;
	commandHandlerSelectSleep (0, 500);
	fprintf ( stderr, " :: Elba Message Queue \n" );
	if ( pq->n_indata != n_temp ) 
		fprintf ( stderr, " - QueueIn           [OK] \n" );
	else
		fprintf ( stderr, " - QueueIn           [FAIL] (%d)\n", n_temp );

	n_temp = pq->n_outdata;
	commandHandlerSelectSleep (0, 500);
	if ( pq->n_outdata != n_temp ) 
		fprintf ( stderr, " - QueueOut          [OK] \n" );
	else
		fprintf ( stderr, " - QueueOut          [FAIL] (%d)\n", n_temp );

	fprintf ( stderr, " - QueueFull         [%d] \n", pq->n_full );
	fprintf ( stderr, " - QueueEmpty        [%d] \n", pq->n_empty );

	fprintf ( stderr, "Elba Thread Test Complete \n\n" );
	fflush(stderr);

	fprintf ( stderr, "Net32 Thread Testing ################ \n" );
	pt = (STATUS_THREAD_T *) &gp_status->st_net32_t;
	if ( pt->n_connetion == 1 ) 
		fprintf ( stderr, " - Connect           [OK] \n" );
	else
		fprintf ( stderr, " - Connect           [FAIL] \n" );
	
	n_temp = pt->n_loopcnt;
	commandHandlerSelectSleep (0, 500);
	if ( pt->n_loopcnt != n_temp ) 
		fprintf ( stderr, " - LoopCnt           [OK] \n" );
	else
		fprintf ( stderr, " - LoopCnt           [FAIL] (%d)\n", n_temp );

	n_temp = pt->n_chkqueue;
	commandHandlerSelectSleep (0, 500);
	if ( pt->n_chkqueue != n_temp ) 
		fprintf ( stderr, " - ChkQueue          [OK] \n" );
	else
		fprintf ( stderr, " - ChkQueue          [FAIL] (%d)\n", n_temp );

	n_temp = pt->n_tx;
	commandHandlerSelectSleep (0, 500);
	if ( pt->n_tx != n_temp ) 
		fprintf ( stderr, " - MsgTX             [OK] \n" );
	else
		fprintf ( stderr, " - MsgTX             [FAIL] (%d)\n", n_temp );

	n_temp = pt->n_rx;
	commandHandlerSelectSleep (0, 500);
	if ( pt->n_rx != n_temp ) 
		fprintf ( stderr, " - MsgRX             [OK] \n" );
	else
		fprintf ( stderr, " - MsgRX             [FAIL] (%d)\n", n_temp );

	pq = (STATUS_QUEUE_T *) &gp_status->st_net32;
	n_temp = pq->n_indata;
	commandHandlerSelectSleep (0, 500);
	fprintf ( stderr, " :: Net32 Queue \n" );
	if ( pq->n_indata != n_temp ) 
		fprintf ( stderr, " - QueueIn           [CNANGE] \n" );
	else
		fprintf ( stderr, " - QueueIn           [NOT CNANGE] (%d)\n", n_temp );

	n_temp = pq->n_outdata;
	commandHandlerSelectSleep (0, 500);
	if ( pq->n_outdata != n_temp ) 
		fprintf ( stderr, " - QueueOut          [CNANGE] \n" );
	else
		fprintf ( stderr, " - QueueOut          [NOT CNANGE] (%d)\n", n_temp );

	fprintf ( stderr, " - QueueFull         [%d] \n", pq->n_full );
	fprintf ( stderr, " - QueueEmpty        [%d] \n", pq->n_empty );

	pq = (STATUS_QUEUE_T *) &gp_status->st_net32msg;
	n_temp = pq->n_indata;
	commandHandlerSelectSleep (0, 500);
	fprintf ( stderr, " :: Net32 Message Queue \n" );
	if ( pq->n_indata != n_temp ) 
		fprintf ( stderr, " - QueueIn           [CNANGE] \n" );
	else
		fprintf ( stderr, " - QueueIn           [NOT CNANGE] (%d)\n", n_temp );

	n_temp = pq->n_outdata;
	commandHandlerSelectSleep (0, 500);
	if ( pq->n_outdata != n_temp ) 
		fprintf ( stderr, " - QueueOut          [CNANGE] \n" );
	else
		fprintf ( stderr, " - QueueOut          [NOT CNANGE] (%d)\n", n_temp );

	fprintf ( stderr, " - QueueFull         [%d] \n", pq->n_full );
	fprintf ( stderr, " - QueueEmpty        [%d] \n", pq->n_empty );

	fprintf ( stderr, "Net32 Thread Test Complete \n\n" );
	fflush(stderr);
	return SUCCESS;
}























#if 0
/*******************************************************************/
void commandHandlerSelectSleep(int sec,int msec) 
/*******************************************************************/
{
    struct timeval tv;
    tv.tv_sec=sec;
    tv.tv_usec=msec*1000;                
    select(0,NULL,NULL,NULL,&tv);
    return;
}


/*******************************************************************/
void* command_handler_main(void* arg)
/*******************************************************************/
{
	//Variables
	int i, j;
	//
	int fd_max;
	fd_set reads, temps;
	struct timeval tv;
    struct timespec req;
    struct timespec rem;	
	//
	int server_socket;
	int client_socket;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	//
	int fd;
	int sin_size;
	int server_port;
	int status;
	

	//Initialize
	i = j = 0;
	//
	fd_max = 0;
	FD_ZERO(&reads);
	FD_ZERO(&temps);
	tv.tv_sec = 1;
	tv.tv_usec = 0; //0.01sec
	//
	server_socket = 0;
	client_socket = 0;
	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));
	//
	fd = 0;
	sin_size = sizeof(client_addr);
	server_port = COMMAND_SERVER_PORT;
	status = INIT_PROCESS;
	
	//Ignore broken_pipe signal
	signal(SIGPIPE, SIG_IGN);
	sleep(1);	 
	//usleep(3000);
	
	while(1)
	{
		//commandHandlerSemaWait(100);
		switch(status)
		{
			case INIT_PROCESS:
			{
				server_socket = socket(PF_INET, SOCK_STREAM, 0);
				printf("Command Server Started, Port [%d]\n", server_port);
				//
				server_addr.sin_family = AF_INET;
				server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
				server_addr.sin_port = htons(server_port);
				//
				setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
				//
				if(bind(server_socket, (struct sockaddr *)&server_addr,
							sizeof(server_addr)) < 0)
				{
					printf("[ERROR] In Bind of Command Server\n");
					close(server_socket);
					status = INIT_PROCESS;
					sleep(10);
					//commandHandlerSemaWait(5000);
					break;
				}
				//
				if(listen(server_socket, 10) < 0)
				{
					printf("[ERROR] In Listen of Command Server\n");
					close(server_socket);
					status = INIT_PROCESS;
					sleep(10);
					//commandHandlerSemaWait(5000);
					break;
				}
				//
				FD_ZERO(&reads);
				FD_SET(server_socket, &reads);
				fd_max = server_socket;
				//
				status = SELECT_PROCESS;
				break;
			}
			//
			case SELECT_PROCESS:
			{
				//commandHandlerSemaWait(500);
				/*
				CPU 점유율을 낮추기 위하여 slectsleep 합니다.
				*/				
				commandHandlerSelectSleep(0,200);
				//
				temps = reads;
				//
				if(select(fd_max+1, &temps, 0, 0, &tv) == -1)
				{
					printf("[ERROR] In Select of Command Server\n");
					status = INIT_PROCESS;
					sleep(10);
					//commandHandlerSemaWait(5000);
					break;
				}
				//
				status = SELECT_PROCESS;
				//
				for(fd = 0; fd < (fd_max + 1); fd++)
				{
					if(FD_ISSET(fd, &temps))
					{	
						//if connection's been requested,
						if(fd == server_socket)
						{
							status = CONNECTION_REQUESTED;
							break;
						}
						else
						{
							status = HANDLE_COMMAND;
							break;
						}
					}
				}
				break;
			}
			//
			case CONNECTION_REQUESTED:
			{
				printf("B");
				client_socket = accept(server_socket, (struct sockaddr *)&client_addr,
						               &sin_size);
				//
				if (client_socket == -1)
				{
					printf("[Error] In Accept\n");
					status = SELECT_PROCESS;
					break;
				}
				//
				FD_SET(client_socket, &reads);
				if(fd_max < client_socket)
					fd_max = client_socket;
				//
				status = SELECT_PROCESS;
				break;
			}
			//
			case HANDLE_COMMAND:
			{
				do_handle_command(fd, &reads);
				status = SELECT_PROCESS;
				break;
			}
		}			
	}
	//
	//return;
}

/************************************************************/
int
do_handle_command(int fd, fd_set* reads)
/************************************************************/
{
	//Variables
	int i;
	int rxmsg_length;
	int status;
	
	//Initialize
	i = 0;
	rxmsg_length = 0;
	status = GET_COMMAND_TYPE;
	
	memset(cmd_rx_msg, 0, sizeof(cmd_rx_msg));

	rxmsg_length = recv(fd, cmd_rx_msg, sizeof(cmd_rx_msg), 0);
    //
	if(rxmsg_length == 0)
	{
		FD_CLR(fd, reads);
		close(fd);
		//printf("Command client closed, file descriptor [%d]\n", fd);
		return SUCCESS;
	}

#if 0
	printf("Command Handler RX msg[%d] : \n", rxmsg_length);
	for(i = 0; i < rxmsg_length; i++)
	{
		printf("%02x ", cmd_rx_msg[i]);
	}
	printf("\n");
#endif

	while(1)
	{
		switch(status)
		{
			case GET_COMMAND_TYPE:
				status = cmd_rx_msg[0];
				break;

			case COMMAND_PSET:
				return handleCommandPset(fd, rxmsg_length);

			case COMMAND_PGET_MULTI_POINT:
				return handleCommandPgetMultiPoint(fd, rxmsg_length);

			case COMMAND_DEBUG_TCP:
				return handleCommandDebugTCP(fd, rxmsg_length);

			case COMMAND_DEBUG_NTX:
				return handleCommandDebugNTX(fd, rxmsg_length);

			case COMMAND_DEBUG_NRX:
				return handleCommandDebugNRX(fd, rxmsg_length);

			case COMMAND_STATUS:
				return handleCommandStatus(fd, rxmsg_length);

			default : 
				printf("Unknown Command Type [%d]\n", status);
				return ERROR;
		}
	}
}

/************************************************************/
int 
handleCommandPset(int fd, int rxmsg_length)
/************************************************************/
{
	//Variables
	int i;
	point_info point;
	
	//Initialize
	i = 0;
	memset(&point, 0, sizeof(point));
	
	if(rxmsg_length < 9)
	{
		printf("[ERROR] Incorrect pSet message in command handler\n");
		return ERROR;
	}

	memcpy(&point.pcm,	 &cmd_rx_msg[1], sizeof(short));
	memcpy(&point.pno,	 &cmd_rx_msg[3], sizeof(short));
	memcpy(&point.value, &cmd_rx_msg[5], sizeof(float));

	if(point.pcm < 0 || point.pcm >= MAX_NET32_NUMBER || 
	   point.pno < 0 || point.pno >= MAX_POINT_NUMBER)
	{
		printf("[ERROR] Incorrect pSet pcm [%hd] pno [%hd]\n", point.pcm, point.pno);
		return ERROR;
	}
/*
	//Update point table
	pthread_mutex_lock(&pointTable_mutex);
	point_table[point.pcm][point.pno] = point.value;
	pthread_mutex_unlock(&pointTable_mutex);

	//put to interface_message_queue
	point.message_type = CMD_PSET;
	pthread_mutex_lock(&interfaceMessageQ_mutex);
	status.interface_message_in++;
	putq(&interface_message_queue, point);
	pthread_mutex_unlock(&interfaceMessageQ_mutex);				
*/
	pSet(point.pcm, point.pno, point.value);
	return SUCCESS;
}

/************************************************************/
int 
handleCommandPgetMultiPoint(int fd, int rxmsg_length)
/************************************************************/
{
	//Variables
	int i;
	short pcm;
	short pno;
	int number;
	float* fvalue;
	point_info point;
	
	//Initialze
	i       = 0;
	pcm     = 0;
	pno     = 0;
	number  = 0;
	memset(&point, 0, sizeof(point));
	memset(cmd_tx_msg, 0, sizeof(cmd_tx_msg));
	
	if(rxmsg_length < 8) {
		//printf("[ERROR] Incorrect pGetPlural message in command handler\n");
		return ERROR;
	}

	memcpy(&pcm,   &cmd_rx_msg[1], sizeof(short));
	memcpy(&pno,   &cmd_rx_msg[3], sizeof(short));
	memcpy(&number, &cmd_rx_msg[5], sizeof(int));

	if(pcm < 0 || pcm >= MAX_NET32_NUMBER  || 
	   pno < 0 || pno >= MAX_POINT_NUMBER  ||
	   number < 0 || number > MAX_POINT_NUMBER)	{
		//printf("[ERROR] Incorrect pGet pcm [%hd] pno [%hd] number [%hd]\n", 
		//	pcm, pno, number);
		return ERROR;
	}

	//Set default value
	point.pcm = pcm;
	point.pno = pno;
	fvalue = (float *) malloc(sizeof(float) * number);
	memset(fvalue, 0, sizeof(fvalue));
	
	//Get values from point_table
	pthread_mutex_lock(&pointTable_mutex);
	for(i = 0; i < number; i++)
		fvalue[i] = point_table[pcm][pno + i];
	pthread_mutex_unlock(&pointTable_mutex);
	point.message_type = COMMAND_PGET_SUCCESS;

	fprintf(stedrr, "\n  PCM         VALUE \n");
	fprintf(stderr, "================================================\n");
	for (i = 0; i <= number; i++) {	
		//printf(" %02d-%03d       %0.2f\n", 
		//    pcm, pno+i, point_table[pcm][pno+i]);
		fprintf(stderr, " %02d-%03d       %0.2f\n", 
		    pcm, pno+i, point_table[pcm][pno+i]);
	}

	fflush(stderr);

	free(fvalue);
	return SUCCESS;
}

/************************************************************/
int 
handleCommandDebugTCP(int fd, int rxmsg_length)
/************************************************************/
{
	if(rxmsg_length < 5)
	{
		printf("[ERROR] %s-%d rxmsg_length = %d\n", 
				__FUNCTION__, 
				__LINE__, 
				rxmsg_length);
		return ERROR;
	}

	if(cmd_rx_msg[4] == 1)
	{
		printf("debug tcp on\n");	
		gc_dbg_txmsg = 1;
		debug_elba_rx = 1;
	}		
	else
	{
		gc_dbg_txmsg = 0;
		debug_elba_rx = 0;
		printf("debug tcp off\n");		
	}

	return SUCCESS;
}

/************************************************************/
int 
handleCommandDebugNTX(int fd, int rxmsg_length)
/************************************************************/
{
	if(rxmsg_length < 5)
	{
		printf("[ERROR] %s-%d rxmsg_length = %d\n", 
				__FUNCTION__, 
				__LINE__, 
				rxmsg_length);
		return ERROR;
	}

	if(cmd_rx_msg[4] == 1)
	{
		printf("debug ntx on\n");	
		gc_dbg_txmsg = 1;
	}		
	else
	{
		gc_dbg_txmsg = 0;
		printf("debug ntx off\n");		
	}

	return SUCCESS;
}

/************************************************************/
int 
handleCommandDebugNRX(int fd, int rxmsg_length)
/************************************************************/
{
	if(rxmsg_length < 5)
	{
		printf("[ERROR] %s-%d rxmsg_length = %d\n", 
				__FUNCTION__, 
				__LINE__, 
				rxmsg_length);
		return ERROR;
	}

	if(cmd_rx_msg[4] == 1)
	{
		printf("debug nrx on\n");	
		debug_elba_rx = 1;
	}		
	else
	{
		debug_elba_rx = 0;
		printf("debug nrx off\n");		
	}

	return SUCCESS;
}


/************************************************************/
int 
handleCommandStatus(int fd, int rxmsg_length)
/************************************************************/
{
	if(rxmsg_length < 7)
	{
		printf("[ERROR] %s-%d rxmsg_length = %d\n", 
				__FUNCTION__, 
				__LINE__, 
				rxmsg_length);
		return ERROR;
	}

	printf("Status\n");
	printf("TxBytes: %08.0f 	RxBytes: %08.0f\n", status.tcp_total_tx, status.tcp_total_rx );
	printf("\nQueue\n");
	printf("elba            InQ: %08.0f    OutQ: %08.0f    Full: %08.0f\n", 
		status.elba_in,
		status.elba_out,
		status.elba_full);
	printf("elba_msg        InQ: %08.0f    OutQ: %08.0f    Full: %08.0f\n", 
		status.elba_message_in,
		status.elba_message_out,
		status.elba_message_full);
	printf("net32 	        InQ: %08.0f    OutQ: %08.0f    Full: %08.0f\n", 
		status.net32_in,
		status.net32_out,
		status.net32_full);
	printf("net32_msg       InQ: %08.0f    OutQ: %08.0f    Full: %08.0f\n", 
		status.net32_message_in,
		status.net32_message_out,
		status.net32_message_full);
	printf("interface_msg   InQ: %08.0f    OutQ: %08.0f    Full: %08.0f\n", 
		status.interface_message_in,
		status.interface_message_out,
		status.interface_message_full);
	printf("bacnet_msg      InQ: %08.0f    OutQ: %08.0f    Full: %08.0f\n", 
		status.bacnet_message_in,
		status.bacnet_message_out,
		status.bacnet_message_full);
	
	return SUCCESS;
}
#endif
