/*
 * file 		: point_mgr.c
 * creator   	: jong2ry
 *
 *
 * Physical Point에 대해서 ADC, DAC 작업을 한다. 
 *
 *
 * version :
 *  	0.0.1 - initiailize
 *  	0.0.2 - DO가 제어가 가능하도록 하였음. (2010-07-08)
 *  	0.0.3 - AO가 가능하도록 하였음. (2010-07-09)
 *  	0.0.4 - Calibration 기능 추가 작업. (2010-07-15)
 *  	0.0.5 - Calibration 기초 테스트 (2010-07-19)
 *  			DO, VO, DI2S, VI, CI, JPT Test Ok.
 *  	0.0.6 - Status 변수를 통한 point_handler()의 동작상태 확인.
 *  	0.0.7 - file name change (point_handler.c -> physical_pnt_handler.c)
 *  	0.0.8 - file name change (physical_pnt_handler.c -> point_mgr.c)
 *  	0.0.9 - code clean 
 *		0.0.10 - 외부에서 사용하는 pSet, pGet 같은 명령어를 분리하였다. (point_haanler.c 로 분리)
 *
 *
 * code 작성시 유의사항
 *		1. global 변수는 'g_' 접두사를 쓴다.
 *		2. 변수를 선언할 때에는 아래와 같은 규칙으로 선언한다. 
 *			int					n
 *			short				w
 *			long				l
 *			float				f
 *			char				ch
 *			unsigned char       b
 *			unsignnd int		un
 *			unsigned short 		uw 
 *			pointer				p
 *			structure			s or nothing
 *		3. 변수명의 첫글자는 대분자로 사용한다. 
 *		4. 변수명에서의 각 글자의 구분은 공백없이 대분자로 한다. 
 *			ex > g_nTest, g_bFlag, g_fPointTable
 *		5. 함수명은 소문자로 '_' 기호를 사용해서 생성한다. 
 *		6. 함수와 함수 사이의 간격은 2줄로 한다. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>  	// strlen()
#include <unistd.h>  	// read, write
#include <fcntl.h>   	// open, close, O_RDWR, O_NONBLOCK
#include <sys/poll.h>		// use poll event
#include <malloc.h>

////////////////////////////////////////////////////////////////////////////////
//define
#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

#include "point_mgr.h"


////////////////////////////////////////////////////////////////////////////////
//extern variable
extern pthread_mutex_t pointTable_mutex;										// point table mutex

// point and node
extern int g_nMyPcm;															// my pcm number
extern int g_nMultiDdcFlag;														// mode select flag
extern unsigned char g_cNode[MAX_NET32_NUMBER];									// node
extern PTBL_INFO_T *g_pPtbl;													// point table
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];						// extension point table

// calibration
extern char *g_chDfnTypeName[];													// point-define type name
extern CAL_INFO_T *g_pCal;														// calibration table
extern int g_nCalReady;															// calibration start flag
extern int g_nCalStatus;														// calibration status
extern int g_nCalGrp;															// calibration group (g0 ~ g3)
extern int g_nCalType;															// calibration type



////////////////////////////////////////////////////////////////////////////////
//global variable
int gn_adc_dbg = 0;
int g_nStayVO = 0;
int g_nPntLog = 0;
unsigned int g_nPntValue[32];

// factory reset count
unsigned int g_nFactoryResetCnt = 0;




static PNT_HANDLER_T *pnt_new(void)
// ----------------------------------------------------------------------------
// CREATE PNT_HANDLER_T structure
// Description : PNT_HANDLER_T structure를 생성한다.
// Arguments   : none
// Returns     : p				pointer of PNT_HANDLER_T structure	
{
	PNT_HANDLER_T *p;

	p = (PNT_HANDLER_T *)malloc( sizeof(PNT_HANDLER_T) );
	memset( p, 0x00, sizeof(PNT_HANDLER_T) );
	return p;
}



static int pnt_open(PNT_HANDLER_T *p)
// ----------------------------------------------------------------------------
// ADC DEVICE OPEN
// Description : device file을 open한다.
// Arguments   : p				pointer of PNT_HANDLER_T structure	
// Returns     : 0				Is successful
//				 -1				Is fail
{
	p->n_dev = open( DEV_FILENAME, O_RDWR );

	p->p_rx = (unsigned char *) malloc(MAX_BUFFER_SIZE);
	p->p_tx = (unsigned char *) malloc(MAX_BUFFER_SIZE);

	memset( p->p_rx, 0x00, MAX_BUFFER_SIZE );
	memset( p->p_tx, 0x00, MAX_BUFFER_SIZE );	
	
	p->w_pno = 0;

	return p->n_dev;
}


static void pnt_close(PNT_HANDLER_T *p)
// ----------------------------------------------------------------------------
// CLOSE PNT_HANDLER_T structure
// Description : PNT_HANDLER_T structure를 close한다.
// Arguments   : p				pointer of PNT_HANDLER_T structure	
// Returns     : none
{
	if( p->n_dev > 0 )		
		close(p->n_dev);
}


static void pnt_make_lowdata(PNT_HANDLER_T *p)
// ----------------------------------------------------------------------------
// MAKE POINT MESSAGE
// Description : device 와 통신하기 위한 message를 만든다.
// Arguments   : p				pointer of PNT_HANDLER_T structure	
// Returns     : none
{
	CAL_VAL_T *p_cal = NULL;
	PNT_DEV_MSG_T *p_msg = NULL;

	p_msg = (PNT_DEV_MSG_T *)p->p_tx;
	p_cal = &g_pCal[p->w_pno].cal_vo;

	// Type에 맞도록 각각 low data를 구한다. 
	switch ( p_msg->c_type ) {
		case DFN_PNT_VR:
			break;

		case DFN_PNT_DO:
			pthread_mutex_lock( &pointTable_mutex );
			p_msg->n_val = (unsigned int)g_fExPtbl[g_nMyPcm][p->w_pno];
			pthread_mutex_unlock( &pointTable_mutex );
			break;

		case DFN_PNT_VO:
			//p_cal = &g_pCal[p->w_pno].cal_vo;
			pthread_mutex_lock( &pointTable_mutex );
			p_msg->n_val = (p_cal->f_a * (g_fExPtbl[g_nMyPcm][p->w_pno] * 0.1)) + p_cal->f_b;
			g_pPtbl[p->w_pno].n_adc = p_msg->n_val;
			pthread_mutex_unlock( &pointTable_mutex );
			//printf("%d > %d   (%f)\n", p->w_pno, g_pPtbl[p->w_pno].n_adc, g_fExPtbl[g_nMyPcm][p->w_pno] );
			break;

		default:
			break;
	}
}


static void pnt_make_msg(PNT_HANDLER_T *p)
// ----------------------------------------------------------------------------
// MAKE POINT MESSAGE
// Description : device 와 통신하기 위한 message를 만든다.
// Arguments   : p				pointer of PNT_HANDLER_T structure	
// Returns     : none
{
	int i = 0;
	PNT_DEV_MSG_T *p_msg = NULL;

	p_msg = (PNT_DEV_MSG_T *)p->p_tx;

	p_msg->c_pno = (char)p->w_pno;
	p_msg->c_type = (char)p->w_type;
	p_msg->c_length = (char)sizeof(PNT_DEV_MSG_T);
	pnt_make_lowdata(p);
	p_msg->c_chksum = 0;
	for( i = 0; i < (p_msg->c_length - 1); i++) {
		p_msg->c_chksum -= p->p_tx[i];
	}

	if ( gn_adc_dbg == 1 ) {
		fprintf( stderr, "\nTX (%d) : ", sizeof(PNT_DEV_MSG_T) );
		for( i = 0; i < p_msg->c_length; i++) {
			fprintf( stderr, "0x%x ", p->p_tx[i] );
		}	
		fflush(stderr);
	}
}


static int pnt_send_msg(PNT_HANDLER_T *p)
// ----------------------------------------------------------------------------
// WRITE MESSAGE
// Description : device에 message를 write한다.
// Arguments   : p				pointer of PNT_HANDLER_T structure	
// Returns     : none
{
	int ret = 0;

	ret = write( p->n_dev, p->p_tx, sizeof(PNT_DEV_MSG_T) );

	if ( ret < 0 ) {
		return -1;
	}


	return ret;
}


static int pnt_recv_msg(PNT_HANDLER_T *p)
// ----------------------------------------------------------------------------
// READ MESSAGE
// Description : device로부터 message를 read한다.
// Arguments   : p				pointer of PNT_HANDLER_T structure	
// Returns     : n_ret			Is length of message
{
	int n_ret = 0;

	n_ret = read( p->n_dev, p->p_rx, sizeof(PNT_DEV_MSG_T) );

	return n_ret;
}


static void pnt_conv_di2s(PNT_DEV_MSG_T *p_msg)
// ----------------------------------------------------------------------------
// MESSAGE CONVERT ADC VALUE
// Description : message로 받은 값을 DI2S에 맞게 분석하여 point-table에 값을 쓴다.
// Arguments   : p				pointer of PNT_HANDLER_T structure	
// Returns     : none
{
	point_info point;

	point.pcm = g_nMyPcm;
	point.pno = p_msg->c_pno;

	//fprintf( stderr, "\n>>pno = %d, type = %d, n_val = %d", p_msg->c_pno, p_msg->c_type, p_msg->n_val );

	// get row adc value
	g_pPtbl[p_msg->c_pno].n_adc = p_msg->n_val;

	// 0xfff (4096)을 3등분하면  0~1365, 1366~2730 ,2731~4096으로 나뉜다.
	// 3등분하여 구분한 값과 ADC의 값을 비교해 DI2S의 상태를 결정한다. 
	// DI2S ON
	if ( p_msg->n_val < 1365) {								
		point.value = 1;
		pnt_local_adc(&point);
	}
	// DI2S OFF
	else if( p_msg->n_val > 2730 ) {
		point.value = 0;									
		pnt_local_adc(&point);
	}
	// 이전 상태 유지.
	else{
		; 													
	}
}


static void pnt_conv_vi(PNT_DEV_MSG_T *p_msg)
// ----------------------------------------------------------------------------
// MESSAGE CONVERT ADC VALUE
// Description : message로 받은 값을 VO에 맞게 분석하여 point-table에 값을 쓴다.
// Arguments   : p				pointer of PNT_HANDLER_T structure	
// Returns     : none
{
	CAL_VAL_T *p_cal;
	point_info point;
	float f_conv0, f_conv1, f_sig1, f_max;

	p_cal = &g_pCal[p_msg->c_pno].cal_vi;
	point.pcm = g_nMyPcm;
	point.pno = p_msg->c_pno;

	// get row adc value for calibration table
	if ( g_nCalReady == 1 &&  g_nCalStatus == CAL_ZERO ) {
		g_pCal[p_msg->c_pno].f_zero = p_msg->n_val;
		return;
	}
	else if ( g_nCalReady == 1 &&  g_nCalStatus == CAL_VI ) {
		p_cal->f_conv[1] = p_msg->n_val;
		p_cal->f_conv[0] = g_pCal[p_msg->c_pno].f_zero;

		p_cal->f_sig[1] = 10.0;
		p_cal->f_sig[0] = 0;
		
		f_conv1 = p_cal->f_conv[1];
		f_conv0 = p_cal->f_conv[0];
		f_sig1 = p_cal->f_sig[1];

		p_cal->f_max = (((f_conv1 - f_conv0) / f_sig1) * 10.0 + f_conv0) * 5.0;

		f_max = p_cal->f_max;

		p_cal->f_a = 100 / ((f_max - f_conv0) / 5.0);
		p_cal->f_b = (-1 * p_cal->f_a) * (f_conv0 / 5.0);
		return;
	}

	// get row adc value 
	g_pPtbl[p_msg->c_pno].n_adc = p_msg->n_val;
	
	// calculate value
	point.value = p_cal->f_a * ( g_pPtbl[p_msg->c_pno].n_adc + p_cal->f_b );

	// apply scale and offset
	point.value = g_pPtbl[p_msg->c_pno].f_scale * (point.value + g_pPtbl[p_msg->c_pno].f_offset);

	pnt_local_adc(&point);
}


static void pnt_conv_ci(PNT_DEV_MSG_T *p_msg)
// ----------------------------------------------------------------------------
// MESSAGE CONVERT ADC VALUE
// Description : message로 받은 값을 CI에 맞게 분석하여 point-table에 값을 쓴다.
// Arguments   : p				pointer of PNT_HANDLER_T structure	
// Returns     : none
{
	CAL_VAL_T *p_cal;
	point_info point;
	float f_conv0, f_conv1, f_sig1, f_max;

	p_cal = &g_pCal[p_msg->c_pno].cal_ci;
	point.pcm = g_nMyPcm;
	point.pno = p_msg->c_pno;

	// get row adc value for calibration table
	if ( g_nCalReady == 1 &&  g_nCalStatus == CAL_ZERO ) {
		g_pCal[p_msg->c_pno].f_zero = p_msg->n_val;
		return;
	}
	else if ( g_nCalReady == 1 &&  g_nCalStatus == CAL_CI ) {
		p_cal->f_conv[1] = p_msg->n_val;
		p_cal->f_conv[0] = g_pCal[p_msg->c_pno].f_zero;

		p_cal->f_sig[1] = 19.9;
		p_cal->f_sig[0] = 0;

		f_conv1 = p_cal->f_conv[1];
		f_conv0 = p_cal->f_conv[0];
		f_sig1 = p_cal->f_sig[1];

		//p_cal->f_max = (((f_conv1 - f_conv0) / f_sig1) * 19.9 + f_conv0) * 5.0;
		p_cal->f_max = f_conv1 * 5.0;

		f_max = p_cal->f_max;

		p_cal->f_a = 100 / ((f_max - f_conv0) / 5.0);
		p_cal->f_b = (-1 * p_cal->f_a) * (f_conv0 / 5.0);
		return;
	}

	// get row adc value
	g_pPtbl[p_msg->c_pno].n_adc = p_msg->n_val;
	
	// calculate value
	point.value = p_cal->f_a * ( g_pPtbl[p_msg->c_pno].n_adc + p_cal->f_b );

	//fprintf ( stderr, "(%d) adc = %d, fval = %f ", p_msg->c_pno, g_pPtbl[p_msg->c_pno].n_adc, point.value);
	
	// apply scale and offset
	point.value = g_pPtbl[p_msg->c_pno].f_scale * (point.value + g_pPtbl[p_msg->c_pno].f_offset);

	//fprintf ( stderr, " fval = %f \n", point.value);
	//fflush(stderr);

	pnt_local_adc(&point);
}


static void pnt_conv_jpt(PNT_DEV_MSG_T *p_msg)
// ----------------------------------------------------------------------------
// MESSAGE CONVERT ADC VALUE
// Description : message로 받은 값을 JPT1000에 맞게 분석하여 point-table에 값을 쓴다.
// Arguments   : p				pointer of PNT_HANDLER_T structure	
// Returns     : none
{
	CAL_VAL_T *p_cal;
	point_info point;
	float K = 0, M = 0, Rt = 0;
	float f_tmpval = 0;
	//float f_conv0, f_conv1, f_sig1, f_max;

	p_cal = &g_pCal[p_msg->c_pno].cal_jpt;
	point.pcm = g_nMyPcm;
	point.pno = p_msg->c_pno;

	// get row adc value for calibration table
	if ( g_nCalReady == 1 &&  g_nCalStatus == CAL_JPT ) {
		g_pPtbl[p_msg->c_pno].n_adc = p_msg->n_val;
		return;
	}

	// get row adc value
	g_pPtbl[p_msg->c_pno].n_adc = p_msg->n_val;
	
	// calculate value
	f_tmpval = p_cal->f_a * g_pPtbl[p_msg->c_pno].n_adc + p_cal->f_b ;

	//fprintf ( stderr, "%d n_val = %d, a = %f, b = %f, f_val = %f\n", p_msg->c_pno,  p_msg->n_val, p_cal->f_a, p_cal->f_b, point.value);

	/* 
	Reference Code (Velos). 
	 
	K = (RTD110_R2/RTD110_R3 + RTD110_R2/RTD110_R1 + 1.0);
	M = ((RTD110_Vref * RTD110_R2/RTD110_R1) / K + fval * ((5.0 / 100.0) / K));
	Rt = RTD110_Rs * M / (RTD110_Vref - M); 
	 
	fval = (((JPT1000A * Rt + JPT1000B) * Rt + JPT1000C) * Rt + JPT1000D) * Rt + JPT1000E;
	//fval = (((DPT1000A * Rt + DPT1000B) * Rt + DPT1000C) * Rt + DPT1000D) * Rt + DPT1000E;
	*/
	K = (RTD110_R2/RTD110_R3 + RTD110_R2/RTD110_R1 + 1.0);
	M = ((RTD110_Vref * RTD110_R2/RTD110_R1) / K + f_tmpval * ((5.0 / 100.0) / K));
	Rt = RTD110_Rs * M / (RTD110_Vref - M);

	f_tmpval = (((JPT1000A * Rt + JPT1000B) * Rt + JPT1000C) * Rt + JPT1000D) * Rt + JPT1000E;

	point.value = f_tmpval;

	// apply scale and offset
	point.value = g_pPtbl[p_msg->c_pno].f_scale * (point.value + g_pPtbl[p_msg->c_pno].f_offset);
	
	//fprintf ( stderr, "k = %f, m = %f, rt = %f\n", K, M, Rt);
	//fprintf ( stderr, "point.value = %f\n", point.value);
	//fprintf ( stderr, "\n\n");
	//fflush(stderr);
	pnt_local_adc(&point);
}


