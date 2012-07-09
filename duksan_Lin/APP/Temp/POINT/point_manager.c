/* file : point_manger.c
 * author : park jong il
 * 
 * 
 * pset�� pget�� �����Ѵ�. 
 *
 * pget�� ��쿡�� 
 * point-table�� �׻� �ֽ��� ���� �����ϴ� ���� ������ �ϱ� ������ 
 * point-table�� ���� �״�� ������ �ش�.
 * |--------------|                  |--------------------------|              
 * | other thread | == call pGet ==> | point_manger.c           | 
 * |              |                  | return point-table value |
 * |--------------|                  |--------------------------|              
 *
 *
 * multi-ddc mode�� pSet�� ���
 * point-table�� ���� �� �Ŀ� elba_queue�� data�� ������. 
 * |--------------|                  |-----------------------|              
 * | other thread | == call pSet ==> | point_manger.c        | == elba_queue ==>
 * |              |                  | set point-table value |
 * |--------------|                  |-----------------------|              
 *
 * net32 mode�� pSet�� ���
 * data�� �ڽ��� ���� ��� point-table�� ���� ������Ʈ �ϰ� 
 * ������Ʈ�� data�� net32_message_queue�� ������, 
 * data�� �ٸ� PCM�� ��� net32_message_queue�� data�� ������. 
 * |--------------|                  |-----------------------|              
 * | other thread | == call pSet ==> | point_manger.c        | == elba_queue ==>
 * |              |                  | set point-table value |
 * |--------------|                  |-----------------------|              
 * 
 * 
 * version :
 * 		0.0.1 - code clean.
 * 		0.0.2 - point-table�� ���� �͵��� �����Ͽ����ϴ�. (hyst, offset, min, max...)
 * 		0.0.3 - calibration �߰�.
 * 
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
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/poll.h>		// use poll event

#include "define.h"
#include "queue.h"
#include "net32.h"
#include "elba.h"				// elba_put_queue
#include "point_manager.h"

// from main.c
extern int my_pcm;
extern int multi_ddc;
extern PTBL_INFO_T *gp_ptbl;
extern CAL_INFO_T *gp_cal;
extern int cal_ready;
extern float point_table[MAX_NET32_NUMBER][MAX_POINT_NUMBER];
extern pthread_mutex_t pointTable_mutex;
extern unsigned char g_cNode[32];


void pnt_ismax(point_info *p)
// ----------------------------------------------------------------------------
// VALUE CHECK MAXIMUM VALUE
// Description : If bigger than define_max_value, set maximum value
// Arguments   : pcm			Is a pcm number
// 				 pno			Is a pno number
// 				 value 			Is a pointer of value
// Returns     : none
{
	if ( gp_ptbl[p->pno].f_max < p->value ) {
		p->value = gp_ptbl[p->pno].f_max;
	}
}


void pnt_ismin(point_info *p)
// ----------------------------------------------------------------------------
// VALUE CHECK MINIMUM VALUE
// Description : If smmaller than define_min_value, set minimum value
// Arguments   : pcm			Is a pcm number
// 				 pno			Is a pno number
// 				 value 			Is a pointer of value
// Returns     : none
{
	if ( p->value < gp_ptbl[p->pno].f_min ) {
		p->value = gp_ptbl[p->pno].f_min;
	}
}


int pnt_is_hyst(point_info *p)
// ----------------------------------------------------------------------------
// VALUE CHECK HYSTERESIS VALUE
// Description : ���� ���� ���� �������� hysteresis���� ���Ѵ�. 
// Arguments   : p					Is a poirnter of point_info
// Returns     : PNT_OVER_HYST		hysteresis���� �ʰ��Ͽ���. 
//			 	 PNT_UNDER_HYST		hysteresis���� �ʰ����� �ʾҴ�. 
{
	float f_diff;

	// Is is calculate diff value 
	if ( gp_ptbl[p->pno].f_preval > p->value ) {
		f_diff = gp_ptbl[p->pno].f_preval - p->value;
	}
	else if ( gp_ptbl[p->pno].f_preval < p->value ) {
		f_diff = p->value - gp_ptbl[p->pno].f_preval;
	}
	else {
		return PNT_UNDER_HYST;
	}

	// check diff value and hysteresis
	if ( f_diff >= gp_ptbl[p->pno].f_hyst ) {
		gp_ptbl[p->pno].f_preval = p->value;
		return PNT_OVER_HYST;
	}
	else {
		return PNT_UNDER_HYST;
	}
}


void pnt_local_pset(point_info *p)
// ----------------------------------------------------------------------------
// SET LOCAL POINT VALUE
// Description : set local point value
// Arguments   : n_pcm			Is a pcm number
// 				 n_pno			Is a pno number
// 				 f_value 		Is a pointer of value
// Returns     : none
{
	if ( gp_ptbl[p->pno].n_type == DFN_PNT_VR ||
		 gp_ptbl[p->pno].n_type == DFN_PNT_DO ||
		 gp_ptbl[p->pno].n_type == DFN_PNT_VO ) {
		// �ִ밪�� �ּҰ��� ���Ѵ�. 
		pnt_ismax(p);
		pnt_ismin(p);

		// debug code
		//fprintf ( stderr, "pcm =%d, pno =%d, value = %f, f_val = %f\n",
		//		  p->pcm, p->pno, p->value, gp_ptbl[p->pno].f_val );
		//fflush(stderr);

		if ( gp_ptbl[p->pno].f_val != p->value ) {
			pthread_mutex_lock( &pointTable_mutex );
			gp_ptbl[p->pno].f_val = p->value; 
			point_table[p->pcm][p->pno] = p->value;
			pthread_mutex_unlock( &pointTable_mutex );
	
			p->message_type = NET32_TYPE_REPORT;
			
			if ( pnt_is_hyst(p) == PNT_OVER_HYST )
				net32_put_msgqueue(p);	
		}
	}
	else {
		// �Է°��� ���� ���.. ��� �ؾ� �ϳ�???
		// ���...
		//fprintf ( stderr, "\n >> 3 %d %d %f\n", p->pcm, p->pno, p->value );
		//fflush(stderr);
	}

	//fprintf ( stderr, "\n >> 4 %d %d %f\n", p->pcm, p->pno, p->value );
	//fflush(stderr);

}


void pnt_local_adc(point_info *p)
// ----------------------------------------------------------------------------
// SET LOCAL POINT VALUE
// Description : set local point value
// Arguments   : n_pcm			Is a pcm number
// 				 n_pno			Is a pno number
// 				 f_value 		Is a pointer of value
// Returns     : none
{
	if ( gp_ptbl[p->pno].n_type == DFN_PNT_DI2S ||
		 gp_ptbl[p->pno].n_type == DFN_PNT_JPT  ||
		 gp_ptbl[p->pno].n_type == DFN_PNT_CI   ||
		 gp_ptbl[p->pno].n_type == DFN_PNT_VI ) {

		if ( gp_ptbl[p->pno].f_val != p->value ) {
			// �ִ밪�� �ּҰ��� ���Ѵ�. 
			pnt_ismax(p);
			pnt_ismin(p);

			pthread_mutex_lock( &pointTable_mutex );
			point_table[p->pcm][p->pno] = p->value;
			gp_ptbl[p->pno].f_val = p->value; 
			pthread_mutex_unlock( &pointTable_mutex );
	
			p->message_type = NET32_TYPE_REPORT;
			
			if ( pnt_is_hyst(p) == PNT_OVER_HYST ) {
				net32_put_msgqueue(p);	
				//fprintf( stderr, "\n >> change pno = %d,  f_val = %0.2f ",
				//		 p->pno, p->value ); 
				//fprintf( stderr, " put net32...." );
			}
			//fflush(stderr);
		}
	}
}


int pSet(int pcm, int pno, float value)
// ----------------------------------------------------------------------------
// SET VALUE IN POINT_TABLE
// Description : If change value, call elba_put_queue().
// 				 elba_put_queue() insert data in elba_queue.
// Arguments   : pcm			Is a pcm number
// 				 pno			Is a pno number
// 				 value 			Is a value
// Returns     : SUCCESS		always return SUCCESS
{
	int i = 0;
	int nChange = 0;
	point_info point;

	point.pcm = pcm;
	point.pno = pno;
	point.value = value;

	//fprintf( stderr, "point_info %d %d %f\n", pcm, pno, value );

	switch( multi_ddc ) {
	case MULTI_DDC_ON:
		// multi_ddc mode
		// multi_ddc mode�� ��쿡 pSet�� ȣ��Ǹ�,
		// point_table�� ���� �ֱ��� ���� �����ϱ� ������
		// point_table�� ���� ���� elbaQueue�� data�� ������.
	
		pthread_mutex_lock(&pointTable_mutex);
		if ( point_table[pcm][pno] != value ) {
			nChange = 1;
			point_table[pcm][pno] = value;
		}
		pthread_mutex_unlock(&pointTable_mutex);
		
		if ( nChange > 0 ) 
			elba_put_queue(&point);
		break;

	case MULTI_DDC_OFF:
		// net32 mode
		// net32 mode�� ��쿡 pSet�� ȣ��Ǹ�,
		// point_table�� ���� �����ʰ� net32MessageQueue�� data�� ������.

		// �ڽ��� point-table�� ���� ���� ���� �ƴϸ� Net32�� ������. (only alive pcm)
		if ( my_pcm != pcm ) {
			point.message_type = NET32_TYPE_COMMAND;

			for ( i = 0; i < 32; i++ ) {
				if ( g_cNode[i] > 0 && i == pcm )	
					net32_put_msgqueue( &point );	
			}	
		}
		else {
			pnt_local_pset(&point);
		}
		break;
		

	default:
		break;
	}

	return SUCCESS;
}


float pGet(int pcm, int pno)
// ----------------------------------------------------------------------------
// RETURN VALUE IN POINT_TABLE
// Description : This function is called by threads to return value in point_table.
// Arguments   : pcm			Is a pcm number
// 				 pno			Is a pno number
// Returns     : value			point_table value (float type)
{
	float f_val;
	pthread_mutex_lock( &pointTable_mutex );
	f_val = point_table[pcm][pno];
	pthread_mutex_unlock( &pointTable_mutex );	
	return f_val;
}


void pReq(int pcm, int pno)
// ----------------------------------------------------------------------------
// REQUEST VALUE THE OTHER PCM
// Description : If alive pcm, call net32_put_msgqueue().
// 				 net32_put_msgqueue function insert data in net32_message_queue.
// Arguments   : pcm			Is a pcm number
// 				 pno			Is a pno number
// Returns     : none
{
	int i = 0;
	point_info point;
	
	point.pcm = pcm;
	point.pno = pno;
	point.value = 0;			
	point.message_type = NET32_TYPE_REQUIRE;
	
	// only alive pcm
	for ( i = 0; i < 32; i++ ) {
		if ( g_cNode[i] > 0 && i == pcm )	
			net32_put_msgqueue( &point );
	}	
}


void pDef(void)
// ----------------------------------------------------------------------------
// RE-CREATE POINT DEFINE FILE
// Description : pdef ����� ȣ��� �� ���� PointDefine.dat�� ���� �����Ѵ�. 
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
		fwrite( gp_ptbl, n_ptbl_size, 1, pf );
	}

	fclose(pf);
	fprintf( stderr, "\nSuccess PointDefine\n");
	fflush(stderr);
}


