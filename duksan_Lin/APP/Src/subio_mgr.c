#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/poll.h>		// use poll event

#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

#include "iface_subio.h"


extern int g_nMyPcm;
extern PTBL_INFO_T *g_pPtbl;													// point-table


unsigned char subio_txmsg[MAX_BUFFER_SIZE];										// subio tx buffer
unsigned char subio_rxmsg[MAX_BUFFER_SIZE];										// subio rx buffer

int g_subiotest = 1;
int g_subio_dbg = 0;

unsigned int g_nSubioAliveChk[4];


static void sub_sleep(int sec,int msec)
// ----------------------------------------------------------------------------
// WAIT TIMER
// Description : use select function for timer.
// Arguments   : sec		Is a second value.
//				 usec		Is a mili-second value. 
// Returns     : none
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = msec * 1000;
    select( 0, NULL, NULL, NULL, &tv );
    return;
}


static IFACE_SUBIO_T *subio_new(void)
// ----------------------------------------------------------------------------
// CREATE IFACE_SUBIO_T structure
// Description : IFACE_SUBIO_T structure를 생성한다.
// Arguments   : none
// Returns     : p				pointer of IFACE_SUBIO_T structure	
{
	IFACE_SUBIO_T *p;

	p = (IFACE_SUBIO_T *)malloc( sizeof(IFACE_SUBIO_T) );
	memset( p, 0x00, sizeof(IFACE_SUBIO_T) );
	return p;
}


static int subio_open(IFACE_SUBIO_T *p)
// ----------------------------------------------------------------------------
// SUBIO UART2 OPEN
// Description : device file을 open한다.
// Arguments   : p				pointer of IFACE_SUBIO_T structure	
// Returns     : 0				Is successful
//				 -1				Is fail
{
	struct termios oldtio, newtio;
	
	/* uart2 initiailze */
	p->n_fd = open(UART2DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (p->n_fd < 0) { 
		printf("Serial UART2 Open fail\n");
		return -1; 
	}
	else
		printf("Serial UART2 Open success\n");
	
	if (tcgetattr(p->n_fd, &oldtio) < 0) {
		perror("error in tcgetattr");
		return -1; 
	}
	
	bzero(&newtio, sizeof(newtio));
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
	
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN]  = 1;
	
	tcflush(p->n_fd, TCIFLUSH);
	tcsetattr(p->n_fd,TCSANOW,&newtio);	

	fcntl(p->n_fd, F_SETFL, FNDELAY); 

	// poll initiailze 
	p->pollevents.fd = p->n_fd; 

	// Reference code
	//#define POLLIN 0x0001 // 읽을 데이터가 있다. 
	//#define POLLPRI 0x0002 // 긴급한 읽을 데이타가 있다. 
	//#define POLLOUT 0x0004 // 쓰기가 봉쇄(block)가 아니다. 
	//#define POLLERR 0x0008 // 에러발생 
	//#define POLLHUP 0x0010 // 연결이 끊겼음 
	//#define POLLNVAL 0x0020 // 파일지시자가 열리지 않은 것 같은, Invalid request (잘못된 요청) 	
	p->pollevents.events  = POLLIN | POLLERR;
	p->pollevents.revents = 0;	

	p->p_rx = (unsigned char *) &subio_txmsg;
	p->p_tx = (unsigned char *) &subio_rxmsg;
		
	memset( p->p_rx, 0x00, sizeof(MAX_BUFFER_SIZE) );
	memset( p->p_tx, 0x00, sizeof(MAX_BUFFER_SIZE) );

	return p->n_fd;
}


static void subio_close(IFACE_SUBIO_T *p)
// ----------------------------------------------------------------------------
// SUBIO UART2 CLOSE
// Description : device file을 CLOSE한다.
// Arguments   : p				pointer of IFACE_SUBIO_T structure	
// Returns     : none
{
	close(p->n_fd);
	memset( p, 0, sizeof(IFACE_SUBIO_T) );
	p->n_fd = -1;
}


