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
#include <sys/poll.h>
#include <strings.h>
#include <netdb.h>


#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"
#include "queue_handler.h"									// queue handler
#include "demo_handler.h"

#define		word		unsigned short
#define  	Px(P)		iPget(P)
#define  	Py	   		iPset
#define 	PCM(X,Y)   	(((X)*256)+(Y))


/*
#define    SMCON_00				PCM(1,0)		// Test
#define    SMCON_01				PCM(1,1)		// Test
#define    SMCON_02				PCM(1,2)		// Test

#define    CTCON_mode			PCM(1,10)		// Test
#define    CTCON_out			PCM(1,11)		// Test
#define    CTCON_real			PCM(1,12)		// Test
#define    SCTCON_on			PCM(1,13)		// Test
#define    CTCON_off			PCM(1,14)		// Test

#define    DCONT_out			PCM(1,20)		//
#define    DCONT_real			PCM(1,21)		//
#define    DCONT_set			PCM(1,22)		//

#define    D_CONT_out			PCM(1,30)		//
#define    D_CONT_real			PCM(1,31)		//
#define    D_CONT_set			PCM(1,32)		//

#define    DCOMP_data1			PCM(1,40)		//
#define    DCOMP_data2			PCM(1,41)		//
#define    DCOMP_value			PCM(1,42)		//

#define    CHANGECONT_mode		PCM(1,50)		//
#define    CHANGECONT_out1		PCM(1,51)		//
#define    CHANGECONT_out2		PCM(1,52)		//
#define    CHANGECONT_count		PCM(1,53)		//
*/

#define    DO_000				PCM(0,0)
#define    DO_001				PCM(0,1)
#define    DO_002				PCM(0,2)
#define    DO_003				PCM(0,3)
#define    DO_004				PCM(0,4)
#define    DO_005				PCM(0,5)
#define    DO_006				PCM(0,6)
#define    DO_007				PCM(0,7)

#define    VO_008				PCM(0,8)
#define    VO_009				PCM(0,9)
#define    VO_010				PCM(0,10)
#define    VO_011				PCM(0,11)
#define    VO_012				PCM(0,12)
#define    VO_013				PCM(0,13)
#define    VO_014				PCM(0,14)
#define    VO_015				PCM(0,15)

#define    DI_017				PCM(0,17)
#define    DI_018				PCM(0,18)
#define    DI_019				PCM(0,19)
#define    DI_020				PCM(0,20)
#define    DI_021				PCM(0,21)
#define    DI_022				PCM(0,22)
#define    DI_023				PCM(0,23)

#define    DO_100				PCM(1,0)
#define    DO_101				PCM(1,1)
#define    DO_102				PCM(1,2)
#define    DO_103				PCM(1,3)
#define    DO_104				PCM(1,4)
#define    DO_105				PCM(1,5)
#define    DO_106				PCM(1,6)
#define    DO_107				PCM(1,7)

#define    DI_116				PCM(1,16)
#define    DI_117				PCM(1,17)
#define    DI_118				PCM(1,18)
#define    DI_119				PCM(1,19)
#define    DI_120				PCM(1,20)
#define    DI_121				PCM(1,21)
#define    DI_122				PCM(1,22)
#define    DI_123				PCM(1,23)


#define    DO_200				PCM(2,0)
#define    DO_201				PCM(2,1)
#define    DO_202				PCM(2,2)
#define    DO_203				PCM(2,3)
#define    DO_204				PCM(2,4)
#define    DO_205				PCM(2,5)
#define    DO_206				PCM(2,6)
#define    DO_207				PCM(2,7)

#define    DI_216				PCM(2,16)
#define    DI_217				PCM(2,17)
#define    DI_218				PCM(2,18)
#define    DI_219				PCM(2,19)
#define    DI_220				PCM(2,20)
#define    DI_221				PCM(2,21)
#define    DI_222				PCM(2,22)
#define    DI_223				PCM(2,23)