static void pnt_handler(PNT_HANDLER_T *p)
// ----------------------------------------------------------------------------
// PARSING MESSAGE
// Description : device로부터 받은 message를 분석한다.
// Arguments   : p				pointer of PNT_HANDLER_T structure	
// Returns     : none
{
	int i = 0;
	unsigned char c_chksum = 0;
	PNT_DEV_MSG_T *p_msg;

	p_msg = (PNT_DEV_MSG_T *)p->p_rx;

	for( i = 0; i < (p_msg->c_length ); i++) {
		c_chksum += p->p_rx[i];
	}

	if ( gn_adc_dbg == 1 ) {
		fprintf( stderr, "\nRX (%d) : ", sizeof(PNT_DEV_MSG_T) );
		for( i = 0; i < p_msg->c_length; i++) {
			fprintf( stderr, "0x%x ", p->p_rx[i] );
		}	
		fflush(stderr);
	}

	if ( c_chksum == 0 ) {
		
		if ( gn_adc_dbg == 1 ) {
			fprintf( stderr, "\n>>pno = %d, type = %d, n_val = %d", 
					 p_msg->c_pno, p_msg->c_type, p_msg->n_val );
			fflush(stderr);
		}		
		
		
		//fprintf( stderr, "\n>>Factory Reset = %d", p_msg->c_factoryReset );
		//fflush(stderr);
		if ( p_msg->c_factoryReset > 0 )
			g_nFactoryResetCnt = 0; 
		else 
			g_nFactoryResetCnt++; 
			
		// factory reset mode
		if (g_nFactoryResetCnt > 100 ) {
			system("sync");
			system("sync");
			system("sync");
			system("reboot");
		}
			
		g_nPntValue[p_msg->c_pno] = p_msg->n_val ;
				
		switch( p_msg->c_type ) {
			case DFN_PNT_DI2S:
				pnt_conv_di2s(p_msg);
				break;
	
			case DFN_PNT_JPT:
				pnt_conv_jpt(p_msg);
				break;
	
			case DFN_PNT_VI:
				pnt_conv_vi(p_msg);
				break;
	
			case DFN_PNT_CI:
				pnt_conv_ci(p_msg);
				break;
	
			default:
				break;			
		}
	}
}