static int subio_get_type(IFACE_SUBIO_T *p)
// ----------------------------------------------------------------------------
// GET SUBIO TYPE
// Description : SUBIO의 type을 결정한다. 
// Arguments   : p				pointer of IFACE_SUBIO_T structure	
// Returns     : n_type			return subio type
{
	// 그룹에 해당하는 point number를 찾는다.
	switch(p->n_grp) {
	// group 0 (pno 32 ~ 39)
	case SUBIO_GRP_0:
		p->n_pno = SUBIO_GRP_0_PNO;
		break;

	// group 1 (pno 40 ~ 47)
	case SUBIO_GRP_1:
		p->n_pno = SUBIO_GRP_1_PNO;
		break;

	// group 2 (pno 48 ~ 55)
	case SUBIO_GRP_2:
		p->n_pno = SUBIO_GRP_2_PNO;
		break;

	// group 3 (pno 56 ~ 63)
	case SUBIO_GRP_3:
		p->n_pno = SUBIO_GRP_3_PNO;
		break;

	// etc...
	default:
		return SUBIO_TYPE_NONE;
	}

	// point number에 해당하는 type을 찾는다.
	switch(g_pPtbl[p->n_pno].n_type) {
	// point type is DO
	case DFN_PNT_DO:
		p->n_type= SUBIO_TYPE_DO;
		// debug code
		if ( g_subio_dbg > 0 ) {
			fprintf( stdout, "\n[SUBIO] Type is DO\n" );
			fflush(stdout);
		}
		break;

	// point type is VO
	case DFN_PNT_VO:
		p->n_type = SUBIO_TYPE_VO;
		// debug code
		if ( g_subio_dbg > 0 ) {
			fprintf( stdout, "\n[SUBIO] Type is VO\n" );
			fflush(stdout);
		}

		break;

	// point type is ADI
	case DFN_PNT_DI2S:
	case DFN_PNT_VI:
	case DFN_PNT_CI:
	case DFN_PNT_JPT:
		// debug code
		if ( g_subio_dbg > 0 ) {
			fprintf( stdout, "\n[SUBIO] Type is ADI\n" );
			fflush(stdout);
		}

		p->n_type = SUBIO_TYPE_ADI;
		break;

	// etc...
	default:
		return SUBIO_TYPE_NONE;
	}


	return p->n_type;
}


static unsigned short subio_get_crc(unsigned char *p, unsigned int n_length)
// ----------------------------------------------------------------------------
// GET CRC OF SUBIO MESSAGE
// Description : SUBIO의 message의 crc를 계산한다.
// Arguments   : p				pointer of SUBUIO Tx message
// 				 n_length		crc를 제외한 data length
// Returns     : w_crc			return crc
{
	int i = 0;
	int k = 0;
	unsigned short w_crc = 0xffff;

	for( k = 0; k < n_length; k++ ) {
		w_crc ^=  (unsigned short)(p[k]) ;
		for ( i = 8; i > 0; i-- ) {				
			if( w_crc & 0x0001 ) 
				w_crc = (w_crc >> 1) ^ 0xA001;		  // Polynomial = 0xA001 
			else
				w_crc >>= 1;
		}
	}

	return 	w_crc;
}


static void subio_swap_short(unsigned short *p)
{
    unsigned char *pCh;
    unsigned char temp1;
    unsigned char temp2;
    
    pCh = (unsigned char *)p;

	temp1 = pCh[0];
    temp2 = pCh[1];

    pCh[0] = temp2;
    pCh[1] = temp1;
}


static unsigned short subio_get_dovalue(IFACE_SUBIO_T *p)
{
	int i = 0;
	unsigned short w_val = 0;

	for ( i = 0; i < SUBIO_POINT_COUNT; i++ ) {
		if ( g_pPtbl[p->n_pno + i].f_val > 0 )	
			w_val = w_val | (0x01 << i);
		else
			w_val = w_val | (0x00 << i); 
	}
	return w_val;
}