extern int g_nMyPcm;


static void iface_demo_sleep(int sec, int msec) 
{
    fd_set reads, temps;
    struct timeval tv;
    
    FD_ZERO(&reads);
    FD_SET(0, &reads);
    
    tv.tv_sec = sec;
    tv.tv_usec = 1000 * msec;
    temps = reads;
    
    select( 0, &temps, 0, 0, &tv );
    
	return;
}

int iPget(word ctrl_num)
{
	int nPcm = 0;
	int nPno = 0;
	
	nPcm = ctrl_num/256;
	nPno = ctrl_num%256;
	
	return (int)pGet(nPcm, nPno);

}

void iPset(word ctrl_num, int value)
{
	int nPcm = 0;
	int nPno = 0;
	
	nPcm = ctrl_num/256;
	nPno = ctrl_num%256;
	
	pSet(nPcm, nPno, value);	
}

//==========================================================================================
/*SMCON(�����,�溸��,��°�)*/
/*����溸*/
// ���� �溸�� �Ӱ����� ���� (�Ǵ� �˶��� 1�̸�, �����ϴ� (0,1)���� ��½�Ų��)
void SMCON(word pOut , word pAlarm ,int Chul)
{		
	printf("%s :: ", __FUNCTION__);
	if ( Px ( pAlarm ) == 1 ) {
		printf("Control ( pOut = %d, %d )\n", pOut, Chul);
		Py( pOut , Chul);
		return;
	}
	printf("Not Control\n" );
}		


//==========================================================================================
/*CTCON(��°�,������,on������,off������,�÷���)*/
/*������ co ���� �������� ���Ͽ� ������ ���� on/off ��Ų��.*/
void CTCON(word pMode,word pOut,word pRea,word pOnSet,word pOffSet,char sta)
{
	double	xs1 , xs2 , xi ;
	
	printf("%s :: ", __FUNCTION__);
	
	xs1  = Px (pOnSet) ;
	xs2  = Px (pOffSet) ;
	xi   = Px (pRea) ;
	
	if ( Px(pMode) == 1 ){
		if ( xs2 >= xs1 ) {
			Py ( pMode , 0 );
			printf(" [1] (%f, %f) 0\n", xs1, xs2);
			return;
		}
		else {
			if ( sta == 0 ) {
				Py ( pOut , 0 );
				printf(" [2]  (%f, %f) 0\n", xs1, xs2);
				return;
			}
			else {
				if ( xi >= xs1 ) {      		// �⵿ {
					Py ( pOut , 1 );
					printf(" [3]  (%f, %f) 1\n", xs1, xs2);
					return;					
				}
				else if ( xi < xs2 ) {	    // ���� {
					Py ( pOut , 0 );
					printf(" [4]  (%f, %f) 0\n", xs1, xs2);
					return;										
				}
			}
		}
	}
	
	printf("Not Works\n");
	return;	
}


//==========================================================================================
/*DCONT(��°�,������,������,�����׸��ý�,�÷���)*/
/*�����������*/	
// ������긦 �������� �����ҋ� ���� on �������� ������ off�� �ȴ� 
void DCONT(word pOut, word pReal,word pSet, double Hyst)
{
	double	IMSIP , IMSIM ;
	
	printf("%s :: ", __FUNCTION__);
	
	IMSIP = Px ( pSet ) + Hyst ;
	IMSIM = Px ( pSet ) - Hyst ;
	
	if ( Px ( pReal ) < IMSIM ) 	{
		Py ( pOut , 1 ) ;
		printf(" (%f, %f, %f) 1\n", (float)Px ( pReal ), IMSIP, IMSIM);
		return;
	}
	
	if ( Px ( pReal ) > IMSIP ) 	{
		Py ( pOut , 0 ) ;	
		printf(" (%f, %f, %f) 0\n", (float)Px ( pReal ), IMSIP, IMSIM);
		return;
	}
	
	printf("Not Works\n");
	return;
}


