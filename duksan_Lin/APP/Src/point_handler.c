/*
 * file 		: point_handler.c
 * creator   	: jong2ry
 *
 *
 * 외부에서 사용되는 pset, pinfo, pdef, node 등의 명령어를 수행하도록 한다.  
 *
 *
 * version :
 *  	0.0.1 - initiailize
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
#include <string.h>  		// strlen()
#include <unistd.h>  		// read, write
#include <fcntl.h>   		// open, close, O_RDWR, O_NONBLOCK
#include <sys/poll.h>		// use poll event
#include <malloc.h>

////////////////////////////////////////////////////////////////////////////////
//define
#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

////////////////////////////////////////////////////////////////////////////////
//extern variable
extern pthread_mutex_t pointTable_mutex;										// point table mutex

// point and node
extern int g_nMyPcm;															// my pcm number
extern int g_nMultiDdcFlag;														// mode select flag
extern unsigned char g_cNode[MAX_NET32_NUMBER];									// node
extern PTBL_INFO_T *g_pPtbl;													// point table
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];						// extension point table


// ----------------------------------------------------------------------------
void pnt_ismax(point_info *p)
// ----------------------------------------------------------------------------
// VALUE CHECK MAXIMUM VALUE
// Description : If bigger than define_max_value, set maximum value
// Arguments   : pcm			Is a pcm number
// 				 pno			Is a pno number
// 				 value 			Is a pointer of value
// Returns     : none
{
	if ( g_pPtbl[p->pno].f_max < p->value ) {
		p->value = g_pPtbl[p->pno].f_max;
	}
}


// ----------------------------------------------------------------------------
void pnt_ismin(point_info *p)
// ----------------------------------------------------------------------------
// VALUE CHECK MINIMUM VALUE
// Description : If smmaller than define_min_value, set minimum value
// Arguments   : pcm			Is a pcm number
// 				 pno			Is a pno number
// 				 value 			Is a pointer of value
// Returns     : none
{
	if ( p->value < g_pPtbl[p->pno].f_min ) {
		p->value = g_pPtbl[p->pno].f_min;
	}
}


// ----------------------------------------------------------------------------
int pnt_is_hyst(point_info *p)
// ----------------------------------------------------------------------------
// VALUE CHECK HYSTERESIS VALUE
// Description : 새로 쓰는 값과 이전값의 hysteresis값을 비교한다. 
// Arguments   : p					Is a poirnter of point_info
// Returns     : PNT_OVER_HYST		hysteresis값을 초과하였다. 
//			 	 PNT_UNDER_HYST		hysteresis값을 초과하지 않았다. 
{
	float f_diff;

	// Is is calculate diff value 
	if ( g_pPtbl[p->pno].f_preval > p->value ) {
		f_diff = g_pPtbl[p->pno].f_preval - p->value;
	}
	else if ( g_pPtbl[p->pno].f_preval < p->value ) {
		f_diff = p->value - g_pPtbl[p->pno].f_preval;
	}
	else {
		return PNT_UNDER_HYST;
	}

	// check diff value and hysteresis
	if ( f_diff >= g_pPtbl[p->pno].f_hyst ) {
		g_pPtbl[p->pno].f_preval = p->value;
		return PNT_OVER_HYST;
	}
	else {
		return PNT_UNDER_HYST;
	}
}


// ----------------------------------------------------------------------------
void pnt_local_pset(point_info *p)
// ----------------------------------------------------------------------------
// SET LOCAL POINT VALUE
// Description : set local point value
// Arguments   : n_pcm			Is a pcm number
// 				 n_pno			Is a pno number
// 				 f_value 		Is a pointer of value
// Returns     : none
{
	if ( g_pPtbl[p->pno].n_type == DFN_PNT_VR ||
		 g_pPtbl[p->pno].n_type == DFN_PNT_DO ||
		 g_pPtbl[p->pno].n_type == DFN_PNT_VO ) {
		// 최대값과 최소값을 비교한다. 
		pnt_ismax(p);
		pnt_ismin(p);

		if ( g_pPtbl[p->pno].f_val != p->value ) {
			pthread_mutex_lock( &pointTable_mutex );
			g_pPtbl[p->pno].f_val = p->value; 
			g_fExPtbl[p->pcm][p->pno] = p->value;
			pthread_mutex_unlock( &pointTable_mutex );
	
			p->message_type = NET32_TYPE_REPORT;

			if ( pnt_is_hyst(p) == PNT_OVER_HYST ) {
				net32_push_queue(p);				
			}


			// debug code
			/*
			fprintf ( stdout, "pcm =%d, pno =%d, value = %f, f_val = %f\n",
					  p->pcm, p->pno, p->value, g_pPtbl[p->pno].f_val );
			fflush(stdout);
			*/
		}
		//p->message_type = NET32_TYPE_REPORT;
		elba_push_queue(p);
		//uclient_push_queue(p);
	}
	else {
		// 입력값에 쓰는 경우.. 어떠케 해야 하나???
		// 고민...
		//fprintf ( stderr, "\n >> 3 %d %d %f\n", p->pcm, p->pno, p->value );
		//fflush(stderr);
	}
}