void pnt_calibration(PNT_HANDLER_T *p)
// ----------------------------------------------------------------------------
// CALIBRATION
// Description : 각 type에 맞게 calibration 작업을 진행한다. 
// Arguments   : p				pointer of PNT_HANDLER_T structure	
// Returns     : none
{
	int i = 0;
	int n_length = 0;
	unsigned short w_pno = 0;								// point number.
	PNT_DEV_MSG_T *p_msg;
	unsigned char buf[12];
	struct timespec ts;

	p_msg = (PNT_DEV_MSG_T *) buf;
	memset ( buf, 0x00, sizeof(buf) );

	switch ( g_nCalStatus ) {
		// Zero Calibration.	
		case CAL_ZERO:
			g_nStayVO = 0;
			fprintf( stderr, "\n\n" );
			for ( w_pno = 16; w_pno < 32; w_pno++ ) {
				// make message
				p_msg->c_pno = (char) w_pno;
				p_msg->c_type = DFN_PNT_VI;
				p_msg->c_length = (char)sizeof(PNT_DEV_MSG_T);
				p_msg->n_val = 0;
				p_msg->c_chksum = 0;
				for( i = 0; i < (p_msg->c_length - 1); i++) 
					p_msg->c_chksum -= buf[i];
				
				// send message
				write( p->n_dev, p_msg, 8 );
	
				// wait 10ms...
				ts.tv_sec = 0;
				ts.tv_nsec = 10 * M_SEC;				
				nanosleep(&ts, NULL);				
	
				// read message
				n_length =  pnt_recv_msg(p);
				if( n_length > 0 ) {
					pnt_handler(p);
	
					fprintf( stderr, 
							 "\n Cal   %03d-%02d  : ZERO [ %4.2f ]  " ,
							 g_nMyPcm, 
							 w_pno, 
							 g_pCal[w_pno].f_zero );
					fflush(stderr);
				}
			} 

			fprintf( stderr, "\n\n Zero Calibration Complete\n\n");
			fflush(stderr);
	
			g_nCalStatus = CAL_NONE;
			break;
	
		// VI point Calibration.	
		case CAL_VI:
			g_nStayVO = 0;
			for ( w_pno = (g_nCalGrp * 8); w_pno < ((g_nCalGrp * 8) + 8); w_pno++ ) {
				// make message
				p_msg->c_pno = (char) w_pno;
				p_msg->c_type = DFN_PNT_VI;
				p_msg->c_length = (char)sizeof(PNT_DEV_MSG_T);
				p_msg->n_val = 0;
				p_msg->c_chksum = 0;
				for( i = 0; i < (p_msg->c_length - 1); i++) 
					p_msg->c_chksum -= buf[i];
				
				// send message
				write( p->n_dev, p_msg, 8 );
	 
				// wait 10ms...
				ts.tv_sec = 0;
				ts.tv_nsec = 10 * M_SEC;				
				nanosleep(&ts, NULL);								
	
				// read message
				n_length =  pnt_recv_msg(p);
				if( n_length > 0 ) {
					pnt_handler(p);
	
					fprintf( stderr, 
							 "\n Cal   %03d-%02d  : VI [ %4.2f ]  %f  %f" ,
							 g_nMyPcm, 
							 w_pno, 
							 g_pCal[w_pno].cal_vi.f_conv[1],
							 g_pCal[w_pno].cal_vi.f_a,
							 g_pCal[w_pno].cal_vi.f_b );
					fflush(stderr);
				}
			} 
	
			fprintf( stderr, "\n\n VI Calibration Complete\n\n");
			fflush(stderr);

			g_nCalStatus = CAL_NONE;
			break;
	
		// VI point Calibration.	
		case CAL_CI:
			g_nStayVO = 0;
			for ( w_pno = (g_nCalGrp * 8); w_pno < ((g_nCalGrp * 8) + 8); w_pno++ ) {
				// make message
				p_msg->c_pno = (char) w_pno;
				p_msg->c_type = DFN_PNT_CI;
				p_msg->c_length = (char)sizeof(PNT_DEV_MSG_T);
				p_msg->n_val = 0;
				p_msg->c_chksum = 0;
				for( i = 0; i < (p_msg->c_length - 1); i++) 
					p_msg->c_chksum -= buf[i];
				
				// send message
				write( p->n_dev, p_msg, 8 );
	
				// wait 10ms...
				ts.tv_sec = 0;
				ts.tv_nsec = 10 * M_SEC;				
				nanosleep(&ts, NULL);								
	
				// read message
				n_length =  pnt_recv_msg(p);
				if( n_length > 0 ) {
					pnt_handler(p);
	
					fprintf( stderr, 
							 "\n Cal   %03d-%02d  : CI [ %4.2f ]  %f  %f" ,
							 g_nMyPcm, 
							 w_pno, 
							 g_pCal[w_pno].cal_ci.f_conv[1],
							 g_pCal[w_pno].cal_ci.f_a,
							 g_pCal[w_pno].cal_ci.f_b  );
					fflush(stderr);
				}
			} 
	
			fprintf( stderr, "\n\n VI Calibration Complete\n\n");
			fflush(stderr);

			g_nCalStatus = CAL_NONE;
			break;
	
	
		case CAL_JPT:
			g_nStayVO = 0;
			fprintf( stderr, "\nJPT Low..\n" );
			fflush(stderr);

			for ( w_pno = (g_nCalGrp * 8); w_pno < ((g_nCalGrp * 8) + 8); w_pno++ ) {
				// make message
				p_msg->c_pno = (char) w_pno;
				p_msg->c_type = DFN_PNT_JPT;
				p_msg->c_length = (char)sizeof(PNT_DEV_MSG_T);
				p_msg->n_val = 0;
				p_msg->c_chksum = 0;
				for( i = 0; i < (p_msg->c_length - 1); i++) 
					p_msg->c_chksum -= buf[i];
				
				// send message
				write( p->n_dev, p_msg, 8 );
	
				// wait 10ms...
				ts.tv_sec = 0;
				ts.tv_nsec = 10 * M_SEC;				
				nanosleep(&ts, NULL);								
	
				// read message
				n_length =  pnt_recv_msg(p);
				if( n_length > 0 ) {
					pnt_handler(p);

					g_pCal[w_pno].cal_jpt.f_conv[0] = g_pPtbl[w_pno].n_adc;
	
					fprintf( stderr, 
							 "\n Cal   %03d-%02d  : JPT [ %4.2f ]  %f  %f" ,
							 g_nMyPcm, 
							 w_pno, 
							 g_pCal[w_pno].cal_jpt.f_conv[0],
							 g_pCal[w_pno].cal_jpt.f_a,
							 g_pCal[w_pno].cal_jpt.f_b  );
					fflush(stderr);
				}
			} 

			fprintf( stderr, "\n\n JPT1000 Min value calibration complete" );
			fprintf( stderr, "\n Repeat jpt command for JPT1000 Max value\n" );
			fflush(stderr);

			g_nCalStatus = CAL_NONE;

			// wait...
			// jpt에 관한 메시지가 한번 더 입력될 때 까지 기다린다. 
			for (;;) {
				if ( g_nCalStatus == CAL_JPT )
					break;
				else {
					ts.tv_sec = 0;
					ts.tv_nsec = 10 * M_SEC;				
					nanosleep(&ts, NULL);									
				}
			}

			fprintf( stderr, "\nJPT Hi..\n" );
			fflush(stderr);

			for ( w_pno = (g_nCalGrp * 8); w_pno < ((g_nCalGrp * 8) + 8); w_pno++ ) {
				// make message
				p_msg->c_pno = (char) w_pno;
				p_msg->c_type = DFN_PNT_JPT;
				p_msg->c_length = (char)sizeof(PNT_DEV_MSG_T);
				p_msg->n_val = 0;
				p_msg->c_chksum = 0;
				for( i = 0; i < (p_msg->c_length - 1); i++) 
					p_msg->c_chksum -= buf[i];
				
				// send message
				write( p->n_dev, p_msg, 8 );
	
				// wait 10ms...
				ts.tv_sec = 0;
				ts.tv_nsec = 10 * M_SEC;				
				nanosleep(&ts, NULL);								
	
				// read message
				n_length =  pnt_recv_msg(p);
				if( n_length > 0 ) {
					pnt_handler(p);

					g_pCal[w_pno].cal_jpt.f_conv[1] = g_pPtbl[w_pno].n_adc;
	
					fprintf( stderr, 
							 "\n Cal   %03d-%02d  : JPT [ %4.2f ]  %f  %f" ,
							 g_nMyPcm, 
							 w_pno, 
							 g_pCal[w_pno].cal_jpt.f_conv[1],
							 g_pCal[w_pno].cal_jpt.f_a,
							 g_pCal[w_pno].cal_jpt.f_b  );
					fflush(stderr);
				}
			} 

			fprintf( stderr, "\n\n JPT1000 Calibration Complete\n\n");
			fflush(stderr);

			g_nCalStatus = CAL_NONE;
			break;
	
		// VO point Calibration.	
		case CAL_VO:
			
			g_nStayVO = 1;
			
			for ( w_pno = (g_nCalGrp * 8); w_pno < ((g_nCalGrp * 8) + 8); w_pno++ ) {
				// make message
				p_msg->c_pno = (char) w_pno;
				p_msg->c_type = DFN_PNT_VO;
				p_msg->c_length = (char)sizeof(PNT_DEV_MSG_T);
				p_msg->n_val = 0xffff;
				p_msg->c_chksum = 0;
				for( i = 0; i < (p_msg->c_length - 1); i++) 
					p_msg->c_chksum -= buf[i];
				
				// send message
				write( p->n_dev, p_msg, 8 );
	
				// wait 10ms...
				ts.tv_sec = 0;
				ts.tv_nsec = 10 * M_SEC;				
				nanosleep(&ts, NULL);								
	
				// read message
				n_length =  pnt_recv_msg(p);
				if( n_length > 0 ) {
					pnt_handler(p);
				}
	
				g_pCal[w_pno].cal_vo.f_conv[1] = 4095;
				g_pCal[w_pno].cal_vo.f_conv[0] = 0;
				g_pCal[w_pno].cal_vo.f_sig[1] = 10.0;
				g_pCal[w_pno].cal_vo.f_sig[0] = 0;

				fprintf( stderr, 
						 "\n Cal   %03d-%02d  : VO [ %4.2f ]  %f  %f" ,
						 g_nMyPcm, 
						 w_pno, 
						 g_pCal[w_pno].cal_vo.f_conv[1],
						 g_pCal[w_pno].cal_vo.f_a,
						 g_pCal[w_pno].cal_vo.f_b  );
				fflush(stderr);
			} 
	
			fprintf( stderr, "\n\n VO Calibration Complete\n\n");
			fflush(stderr);

			g_nCalStatus = CAL_NONE;
			break;
	
		default:
			break;
	}
	
	if ( g_nStayVO == 1 ) {
		for ( w_pno = 8; w_pno < 16; w_pno++ ) {
			// make message
			p_msg->c_pno = (char) w_pno;
			p_msg->c_type = DFN_PNT_VO;
			p_msg->c_length = (char)sizeof(PNT_DEV_MSG_T);
			p_msg->n_val = 0xffff;
			p_msg->c_chksum = 0;
			for( i = 0; i < (p_msg->c_length - 1); i++) 
				p_msg->c_chksum -= buf[i];
			
			// send message
			write( p->n_dev, p_msg, 8 );

			// wait 10ms...
			ts.tv_sec = 0;
			ts.tv_nsec = 10 * M_SEC;				
			nanosleep(&ts, NULL);							

			// read message
			n_length =  pnt_recv_msg(p);
			//if( n_length > 0 ) {
			//	pnt_handler(p);
			//}
		}		
	}
}