//==========================================================================================
/*D_CONT(��°�,������,������,�����׸��ý�,�÷���)*/
/*�ð�ž����*/	
// �ð�ž�� �������� �Ѿ�� on ������ ���ϰ� �Ǹ� off
void D_CONT(word pOut, word pReal,word pSet, double Hyst)
{
	double	IMSIP , IMSIM ;
	
	printf("%s ::", __FUNCTION__);
	
	IMSIP = Px ( pSet ) + Hyst ;
	IMSIM = Px ( pSet ) - Hyst ;
	
	if( Px ( pReal ) < IMSIM ) 	{
		Py ( pOut , 0 ) ;
		printf(" (%f, %f, %f) 0\n", (float)Px ( pReal ), IMSIP, IMSIM);
		return;
	}
	
	if( Px ( pReal ) > IMSIP ) 	{
		Py ( pOut , 1 ) ;
		printf(" (%f, %f, %f) 1\n", (float)Px ( pReal ), IMSIP, IMSIM);
		return;
	}
	
	printf("Not Works\n");
	return;	
}



//==================================================================================================
/*DCOMP(������1,������2,�����)*/
/*�ܱ�µ� ȯ��µ���*/
//�ܱ� �µ��� ȯ��µ�(�ǳ��µ�)�� ���Ͽ� 0,1�� ���ġ�� ������	
void DCOMP(word pData1,word pData2,word pValue)
{	
	double	IMSI ;

	printf("%s :: ", __FUNCTION__);	
	
	IMSI = Px ( pData1 ) - Px ( pData2 );

	if ( IMSI > 0 ) {
		Py ( pValue , 1 );				//data1 > data2
		printf(" [1] ( %f) 1\n", IMSI);
		return;	    		
	}
	else {
		if ( IMSI < -2 ) {
			Py ( pValue , 0 );			//data1 < data2
			printf(" [2] ( %f) 0\n", IMSI);
			return;	    			
		}
    }
	
	printf("Not Works\n");
	return;	    
}


//==================================================================================================
/*DAMCON(��°�,������,������,��ʴ�,�÷���1,�÷���2,������)*/
/*��������*/
//�ܱ�µ��� ȯ��µ�(�ǳ��µ�)�� ���� ���ġ�� �����µ��� ȯ��µ��� ���Ͽ� ���۸� open,close�Ѵ�.
void	DAMCON(word pOut,word pRea,word pSet,double sca,word pSwFlag,word pStaFlag,word pShiftVal)
{	
	double	xs = 0 , xi = 0 , yp = 0, ym = 0;
	double	diff = 0 , off = 0 , yc = 0 ;
	int	sta , sw ;

	printf("%s\n", __FUNCTION__);	
	
	off  = Px (pShiftVal) ;
	xs   = Px (pSet) ;
	xi   = Px (pRea) ;
	sw   = Px (pSwFlag) ;
	sta  = Px (pStaFlag);	
	diff = xs- xi ;

	yc = diff * sca + 50 ;
	
	if(yc < 30)
		yp = ((-1) * 3.3333 * yc)+100;
	else {
		yp = 0;
		if(yc > 70)
			ym = 3.3333 * (yc-70);
		else
			ym = 0;
	}	
		
	if ( sta == 0)
		Py ( pOut , 0 );
	else {	
		if ( sw == 0 )					//outair temp < return temp
			Py ( pOut , yp + off );
		else if ( sw == 1 )
			Py ( pOut , ym + off );		//outair temp > return temp
	}
}


//======================================================================================================
/*PICON(�Լ�NO,��°�,������,������,���а�,�÷���1,�÷���2,�÷���3,���нð�)*/    //���нð�: 2   - 180 sec
/*��.����.HWG.HX �������*/						       //���а�  : 0.1 - 5
//�ó��� ��긦 �������� ���Ͽ� ������� ��� �Ѵ�.
double	CONT(double y, double scale, double difference)
{
	double	IMSI=0;		

	printf("%s\n", __FUNCTION__);	
		
	IMSI = difference * scale ;

	y = IMSI + y ; 

	if ( y > 100 )
		y = 100;
	else if ( y < 0 )
		y = 0 ;

	return( y );
}