static void subio_get_vovalue(IFACE_SUBIO_T *p, SUBIO_VO_MSG_T  *pVo)
{
	int i = 0;
	unsigned char *pCh;
	unsigned short w_value = 0;

	pCh = (unsigned char *)&w_value;

	for ( i = 0; i < SUBIO_POINT_COUNT; i++ ) {
		w_value = (unsigned short)g_pPtbl[p->n_pno + i].f_val;
		// word 형태의 값의 상위 Byte와 하위 Byte를 swap 한다. 
		pVo->w_data[i]= ((pCh[0] << 8) & 0xff00) + pCh[1];
	}
}


static void subio_get_adivalue(IFACE_SUBIO_T *p, unsigned char *pc_val)
{
	int i = 0;

	for ( i = 0; i < SUBIO_POINT_COUNT; i++ ) {
		switch(g_pPtbl[p->n_pno + i].n_type) {
		// type is di2s
		case DFN_PNT_DI2S:
			*(pc_val + i) = SUBIO_ADI_DI;
			break;

		// type is vi
		case DFN_PNT_VI:
			*(pc_val + i) = SUBIO_ADI_VI;
			break;

		// type is ci
		case DFN_PNT_CI:
			*(pc_val + i) = SUBIO_ADI_CI;
			break;

		// type is jpt
		case DFN_PNT_JPT:
			*(pc_val + i) = SUBIO_ADI_JPT1000;
			break;

		// defulut type is di
		default:
			*(pc_val + i) = SUBIO_ADI_DI;
		}
	}
}


static void subio_domsg_request(IFACE_SUBIO_T *p)
// ----------------------------------------------------------------------------
// MAKE SUBIO DO MESSAGE
// Description : SUBIO의 DO message를 생성한다. 
// Arguments   : p				pointer of IFACE_SUBIO_T structure	
// Returns     : none
{
	int n_length;
	SUBIO_DO_MSG_T *p_do;
    //unsigned short w_value;
    unsigned short w_crc;
		
	memset ( p->p_tx, 0x00, MAX_BUFFER_SIZE );
	n_length = sizeof(SUBIO_DO_MSG_T);
	p_do = (SUBIO_DO_MSG_T *) p->p_tx;
	w_crc = 0;

    p_do->c_id = p->n_grp + 1;
    p_do->c_cmd = SUBIO_CMD_DO; 			
    p_do->w_addr = SUBIO_CMD_START_ADDR;		
    p_do->w_datacnt = SUBIO_CMD_DATA_CNT;		
    p_do->c_bytecnt = SUBIO_CMD_DO_BYTE_CNT; 

	p_do->w_data = subio_get_dovalue(p);
    subio_swap_short((unsigned short *)&p_do->w_data); 

	w_crc = subio_get_crc( p->p_tx, n_length - 2 );
    p_do->c_crchi = w_crc & SUBIO_MASK_BYTE;
    p_do->c_crclo = (w_crc >> 8)  & SUBIO_MASK_BYTE; 
	
	if ( g_subio_dbg > 0 ) { 
		fprintf( stdout, "id = 0x%x, \n", p_do->c_id );
		fprintf( stdout, "cmd = 0x%x, \n",  p_do->c_cmd );
		fprintf( stdout, "w_addr = 0x%x, \n",  p_do->w_addr );
		fprintf( stdout, "w_datacnt = 0x%x, \n",  p_do->w_datacnt );
		fprintf( stdout, "p_do->w_data = 0x%x, \n", p_do->w_data );
		fprintf( stdout, "w_crc = 0x%x\n",  w_crc );
	}
}


