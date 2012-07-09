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

#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"
#include "queue_handler.h"									// queue handler

#include "cmd_handler.h"
#include "point_mgr.h"
#include "iface_ccms.h"										// interface ccms
#include "iface_handler.h"										// interface ccms
#include "point_observer.h"										// interface ccms

//extern variable
extern pthread_mutex_t pointTable_mutex;										// point table mutex

// from main.c
extern int g_nMyPcm;
extern char *g_chDfnTypeName[];	// point-define type name
//extern define_Info dfn_point[MAX_POINT_NUMBER];
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];
extern PTBL_INFO_T *g_pPtbl;
//CHK_STATUS_T *g_pStatus;														// g_pStatus

extern DNP_DIO_Point DNP_di_point[512];
extern DNP_DIO_Point DNP_do_point[512];
extern DNP_AIO_Point DNP_ai_point[512];
extern DNP_AIO_Point DNP_ao_point[512];
//
extern int debug_plc_tx;
extern int debug_plc_rx;


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
//extern int g_nStatusNet32;

// from point_handler.c
extern int gn_adc_dbg;

// from iface_subio.c
//extern int g_subio_dbg;

// GHP Interface
extern int g_dbgShow;		// debug-text show enable/disable

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
int cmd_dbgSubio(void *pCmd);
int cmd_dbgIface(void *pCmd);
int cmd_rpset(void *pCmd);
int cmd_hset(void *pCmd);
int cmd_hget(void *pCmd);
int cmd_dinfo(void *pCmd);
int cmd_pntlog(void *pCmd);
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
		case COMMAND_DEBUG_SUBIO:		cmd_dbgSubio(p);	break;
		case COMMAND_DEBUG_IFACE:		cmd_dbgIface(p);	break;
		case COMMAND_PREQ:				p->on_preq(p);		break;
		case COMMAND_MTR:				p->on_mtr(p);		break;
		case COMMAND_NODE:				p->on_node(p);		break;
		case COMMAND_VIEW:				p->on_view(p);		break;
		case COMMAND_APSET:				cmd_apset(p);		break;
		case COMMAND_PDEF:				cmd_pdef(p);		break;
		case COMMAND_CAL:				cmd_calibration(p);	break;
		case COMMAND_STATUS:			cmd_status(p);		break;
		case COMMAND_RPSET:				cmd_rpset(p);		break;
		case COMMAND_HSET:				cmd_hset(p);		break;
		case COMMAND_HGET:				cmd_hget(p);		break;
		case COMMAND_DINFO:				cmd_dinfo(p);		break;
		case COMMAND_PNT_LOG:			cmd_pntlog(p);		break;
			
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
		fprintf( stdout, "[ERROR] Command thread init error.\n");
		fflush( stdout );		
		system("killall duksan");
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
	syslog_record(SYSLOG_DESTROY_CMD_HANDLER);	
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
	
	if ( pcm == g_nMyPcm ) {
		fprintf( stderr, "\n  PCM       VALUE   TYPE   HYST   OFFSET   SCALE       MIN         MAX       ADC\n");
		fprintf( stderr, "====================================================================================\n");
		for (i = 0; (pno + i) <= number; i++) {
			pthread_mutex_lock( &pointTable_mutex );
			fprintf( stderr, " %02d-%03d   %7.2f    %s   %0.2f     %0.2f    %0.2f   %6.2f   %6.2f   %4d\n", 
					 //pcm, pno+i, g_fExPtbl[pcm][pno+i],
					 pcm, pno+i, g_pPtbl[pno+i].f_val,
					 g_chDfnTypeName[g_pPtbl[pno+i].n_type], 
					 g_pPtbl[pno+i].f_hyst, 
					 g_pPtbl[pno+i].f_offset, 
					 g_pPtbl[pno+i].f_scale, 
					 g_pPtbl[pno+i].f_min, 
					 g_pPtbl[pno+i].f_max, 
					 g_pPtbl[pno+i].n_adc);
			pthread_mutex_unlock( &pointTable_mutex );
		}
	}
	else {
		fprintf( stderr, "\n  PCM       VALUE      INFORMATION\n");
		fprintf( stderr, "====================================\n");
		for (i = 0; (pno + i) <= number; i++) {
			pthread_mutex_lock( &pointTable_mutex );
			fprintf( stderr, " %02d-%03d   %7.2f    No Information\n", 
				   pcm, pno+i, g_fExPtbl[pcm][pno+i] );
			pthread_mutex_unlock( &pointTable_mutex );
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

/************************************************************/
int cmd_dbgSubio(void *pCmd)
/************************************************************/
{
	COMMAND_T *p;
	unsigned char *pBuf;
	
	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;

	if(p->recvLength < 5) {
		printf("[ERROR] Incorrect dbgSubio message in command handler\n");
		return ERROR;
	}

	if(pBuf[4] == 1) {
		printf("debug subio on\n");	
		//g_subio_dbg = 1;
	}		
	else {
		//g_subio_dbg = 0;
		printf("debug subio off\n");		
	}

	return SUCCESS;
}



/************************************************************/
int cmd_dbgIface(void *pCmd)
/************************************************************/
{
	COMMAND_T *p;
	unsigned char *pBuf;
	
	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;

	if(p->recvLength < 5) {
		printf("[ERROR] Incorrect dbgSubio message in command handler\n");
		return ERROR;
	}

	if(pBuf[4] == 1) {
		printf("debug iface on\n");	
		g_dbgShow = 1;
	}		
	else {
		g_dbgShow = 0;
		printf("debug iface off\n");		
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

	fprintf(stderr, "\nMy Id  : %02d", g_nMyPcm);

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
	/*
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
	*/
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
	unsigned short w_pcm;
	unsigned char *p_buf;
	
	p = (COMMAND_T *)pCmd;
	p_buf = (unsigned char *)p->rxbuf;
	
	if(p->recvLength < 7) {
		fprintf(stdout, "[ERROR] Incorrect apset message in command handler\n");
		fflush(stdout);
		return ERROR;
	}

	memcpy( &w_pcm, &p_buf[1], sizeof(unsigned short) );
	memcpy( &f_val, &p_buf[3], sizeof(float) );
	
	fprintf(stdout, "APSET : %d, %f\n", w_pcm, f_val);
	fflush(stdout);

	for (n_pno = 0; n_pno < MAX_POINT_NUMBER; n_pno++ ) {
		pSet( w_pcm, n_pno, f_val);
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
		
		if ( w_pcm != g_nMyPcm ) {
			fprintf( stderr, "[ERR] pdef command only g_nMyPcm\n" );
			fflush(stderr);
			return SUCCESS;
		}

		w_type = (unsigned short)g_pPtbl[w_pno].n_type;
		f_hyst = g_pPtbl[w_pno].f_hyst;
		f_scale = g_pPtbl[w_pno].f_scale;
		f_offset = g_pPtbl[w_pno].f_offset;
		f_min = g_pPtbl[w_pno].f_min;
		f_max = g_pPtbl[w_pno].f_max;
		
		fprintf( stderr, "pcm = %d, pno = %d, type = %s\n", w_pcm, w_pno, g_chDfnTypeName[w_type] );
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

	if ( w_pcm != g_nMyPcm ) {
		fprintf( stderr, "[ERR] pdef command only g_nMyPcm\n" );
		fflush(stderr);
		return SUCCESS;
	}

	fprintf( stderr, "pdef %d %d %s   %0.2f  %0.2f  %0.2f  %0.2f  %0.2f\n",
			 w_pcm, w_pno, g_chDfnTypeName[w_type],
			 f_hyst, f_scale, f_offset, f_min, f_max);
	fflush(stderr);

	g_pPtbl[w_pno].n_type = w_type;
	g_pPtbl[w_pno].f_hyst = f_hyst;
	g_pPtbl[w_pno].f_scale = f_scale;
	g_pPtbl[w_pno].f_offset = f_offset;
	g_pPtbl[w_pno].f_min = f_min;
	g_pPtbl[w_pno].f_max = f_max;
	
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
	/*
	STATUS_QUEUE_T *pq;
	STATUS_THREAD_T *pt;
	unsigned int n_temp;


	fprintf ( stderr, "\n\n" );
	fprintf ( stderr, "Point Thread Testing ################ \n" );
	pt = (STATUS_THREAD_T *) &g_pStatus->st_pnt_handler_t;
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
	pt = (STATUS_THREAD_T *) &g_pStatus->st_elba_t;
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

	pq = (STATUS_QUEUE_T *) &g_pStatus->st_elba;
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

	pq = (STATUS_QUEUE_T *) &g_pStatus->st_elbamsg;
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
	pt = (STATUS_THREAD_T *) &g_pStatus->st_net32_t;
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

	pq = (STATUS_QUEUE_T *) &g_pStatus->st_net32;
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

	pq = (STATUS_QUEUE_T *) &g_pStatus->st_net32msg;
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
*/	
	return SUCCESS;
}



int cmd_rpset(void *pCmd)
{
	COMMAND_T 		*p;
	unsigned short 	wRemoteId = 0;
	unsigned short 	wPcm = 0;
	unsigned short 	wPno = 0;
	unsigned short	wValue = 0;
	unsigned char 	*p_buf;
	point_info 		point;
	
	p = (COMMAND_T *)pCmd;
	p_buf = (unsigned char *)p->rxbuf;
	
	if(p->recvLength < 9) {
		fprintf(stdout, "\n[ERROR] Incorrect rpset message in command handler\n");
		fflush(stdout);
		return ERROR;
	}

	memcpy( &wRemoteId, &p_buf[1], sizeof(unsigned short) );
	memcpy( &wPcm, &p_buf[3], sizeof(unsigned short) );
	memcpy( &wPno, &p_buf[5], sizeof(unsigned short) );
	memcpy( &wValue, &p_buf[7], sizeof(unsigned short) );
	
	fprintf(stdout, "\nRPSET : %d, %d, %d, %d \n", 
			wRemoteId, wPcm,
			wPno, wValue);
	fflush(stdout);

	point.pcm = wPcm;
	point.pno = wPno;
	point.value = (float)wValue;
	point.message_type = (int)wRemoteId;

	//ccms_put_queue(&point);

	return SUCCESS;
}





void cmd_make_pctrl_file(point_info *p)
{
	int i = 0;
	FILE *fp;
	char chPer = 0x25;		// %
	char chChk = 0x22;		// "
	char chCr = 0x0d;		// 엔터
	int nIndex = 0; 
	char chFileName[32];

	//int nCount = 2;

	printf("%s() \n", __FUNCTION__);

	memset(chFileName, 0, sizeof(chFileName));
	sprintf(chFileName, "/httpd/pctrl.html");
	fp = fopen(chFileName, "w");
	if( fp == NULL ) {
		return;		
	}
	
	fprintf(fp, "<!DOCTYPE HTML PUBLIC %c-//WAPFORUM//DTD XHTML Mobile 1.2//EN%c %chttp://www.wapforum.org/DTD/xhtml-mobile12.dtd%c>", 
			chChk, chChk, chChk, chChk);
	fprintf(fp, "%c", chCr);
	fprintf(fp, "<html xmlns=%chttp://www.w3.org/1999/xhtml%c lang=%cko%c xml:lang=%cko%c>", 
			chChk, chChk, chChk, chChk, chChk, chChk);


	fprintf(fp, "%c", chCr);
	fprintf(fp, "<html>");
	fprintf(fp, "%c", chCr);

	fprintf(fp, "<header manifest=%cduksan.manifest%c>", chChk, chChk);
	fprintf(fp, "%c", chCr);

	// 항상 서버로부터 갱신되도록 하는 코드
	fprintf(fp, "<meta http-equiv=%cExpires%c content=%c0%c/> ",
			chChk, chChk, chChk, chChk);
	fprintf(fp, "%c", chCr);
	fprintf(fp, "<meta http-equiv=%cPragma%c content=%cno-cache%c/>",
			chChk, chChk, chChk, chChk);
	fprintf(fp, "%c", chCr);
 
/*
	fprintf(fp, "<meta http-equiv=%cContent-Type%c content=%ctext/html; charset=euc-kr%c />",
			chChk, chChk, chChk, chChk);
	fprintf(fp, "<meta name=%cviewport%c content=%cuser-scalable=no, initial-scale=1.0, maximum-scale=1.0, minimum-scale=1.0, ",
			chChk, chChk, chChk);
	fprintf(fp, "width=device-width%c />", chChk);
*/
	
	fprintf(fp, "<title>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "DDC Control Page - %02d", p->pcm);
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</title>");
	fprintf(fp, "%c", chCr);

	fprintf(fp, "<h2><b>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "DDC Control Page");
	fprintf(fp, "<br>");
	fprintf(fp, "Pcm %02d", p->pcm);
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</b></h2>");
	fprintf(fp, "%c", chCr);

	fprintf(fp, "</header>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "<body >");
	fprintf(fp, "%c", chCr);

	// Page 이동 Table 시작
	fprintf(fp, "<TABLE width=100%c height=40  height=30 border=1 cellpadding=0 cellspacing=0>",chPer);
	fprintf(fp, "%c", chCr);
	fprintf(fp, "<tr>");
	fprintf(fp, "%c", chCr);

	// PCM 선택
	fprintf(fp, "<td bgcolor=gold width=30%c ALIGN=CENTER>", chPer);
	fprintf(fp, "%c", chCr);
	fprintf(fp, "<b>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, " PCM 선택 ");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</b>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</td>");
	fprintf(fp, "%c", chCr);

	// select
	fprintf(fp, "<td  width=70%c ALIGN=CENTER>",chPer);
	fprintf(fp, "%c", chCr);
	fprintf(fp, "<form action=%cpctrl_wait.html%c method=%cpost%c name=%cst_form%c id=%cst_form%c>",
			chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);
	fprintf(fp, "%c", chCr);
	fprintf(fp, "<SELECT NAME=%cpCtrlGet%c onchange=%cthis.form.submit()%c> ",
			chChk, chChk, chChk, chChk);
	fprintf(fp, "%c", chCr);
	
	fprintf(fp, "<OPTION> 이동할 PCM 선택");
	fprintf(fp, "%c", chCr);
	for ( i = 0; i < 32; i++) {
		fprintf(fp, "<OPTION value=%d> PCM %02d", i, i);
		fprintf(fp, "%c", chCr);
	}
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</SELECT>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</form>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</td>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</tr>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</table>");
	fprintf(fp, "%c", chCr);

	// 중간 라인
	fprintf(fp, "<hr color =silver size=3>");
	fprintf(fp, "%c", chCr);

	// Table 시작
	fprintf(fp, "<TABLE width=100%c height=30  height=30 border=1 cellpadding=0 cellspacing=0>",chPer);
	fprintf(fp, "%c", chCr);
	
	// name
	fprintf(fp, "<b>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "<tr>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "<td bgcolor=skyblue width=30%c height=40 ALIGN=CENTER>", chPer);
	fprintf(fp, "%c", chCr);
	fprintf(fp, " 이름 ");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</td>");
	fprintf(fp, "%c", chCr);

	// status
	fprintf(fp, "<td bgcolor=skyblue width=30%c ALIGN=CENTER>",chPer);
	fprintf(fp, "%c", chCr);
	fprintf(fp,	" 상태 ");	
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</td>");
	fprintf(fp, "%c", chCr);

	// control Check
	fprintf(fp, "<td bgcolor=skyblue width=40%c ALIGN=CENTER>",chPer);
	fprintf(fp, "%c", chCr);
	fprintf(fp,	" 제어값 ");
	fprintf(fp, "%c", chCr);	
	fprintf(fp, "</td>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</tr>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</b>");
	fprintf(fp, "%c", chCr);

	for ( nIndex = 0; nIndex < 256; nIndex++) {
		fprintf(fp, "<tr>");
		fprintf(fp, "%c", chCr);
		// name
		fprintf(fp, "<td width=30%c ALIGN=CENTER>", chPer);
		fprintf(fp, "%c", chCr);
		fprintf(fp, " VR_%d_%d ", p->pcm, nIndex );
		fprintf(fp, "%c", chCr);
		fprintf(fp, "</td>");
		fprintf(fp, "%c", chCr);

		if ( p->pcm == g_nMyPcm ) {
			// status
			fprintf(fp, "<td width=30%c ALIGN=CENTER>",chPer);
			fprintf(fp, "%c", chCr);
			fprintf(fp,	" %.1f ", g_pPtbl[nIndex].f_val);	
			fprintf(fp, "%c", chCr);
			fprintf(fp, "</td>");
			fprintf(fp, "%c", chCr);
		}
		else {
			// status
			fprintf(fp, "<td width=30%c ALIGN=CENTER>",chPer);
			fprintf(fp, "%c", chCr);
			pthread_mutex_lock( &pointTable_mutex );
			fprintf(fp,	" %.1f ", g_fExPtbl[p->pcm][nIndex]);	
			pthread_mutex_unlock( &pointTable_mutex );
			fprintf(fp, "%c", chCr);
			fprintf(fp, "</td>");
			fprintf(fp, "%c", chCr);
		}
		// control
		fprintf(fp, "<td width=40%c ALIGN=CENTER>",chPer);
		fprintf(fp, "%c", chCr);

		fprintf(fp, "<form action=%cpctrl_wait.html%c method=%cpost%c name=%cst_form%c class=%cstyle2%c id=%cst_form%c>", 
				chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);
		//fprintf(fp, "<form action=%cpctrl.html%c method=%cpost%c name=%cst_form%c class=%cstyle2%c id=%cst_form%c>", 
		//		chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);
		fprintf(fp, "%c", chCr);
		fprintf(fp, "<label>");
		fprintf(fp, "%c", chCr);
		fprintf(fp, "<INPUT TYPE=%ctext%c  name=%cpCtrlVal_%d_%d%c align=%cabsbottom%c  size=10 /> ",
				chChk, chChk, chChk, p->pcm, nIndex, chChk, chChk, chChk);
		fprintf(fp, "%c", chCr);
		fprintf(fp, "</label>");
		fprintf(fp, "%c", chCr);
		fprintf(fp, "&nbsp;");
		fprintf(fp, "%c", chCr);
		fprintf(fp, "<label>");
		fprintf(fp, "%c", chCr);
		fprintf(fp, "<INPUT TYPE=%csubmit%c value=%c적 용%c align=%cabsbottom%c onclick=%cwindow.location.reload(true);%c/>",
				chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);
		fprintf(fp, "%c", chCr);
		fprintf(fp, "</label>");
		fprintf(fp, "%c", chCr);
		fprintf(fp, "</form>");
		fprintf(fp, "%c", chCr);

		fprintf(fp, "</td>");
		fprintf(fp, "%c", chCr);
		fprintf(fp, "</tr>");
		fprintf(fp, "%c", chCr);
	}
	
	fprintf(fp, "</table>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</body>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</html>");
	fprintf(fp, "%c", chCr);
	fclose(fp);
}


/*******************************************************************/
int cmd_hset(void *pCmd)
/*******************************************************************/
{
	//Variables
	//int i;
	point_info point;
	COMMAND_T *p;
	unsigned char *pBuf;

	printf("%s() \n", __FUNCTION__);
	
	//Initialize
	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;
	memset(&point, 0, sizeof(point));
	
	if(p->recvLength < 9) {
		printf("[ERROR] Incorrect hset message in command handler\n");
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

	printf("%s() \n", __FUNCTION__);

	pSet(point.pcm, point.pno, point.value);
	commandHandlerSelectSleep(0, 500);
	cmd_make_pctrl_file(&point);
	commandHandlerSelectSleep(0, 500);
	pnt_observer_value_check();
	commandHandlerSelectSleep(1, 0);
	system("m_maker");
	printf("hpset done\n");


	return SUCCESS;
}


/*******************************************************************/
int cmd_hget(void *pCmd)
/*******************************************************************/
{
	//Variables
	//int i;
	point_info point;
	COMMAND_T *p;
	unsigned char *pBuf;

	printf("%s() \n", __FUNCTION__);
	
	//Initialize
	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;
	memset(&point, 0, sizeof(point));
	
	if(p->recvLength < 3) {
		printf("[ERROR] Incorrect hget message in command handler\n");
		return ERROR;
	}

	memcpy(&point.pcm,	 &pBuf[1], sizeof(short));

	if(point.pcm < 0 || point.pcm >= MAX_NET32_NUMBER) {
		printf("[ERROR] Incorrect pSet pcm [%hd]\n", point.pcm);
		return ERROR;
	}

	printf("%s() \n", __FUNCTION__);
	cmd_make_pctrl_file(&point);
	printf("hget done\n");

	return SUCCESS;
}



/*******************************************************************/
int cmd_dinfo(void *pCmd)
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

/*
	if ( pcm == g_nMyPcm ) {
		fprintf( stderr, "\n  PCM       VALUE   TYPE   HYST   OFFSET   SCALE       MIN         MAX       ADC\n");
		fprintf( stderr, "====================================================================================\n");
		for (i = 0; (pno + i) <= number; i++) {
			fprintf( stderr, " %02d-%03d   %7.2f    %s   %0.2f     %0.2f    %0.2f   %6.2f   %6.2f   %4d\n", 
					 //pcm, pno+i, g_fExPtbl[pcm][pno+i],
					 pcm, pno+i, g_pPtbl[pno+i].f_val,
					 g_chDfnTypeName[g_pPtbl[pno+i].n_type], 
					 g_pPtbl[pno+i].f_hyst, 
					 g_pPtbl[pno+i].f_offset, 
					 g_pPtbl[pno+i].f_scale, 
					 g_pPtbl[pno+i].f_min, 
					 g_pPtbl[pno+i].f_max, 
					 g_pPtbl[pno+i].n_adc);
		}
	}
	else {
		fprintf( stderr, "\n  PCM       VALUE      INFORMATION\n");
		fprintf( stderr, "====================================\n");
		for (i = 0; (pno + i) <= number; i++) {
			fprintf( stderr, " %02d-%03d   %7.2f    No Information\n", 
				   pcm, pno+i, g_fExPtbl[pcm][pno+i] );
		}
	}
*/
/*
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
/*
DNP_DIO_Point DNP_di_point[512];
DNP_DIO_Point DNP_do_point[512];
DNP_AIO_Point DNP_ai_point[512];
DNP_AIO_Point DNP_ao_point[512];
*/
	fprintf( stderr, "\n  Dinfo %d %d %d \n", pcm, pno, number);
	fprintf( stderr, "====================================================================================\n");
	
	for ( i = pno; i < pno + number; i++ ) {
		switch (pcm) {
			case 1:
			fprintf( stderr, "DI Pno[%03d]  VAL  : [%2d],  PlcAddr = %04d,   OnAddr = %04d,  OffAddr = %04d\n", 
			i, DNP_di_point[i].val, DNP_di_point[i].PLCaddr, DNP_di_point[i].ONaddr, DNP_di_point[i].OFFaddr );	
			fflush(stderr);
			break;

			case 2:
			fprintf( stderr, "DO Pno[%03d]  VAL  : [%2d],  PlcAddr = %04d,   OnAddr = %04d,  OffAddr = %04d\n", 
			i, DNP_do_point[i].val, DNP_do_point[i].PLCaddr, DNP_do_point[i].ONaddr, DNP_do_point[i].OFFaddr );	
			fflush(stderr);
			break;			

			case 3:
			fprintf( stderr, "AI PNO : [%d]    VAL : [%10.3f]    sADDR : [%d]    SCALE : [%10.3f]    MIN : [%d]    MAX : [%d] \n", 
			i,  DNP_ai_point[i].val, DNP_ai_point[i].PLCaddr, DNP_ai_point[i].scale, DNP_ai_point[i].MinVal, DNP_ai_point[i].MaxVal);	
			fflush(stderr);
			break;			
			
			case 4:
			fprintf( stderr, "AI PNO : [%d]    VAL : [%10.3f]    sADDR : [%d]    SCALE : [%10.3f]    MIN : [%d]    MAX : [%d] \n", 
			i, DNP_ao_point[i].val, DNP_ao_point[i].PLCaddr, DNP_ao_point[i].scale, DNP_ao_point[i].MinVal, DNP_ao_point[i].MaxVal);	
			fflush(stderr);
			break;						
		}
	}
	

	fflush(stderr);
	return SUCCESS;
}

/*******************************************************************/
int cmd_pntlog(void *pCmd)
/*******************************************************************/
{	
	/*
	COMMAND_T *p;
	unsigned char *pBuf;

	//Initialize
	p = (COMMAND_T *)pCmd;
	pBuf = (unsigned char *)p->rxbuf;
	
	printf("%s() command = %d\n", __FUNCTION__, (int)pBuf[4]);

	pnt_loger((int)pBuf[4]);
	
	*/

	return SUCCESS;
}