//======================================================================================================
//��������
// CHANGECONT(���,���1,���2,�÷���)
// �ν��� ���� �� ���� ����� ������� �Ѵ�.
void  CHANGECONT(word pMode, word pOut1, word pOut2, word pCont)
{
   static int ii=0, jj=0;

	printf("%s :: ", __FUNCTION__);	
	      
	if ( Px(pMode) ) {
		if ( Px(pCont)==0 ) {
			Py(pOut1,0);
			Py(pOut2,0);
			
			if( jj == 0 ) { 
				jj = 1; 
				ii++;
				if( ii > 1 )
				ii=0;
			}
		}
		else {
			jj = 0;
			if( ii == 0 ) {
				Py(pOut1,1);
				Py(pOut2,0);
			}       		 
			else {
				Py(pOut1,0);
				Py(pOut2,1);
			}       		 
		}     
	}
		
	printf("Not Works\n");
	return;	
}


//====================================================================================================
/*PPCON(��°�,������,on������,off������,�÷���)*/
//�������� ���Ͽ� ����, ����, ����������� on/off ���� �Ѵ�.
void	PPCON(word pMode,word pOut,word pRea,word pOnSet,word pOffSet,char sta)
{
	double	xs1 , xs2 , xi ;

	printf("%s\n", __FUNCTION__);	
	      
	xs1  = Px (pOnSet) ;
	xs2  = Px (pOffSet) ;
	xi   = Px (pRea) ;

	if ( Px(pMode) == 1 )
	{
		if ( xs2 <= xs1 )
			Py ( pMode , 0 );
		else
		{
			if ( sta == 0 )
				Py ( pOut , 0 );
			else
			{
				if ( xi <= xs1 )       // �⵿
					Py ( pOut , 1 );
				else if ( xi > xs2 )	    // ����
					Py ( pOut , 0 );
			}
		}
	}

}			

//====================================================================================================
/*RPPCON(��°�,������,on������,off������,�÷���)*/
//�������� ���Ͽ� �ð���, �ù��������� on/off ���� �Ѵ�.
void	RPPCON(word pMode,word pOut,word pRea,word pOnSet,word pOffSet,char sta)

{
	double	xs1 , xs2 , xi ;

	printf("%s\n", __FUNCTION__);	
	      
	xs1  = Px (pOnSet) ;
	xs2  = Px (pOffSet) ;
	xi   = Px (pRea) ;

	if ( Px(pMode) == 1 )
	{
		if ( xs2 <= xs1 )
			Py ( pMode , 0 );
		else
		{
			if ( sta == 0 )
				Py ( pOut , 0 );
			else
			{
				if ( xi >= xs1 )       		// �⵿
					Py ( pOut , 1 );
				else if ( xi < xs2 )	    // ����
					Py ( pOut , 0 );
			}
		}
	}

}	   	

	
//=====================================================================================================
//�迭1-20�� ����Ͽ���, ������ ���� ����ġ�� ����ġ�� ���Ͽ� ��°��� ���´�
//HcFlag==2 �ϋ� �������� ������ �����Ͽ� ��°��� ���´�
//HcFlag==0 �ϋ� ������(��½�(+) : ����ġ (��) : ����ġ > ����ġ ,	 0�� (-) : ����ġ(��) < ����ġ
//HcFlag==1 �϶� HcFlag==0�� �ݴ� ����
void	PICON(unsigned char n,word pOut,word pRea,word pSet,double sca,word pSwFlag,word pStaFlag,double HcFlag,int ITime)

