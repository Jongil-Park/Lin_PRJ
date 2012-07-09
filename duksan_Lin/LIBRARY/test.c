#include <stdio.h>
#include "api.h"

#define Px(P)		iPget(P)
#define Py	   		iPset
#define PCM(X,Y)	(((X)*256)+(Y))
#define send		lib_uart2_send
#define recv		lib_uart2_recv

//2010.3.2 �������ý����̼�Ÿ(����)
//==PCM 1
//DO
#define CKT1_SST         PCM(0, 0)//�ð�ž1 �⵿/����
#define CKT2_SST         PCM(0, 1)//�ð�ž2 �⵿/����
#define HW1_SST          PCM(0, 2)//�ÿ¼���1 �⵿/����
#define HW2_SST          PCM(0, 3)//�ÿ¼���2 �⵿/����
#define HWSP1_SST        PCM(0, 4)//�ÿ¼���ȯ����1 �⵿/����
#define HWSP2_SST        PCM(0, 5)//�ÿ¼���ȯ����2 �⵿/����
#define HWSP3_SST        PCM(0, 6)//�ÿ¼���ȯ����3 �⵿/����
#define CWSP1_SST        PCM(0, 7)//�ð�����ȭ����1 �⵿/����
//AO
#define CWSFT1           PCM(7, 8)//�ð����޼��µ�1
#define CWRFT1           PCM(7, 9)//�ð���ȯ���µ�1
#define CWSFT2           PCM(7,10)//�ð����޼��µ�2
#define CWRFT2           PCM(7,11)//�ð���ȯ���µ�2
#define HWSFT1           PCM(7,12)//�ÿ¼���޼��µ�1
#define HWRFT1           PCM(7,13)//�ÿ¼���ȯ���µ�1
#define HWSFT2           PCM(7,14)//�ÿ¼���޼��µ�2
#define HWRFT2           PCM(7,15)//�ÿ¼���ȯ���µ�2
								  // 
								  // 
#define INVET1_STA       PCM(1,16)//�ι���2 ����
#define INVET1_A         PCM(1,17)//�ι���2 �溸
#define HXRFT            PCM(1,18)//����ȯ���µ�
#define HBOT             PCM(1,19)//�¼����Ϸ��µ�
#define HWT              PCM(1,20)//������ũ�µ�
#define HWRT             PCM(1,21)//����ȯ���µ�
#define F1BT1            PCM(1,22)//1��-1 ����µ� �ޱ�
#define F1BT2            PCM(1,23)//1��-2 ����µ� ȯ��
#define F2BT1            PCM(1,24)//2��-1 ����µ� �ޱ�
#define F2BT2            PCM(1,25)//2��-2 ����µ� ȯ��
#define F3BT1            PCM(1,26)//3��-1 ����µ� �ޱ�
#define F3BT2            PCM(1,27)//3��-2 ����µ� ȯ��
#define F4BT1            PCM(1,28)//4��-1 ����µ� �ޱ�
#define F4BT2            PCM(1,29)//4��-2 ����µ� ȯ��
#define F4BT3            PCM(1,30)//4��-3 ����µ� �ޱ�
#define F4BT4            PCM(1,31)//4��-4 ����µ� ȯ��

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









