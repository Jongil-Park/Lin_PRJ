#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/tcp.h>
#include <sys/poll.h>		// use poll event

#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"
#include "queue_handler.h"									// queue handler
#include "plc_observer.h"
#include "elba_mgr.h"
#include "net32_to_dnp.h"

#define DNP_DI_SEL      1
#define DNP_DO_SEL      2
#define DNP_AI_SEL      3
#define DNP_AO_SEL      4

extern void putElbaQ(point_info *pPoint);
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];

extern DNP_DIO_Point DNP_di_point[512];
extern DNP_DIO_Point DNP_do_point[512];
extern DNP_AIO_Point DNP_ai_point[512];
extern DNP_AIO_Point DNP_ao_point[512];
//
extern int debug_plc_tx;
extern int debug_plc_rx;


/*******************************************************************/
void net32todnpHandlerSelectSleep(int sec,int msec) 
/*******************************************************************/
{
    struct timeval tv;
    tv.tv_sec=sec;
    tv.tv_usec=msec;                
    select(0,NULL,NULL,NULL,&tv);
    return;
}


void DNP_DIO_define(int DIO_sel, short pno, short sAddr, unsigned char EnUnsol)
{
  int pcm, pcm_pno;
  DNP_DIO_Point *DNP_p = NULL;
  
  if(DIO_sel == DNP_DI_SEL)
    DNP_p = &DNP_di_point[pno];
  else 
    DNP_p = &DNP_do_point[pno];
  
  DNP_p->Unsol = 0;
  DNP_p->val = 0;
  DNP_p->PrevVal = 0;
  DNP_p->WrFlag = 0;  
  DNP_p->PLCaddr = sAddr;
  DNP_p->EnUnsol = EnUnsol;
  
  pcm = sAddr / 1000;
  pcm_pno = sAddr % 1000;
  
  //Preq(pcm, pcm_pno);
  pReq(pcm, pcm_pno);
}  
  
  
void DNP_AIO_define(int AIO_sel, short pno, short sAddr, short scale, short minval, short maxval)
{
  int pcm, pcm_pno;
  DNP_AIO_Point *DNP_p = NULL;
  
  if(AIO_sel == DNP_AI_SEL)
    DNP_p = &DNP_ai_point[pno];
  else if(AIO_sel == DNP_AO_SEL)
    DNP_p = &DNP_ao_point[pno];
  
  DNP_p->Unsol = 0;
  DNP_p->val = 0;
  DNP_p->PrevVal = 0;
  DNP_p->WrFlag = 0;  
  DNP_p->PLCaddr = sAddr;
  DNP_p->scale = scale;
  DNP_p->offset = 0;
  DNP_p->MinVal = minval;
  DNP_p->MaxVal = maxval;
  
  
  pcm = sAddr / 1000;
  pcm_pno = sAddr % 1000;
  
  //Preq(pcm, pcm_pno);
  pReq(pcm, pcm_pno);
}