{	
	double	xs , xi , yp  , ym ;
	double	diff ;
	int	sta , sw;
 	static char tim[20] ;

	printf("%s\n", __FUNCTION__);	
	      
	yp   = Px ( pOut ) ;	
	ym   = Px ( pOut ) ;
	xs   = Px ( pSet ) ;
	xi   = Px ( pRea ) ;
	sw   = Px ( pSwFlag ) ;
	sta  = Px ( pStaFlag );	
	diff = xs- xi ;

	if( tim[n] > 180)
		tim[n] =  0;
	tim[n]++;

	if ( sta == 0)
		Py ( pOut , 0 );
	else if( (int)tim[n] % (int)ITime == 0 )
	{	
		if ( HcFlag == 2 )				//win.sum change
		{
			if ( sw == 0 )         			//win      
			{	
				yp = CONT( yp, sca, diff );
				Py ( pOut , yp );
			}
			else if ( sw == 1 ) 			//sum  
			{
				ym = CONT( ym, sca*(-1), diff );
				Py ( pOut , ym );
			}
		}
		else	if ( HcFlag == 0 )			//tcv
		{
			yp = CONT( yp, sca, diff );
			Py ( pOut , yp );
		}
		else	if ( HcFlag == 1 )			//ccv
		{
			ym = CONT( ym, sca*(-1), diff );
			Py ( pOut , ym );
		}
	}	
}



//======================================================================================================
// ����
// ������ ���� ����ġ�� ����ġ�� ���Ͽ� �ù� ���� �����긦 ������� �Ѵ�.
void PICONTROL(unsigned char n,word pOut,word pRea, word pSet, double sca,word pSwFlag,int sta,double HcFlag,double ITime)
{	
	double	xi, xs ;
	double	diff, imsi ;
	int	sw ;
 	static double ii[50], yy[50], pp[50];

	printf("%s\n", __FUNCTION__);	
	      
	xi   = Px ( pRea ) ;
        xs   = Px ( pSet ) ;
	sw   = Px ( pSwFlag ) ;
	diff = xs- xi ;

	if ( sta == 0)
            Py ( pOut , 0 );
	else 
	{	
		if ( HcFlag == 2 )				//win.sum change
		{
			if ( sw == 0 )         			//win      
                   {	
         		pp[n] =  sca * diff ;
                        imsi  = (1 / ITime);
                        ii[n] = ii[n] + pp[n] * imsi;
                           	if(ii[n] > 100)
                		ii[n] = 100;
                        	if(ii[n] < 0 )
                     		ii[n] = 0 ;
                        	if(pp[n] > 100)
	                	pp[n] = 100;
                        	if(pp[n] < 0 )
                		pp[n] = 0 ;
                        yy[n]  = pp[n] + ii[n];
                          	if(yy[n] > 100)
                		yy[n] = 100;
                        	if(yy[n] < 0 )
                        	yy[n] = 0 ;
	        	 Py ( pOut , yy[n] );
		   }
			else if ( sw == 1 ) 			//sum  
		   {
         		pp[n] = (sca*(-1)) * diff ;
                        imsi  = (1 / ITime);
                        ii[n] = ii[n] + pp[n] * imsi;
                           	if(ii[n] > 100)
                		ii[n] = 100;
                        	if(ii[n] < 0 )
                     		ii[n] = 0 ;
                        	if(pp[n] > 100)
	                	pp[n] = 100;
                        	if(pp[n] < 0 )
                		pp[n] = 0 ;
                        yy[n]  = pp[n] + ii[n];
                          	if(yy[n] > 100)
                		yy[n] = 100;
                        	if(yy[n] < 0 )
                        	yy[n] = 0 ;
	        	 Py ( pOut , yy[n] );
		   }
		}
		else	if ( HcFlag == 0 )			//tcv
		{
         		pp[n] =  sca * diff ;
                        imsi  = (1 / ITime);
                        ii[n] = ii[n] + pp[n] * imsi;
                           	if(ii[n] > 100)
                		ii[n] = 100;
                        	if(ii[n] < 0 )
                     		ii[n] = 0 ;
                        	if(pp[n] > 100)
	                	pp[n] = 100;
                        	if(pp[n] < 0 )
                		pp[n] = 0 ;
                        yy[n]  = pp[n] + ii[n];
                          	if(yy[n] > 100)
                		yy[n] = 100;
                        	if(yy[n] < 0 )
                        	yy[n] = 0 ;
	        	 Py ( pOut , yy[n] );
              	}
		else	if ( HcFlag == 1 )			//ccv
		{
         		pp[n] = (sca*(-1)) * diff ;
                        imsi  = (1 / ITime);
                        ii[n] = ii[n] + pp[n] * imsi;
                           	if(ii[n] > 100)
                		ii[n] = 100;
                        	if(ii[n] < 0 )
                     		ii[n] = 0 ;
                        	if(pp[n] > 100)
	                	pp[n] = 100;
                        	if(pp[n] < 0 )
                		pp[n] = 0 ;
                        yy[n]  = pp[n] + ii[n];
                          	if(yy[n] > 100)
                		yy[n] = 100;
                        	if(yy[n] < 0 )
                        	yy[n] = 0 ;
	        	 Py ( pOut , yy[n] );
		}
	}	
}

