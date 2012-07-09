#include <stdio.h>
#include "api.h"

#define Px(P)		iPget(P)
#define Py	   		iPset
#define PCM(X,Y)	(((X)*256)+(Y))
#define send		lib_uart2_send
#define recv		lib_uart2_recv

//2010.3.2 국제템플스테이센타(기계실)
//==PCM 1
//DO
#define CKT1_SST         PCM(0, 0)//냉각탑1 기동/정지
#define CKT2_SST         PCM(0, 1)//냉각탑2 기동/정지
#define HW1_SST          PCM(0, 2)//냉온수기1 기동/정지
#define HW2_SST          PCM(0, 3)//냉온수기2 기동/정지
#define HWSP1_SST        PCM(0, 4)//냉온수순환펌프1 기동/정지
#define HWSP2_SST        PCM(0, 5)//냉온수순환펌프2 기동/정지
#define HWSP3_SST        PCM(0, 6)//냉온수순환펌프3 기동/정지
#define CWSP1_SST        PCM(0, 7)//냉각수순화펌프1 기동/정지
//AO
#define CWSFT1           PCM(7, 8)//냉각수급수온도1
#define CWRFT1           PCM(7, 9)//냉각수환수온도1
#define CWSFT2           PCM(7,10)//냉각수급수온도2
#define CWRFT2           PCM(7,11)//냉각수환수온도2
#define HWSFT1           PCM(7,12)//냉온수기급수온도1
#define HWRFT1           PCM(7,13)//냉온수기환수온도1
#define HWSFT2           PCM(7,14)//냉온수기급수온도2
#define HWRFT2           PCM(7,15)//냉온수기환수온도2
								  // 
								  // 
#define INVET1_STA       PCM(1,16)//인버터2 상태
#define INVET1_A         PCM(1,17)//인버터2 경보
#define HXRFT            PCM(1,18)//난방환수온도
#define HBOT             PCM(1,19)//온수보일러온도
#define HWT              PCM(1,20)//급탕탱크온도
#define HWRT             PCM(1,21)//급탕환수온도
#define F1BT1            PCM(1,22)//1층-1 배관온도 급기
#define F1BT2            PCM(1,23)//1층-2 배관온도 환기
#define F2BT1            PCM(1,24)//2층-1 배관온도 급기
#define F2BT2            PCM(1,25)//2층-2 배관온도 환기
#define F3BT1            PCM(1,26)//3층-1 배관온도 급기
#define F3BT2            PCM(1,27)//3층-2 배관온도 환기
#define F4BT1            PCM(1,28)//4층-1 배관온도 급기
#define F4BT2            PCM(1,29)//4층-2 배관온도 환기
#define F4BT3            PCM(1,30)//4층-3 배관온도 급기
#define F4BT4            PCM(1,31)//4층-4 배관온도 환기

int g_nHundCount = 0;
int g_nTestValue = 0;
int g_nSecCount = 0;

void    PointDefine(void)
{
	byte    pcm, pno;
/*
	PdefDo2s(pcm, pno++, INIT_RESUME);
	PdefJpt1000(pcm, pno++, 3.0, 0, -30, 130);
	PdefDi2s(pcm, pno++);
    PdefVi(pcm, 11, 2.0,  1,   0,  0, 100);  
    PdefCi(pcm, 16, 1.0, 1.25, -20, 0, 100);
    PdefVr(pcm, pno++, 0.1, -100, 150);
*/    

	printf("%s()\n", __FUNCTION__);
	/*
	
    pcm = 9;
	pno = 0;
	while ( pno < 8 )
		PdefDo2s(pcm, pno++, INIT_RESUME);
    while ( pno < 16 )
        PdefVo(pcm, pno++, 0.5, 1, 0, 0, 100);      
    while ( pno < 32 )
		PdefJpt1000(pcm, pno++, 3.0, 0, -30, 130);
    while ( pno < 128)
         PdefVr(pcm, pno++, 0.1, -100, 150);    
	
    pcm = 10;
	pno = 0;
	while ( pno < 8 )
		PdefDo2s(pcm, pno++, INIT_RESUME);
    while ( pno < 16 )
        PdefVo(pcm, pno++, 0.5, 1, 0, 0, 100);      
    while ( pno < 32 )
		PdefJpt1000(pcm, pno++, 3.0, 0, -30, 130);
    while ( pno < 128)
         PdefVr(pcm, pno++, 0.1, -100, 150);    	
		
	pcm = 11;
	pno = 0;
	while ( pno < 8 )
		PdefDo2s(pcm, pno++, INIT_RESUME);
	while ( pno < 16 )
		PdefVo(pcm, pno++, 0.5, 1, 0, 0, 100);      
	while ( pno < 20 )
		PdefJpt1000(pcm, pno++, 3.0, 0, -30, 130);
	while ( pno < 24 )
		PdefVi(pcm, pno++, 2.0,  1,   0,  0, 100);
	while ( pno < 28 )
		PdefCi(pcm, pno++, 1.0, 1.25, -20, 0, 100);
	while ( pno < 32 )
		PdefDi2s(pcm, pno++);
	while ( pno < 128)
		 PdefVr(pcm, pno++, 0.1, -100, 150);    	

	pcm = 12;
	pno = 0;
	while ( pno < 128)
		 PdefVr(pcm, pno++, 0.1, -100, 150);    
		 */
}

void ApgInit(void)
{
	int nRtn = 0;
	
	nRtn = lib_uart2_open(b19200);
	printf("nRtn = %d\n", nRtn);
}

void SecondTimer(void)
{
	send("0123456789", 10, 1);

	fprintf(stdout, "S\n");
	fflush(stdout);	
	
	g_nSecCount++;
	if ( g_nSecCount > 5 ) {
		g_nSecCount = 0;
		if ( g_nTestValue == 0 ) {
			g_nTestValue = 1;
			Py(CKT1_SST, g_nTestValue);
		}
		else {
			g_nTestValue = 0;
			Py(CKT1_SST, g_nTestValue);
		}	
	}
}


void MinuteTimer(void)
{
	fprintf(stdout, "M\n");
	fflush(stdout);
}

void ApgMain(void)
{
	static char sec = 0, minute =0;  

	if ( sec == _sec )
		return;
	sec = _sec;
	SecondTimer();
	
	if ( minute == _minute )
		return;
	minute = _minute;
	MinuteTimer();
}