void eusadang_substation()
{
    int i = 0;
    
    //DI  
    for(i=0;i<7;i++)
        DNP_DIO_define(DNP_DI_SEL, i, i+1011, 1);
    for(i=0;i<8;i++)
        DNP_DIO_define(DNP_DI_SEL, i+7, i+1019, 1);
    for(i=0;i<4;i++)
        DNP_DIO_define(DNP_DI_SEL, i+15, i+1028, 1);
    for(i=0;i<4;i++)
        DNP_DIO_define(DNP_DI_SEL, i+19, i+2008, 1);
    for(i=0;i<13;i++)
        DNP_DIO_define(DNP_DI_SEL, i+23, i+2013, 1);
    for(i=0;i<4;i++)
        DNP_DIO_define(DNP_DI_SEL, i+36, i+2028, 1);
    for(i=0;i<4;i++)
        DNP_DIO_define(DNP_DI_SEL, i+40, i+4011, 1);
    for(i=0;i<12;i++)
        DNP_DIO_define(DNP_DI_SEL, i+44, i+4019, 1);
    for(i=0;i<6;i++)
        DNP_DIO_define(DNP_DI_SEL, i+56, i+5019, 1);
    for(i=0;i<2;i++)
        DNP_DIO_define(DNP_DI_SEL, i+62, i+6008, 1);
    for(i=0;i<3;i++)
        DNP_DIO_define(DNP_DI_SEL, i+64, i+6013, 1);
    for(i=0;i<2;i++)
        DNP_DIO_define(DNP_DI_SEL, i+67, i+6030, 1);
    
    DNP_DIO_define(DNP_DI_SEL, 69, 6004, 1);
    
    for(i=0;i<2;i++)
        DNP_DIO_define(DNP_DI_SEL, i+70, i+13012, 1);
   	
	// jong2ry. 2010/01/20
	// add point
    DNP_DIO_define(DNP_DI_SEL, 72, 2055, 1);
    DNP_DIO_define(DNP_DI_SEL, 73, 2056, 1);

    //DO  
    for(i=0;i<6;i++)
        DNP_DIO_define(DNP_DO_SEL, i, i+2048, 0);
    for(i=0;i<3;i++)
        DNP_DIO_define(DNP_DO_SEL, i+6, i+2001, 0);
    
    DNP_DIO_define(DNP_DO_SEL, 9, 2005, 0);
    DNP_DIO_define(DNP_DO_SEL, 10, 2000, 0);
    
    for(i=0;i<4;i++)
        DNP_DIO_define(DNP_DO_SEL, i+11, i+3057, 0);
    for(i=0;i<2;i++)
        DNP_DIO_define(DNP_DO_SEL, i+15, i+3032, 0);
    for(i=0;i<2;i++)
        DNP_DIO_define(DNP_DO_SEL, i+17, i+4057, 0);

    DNP_DIO_define(DNP_DO_SEL, 19, 6004, 0);
    
    DNP_DIO_define(DNP_DO_SEL, 20, 6050, 1);
    DNP_DIO_define(DNP_DO_SEL, 21, 6056, 1);
    DNP_DIO_define(DNP_DO_SEL, 22, 6054, 1);
    DNP_DIO_define(DNP_DO_SEL, 23, 6057, 1);
    DNP_DIO_define(DNP_DO_SEL, 24, 6055, 1);
    DNP_DIO_define(DNP_DO_SEL, 25, 6058, 1);
    DNP_DIO_define(DNP_DO_SEL, 26, 6065, 1);
    DNP_DIO_define(DNP_DO_SEL, 27, 6064, 1);
    
    for(i=0;i<2;i++)
        DNP_DIO_define(DNP_DO_SEL, i+28, i+6048, 1);
    
    DNP_DIO_define(DNP_DO_SEL, 30, 6051, 1);
    
    for(i=0;i<2;i++)
        DNP_DIO_define(DNP_DO_SEL, i+31, i+6062, 1);
    
    DNP_DIO_define(DNP_DO_SEL, 33, 6053, 1);
    
    //AI  
    for(i=0;i<2;i++)
        DNP_AIO_define(DNP_AI_SEL, i, i+1008, 10, 0, 16000);        
    
    DNP_AIO_define(DNP_AI_SEL, 2, 1010, 10, -300, 15700);
    
    for(i=0;i<6;i++)
        DNP_AIO_define(DNP_AI_SEL, i+3, i+3008, 10, 0, 16000);        
    for(i=0;i<3;i++)
        DNP_AIO_define(DNP_AI_SEL, i+9, i+3020, 10, 0, 16000);        
    for(i=0;i<3;i++)
        DNP_AIO_define(DNP_AI_SEL, i+12, i+3024, 10, 0, 16000);
    for(i=0;i<2;i++)
        DNP_AIO_define(DNP_AI_SEL, i+15, i+3028, 10, 0, 16000);
    for(i=0;i<3;i++)
        DNP_AIO_define(DNP_AI_SEL, i+17, i+4008, 10, 0, 16000);
    for(i=0;i<2;i++)
        DNP_AIO_define(DNP_AI_SEL, i+20, i+7008, 10, 0, 16000);
    for(i=0;i<6;i++)
        DNP_AIO_define(DNP_AI_SEL, i+22, i+1048, 1, 0, 16000);
        
	// jong2ry. 2010/01/20
	// add point
	DNP_AIO_define(DNP_AI_SEL, 28, 2027, 10, -300, 15700);
          
    //AO            
    DNP_AIO_define(DNP_AO_SEL, 0, 3048, 10, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 1, 3050, 10, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 2, 3051, 10, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 3, 3053, 10, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 4, 3054, 10, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 5, 3056, 10, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 6, 3049, 10, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 7, 4048, 10, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 8, 3052, 10, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 9, 4049, 10, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 10, 3055, 10, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 11, 4050, 10, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 12, 1057, 10, 0, 16000);
    
    for(i=0;i<2;i++)
        DNP_AIO_define(DNP_AO_SEL, i+13, i+4051, 10, 0, 16000);
    for(i=0;i<2;i++)
        DNP_AIO_define(DNP_AO_SEL, i+15, i+4055, 10, 0, 16000);
    for(i=0;i<4;i++)
        DNP_AIO_define(DNP_AO_SEL, i+17, i+5048, 10, 0, 16000);
    for(i=0;i<3;i++)
        DNP_AIO_define(DNP_AO_SEL, i+21, i+6059, 1, 0, 16000);
	
	// jong2ry. 2010/01/20
	// add point
    DNP_AIO_define(DNP_AO_SEL, 24, 2057, 1, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 25, 2058, 1, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 26, 2059, 1, 0, 16000);
    DNP_AIO_define(DNP_AO_SEL, 27, 2060, 1, 0, 16000);
}