#if 1
//======================================================================================================
//���� ����ġ ���
void     FMCCON(char unsigned  no, word pOut, word pVir, word pSel)
{
	static char PreS[30], PreV[30];
	
	printf("%s\n", __FUNCTION__);	
	  
	if(Px(pSel)  != PreS[no]) {
		Py(pOut, Px(pSel));
		Py(pVir, Px(pSel));
		PreS[no] = Px(pSel);
	}
	else if(Px(pVir) != PreV[no]) {
		Py(pOut, Px(pVir));
		PreV[no] = Px(pVir);
	}
}


//==========================================================================================
//*OATEMP-AUTOSET(����,��������,�溸)*/
//OALowSet=OA1, OAHighSet=OA2, SETLowSet=ST1, SETHighSet=ST2, OATemp=OA, SetTemp=ST
//AUTO SETTEMP=ST1+(OA2-OA)(ST2-ST1)%(OA2-OA1) 
void	AUTOSET(word pST, word pOA, word pOA1, word pOA2, word pST1, word pST2)
{

	printf("%s\n", __FUNCTION__);	
	         
     if(Px(pOA) < Px(pOA1))
        Py(pST, Px(pST2)); 
     if(Px(pOA) > Px(pOA2))
        Py(pST, Px(pST1));
     if(Px(pOA1) < Px(pOA) && Px(pOA) < Px(pOA2))
        Py(pST,(Px(pST1)+(Px(pOA2) - Px(pOA))*(Px(pST2) - Px(pST1)) / (Px(pOA2) - Px(pOA1)))); 
}

#endif


// IDC 2���� �����Ǵ� APG
void Demo_IDC_02(void) 
{
	// ICU-M�� 8�� ����
	if ( Px(DO_100) > 0 ) 
		Py(DO_201, 1) ;
	else
		Py(DO_201, 0) ;				

	// ICU-M�� 9�� ����
	if ( Px(DO_101) > 0 ) 
		Py(DO_202, 1) ;
	else
		Py(DO_202, 0) ;				
		
	// ICU-M�� 10�� ����
	if ( Px(DO_102) > 0 ) 
		Py(DO_203, 1) ;
	else
		Py(DO_203, 0) ;		
}