void pnt_log(int type)
{
	FILE *fp = NULL;
	time_t     tm_nd;
	struct tm *tm_ptr;
	
	fp = fopen("/duksan/FILE/phy_mgr.txt", "a+");
	if( fp == NULL ) {
		printf("[ERROR] File Open with Option 'r'\n");
		//system("reboot");
		return;		
	}

	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);

	// write Communication Error
	if ( type == 0 ) {
		fprintf(fp, "[Com Error]  %d/%d %d:%d\n", tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min);
		fprintf(fp, "[Reboot]  System rebooting....\n" );
	}

	fclose(fp);
}


void *physical_pnt_mgr_main(void* arg)
// ----------------------------------------------------------------------------
// POINT_HANDLER MAIN LOOP
// Description : ADC device file을 열고, Physical point에 대해 ADC, DAC 작업을 한다.
// Arguments   : none
// Returns     : none
{
	int i = 0;
	int n_length = 0;
	PNT_HANDLER_T *p;										// pointer of PNT_HANDLER_T
	unsigned short w_pno = 0;								// point number.
	//struct timeval diff_time;
	time_t     tm_nd;
	struct tm *tm_ptr;
	int now_time = 0;
	int pre_time = 0;
	int n_recv_cnt = 0;
	int ret = 0;
	struct timespec ts;

	signal(SIGPIPE, SIG_IGN);								// Ignore broken_pipe signal.

	p = pnt_new();

	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);
	now_time = (tm_ptr->tm_hour * 100) +  tm_ptr->tm_min; 
	pre_time = (tm_ptr->tm_hour * 100) +  tm_ptr->tm_min; 

	while(1)
	{
		sched_yield();
		ts.tv_sec = 0;
		ts.tv_nsec = 10 * M_SEC;				
		nanosleep(&ts, NULL);
					
		// open adc-device
		if( pnt_open( p ) < 0 ) {
			fprintf( stderr, "ADC Device Open Error\n");
			fflush( stderr );
			pnt_close( p );
			sleep(1);

			syslog_record(SYSLOG_NET32_FAIL_REBOOT);
			system("sync");
			system("sync");
			system("sync");
			system("killall duksan");
			continue;
		}

		for(;;) {
			// log 파일을 기록한다. 
			ts.tv_sec = 0;
			ts.tv_nsec = 10 * M_SEC;				
			nanosleep(&ts, NULL);

			time(&tm_nd);
			tm_ptr = localtime(&tm_nd);
			now_time = (tm_ptr->tm_hour * 100) +  tm_ptr->tm_min; 
	
			if ( now_time != pre_time  ) {
				now_time = pre_time;
				if ( n_recv_cnt == 0 ) {

					pnt_log(0);

					syslog_record(SYSLOG_ADCDRV_FAIL_REBOOT);
					printf("System Terminated..\n");
					system("sync");
					system("sync");
					system("sync");
					system("killall duksan");
				}
			}
	
			if ( g_nCalReady == 1 ) {
				pnt_calibration(p);
				n_recv_cnt++;
				ts.tv_sec = 0;
				ts.tv_nsec = 10 * M_SEC;				
				nanosleep(&ts, NULL);								
				continue;
			}

		
			if ( g_nPntLog == 1 ) {
				fprintf( stdout, ">>, ");
				for ( i = 16; i < 32; i++ ) 
					//fprintf( stdout, "%d, ", g_nPntValue[i] );
					fprintf( stdout, "%3.1f, ", g_pPtbl[i].f_val  );
				fprintf( stdout, "\n");
				fflush(stdout);
			}				
			
			for( w_pno = 0; w_pno < MAX_PHY_POINT_CNT; w_pno++ ) {

				// get point number, point type.
				p->w_pno = w_pno;
				p->w_type = g_pPtbl[w_pno].n_type;

				// make message
				pnt_make_msg(p);

				// send message
				ret = pnt_send_msg(p);
				if ( ret < 0 ) {
					sleep(5);
					break;
				}

				// wait 10ms...
				ts.tv_sec = 0;
				ts.tv_nsec = 10 * M_SEC;				
				nanosleep(&ts, NULL);								

				// read message
				n_length =  pnt_recv_msg(p);
				if( n_length > 0 ) {
					n_recv_cnt++;

					pnt_handler(p);
				}
				else if ( n_length == 0 ) {
					sleep(5);
					break;
				}
				else {
					//printf("$");
				}
			}
		}

		pnt_close( p );

		free(p->p_rx);
		free(p->p_tx);
		
		continue;
	}
	syslog_record(SYSLOG_DESTROY_POINT_MANAGER);	
}