static void subio_vomsg_request(IFACE_SUBIO_T *p)
// ----------------------------------------------------------------------------
// MAKE SUBIO AO MESSAGE
// Description : SUBIO의 AO message를 생성한다. 
// Arguments   : p				pointer of IFACE_SUBIO_T structure	
// Returns     : none
{
	int n_length;
	SUBIO_VO_MSG_T *p_vo;
    unsigned short w_crc;
		
	memset ( p->p_tx, 0x00, MAX_BUFFER_SIZE );
	n_length = sizeof(SUBIO_VO_MSG_T);
	p_vo = (SUBIO_VO_MSG_T *) p->p_tx;
	w_crc = 0;

    p_vo->c_id = p->n_grp + 1;
    p_vo->c_cmd = SUBIO_CMD_VO; 			
    p_vo->w_addr = SUBIO_CMD_START_ADDR;		
    p_vo->w_datacnt = SUBIO_CMD_DATA_CNT;		
    p_vo->c_bytecnt = SUBIO_CMD_AO_BYTE_CNT; 
	
	subio_get_vovalue(p, p_vo);
    
	w_crc = subio_get_crc( p->p_tx, n_length - 2 );
    p_vo->c_crchi = w_crc & SUBIO_MASK_BYTE;
    p_vo->c_crclo = (w_crc >> 8)  & SUBIO_MASK_BYTE; 

	if ( g_subio_dbg > 0 ) { 
		fprintf( stdout, "id = 0x%x\n", p_vo->c_id );
		fprintf( stdout, "cmd = 0x%x \n",  p_vo->c_cmd );
		fprintf( stdout, "w_addr = 0x%x\n",  p_vo->w_addr );
		fprintf( stdout, "w_datacnt = 0x%x\n",  p_vo->w_datacnt );
		fprintf( stdout, "w_crc = 0x%x\n",  w_crc );
	}
}


static void subio_adimsg_request(IFACE_SUBIO_T *p)
// ----------------------------------------------------------------------------
// MAKE SUBIO AO MESSAGE
// Description : SUBIO의 ADI message를 생성한다. 
// Arguments   : p				pointer of IFACE_SUBIO_T structure	
// Returns     : none
{
	int n_length;
	SUBIO_ADI_MSG_T *p_adi;
    unsigned short w_value;
    unsigned short w_crc;
		
	memset ( p->p_tx, 0x00, MAX_BUFFER_SIZE );
	n_length = sizeof(SUBIO_ADI_MSG_T);
	p_adi = (SUBIO_ADI_MSG_T *) p->p_tx;
	w_value = 0;
	w_crc = 0;

    p_adi->c_id = p->n_grp + 1;
    p_adi->c_cmd = SUBIO_CMD_ADI; 			
    p_adi->w_addr = SUBIO_CMD_DEFINE_ADDR;		

	subio_get_adivalue(p, p_adi->c_data);

	w_crc = subio_get_crc( p->p_tx, n_length - 2 );
    p_adi->c_crchi = w_crc & SUBIO_MASK_BYTE;
    p_adi->c_crclo = (w_crc >> 8)  & SUBIO_MASK_BYTE; 

	if ( g_subio_dbg > 0 ) { 
		fprintf( stdout, "id = 0x%x\n", p_adi->c_id );
		fprintf( stdout, "cmd = 0x%x\n",  p_adi->c_cmd );
		fprintf( stdout, "w_addr = 0x%x\n",  p_adi->w_addr );
		fprintf( stdout, "w_crc = 0x%x\n",  w_crc );
	}
}


static void subio_send_msg(IFACE_SUBIO_T *p, int n_length)
// ----------------------------------------------------------------------------
// MESSAGE SEND TO SUBIO 
// Description : Message를 SUBIO로 전송한다. 
// Arguments   : p				pointer of IFACE_SUBIO_T structure	
// 				 n_length		length of tx message
// Returns     : none
{
	int i = 0;
	IFACE_SUBIO_T *p_uart2;
	
	p_uart2 = (IFACE_SUBIO_T *) p;
	write( p_uart2->n_fd, p_uart2->p_tx, n_length );

	if ( g_subio_dbg > 0 ) {
		fprintf( stdout, "Tx (%d) = ", n_length );
		for ( i = 0; i < n_length; i++ ) {
			fprintf( stdout, "0x%x ", p->p_tx[i] );
		}
		fprintf( stdout, "\n" );
		fflush(stdout);
	}
}