void sunrung_substation()
{
    int i = 0;

    //DO  운전정지 
    DNP_DIO_define(DNP_DO_SEL, i++, 5000, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 5001, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 5003, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 5004, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 5006, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 5007, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 5033, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 5034, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 5036, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 5037, 0);
            
    DNP_DIO_define(DNP_DO_SEL, i++, 1000, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 1001, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 1002, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 1003, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 1004, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 1005, 0);
    
	DNP_DIO_define(DNP_DO_SEL, i++, 1006, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 1007, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 2000, 0);
    
	DNP_DIO_define(DNP_DO_SEL, i++, 2001, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 2002, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 2003, 0);
    
    DNP_DIO_define(DNP_DO_SEL, i++, 7002, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7003, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7006, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7007, 0);
    
    DNP_DIO_define(DNP_DO_SEL, i++, 7034, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7035, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7038, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7039, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7042, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7043, 0);
    
    DNP_DIO_define(DNP_DO_SEL, i++, 13000, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 13001, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7000, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7001, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7004, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7005, 0);
    
    DNP_DIO_define(DNP_DO_SEL, i++, 7032, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7033, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7036, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7037, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7040, 0);
    DNP_DIO_define(DNP_DO_SEL, i++, 7041, 0);

    
    //DO  자동수동
    i = 100;
    DNP_DIO_define(DNP_DO_SEL, i++, 5052, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 5055, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 5058, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 5061, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 5064, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 5067, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 5070, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 5073, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 5076, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 5079, 1);
  
	DNP_DIO_define(DNP_DO_SEL, i++, 0, 1);
	DNP_DIO_define(DNP_DO_SEL, i++, 0, 1);
	DNP_DIO_define(DNP_DO_SEL, i++, 0, 1);
  
    DNP_DIO_define(DNP_DO_SEL, i++, 1048, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 1048, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 1048, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 1055, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 1055, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 1055, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 1062, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 1062, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 1062, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 1069, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 1069, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 1069, 1);
    
    DNP_DIO_define(DNP_DO_SEL, i++, 7058, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 7058, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 7059, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 7059, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 7060, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 7060, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 7061, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 7061, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 7062, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 7062, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 5048, 1);
    DNP_DIO_define(DNP_DO_SEL, i++, 5048, 1);

	// DI
	i = 0;
	DNP_DIO_define(DNP_DI_SEL,i++,5020,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6027,1);
	DNP_DIO_define(DNP_DI_SEL,i++,5021,1);
	DNP_DIO_define(DNP_DI_SEL,i++,5022,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6020,1);
	DNP_DIO_define(DNP_DI_SEL,i++,5023,1);
	DNP_DIO_define(DNP_DI_SEL,i++,5026,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6028,1);
	DNP_DIO_define(DNP_DI_SEL,i++,5025,1);
	DNP_DIO_define(DNP_DI_SEL,i++,5028,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6021,1);
	DNP_DIO_define(DNP_DI_SEL,i++,5027,1);
	DNP_DIO_define(DNP_DI_SEL,i++,5030,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6029,1);
	DNP_DIO_define(DNP_DI_SEL,i++,5031,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6008,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6024,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6009,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6012,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6030,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6011,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6014,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6025,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6013,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6017,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6031,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6016,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6019,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6026,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6018,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6022,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6023,1);
	DNP_DIO_define(DNP_DI_SEL,i++,6010,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,2019,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2020,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2021,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2022,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2023,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2024,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2025,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2026,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2027,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2028,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2029,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2030,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,3012,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3011,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3010,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3009,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3008,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2031,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,3018,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3017,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3016,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3015,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3014,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3013,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,3019,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3020,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3021,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3022,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3023,1);
	DNP_DIO_define(DNP_DI_SEL,i++,3024,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1024,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1025,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1026,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1027,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1028,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1029,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1032,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1033,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1034,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1035,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1036,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1037,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1040,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1041,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1042,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1043,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1044,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1045,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2008,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2009,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2010,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2011,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2012,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2013,1);
	DNP_DIO_define(DNP_DI_SEL,i++,7021,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10009,1);
	DNP_DIO_define(DNP_DI_SEL,i++,7022,1);
	DNP_DIO_define(DNP_DI_SEL,i++,7023,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10010,1);
	DNP_DIO_define(DNP_DI_SEL,i++,7024,1);
	DNP_DIO_define(DNP_DI_SEL,i++,7025,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10011,1);
	DNP_DIO_define(DNP_DI_SEL,i++,7026,1);
	DNP_DIO_define(DNP_DI_SEL,i++,7027,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10012,1);
	DNP_DIO_define(DNP_DI_SEL,i++,7028,1);
	DNP_DIO_define(DNP_DI_SEL,i++,7029,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10013,1);
	DNP_DIO_define(DNP_DI_SEL,i++,7030,1);
	DNP_DIO_define(DNP_DI_SEL,i++,7031,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10014,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8008,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8009,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10015,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8010,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8011,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10016,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8012,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8013,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10017,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8014,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8015,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10018,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8016,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8017,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10019,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8018,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8019,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10020,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8020,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8021,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10021,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8022,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8023,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10022,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8024,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8025,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10023,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8026,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8027,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10024,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8028,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8029,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,8030,1);
	DNP_DIO_define(DNP_DI_SEL,i++,8031,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,9008,1);
	DNP_DIO_define(DNP_DI_SEL,i++,9009,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,9010,1);
	DNP_DIO_define(DNP_DI_SEL,i++,9011,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,9012,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,9013,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10025,1);
	DNP_DIO_define(DNP_DI_SEL,i++,9014,1);
	DNP_DIO_define(DNP_DI_SEL,i++,9015,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10026,1);
	DNP_DIO_define(DNP_DI_SEL,i++,9016,1);
	DNP_DIO_define(DNP_DI_SEL,i++,10008,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1030,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1038,1);
	DNP_DIO_define(DNP_DI_SEL,i++,1046,1);
	DNP_DIO_define(DNP_DI_SEL,i++,2014,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,0,1);
	
	DNP_DIO_define(DNP_DI_SEL,i++,9017,1);
	DNP_DIO_define(DNP_DI_SEL,i++,9018,1);
	
	//AI
	i = 0;
    DNP_AIO_define(DNP_AI_SEL, i++, 5018, 10, 0, 16000);            
    DNP_AIO_define(DNP_AI_SEL, i++, 5019, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 7008, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 7009, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 7010, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 7011, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 7012, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 5008, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 5010, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 5012, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 5014, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 5016, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 5009, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 5011, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 5013, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 5015, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 5017, 10, -300, 15700);	
    DNP_AIO_define(DNP_AI_SEL, i++, 7019, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 7018, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 7017, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 7016, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 7015, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 7020, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 1008, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 1009, 10, 0, 16000);		
    DNP_AIO_define(DNP_AI_SEL, i++, 1010, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 1011, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 1012, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 1013, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 1014, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 1015, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 1016, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 1017, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 1018, 10, 0, 16000);	
    DNP_AIO_define(DNP_AI_SEL, i++, 1019, 10, 0, 16000);	
    
    // AO
    i = 0;
    DNP_AIO_define(DNP_AO_SEL, i++, 5053, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5054, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5059, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5060, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5065, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5066, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5071, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5072, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5077, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5078, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5056, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5057, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5062, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5063, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5068, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5069, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5074, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5075, 10, -300, 15700);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5080, 10, -300, 15700);	    
    DNP_AIO_define(DNP_AO_SEL, i++, 5081, 10, -300, 15700);	    
    
    DNP_AIO_define(DNP_AO_SEL, i++, 5050, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 5051, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1049, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1050, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1051, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1052, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1053, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1054, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1056, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1057, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1058, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1059, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1060, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1061, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1063, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1064, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1065, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1066, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1067, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1068, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1070, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1071, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1072, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1073, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1074, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 1075, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 7048, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 7050, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 7052, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 7054, 10, 0, 16000);	
    DNP_AIO_define(DNP_AO_SEL, i++, 7056, 10, 0, 16000);	
}