void pnt_cal_load()
// ----------------------------------------------------------------------------
// CALIBRATION LOAD DATA
// Description : calibration 작업을 시작하기 위한 작업을 한다. 
// 				 cal_ready의 값을 1로 변경하여, 
// 				 다른 Calibration 명령이 수행될 수 있도록 한다.
// Arguments   : none
// Returns     : none
{
	FILE *pf;
	int n_cal_size = 0;

	// 32개의 물리적 포인트를 가지고 있기 때문에 * 32를 한다. 
	n_cal_size = sizeof(CAL_INFO_T) * 32;

	// chanage g_nCalReady value
	g_nCalReady = 1;
	g_nCalStatus = CAL_NONE;

	// open PointCalibration.dat file. and get calibration value
	if ( ( pf = fopen("/duksan/DATA/PointCalibration.dat", "r") ) == NULL ) {
		memset (g_pCal, 0x00, sizeof(CAL_INFO_T) );
		fprintf( stderr, "\nCalibration table init load\n");
		fflush(stderr);
		return;
	}
	else {
		fread( g_pCal, n_cal_size, 1, pf );
	}

	fclose(pf);
	fprintf( stderr, "\nCalibration table Initialize. Calibration flag ON\n\n");
	fflush(stderr);
	return;
}


void pnt_cal_init()
// ----------------------------------------------------------------------------
// CALIBRATION VALUE INIT
// Description : 모든 calibration value값을 초기화 한다. 
// Arguments   : none
// Returns     : none
{
	int n_cal_size = 0;

	// 32개의 물리적 포인트를 가지고 있기 때문에 * 32를 한다. 
	n_cal_size = sizeof(CAL_INFO_T) * 32;

	// check g_nCalReady value
	if ( g_nCalReady != 1 ) {
		fprintf( stderr, "\nIt need calibration table loading\n");
		fflush(stderr);
		return;
	}

	// value initiailize
	memset (g_pCal, 0x00, n_cal_size );

	fprintf( stderr, "\nCalibration table Initialize. All point set zero(0)\n\n");
	fflush(stderr);
}