// IDC 1���� �����Ǵ� APG
void Demo_IDC_01(void) 
{
	// ICU-M�� 1�� ����
	if ( Px(DI_017) > 0 ) { 
		Py(DO_000, 1) ;
		Py(DO_001, 1) ;
	}
	else {
		Py(DO_000, 0) ;
		Py(DO_001, 0) ;				
	}

	// ICU-M�� 2�� ����
	if ( Px(DI_018) > 0 ) 
		Py(DO_002, 1) ;
	else
		Py(DO_002, 0) ;				
		
	// ICU-M�� 3�� ����
	if ( Px(DI_019) > 0 ) 
		Py(DO_003, 1) ;
	else
		Py(DO_003, 0) ;		

	// ICU-M�� 4�� ����
	if ( Px(DI_116) > 0 ) 
		Py(DO_004, 1) ;
	else
		Py(DO_004, 0) ;		
		
	// ICU-M�� 5�� ����
	if ( Px(DI_117) > 0 ) 
		Py(DO_005, 1) ;
	else
		Py(DO_005, 0) ;				

	// ICU-M�� 6�� ����
	if ( Px(DI_118) > 0 ) 
		Py(DO_006, 1) ;
	else
		Py(DO_006, 0) ;				
		
	// ICU-M�� 7�� ����
	if ( Px(DI_119) > 0 ) 
		Py(DO_007, 1) ;
	else
		Py(DO_007, 0) ;		

	// ICU-M�� 9�� ����ġ
	if ( Px(DI_121) > 0 ) 
		Py(DO_100, 1) ;
	else
		Py(DO_100, 0) ;	
		
	// ICU-M�� 10�� ����ġ
	if ( Px(DI_122) > 0 ) 
		Py(DO_101, 1) ;
	else
		Py(DO_101, 0) ;									
}