static int subio_recv_msg(IFACE_SUBIO_T *p)
{
	int n_length = 0;
	int n_ret = 0;;
	int n_bufwp = 0;
	int n_loopcnt = 0;
	IFACE_SUBIO_T *p_uart2;
	unsigned char *p_rx;

	p_uart2 = (IFACE_SUBIO_T *) p;
	p_rx = (unsigned char *)p_uart2->p_rx;
	memset( p_rx, 0, MAX_BUFFER_SIZE );

	while(1) {
		p_uart2->n_pollstate = poll(					// poll()을 호출하여 event 발생 여부 확인     
			(struct pollfd*)&p_uart2->pollevents,		// event 등록 변수
			1,  									// 체크할 pollfd 개수
			10);  									// time out 시간 (ms)	

		if ( 0 < p_uart2->n_pollstate) {                            // 발생한 event 가 있음
			if ( p_uart2->pollevents.revents & POLLIN ) {			// event 가 자료 수신?
				n_ret = read( p_uart2->n_fd, &p_rx[n_bufwp], 32 );
				n_bufwp += n_ret;
				n_loopcnt = 0;
			}
			else if ( p_uart2->pollevents.revents & POLLERR ) {	// event 가 에러?
				if ( g_subio_dbg > 0 ) {
					fprintf( stdout,  "Event is Error. Terminal Broken.\n" );
					fflush(stdout);
				}
				return -1;
			}
		}
		else {
			if ( 5 < n_loopcnt++ ) {
				n_length = n_bufwp;
				if ( g_subio_dbg > 0 ) {
					fprintf( stdout, "time out!!  length = %d\n", n_length );
					fflush(stdout);
				}
				//if (iBufWp > 0)
				//	p_uart2->on_recv_hadler(uart2);
				
				return n_length;
			}
		}
	}

	return n_length;
}


static void subio_msg_handler(IFACE_SUBIO_T *p, int n_length)
{
	int i = 0;
	SUBIO_ADI_MSG_RES_T *pAdiRes = NULL;
	unsigned short w_crc = 0;
	unsigned char c_chkcrc = 0;
	unsigned int n_pno = 0;
	unsigned short w_value;
	unsigned char *pCh = NULL;
	point_info pnt;

	if ( g_subio_dbg > 0 ) {
		for ( i = 0; i < n_length; i++ ) {
			fprintf(stdout, "0x%02x ", p->p_rx[i] ) ;
		}
		fprintf( stdout, "\n\n" );
		fflush(stdout);
	}

	if ( n_length < 5 )
		return;

	// only ADI-board parsing message.
	if ( p->n_type == SUBIO_TYPE_ADI ) {
		pAdiRes = (SUBIO_ADI_MSG_RES_T *)p->p_rx;

		w_crc = subio_get_crc( p->p_rx, n_length - 2 );

		c_chkcrc = p->p_rx[n_length - 2] - ( w_crc & 0xff );
		c_chkcrc = c_chkcrc + p->p_rx[n_length - 1] - ( (w_crc >> 8) & 0xff );
		if ( !c_chkcrc ) {
			n_pno = ( p->p_rx[0] * 8 ) + 24;
			
			for ( i = 0; i < SUBIO_POINT_COUNT; i++ ) {
				// subio
				pCh = (unsigned char *) &pAdiRes->w_data[i];
				w_value = ((pCh[0] << 8) & 0xff00) + pCh[1];
				//fprintf(stdout, "Pno = %d \n", n_pno ) ;
				//fprintf(stdout, "pAdiRes->w_data = %x \n", pAdiRes->w_data[i] ) ;	
				//fprintf(stdout, "w_value = %x \n", w_value ) ;	

				// scale과 offset 적용.
				pnt.pcm = g_nMyPcm;
				pnt.pno = n_pno;
				//pnt.value = (w_value * g_pPtbl[n_pno].f_scale) + g_pPtbl[n_pno].f_offset;

				/*
				// type이 VI인 경우 * 0.001을 한다. 
				if ( g_pPtbl[n_pno].n_type == DFN_PNT_VI ) {
					pnt.value  = pnt.value  * 0.001;
				}
				*/
	

				// type이 DI인 경우에만 1과 0으로 표현한다. 
				if ( g_pPtbl[n_pno].n_type == DFN_PNT_DI2S ) {
					pnt.value = (w_value * g_pPtbl[n_pno].f_scale) + g_pPtbl[n_pno].f_offset;
					if ( pnt.value > 0 ) 
						pnt.value = 1;
					else 
						pnt.value = 0;
				}
				else if ( g_pPtbl[n_pno].n_type == DFN_PNT_JPT ) {
					pnt.value = (w_value * g_pPtbl[n_pno].f_scale) + g_pPtbl[n_pno].f_offset;
					pnt.value  = pnt.value  * 0.01;
				}				
				else if ( g_pPtbl[n_pno].n_type == DFN_PNT_CI ) {
					//pnt.value  = w_value  * 0.01;
					//pnt.value = (pnt.value * g_pPtbl[n_pno].f_scale) + g_pPtbl[n_pno].f_offset;
					
					pnt.value  = w_value  * 0.001;					
					pnt.value = (pnt.value * g_pPtbl[n_pno].f_scale) + g_pPtbl[n_pno].f_offset;
				}					
				else if ( g_pPtbl[n_pno].n_type == DFN_PNT_VI ) {
					pnt.value = (w_value * g_pPtbl[n_pno].f_scale) + g_pPtbl[n_pno].f_offset;
					pnt.value  = pnt.value  * 0.001;
				}

				// pnt_handler의 pnt_local_subio()를 통해서 point-table에 값을 쓴다. 
				pnt_local_subio(&pnt);

				//fprintf(stdout, "g_pPtbl[%d].f_val = %f \n", n_pno, g_pPtbl[n_pno].f_val ) ;	
				//fflush(stdout);
				
				// It chanage point number 
				n_pno++;
			}
		}
		else {
			fprintf( stdout, "\n[ERR] SUBIO Wrong CRC \n" );
			fflush(stdout);
		} 
	}

}