void pnt_cal_save()
// ----------------------------------------------------------------------------
// CALIBRATION VALUE SAVE FILE
// Description : 모든 calibration value값을 저장한다.
// Arguments   : none
// Returns     : none
{
	FILE *pf;
	int n_cal_size = 0;

	// 32개의 물리적 포인트를 가지고 있기 때문에 * 32를 한다. 
	n_cal_size = sizeof(CAL_INFO_T) * 32;
	
	// check g_nCalReady value
	if ( g_nCalReady != 1 ) {
		fprintf( stderr, "\nIt need calibration table loading\n");
		fflush(stderr);
		return;
	}
		
	// save calibration value
	if ( ( pf = fopen("/duksan/DATA/PointCalibration.dat", "w") ) == NULL ) {
		fprintf( stderr, "\nCalibration Save Error\n");
		fflush(stderr);
		return;
	}
	else {
		fwrite( g_pCal, n_cal_size, 1, pf );
	}

	fclose(pf);
	fprintf( stderr, "\n\nCalibration table Save OK.\n\n");
	fflush(stderr);
	return;
}


void pnt_cal_display()
// ----------------------------------------------------------------------------
// PRINT CALIBRATION VALUE
// Description : 모든 calibration value값을 화면에 출력한다. 
// Arguments   : none
// Returns     : none
{
	int i = 0;

	fprintf( stderr, "\nCalibration Information");
	for ( i = 0; i < 8; i++ ) {
		fprintf( stderr, "\n Cal   %03d-%02d  : DO " ,
				 g_nMyPcm, 
				 i);
		fflush(stderr);
	}
	for ( i = 8; i < 16; i++ ) {
		fprintf( stderr, "\n Cal   %03d-%02d  : [ %4.2f ] [ %4.2f ]  " ,
				 g_nMyPcm, 
				 i, 
				 g_pCal[i].f_zero,
				 g_pCal[i].cal_vo.f_conv[1] );
		fflush(stderr);
	}

	for ( i = 16; i < 32; i++ ) {
		fprintf( stderr, "\n Cal   %03d-%02d  : [ %4.2f ] [ %4.2f ] [ %4.2f ] [ %4.2f %4.2f ]" ,
				 g_nMyPcm, 
				 i, 
				 g_pCal[i].f_zero,
				 g_pCal[i].cal_vi.f_conv[1], 
				 g_pCal[i].cal_ci.f_conv[1], 
				 g_pCal[i].cal_jpt.f_conv[0], 
				 g_pCal[i].cal_jpt.f_conv[1] );
		fflush(stderr);
	}
	fprintf( stderr, "\n\n" );
	fflush(stderr);
}