void *demo_main(void* arg)
{
	signal(SIGPIPE, SIG_IGN);	// Ignore broken_pipe signal
	sleep(1);
	
	while (1) {

		iface_demo_sleep(1,0);		
		// APG Demo�� �����Ѵ�. 

		if (g_nMyPcm == 2) {
			// �ǳ� �溸�߻�
			if ( Px(DI_221) > 0 )  {
				Py(DO_205, 1) ;			// 5�� ���� �⵿
				Py(DO_200, 1) ;			// �溸�߻�
			}
			else {
				Py(DO_205, 0) ;			//// 5�� ���� ����
				Py(DO_200, 0) ;			// �溸����
			}
					
			// ����ġ ���� ��� 4�� ����ġ�� �����Ѵ�. 
			if ( Px(DI_220) > 0 )  {
				Py(DO_204, 1) ;
				
				// 1�� ����
				if ( Px(DI_217) > 0 ) 
					Py(DO_201, 1) ;
				else
					Py(DO_201, 0) ;					

				// 2�� ����
				if ( Px(DI_218) > 0 ) 
					Py(DO_202, 1) ;
				else
					Py(DO_202, 0) ;										

				// 3�� ����
				if ( Px(DI_219) > 0 ) 
					Py(DO_203, 1) ;
				else
					Py(DO_203, 0) ;					
			}					
			else 
				Py(DO_204, 0) ;
		}
		
		if (g_nMyPcm == 0) {
			
			// IDC 2���� �����Ǵ� APG
			if ( Px(DI_220) == 0 )
				Demo_IDC_02();
			
			// IDC 1���� �����Ǵ� APG
			if ( Px(DI_120) > 0 )
				Demo_IDC_01();			
			
		}		


		iface_demo_sleep(1,0);
		//printf("\n\n");
	}
}


				
		/*
		SMCON(SMCON_00, SMCON_01, Px(SMCON_02));
		
		//CTCON(��°�,������,on������,off������,�÷���)
		//������ co ���� �������� ���Ͽ� ������ ���� on/off ��Ų��.
		//void CTCON(word pMode,word pOut,word pRea,word pOnSet,word pOffSet,char sta)		
		CTCON(CTCON_mode, CTCON_out, CTCON_real, SCTCON_on, CTCON_off, 1);
		
		//DCONT(��°�,������,������,�����׸��ý�,�÷���)
		//�����������
		//������긦 �������� �����ҋ� ���� on �������� ������ off�� �ȴ� 
		//void DCONT(word pOut, word pReal,word pSet, double Hyst)		
		DCONT(DCONT_out, DCONT_real, DCONT_set, 10)	;
		
		
		//D_CONT(��°�,������,������,�����׸��ý�,�÷���)
		//�ð�ž����
		//�ð�ž�� �������� �Ѿ�� on ������ ���ϰ� �Ǹ� off
		//void D_CONT(word pOut, word pReal,word pSet, double Hyst)		
		D_CONT(D_CONT_out, D_CONT_real, D_CONT_set, 10);	
		
		
		//DCOMP(������1,������2,�����)
		//�ܱ�µ� ȯ��µ���
		//�ܱ� �µ��� ȯ��µ�(�ǳ��µ�)�� ���Ͽ� 0,1�� ���ġ�� ������	
		//void DCOMP(word pData1,word pData2,word pValue)
		DCOMP(DCOMP_data1,DCOMP_data2,DCOMP_value);
		
		//DAMCON(��°�,������,������,��ʴ�,�÷���1,�÷���2,������)
		//��������
		//�ܱ�µ��� ȯ��µ�(�ǳ��µ�)�� ���� ���ġ�� �����µ��� ȯ��µ��� ���Ͽ� ���۸� open,close�Ѵ�.
		//void DAMCON(word pOut,word pRea,word pSet,double sca,word pSwFlag,word pStaFlag,word pShiftVal)		
		//DAMCON(word pOut,word pRea,word pSet,double sca,word pSwFlag,word pStaFlag,word pShiftVal);

		//��������
		//CHANGECONT(���,���1,���2,�÷���)
		//�ν��� ���� �� ���� ����� ������� �Ѵ�.
		//void CHANGECONT(word pMode, word pOut1, word pOut2, word pCont)
		CHANGECONT(CHANGECONT_mode, CHANGECONT_out1, CHANGECONT_out2, CHANGECONT_count);

	// CTCON�� ������ �������� �� ����. 	
	//PPCON(��°�,������,on������,off������,�÷���)
	//�������� ���Ͽ� ����, ����, ����������� on/off ���� �Ѵ�.
	//void PPCON(word pMode,word pOut,word pRea,word pOnSet,word pOffSet,char sta)
	//PPCON(word pMode,word pOut,word pRea,word pOnSet,word pOffSet,char sta);
		
	// CTCON�� ������ �������� �� ����. 			
	//RPPCON(��°�,������,on������,off������,�÷���)
	//�������� ���Ͽ� �ð���, �ù��������� on/off ���� �Ѵ�.
	//void RPPCON(word pMode,word pOut,word pRea,word pOnSet,word pOffSet,char sta)
	//RPPCON(word pMode,word pOut,word pRea,word pOnSet,word pOffSet,char sta);
		
			
		//�迭1-20�� ����Ͽ���, ������ ���� ����ġ�� ����ġ�� ���Ͽ� ��°��� ���´�
		//HcFlag==2 �ϋ� �������� ������ �����Ͽ� ��°��� ���´�
		//HcFlag==0 �ϋ� ������(��½�(+) : ����ġ (��) : ����ġ > ����ġ ,	 0�� (-) : ����ġ(��) < ����ġ
		//HcFlag==1 �϶� HcFlag==0�� �ݴ� ����
		//void PICON(unsigned char n,word pOut,word pRea,word pSet,double sca,word pSwFlag,word pStaFlag,double HcFlag,int ITime)
		//PICON(unsigned char n,word pOut,word pRea,word pSet,double sca,word pSwFlag,word pStaFlag,double HcFlag,int ITime);
		
		
		//����
		//������ ���� ����ġ�� ����ġ�� ���Ͽ� �ù� ���� �����긦 ������� �Ѵ�.
		//void PICONTROL(unsigned char n,word pOut,word pRea, word pSet, double sca,word pSwFlag,int sta,double HcFlag,double ITime)
		//PICONTROL(unsigned char n,word pOut,word pRea, word pSet, double sca,word pSwFlag,int sta,double HcFlag,double ITime);
		
		*/		