// ----------------------------------------------------------------------------
void pnt_local_adc(point_info *p)
// ----------------------------------------------------------------------------
// SET LOCAL POINT VALUE
// Description : set local point value
// Arguments   : n_pcm			Is a pcm number
// 				 n_pno			Is a pno number
// 				 f_value 		Is a pointer of value
// Returns     : none
{
	//float fVal = 0;
	
	if ( g_pPtbl[p->pno].n_type == DFN_PNT_DI2S ||
		 g_pPtbl[p->pno].n_type == DFN_PNT_JPT  ||
		 g_pPtbl[p->pno].n_type == DFN_PNT_CI   ||
		 g_pPtbl[p->pno].n_type == DFN_PNT_VI ) {

		if ( g_pPtbl[p->pno].f_val != p->value ) {
			
			// 이전값과 합해서 필터를 적용합니다. 
			// 값이 튀는 것을 방지하려고 적용하였습니다. 
			//fVal = g_pPtbl[p->pno].f_val + p->value;
			//p->value = fVal / 2;
			
			// 최대값과 최소값을 비교한다. 
			pnt_ismax(p);
			pnt_ismin(p);

			pthread_mutex_lock( &pointTable_mutex );
			g_fExPtbl[p->pcm][p->pno] = p->value;
			g_pPtbl[p->pno].f_val = p->value; 
			pthread_mutex_unlock( &pointTable_mutex );
	
			p->message_type = NET32_TYPE_REPORT;
			
			if ( pnt_is_hyst(p) == PNT_OVER_HYST ) {
				net32_push_queue(p);	
				elba_push_queue(p);	
				//uclient_push_queue(p);
			}
		}
	}
}


// ----------------------------------------------------------------------------
void pnt_local_subio(point_info *p)
// ----------------------------------------------------------------------------
// SET LOCAL POINT VALUE
// Description : set local point value
// Arguments   : n_pcm			Is a pcm number
// 				 n_pno			Is a pno number
// 				 f_value 		Is a pointer of value
// Returns     : none
{
	if ( g_pPtbl[p->pno].n_type == DFN_PNT_DI2S ||
		 g_pPtbl[p->pno].n_type == DFN_PNT_JPT  ||
		 g_pPtbl[p->pno].n_type == DFN_PNT_CI   ||
		 g_pPtbl[p->pno].n_type == DFN_PNT_VI ) {

		if ( g_pPtbl[p->pno].f_val != p->value ) {
			// 최대값과 최소값을 비교한다. 
			pnt_ismax(p);
			pnt_ismin(p);

			pthread_mutex_lock( &pointTable_mutex );
			g_fExPtbl[p->pcm][p->pno] = p->value;
			g_pPtbl[p->pno].f_val = p->value; 
			pthread_mutex_unlock( &pointTable_mutex );
	
			p->message_type = NET32_TYPE_REPORT;
			
			if ( pnt_is_hyst(p) == PNT_OVER_HYST ) {
				net32_push_queue(p);	
				elba_push_queue(p);
				uclient_push_queue(p);
			}
		}
	}
}