void pnt_cal_zero()
// ----------------------------------------------------------------------------
// GET ZERO CALIBRATION VALUE
// Description : zero값의 calibration을 수행한다. 
// Arguments   : none
// Returns     : none
{
 	struct timespec ts;
 	  
	// check g_nCalReady value
	if ( g_nCalReady != 1 ) {
		fprintf( stderr, "\nIt need calibration table loading\n");
		fflush(stderr);
		return;
	}

	// calibration의 status를 변경한다. 
	sleep(1);
	g_nCalStatus = CAL_ZERO;
	ts.tv_sec = 0;
	ts.tv_nsec = 10 * M_SEC;				
	nanosleep(&ts, NULL);					

	// calibration의 status가 초기화 될 때 까지 기다린다. 
	// status는 pnt_calibration()에서 calibration을 마치면 초기화가 된다. 
	for (;;) {
		// wait... status..
		if ( g_nCalStatus == CAL_NONE )
			break;
		else {
			ts.tv_sec = 0;
			ts.tv_nsec = 10 * M_SEC;				
			nanosleep(&ts, NULL);							
		}
	}
}



void pnt_cal_grp(unsigned int n_grp, unsigned int n_type)
// ----------------------------------------------------------------------------
// GET CALIBRATION VALUE
// Description : 각각의 Type에 대한 calibration을 수행한다. 
// Arguments   : none
// Returns     : none
{
	struct timespec ts;
	
	// check g_nCalReady value
	if ( g_nCalReady != 1 ) {
		fprintf( stderr, "\nIt need calibration table loading\n");
		fflush(stderr);
		return;
	}

	sleep(1);
	fprintf( stderr, "\n\nCalibration group = %d, type = %d \n", n_grp, n_type);
	fflush(stderr);

	// type과 calibration의 stutus를 설정한다. 
	// 설정된 group과 status로	pnt_calibration()에서 calibration을 진행한다. 
	switch ( n_type ) {
		// Type is VI
		case CMD_CAL_TYPE_VI:
			g_nCalGrp = n_grp;
			g_nCalStatus = CAL_VI;
			break;

		// Type is CI
		case CMD_CAL_TYPE_CI:
			g_nCalGrp = n_grp;
			g_nCalStatus = CAL_CI;
			break;

		// Type is JPT
		case CMD_CAL_TYPE_JPT:
			g_nCalGrp = n_grp;
			g_nCalStatus = CAL_JPT;
			break;

		// Type is VO
		case CMD_CAL_TYPE_VO:
			g_nCalGrp = n_grp;
			g_nCalStatus = CAL_VO;
			break;

		default:
			return;
	}

	// calibration의 status가 초기화 될 때 까지 기다린다. 
	// status는 pnt_calibration()에서 calibration을 마치면 초기화가 된다. 
	for (;;) {
		// wait... status..
		if ( g_nCalStatus == CAL_NONE )
			break;
		else {
			ts.tv_sec = 0;
			ts.tv_nsec = 10 * M_SEC;				
			nanosleep(&ts, NULL);							
		}
	}
}