void subio_value_zero(int nGrp)
{
	int i = 0;
	point_info point;
	
	point.pcm = g_nMyPcm;
	for ( i = 0; i < 8; i++ ) {
		point.pno = 32 + (nGrp * 8) + i;
		point.value = 0;
		pnt_local_subio(&point);		
	}	
}


void *subio_main(void *argv)
{
	int n_grp = 0;
	int n_txlength = 0;
	int n_rxlength = 0;
	IFACE_SUBIO_T *p_subio = NULL;

	p_subio = subio_new();

	while(1) {
		// open subio-driver
		if( subio_open(p_subio) < 0 ) {
			fprintf( stdout, "[ERR] SUBIO DRIVER Open Error\n");
			fflush(stdout);
			subio_close(p_subio);
			sleep( 1 );
			continue;
		}
		
		for(;;) {
			//sub_sleep(0, 300);

			for ( n_grp = 0; n_grp < 4; n_grp++ ) {
				g_nSubioAliveChk[n_grp]++;
				
				if ( g_nSubioAliveChk[n_grp] > 3 ) {
					g_nSubioAliveChk[n_grp] = 0;
					subio_value_zero(n_grp);
				}
				
				// get subio type
				p_subio->n_grp = n_grp;
				if ( subio_get_type(p_subio) == SUBIO_TYPE_NONE ) {
					sub_sleep(0, 100);
					// debug code
					if ( g_subio_dbg > 0 ) {
						fprintf( stdout, "[SUBIO] point not define\n" );
						fflush(stdout);
					}
					continue;
				}

				sub_sleep(0, 300);
				
				// make subio send message
				switch(p_subio->n_type)	{
				// make do message.
				case SUBIO_TYPE_DO:
					subio_domsg_request(p_subio);
					n_txlength = sizeof(SUBIO_DO_MSG_T);
					break;
					
				// make vo message
				case SUBIO_TYPE_VO:
					subio_vomsg_request(p_subio);
					n_txlength = sizeof(SUBIO_VO_MSG_T);
					break;
					
				// make adi message
				case SUBIO_TYPE_ADI:
					subio_adimsg_request(p_subio);
					n_txlength = sizeof(SUBIO_ADI_MSG_T);
					break;
			
				default:
					continue;
				}
				
				// send message
				subio_send_msg(p_subio, n_txlength);

				// receive message
				n_rxlength = subio_recv_msg(p_subio);

				// message handler
				if ( n_rxlength > 0 ) {
					subio_msg_handler( p_subio , n_rxlength );
					g_nSubioAliveChk[n_grp] = 0;
				}
			}
		}

		sub_sleep(0, 300);
	}
}