// ----------------------------------------------------------------------------
int pSet(int pcm, int pno, float value)
// ----------------------------------------------------------------------------
// SET VALUE IN POINT_TABLE
// Description : If change value, call elba_push_queue().
// 				 elba_push_queue() insert data in elba_queue.
// Arguments   : pcm			Is a pcm number
// 				 pno			Is a pno number
// 				 value 			Is a value
// Returns     : SUCCESS		always return SUCCESS
{
	//int i = 0;
	int nChange = 0;
	point_info point;

	point.pcm = pcm;
	point.pno = pno;
	point.value = value;

	if ( g_nMultiDdcFlag ) {
		pthread_mutex_lock(&pointTable_mutex);

		if ( g_fExPtbl[pcm][pno] != value ) {
			nChange = 1;
			g_fExPtbl[pcm][pno] = value;
		}

		if ( g_nMyPcm == pcm ) {
			nChange = 1;
			g_pPtbl[pno].f_val = value; 
		}
		pthread_mutex_unlock(&pointTable_mutex);
		
		// net32 모드가 아닌 경우에 point table에 값을 미리 쓰기 때문에
		// Elba로 전송할 때에는 NET32_TYPE_REPORT type로 전송한다. 
		if ( nChange > 0 ) {
			elba_push_queue(&point);
		}
		return SUCCESS;
	}
	
	// net32 mode
	// net32 mode인 경우에 pSet이 호출되면,
	// point_table에 값을 쓰지않고 net32MessageQueue로 data를 보낸다.
	// 자신의 point-table에 값을 쓰는 것이 아니면 Net32로 보낸다. (only alive pcm)
	if ( g_nMyPcm != pcm ) {
		point.message_type = NET32_TYPE_COMMAND;
		net32_push_queue( &point );
		//uclient_push_queue(&point);		
		return SUCCESS;
	}
	else {
		pnt_local_pset(&point);
		return SUCCESS;
	}

	// ccms interface 
	// ccms interface로 data를 전송할 때에는 
	// net32 노드를 확인한 후에 전송한다. 
	// net32가 연결되어 있다면 
	// ccms interface로 data를 전송하지 않는다. 
	if ( g_nMyPcm != pcm && g_cNode[pcm] == 0) {
		point.message_type = pcm;
		/*
		if ( g_nModeCCMS == CCMS_SERVER ) {
			if ( point.value != g_fExPtbl[point.pcm][point.pno] )
				ccms_put_queue(&point);
		}
		else {
			cclnt_put_queue(&point);
		}
		*/
	} 

	return SUCCESS;
}


// ----------------------------------------------------------------------------
float pGet(int pcm, int pno)
// ----------------------------------------------------------------------------
// RETURN VALUE IN POINT_TABLE
// Description : This function is called by threads to return value in g_fExPtbl.
// Arguments   : pcm			Is a pcm number
// 				 pno			Is a pno number
// Returns     : value			g_fExPtbl value (float type)
{
	float fValue;
	
	pthread_mutex_lock( &pointTable_mutex );
	fValue = g_fExPtbl[pcm][pno];
	pthread_mutex_unlock( &pointTable_mutex );	

	return fValue;
}


// ----------------------------------------------------------------------------
void pReq(int pcm, int pno)
// ----------------------------------------------------------------------------
// REQUEST VALUE THE OTHER PCM
// Description : If alive pcm, call net32_push_queue().
// 				 net32_push_queue function insert data in net32_queue.
// Arguments   : pcm			Is a pcm number
// 				 pno			Is a pno number
// Returns     : none
{
	//int i = 0;
	point_info sPoint;
	
	sPoint.pcm = pcm;
	sPoint.pno = pno;
	sPoint.value = 0;			
	sPoint.message_type = NET32_TYPE_REQUIRE;
	
	// only alive pcm
	//for ( i = 0; i < 32; i++ ) {
	//	if ( g_cNode[i] > 0 && i == pcm )	
	//		net32_push_queue( &sPoint );
	//}	
	net32_push_queue( &sPoint );
}


// ----------------------------------------------------------------------------
void pDef(void)
// ----------------------------------------------------------------------------
// RE-CREATE POINT DEFINE FILE
// Description : pdef 명령이 호출될 때 마다 PointDefine.dat를 새로 생성한다. 
// Arguments   : none
// Returns     : none
{
	FILE *pf;
	int n_ptbl_size = 0;

	n_ptbl_size = sizeof(PTBL_INFO_T) * MAX_POINT_NUMBER;

	if ( ( pf = fopen("/duksan/DATA/PointDefine.dat", "w") ) == NULL ) {
		fprintf( stderr, "\n[ERR] PointDefine.dat Open Error\n");
		fflush(stderr);
		return;
	}
	else {
		fwrite( g_pPtbl, n_ptbl_size, 1, pf );
	}

	fclose(pf);
}