void pnt_cal_setvalue(unsigned char *p)
// ----------------------------------------------------------------------------
// GET CALIBRATION VALUE
// Description : calibraion에 필요한 측정값을 받는다. 
// Arguments   : none
// Returns     : none
{
	//int i = 0;
	int cnt = 0;
	CAL_VAL_T *p_cal;
	unsigned short w_grp = 0;
	unsigned short w_pno = 0;
	unsigned short w_type = 0;
	float f_adc1 = 0, f_adc2 = 0;
	//float f_conv0, f_conv1, f_sig1, f_max;
	float f_val[8];

	// check g_nCalReady value
	if ( g_nCalReady != 1 ) {
		fprintf( stderr, "\nIt need calibration table loading\n");
		fflush(stderr);
		return;
	}

	w_grp = p[0];
	w_type = p[1];

	// 입력값을 받으면 Calibration 값들을 다시 계산한다. 
	switch ( w_type  ) {
		// Type is VO
		case CMD_CAL_TYPE_VO:
			// convert value
			f_val[0] = (float) p[2] * 0.1;
			f_val[1] = (float) p[3] * 0.1;
			f_val[2] = (float) p[4] * 0.1;
			f_val[3] = (float) p[5] * 0.1;
			f_val[4] = (float) p[6] * 0.1;
			f_val[5] = (float) p[7] * 0.1;
			f_val[6] = (float) p[8] * 0.1;
			f_val[7] = (float) p[9] * 0.1;

			for ( w_pno = (w_grp * 8); w_pno < ((w_grp * 8) + 8); w_pno++ ) {
				p_cal = &g_pCal[w_pno].cal_vo;
	
				p_cal->f_sig[1] = f_val[cnt++];
	
				p_cal->f_a = (p_cal->f_conv[1] - p_cal->f_conv[0]) / (p_cal->f_sig[1] - p_cal->f_sig[0]);
				p_cal->f_b = p_cal->f_conv[0] - (p_cal->f_a * p_cal->f_sig[0]);
				p_cal->f_conv[0] = p_cal->f_b;
				p_cal->f_conv[1] = p_cal->f_a * 10.0 + p_cal->f_b;
	
				fprintf( stderr, 
						 "\n Cal   %03d-%02d  : VO [ %4.2f ]  %f  %f" ,
						 g_nMyPcm, 
						 w_pno, 
						 p_cal->f_conv[1],
						 p_cal->f_a,
						 p_cal->f_b  );
				fflush(stderr);
			}
			fprintf( stderr, "\n\n");
			fflush(stderr);
			break;

		// Type is VO
		case CMD_CAL_TYPE_JPT_HI:
			// convert value
			f_val[0] = (float) p[2] + 1400;
			f_val[1] = (float) p[3] + 1400;
			f_val[2] = (float) p[4] + 1400;
			f_val[3] = (float) p[5] + 1400;
			f_val[4] = (float) p[6] + 1400;
			f_val[5] = (float) p[7] + 1400;
			f_val[6] = (float) p[8] + 1400;
			f_val[7] = (float) p[9] + 1400;

			fprintf( stderr, "\n JPT HI\n");
			fflush(stderr);

			for ( w_pno = (w_grp * 8); w_pno < ((w_grp * 8) + 8); w_pno++ ) {
				p_cal = &g_pCal[w_pno].cal_jpt;
	
				p_cal->f_sig[1] = f_val[cnt++];
				/* 
				Reference Code (Velos). 
				 
				adc_c1 = ((RTD110_Vref*calp[no].sig[0] / (calp[no].sig[0]+RTD110_Rs) * (RTD110_R2/RTD110_R1 + RTD110_R2/RTD110_R3 + 1) - RTD110_Vref * RTD110_R2/RTD110_R1) * 4095.0/5.0);
				adc_c2 = ((RTD110_Vref*calp[no].sig[1] / (calp[no].sig[1]+RTD110_Rs) * (RTD110_R2/RTD110_R1 + RTD110_R2/RTD110_R3 + 1) - RTD110_Vref * RTD110_R2/RTD110_R1) * 4095.0/5.0);
				
				a = (calp[no].conv[0] - calp[no].conv[1]) / (adc_c1 - adc_c2);
				b = calp[no].conv[0] - a * adc_c1;
				
				eadri->rtd_min = b * 5.0;
				eadri->rtd_max = (a * 4095 + b) * 5.0;
				*/

				f_adc1 = ((RTD110_Vref * p_cal->f_sig[0] / (p_cal->f_sig[0] + RTD110_Rs) * (RTD110_R2/RTD110_R1 + RTD110_R2/RTD110_R3 + 1) - RTD110_Vref * RTD110_R2/RTD110_R1) * 4095.0/5.0);
				f_adc2 = ((RTD110_Vref * p_cal->f_sig[1] / (p_cal->f_sig[1] + RTD110_Rs) * (RTD110_R2/RTD110_R1 + RTD110_R2/RTD110_R3 + 1) - RTD110_Vref * RTD110_R2/RTD110_R1) * 4095.0/5.0);

				p_cal->f_a = (p_cal->f_conv[0] - p_cal->f_conv[1]) / (f_adc1 - f_adc2);
				p_cal->f_b = p_cal->f_conv[0] - p_cal->f_a * f_adc1;

				p_cal->f_min = p_cal->f_b * 5.0;
				p_cal->f_max = (p_cal->f_a * 4095 + p_cal->f_b) * 5.0;
							
				/*	
				Reference Code (Velos). 
				 						
				cp->adri[HCP_RTD].a	 = 100.0 / ((eadri->rtd_max - eadri->rtd_min) / 5.0);
				cp->adri[HCP_RTD].b	 = -cp->adri[HCP_RTD].a * eadri->rtd_min / 5.0;
				*/

				p_cal->f_a = 100.0 / ((p_cal->f_max- p_cal->f_min) / 5.0);
				p_cal->f_b = -p_cal->f_a * p_cal->f_min / 5.0;

				fprintf( stderr, 
						 "\n Cal   %03d-%02d  : VO [ %4.2f  %4.2f ]  ( %f %f ) [ %f  %f] %f  %f" ,
						 g_nMyPcm, 
						 w_pno, 
						 p_cal->f_conv[0],
						 p_cal->f_conv[1],
						 p_cal->f_sig[0],
						 p_cal->f_sig[1],
						 f_adc1, 
						 f_adc2,
						 p_cal->f_a,
						 p_cal->f_b );
				fflush(stderr);
			}

			fprintf( stderr, "\n\n");
			fflush(stderr);
			break;

		case CMD_CAL_TYPE_JPT_LO:
			// convert value
			f_val[0] = (float) p[2] + 800;
			f_val[1] = (float) p[3] + 800;
			f_val[2] = (float) p[4] + 800;
			f_val[3] = (float) p[5] + 800;
			f_val[4] = (float) p[6] + 800;
			f_val[5] = (float) p[7] + 800;
			f_val[6] = (float) p[8] + 800;
			f_val[7] = (float) p[9] + 800;

			fprintf( stderr, "\n JPT LOW\n");
			fflush(stderr);

			for ( w_pno = (w_grp * 8); w_pno < ((w_grp * 8) + 8); w_pno++ ) {
				p_cal = &g_pCal[w_pno].cal_jpt;
	
				p_cal->f_sig[0] = f_val[cnt++];

				/* 
				Reference Code (Velos). 
				 
				adc_c1 = ((RTD110_Vref*calp[no].sig[0] / (calp[no].sig[0]+RTD110_Rs) * (RTD110_R2/RTD110_R1 + RTD110_R2/RTD110_R3 + 1) - RTD110_Vref * RTD110_R2/RTD110_R1) * 4095.0/5.0);
				adc_c2 = ((RTD110_Vref*calp[no].sig[1] / (calp[no].sig[1]+RTD110_Rs) * (RTD110_R2/RTD110_R1 + RTD110_R2/RTD110_R3 + 1) - RTD110_Vref * RTD110_R2/RTD110_R1) * 4095.0/5.0);
				
				a = (calp[no].conv[0] - calp[no].conv[1]) / (adc_c1 - adc_c2);
				b = calp[no].conv[0] - a * adc_c1;
				
				eadri->rtd_min = b * 5.0;
				eadri->rtd_max = (a * 4095 + b) * 5.0;
				*/

				f_adc1 = ((RTD110_Vref * p_cal->f_sig[0] / (p_cal->f_sig[0] + RTD110_Rs) * (RTD110_R2/RTD110_R1 + RTD110_R2/RTD110_R3 + 1) - RTD110_Vref * RTD110_R2/RTD110_R1) * 4095.0/5.0);
				f_adc2 = ((RTD110_Vref * p_cal->f_sig[1] / (p_cal->f_sig[1] + RTD110_Rs) * (RTD110_R2/RTD110_R1 + RTD110_R2/RTD110_R3 + 1) - RTD110_Vref * RTD110_R2/RTD110_R1) * 4095.0/5.0);

				p_cal->f_a = (p_cal->f_conv[0] - p_cal->f_conv[1]) / (f_adc1 - f_adc2);
				p_cal->f_b = p_cal->f_conv[0] - p_cal->f_a * f_adc1;

				p_cal->f_min = p_cal->f_b * 5.0;
				p_cal->f_max = (p_cal->f_a * 4095 + p_cal->f_b) * 5.0;

				/*	
				Reference Code (Velos). 
				 						
				cp->adri[HCP_RTD].a	 = 100.0 / ((eadri->rtd_max - eadri->rtd_min) / 5.0);
				cp->adri[HCP_RTD].b	 = -cp->adri[HCP_RTD].a * eadri->rtd_min / 5.0;
				*/

				p_cal->f_a = 100.0 / ((p_cal->f_max - p_cal->f_min) / 5.0);
				p_cal->f_b = -p_cal->f_a * p_cal->f_min / 5.0 ;

				fprintf( stderr, 
						 "\n Cal   %03d-%02d  : VO [ %4.2f  %4.2f ]  ( %f %f ) [ %f  %f] %f  %f" ,
						 g_nMyPcm, 
						 w_pno, 
						 p_cal->f_conv[0],
						 p_cal->f_conv[1],
						 p_cal->f_sig[0],
						 p_cal->f_sig[1],
						 f_adc1, 
						 f_adc2,
						 p_cal->f_a,
						 p_cal->f_b );
				fflush(stderr);
			}

			fprintf( stderr, "\n\n");
			fflush(stderr);
			break;
	}
}