void dnp_ptbl_set()
{
    eusadang_substation();
    //sunrung_substation();
}



void update_dnp_ptbl(void)
{
    int i;
    //int pcm, pno, analog_pval;
    int pcm, pno; 
    float analog_pval;
    float float_pval;
    DNP_DIO_Point *DNP_DIO_p;
    DNP_AIO_Point *DNP_AIO_p;
    unsigned char *p;
    point_info point;
    struct timespec ts;
    
    static int preq_chk = 0;
  
    p = (unsigned char *) &float_pval;
    //preq_chk++;
    
    ts.tv_sec = 0;
    ts.tv_nsec = 100000000;
    
    if (preq_chk > 200)
        printf("DEBUG: Preq() DI points !!!\n");
      
    for (i = 0; i < MAX_DNP_DI_POINT; i++)
    {
        DNP_DIO_p = &DNP_di_point[i];
        pcm = DNP_DIO_p->PLCaddr / 1000;
        pno = DNP_DIO_p->PLCaddr % 1000;
    
        if(preq_chk > 200)
        {
            if ((pcm != 0) || (pno != 0))
                pReq(pcm, pno);
            preq_chk = 0;
        }
        
        //GetPtable(&pt, pcm, pno);
        
        //if(Pget(pcm,pno)>0)
        //if(pt.val > 0)
        if(g_fExPtbl[pcm][pno] > 0)
            DNP_DIO_p->val = 1;
        else
            DNP_DIO_p->val = 0;
      
        if(DNP_DIO_p->PrevVal != DNP_DIO_p->val)
        {
            //printf("DNPd: Create DI Unsol Bit, pno %d, PLCaddr %d, value %d, PrevVal %d \n", pno, DNP_DIO_p->PLCaddr, DNP_DIO_p->val, DNP_DIO_p->PrevVal);
            printf("DNPd: Create DI Unsol Bit, pno %d, PLCaddr %d, value %d, PrevVal %d \n", i, DNP_DIO_p->PLCaddr, DNP_DIO_p->val, DNP_DIO_p->PrevVal);
            DNP_DIO_p->Unsol = 1;
            DNP_DIO_p->PrevVal = DNP_DIO_p->val;
        }
    }

    //debug_date();
    //nanosleep(&ts,0);
    net32todnpHandlerSelectSleep(0, 10000);
    //debug_date();
    
    for (i = 0; i < MAX_DNP_DO_POINT; i++)
    {
        DNP_DIO_p = &DNP_do_point[i];
        pcm = DNP_DIO_p->PLCaddr / 1000;
        pno = DNP_DIO_p->PLCaddr % 1000;
        
        if(DNP_DIO_p->WrFlag)
        {
           // pSet(pcm, pno, (float)DNP_DIO_p->WrVal);
            if (DNP_DIO_p->WrVal) 
            {
                float_pval = 1.0f;
                //debug_date();
                printf("DEBUG: DNP DO Write pcm %d, pno %d, DNP_DIO_p->WrVal %d, float_pval %f \n", pcm, pno, DNP_DIO_p->WrVal, float_pval); 
                point.pcm = pcm;
                point.pcm = pno;
                point.value = float_pval;
                elba_push_queue(&point);
                //CommandPval3(pcm, pno, (unsigned char *) &float_pval);
                 pSet(pcm, pno, float_pval);
            }
            else 
            {
                float_pval = 0.0f;
                //debug_date();
                printf("DEBUG: DNP DO Write pcm %d, pno %d, DNP_DIO_p->WrVal %d, float_pval %f \n", pcm, pno, DNP_DIO_p->WrVal, float_pval); 
                //CommandPval3(pcm, pno, (unsigned char *) &float_pval);
                point.pcm = pcm;
                point.pcm = pno;
                point.value = float_pval;
                elba_push_queue(&point);
                pSet(pcm, pno, float_pval);
            }
            DNP_DIO_p->WrFlag = 0;
        }
    
        if(preq_chk > 200)
        {
            //Preq(pcm, pno);
            preq_chk = 0;
        }
      
        //GetPtable(&pt, pcm, pno);
        
        //if(Pget(pcm,pno)>0)
        //if(pt.val > 0)
        if(g_fExPtbl[pcm][pno] > 0)
            DNP_DIO_p->val = 1;
        else
            DNP_DIO_p->val = 0;

        if(DNP_DIO_p->PrevVal != DNP_DIO_p->val)
        {
            //debug_date();
            printf("DNPd: Create DO Unsol Bit, pno %d, PLCaddr %d, value %d, PrevVal %d \n", pno, DNP_DIO_p->PLCaddr, DNP_DIO_p->val, DNP_DIO_p->PrevVal);
            DNP_DIO_p->Unsol = 1;
            DNP_DIO_p->PrevVal = DNP_DIO_p->val;
        }
    }

    //debug_date();
    //nanosleep(&ts,0);
    net32todnpHandlerSelectSleep(0, 10000);
    //debug_date();
    
    for (i = 0; i < MAX_DNP_AI_POINT; i++)
    {
        DNP_AIO_p = &DNP_ai_point[i];
        pcm = DNP_AIO_p->PLCaddr / 1000;
        pno = DNP_AIO_p->PLCaddr % 1000;
        
        //GetPtable(&pt, pcm, pno);
        //analog_pval = Pget(pcm,pno) * DNP_AIO_p->scale;
        //analog_pval = pt.val * DNP_AIO_p->scale;
        analog_pval = g_fExPtbl[pcm][pno] * DNP_AIO_p->scale;
        
        if(analog_pval > DNP_AIO_p->MaxVal)
            analog_pval = DNP_AIO_p->MaxVal;
        else if(analog_pval < DNP_AIO_p->MinVal)
            analog_pval = DNP_AIO_p->MinVal;
        DNP_AIO_p->val = analog_pval;
    }

    //debug_date();
    //nanosleep(&ts,0);
    net32todnpHandlerSelectSleep(0, 10000);
    //debug_date();

    for (i = 0; i < MAX_DNP_AO_POINT; i++)
    {
        DNP_AIO_p = &DNP_ao_point[i];
        pcm = DNP_AIO_p->PLCaddr / 1000;
        pno = DNP_AIO_p->PLCaddr % 1000;
    
        float_pval = DNP_AIO_p->PrevVal / DNP_AIO_p->scale;
    
        if(DNP_AIO_p->WrFlag) {
            //debug_date();
            printf("DEBUG: DNP AO Write pcm %d, pno %d, float_pval %f : %02x %02x %02x %02x \n", pcm, pno, float_pval, p[0], p[1], p[2], p[3]);
            pSet(pcm, pno, float_pval);
            //CommandPval3(pcm, pno, (unsigned char *) &float_pval);
            point.pcm = pcm;
            point.pcm = pno;
            point.value = float_pval;
            elba_push_queue(&point);
            // 20071205 DDC AO DNP_AIO_p->WrFlag = 0;
            // 20071205 DDC AO   // DNP_AIO_p->Unsol = 1;
        }
    
        //analog_pval = Pget(pcm,pno) * DNP_AIO_p->scale;
        //GetPtable(&pt, pcm, pno);
        //analog_pval = pt.val * DNP_AIO_p->scale;
        analog_pval = g_fExPtbl[pcm][pno] * DNP_AIO_p->scale;
        
        //if (i == 24)
        //    printf("DEBUG: Pget3 pt.val %f, analog_pval %f \n", pt.val, analog_pval);
    
        if(analog_pval > DNP_AIO_p->MaxVal)
            analog_pval = DNP_AIO_p->MaxVal;
        else if(analog_pval < DNP_AIO_p->MinVal)
            analog_pval = DNP_AIO_p->MinVal;
        
        DNP_AIO_p->val = analog_pval;
        
        // 20071205 DDC AO
        if (DNP_AIO_p->WrFlag)
        {
            DNP_AIO_p->val = DNP_AIO_p->PrevVal;
            DNP_AIO_p->Unsol = 1;
            DNP_AIO_p->WrFlag = 0;
        }
        // 20071205 DDC AO
    }
    //debug_date();
    //nanosleep(&ts,0);
    net32todnpHandlerSelectSleep(0, 10000);
    //debug_date();
}  

/****************************************************************/
void net32_to_dnp_main(void* arg)
/****************************************************************/
{
    dnp_ptbl_set();

    while(1) {
        update_dnp_ptbl();
    }
}

