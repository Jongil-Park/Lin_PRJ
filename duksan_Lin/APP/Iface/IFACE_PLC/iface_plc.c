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
//
//
#include "define.h"
#include "plc_observer.h"
#include "queue_handler.h"				// jogn2ry plc
#include "FUNCTION.h"
#include "message_handler.h"	// jogn2ry plc
//
#define PLC_TYPE_1			1
//
//#define		KYODAE				1		// 교대변전소
//#define		DOKSAN				2		// 독산변전소
//#define		NOKBERN				3		// 녹번변전소
//#define		BONGCHUN			4		// 봉천변전소
//#define		DEUNGCHON			5		// 등촌변전소
//#define 		GWANGMYEONG			6		// GWANG_MYEONG Substation
#define 		SAMSUNG				7		// SAMSUNG Substation
//
unsigned char gRxD[PLC_BUFFER_SIZE];
unsigned char gTxD[PLC_BUFFER_SIZE];
//
unsigned short LinkId = -1;
//
//
unsigned char chPlcIp[2][20];
//
unsigned char BitTable[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
//
plcInfo plcData[1024];// jong2ry plc
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];

unsigned int g_iChkControl = 0;

/*********************************************************/
//Extern mutex
extern pthread_mutex_t plcQ_mutex;

/*********************************************************/
//Extern point_queue
extern point_queue plc_message_queue;		// jong2ry plc.


//
// 교대변전소 - start
//
#ifdef	 KYODAE
#define PLC_DI_ADDR_CNT   7   
#define PLC_DO_ADDR_CNT   8   
#define PLC_AI_ADDR_CNT   3  
#define PLC_AO_ADDR_CNT   4 
                                                               
static int PLC_DI_ADDR[PLC_DI_ADDR_CNT]  = { 128, 160, 192, 224, 256, 288,   0 };
static int PLC_DI_COUNT[PLC_DI_ADDR_CNT] = {  32,  32,  32,  32,  32,  32,  32 };
static int PLC_DI_POINT[PLC_DI_ADDR_CNT] = {   0,  32,  64,  96, 128, 160, 192 };

static int PLC_DO_ADDR[PLC_DO_ADDR_CNT]  = { 2800, 2832, 2864, 2896, 2400, 2432, 2464, 2496 };
static int PLC_DO_COUNT[PLC_DO_ADDR_CNT] = {   32,   32,   32,   32,   32,   32,   32,   32 };
static int PLC_DO_POINT[PLC_DO_ADDR_CNT] = {    0,   32,   64,   96,  128,  160,  192,  224 };

static int PLC_AI_ADDR[PLC_AI_ADDR_CNT]  = { 200, 232, 264 };
static int PLC_AI_COUNT[PLC_AI_ADDR_CNT] = {  32,  32,  32 };
static int PLC_AI_POINT[PLC_AI_ADDR_CNT] = {   0,  32,  64 };

static int PLC_AO_ADDR[PLC_AO_ADDR_CNT]  = { 400, 432, 464, 496 };
static int PLC_AO_COUNT[PLC_AO_ADDR_CNT] = {  32,  32,  32,  32 };
static int PLC_AO_POINT[PLC_AO_ADDR_CNT] = {   0,  32,  64,  96 };
#endif
//
// 교대변전소 - end
//


//
// 6. 독산변전소 - start
//
#ifdef	 DOKSAN
#define PLC_DI_ADDR_CNT   5   
#define PLC_DO_ADDR_CNT   8   
#define PLC_AI_ADDR_CNT   1  
#define PLC_AO_ADDR_CNT   1 
                                                               
static int PLC_DI_ADDR[PLC_DI_ADDR_CNT]  = {128, 192, 256, 320, 384};
static int PLC_DI_COUNT[PLC_DI_ADDR_CNT] = {32,  32,  32,  32,  32};
static int PLC_DI_POINT[PLC_DI_ADDR_CNT] = {0,   32,  64,  96,  128};

static int PLC_DO_ADDR[PLC_DO_ADDR_CNT]  = {3100, 3132, 3164, 2800, 2832, 2864, 960, 1088 };
static int PLC_DO_COUNT[PLC_DO_ADDR_CNT] = {32,   32,   32,   32,   32,   32,   32,  32 };
static int PLC_DO_POINT[PLC_DO_ADDR_CNT] = {0,    32,   64,   96,   128,  160,  192, 224 };

static int PLC_AI_ADDR[PLC_AI_ADDR_CNT]  = {300};
static int PLC_AI_COUNT[PLC_AI_ADDR_CNT] = {64};
static int PLC_AI_POINT[PLC_AI_ADDR_CNT] = {0};

static int PLC_AO_ADDR[PLC_AO_ADDR_CNT]  = {500};
static int PLC_AO_COUNT[PLC_AO_ADDR_CNT] = {96};
static int PLC_AO_POINT[PLC_AO_ADDR_CNT] = {0};
#endif
//
// 6. 독산변전소 - end
//


//
// 녹번변전소 - start
//
#ifdef	 NOKBERN
#define PLC_DI_ADDR_CNT   9   
#define PLC_DO_ADDR_CNT   13  
#define PLC_AI_ADDR_CNT   1  
#define PLC_AO_ADDR_CNT   1 
                                                               
static int PLC_DI_ADDR[PLC_DI_ADDR_CNT]  = { 128, 160, 192, 224, 256, 288, 320, 352,   0};
static int PLC_DI_COUNT[PLC_DI_ADDR_CNT] = {  32,  32,  32,  32,  32,  32,  32,  32,  32};
static int PLC_DI_POINT[PLC_DI_ADDR_CNT] = {   0,  32,  64,  96, 128, 160, 192, 224, 256};

static int PLC_DO_ADDR[PLC_DO_ADDR_CNT]  = {2800, 2832, 2864, 2896, 2928, 2960, 2400, 2432, 2464, 2496, 2528, 2560, 2592};

static int PLC_DO_COUNT[PLC_DO_ADDR_CNT] = {32,   32,   32,   32,   32,   32,   32,   32,   32,   32,   32,   32,   32  };

static int PLC_DO_POINT[PLC_DO_ADDR_CNT] = {0,    32,   64,   96,   128,  160,  192,  224,  256,  288,  320,  352,  390 };


static int PLC_AI_ADDR[PLC_AI_ADDR_CNT]  = {200};
static int PLC_AI_COUNT[PLC_AI_ADDR_CNT] = {64};
static int PLC_AI_POINT[PLC_AI_ADDR_CNT] = {0};

static int PLC_AO_ADDR[PLC_AO_ADDR_CNT]  = {400};
static int PLC_AO_COUNT[PLC_AO_ADDR_CNT] = {128};
static int PLC_AO_POINT[PLC_AO_ADDR_CNT] = {0};
#endif
//
// 녹번변전소 - end
//


//
// 봉천변전소 - start
//
#ifdef	 BONGCHUN
#define PLC_DI_ADDR_CNT 6
#define PLC_DO_ADDR_CNT 12      // ORG 20071210 --- 7
#define PLC_AI_ADDR_CNT 3
#define PLC_AO_ADDR_CNT 3

static int PLC_DI_ADDR[PLC_DI_ADDR_CNT] =  {160, 256, 304, 352, 480,  16};
static int PLC_DI_COUNT[PLC_DI_ADDR_CNT] = { 32,  32,  32,  32,  32,   8};
static int PLC_DI_POINT[PLC_DI_ADDR_CNT] = {  0,  32,  64,  96, 128, 160};

// ORG 20071210
//static int PLC_DO_ADDR[PLC_DO_ADDR_CNT] =  {16208, 16400, 16448, 640, 576, 208, 456};
//static int PLC_DO_COUNT[PLC_DO_ADDR_CNT] = {   32,    32,    32, 168,  48,  32,  24};
//static int PLC_DO_POINT[PLC_DO_ADDR_CNT] = {  	0,    32,    64,  96, 288, 336, 264}; 

static int PLC_DO_ADDR[PLC_DO_ADDR_CNT] =  {16208, 16400, 16448, 640, 672, 704, 736, 768, 800, 576, 208, 456 };
static int PLC_DO_COUNT[PLC_DO_ADDR_CNT] = {   32,    32,    32,  32,  32,  32,  32,  32,  32,  48,  32,  32 };
static int PLC_DO_POINT[PLC_DO_ADDR_CNT] = {  	0,    32,    64,  96, 128, 160, 192, 224, 256, 288, 336, 368 }; 


static int PLC_AI_ADDR[PLC_AI_ADDR_CNT] =  {70, 500, 735};
static int PLC_AI_COUNT[PLC_AI_ADDR_CNT] = {64,  40,   2};
static int PLC_AI_POINT[PLC_AI_ADDR_CNT] = { 0,  78, 118};

static int PLC_AO_ADDR[PLC_AO_ADDR_CNT] = {700, 739, 142};
static int PLC_AO_COUNT[PLC_AO_ADDR_CNT] = {35,  12,   8};
static int PLC_AO_POINT[PLC_AO_ADDR_CNT] = { 0,  37,  49};
#endif
//
// 봉천변전소 - end
//


//
// DEUNG_CHON - start
//
#ifdef	 DEUNGCHON
#define PLC_DI_ADDR_CNT 6
#define PLC_DO_ADDR_CNT 9      
#define PLC_AI_ADDR_CNT 1
#define PLC_AO_ADDR_CNT 1

static int PLC_DI_ADDR[PLC_DI_ADDR_CNT] =  { 64,  96, 128, 160, 192, 224};
static int PLC_DI_COUNT[PLC_DI_ADDR_CNT] = { 32,  32,  32,  32,  32,   8};
static int PLC_DI_POINT[PLC_DI_ADDR_CNT] = {  0,  32,  64,  96, 128, 160};

static int PLC_DO_ADDR[PLC_DO_ADDR_CNT] =  { 3040,  3072,  3104, 3136, 2640, 2672, 2704, 2736, 2768};
static int PLC_DO_COUNT[PLC_DO_ADDR_CNT] = {   32,    32,    32,   32,   32,   32,   32,   32,   32};
static int PLC_DO_POINT[PLC_DO_ADDR_CNT] = {  	0,    32,    64,   96,  128,  160,  192,  224,  256}; 

static int PLC_AI_ADDR[PLC_AI_ADDR_CNT] =  {600};
static int PLC_AI_COUNT[PLC_AI_ADDR_CNT] = {64};
static int PLC_AI_POINT[PLC_AI_ADDR_CNT] = { 0};

static int PLC_AO_ADDR[PLC_AO_ADDR_CNT] = {497};
static int PLC_AO_COUNT[PLC_AO_ADDR_CNT] = {96};
static int PLC_AO_POINT[PLC_AO_ADDR_CNT] = { 0};
#endif
//
// DEUNG_CHON - end
//


//
// GWANGMYEONG Substation - start
//
#ifdef	 GWANGMYEONG
#define PLC_DI_ADDR_CNT 5
#define PLC_DO_ADDR_CNT 5      
#define PLC_AI_ADDR_CNT 1
#define PLC_AO_ADDR_CNT 5

//static int PLC_DI_ADDR[PLC_DI_ADDR_CNT] =  {  0, 160, 224, 256, 288 };
static int PLC_DI_ADDR[PLC_DI_ADDR_CNT] =  { 160, 224, 256, 288,  0 };
static int PLC_DI_COUNT[PLC_DI_ADDR_CNT] = { 64,  32,  32,  64,  32 };
static int PLC_DI_POINT[PLC_DI_ADDR_CNT] = { 32,  96, 128, 160,  0 };

static int PLC_DO_ADDR[PLC_DO_ADDR_CNT] =  { 352,  384,  576, 2200, 2232 };
static int PLC_DO_COUNT[PLC_DO_ADDR_CNT] = {  32,   32,   32,   32,   32 };
static int PLC_DO_POINT[PLC_DO_ADDR_CNT] = {   0,   32,   64,   96,  128 }; 

static int PLC_AI_ADDR[PLC_AI_ADDR_CNT] =  { 300 };
static int PLC_AI_COUNT[PLC_AI_ADDR_CNT] = {  96 };
static int PLC_AI_POINT[PLC_AI_ADDR_CNT] = {   0 };

/*
static int PLC_AO_ADDR[PLC_AO_ADDR_CNT] =  { 500,  600,  700,  732,  764 };
//static int PLC_AO_ADDR[PLC_AO_ADDR_CNT] =  { 500,  600,  701,  733,  765 };
static int PLC_AO_COUNT[PLC_AO_ADDR_CNT] = {  32,   32,   32,   32,   32 };
static int PLC_AO_POINT[PLC_AO_ADDR_CNT] = {   0,   32,   64,   96,  128 };
*/
static int PLC_AO_ADDR[PLC_AO_ADDR_CNT] =  { 500,  600,  700,  732,  764 };
static int PLC_AO_COUNT[PLC_AO_ADDR_CNT] = {  32,   32,   32,   32,   32 };
static int PLC_AO_POINT[PLC_AO_ADDR_CNT] = {   0,   32,   64,   96,  128 };
#endif
//
// GWANGMYEONG Substation - end
//


//
// 15. 삼성변전소 - start
//
#ifdef SAMSUNG
#define PLC_DI_ADDR_CNT   5   
#define PLC_DO_ADDR_CNT   9   
#define PLC_AI_ADDR_CNT   2  
#define PLC_AO_ADDR_CNT   3 
                                                               
static int PLC_DI_ADDR[PLC_DI_ADDR_CNT]  = {  64,   96,  128,  160,     0};       /* %MX64 */
static int PLC_DI_COUNT[PLC_DI_ADDR_CNT] = {  32,   32,   32,   32,    64};
static int PLC_DI_POINT[PLC_DI_ADDR_CNT] = {   0,   32,   64,   96,   128};

static int PLC_DO_ADDR[PLC_DO_ADDR_CNT]  = {3040, 3072, 3104, 3136, 2640, 2672, 2704, 2736, 2864};     /* %MX3040 */
static int PLC_DO_COUNT[PLC_DO_ADDR_CNT] = {  32,   32,   32,   32,   32,   32,   32,   32,   16};
static int PLC_DO_POINT[PLC_DO_ADDR_CNT] = {   0,   32,   64,   96,  128,  160,  192,  224,  256};

static int PLC_AI_ADDR[PLC_AI_ADDR_CNT]  = { 300,  332};                /* %MX300 */
static int PLC_AI_COUNT[PLC_AI_ADDR_CNT] = {  32,   16};
static int PLC_AI_POINT[PLC_AI_ADDR_CNT] = {   0, 	32};

static int PLC_AO_ADDR[PLC_AO_ADDR_CNT]  = {  500,  532,  564};         /* %MW500 */
static int PLC_AO_COUNT[PLC_AO_ADDR_CNT] = {   32,   32,   31};
static int PLC_AO_POINT[PLC_AO_ADDR_CNT] = {    0,   32,   64};
#endif         
//
//
//


/****************************************************************/
void PLC_DIO_define(int DIO_sel, short pno, short sAddr, short OnAddr, short OffAddr, unsigned char EnUnsol)
/****************************************************************/
{
    DNP_DIO_Point *DNP_p;
    //
    if(DIO_sel == DNP_DI_SEL)
        DNP_p = &DNP_di_point[pno];
    else 
        DNP_p = &DNP_do_point[pno];
    //
    DNP_p->Unsol = 0;
    DNP_p->val = 0;
    DNP_p->PrevVal = 0;
    DNP_p->WrFlag = 0;  
    DNP_p->PLCaddr = sAddr;
    DNP_p->ONaddr = OnAddr;
    DNP_p->OFFaddr = OffAddr;
    DNP_p->EnUnsol = EnUnsol;
}

/****************************************************************/
void PLC_AIO_define(int AIO_sel, short pno, short sAddr, float sScale, short sOffset, int sMinVal, int sMaxVal)
/****************************************************************/
{
    DNP_AIO_Point *DNP_p = NULL;
    //
    if(AIO_sel == DNP_AI_SEL)
        DNP_p = &DNP_ai_point[pno];
    else if(AIO_sel == DNP_AO_SEL)
        DNP_p = &DNP_ao_point[pno];
    //
    DNP_p->Unsol = 0;
    DNP_p->val = 0;
    DNP_p->PrevVal = 0;
    DNP_p->WrFlag = 0;  
    DNP_p->PLCaddr = sAddr;
    DNP_p->scale = sScale;
    DNP_p->offset = sOffset;
    DNP_p->MinVal = sMinVal;
    DNP_p->MaxVal = sMaxVal;            
}

//
// 교대변전소
//
/****************************************************************/
void byun_jun_so_kyodae(void)
/****************************************************************/
{
    int i = 0;
    //
	// DI
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i,     i+128, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+32,  i+160, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+64,  i+192, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+96,  i+224, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+128, i+256, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+160, i+288, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+192, i, 0, 0, 1);
	//
    // DO
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i,     i+2800, i+400,  i+800,  0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+32,  i+2832, i+432,  i+832,  0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+64,  i+2864, i+464,  i+864,  0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+96,  i+2896, i+496,  i+896,  0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+128, i+2400, i+1600, i+2000, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+160, i+2432, i+1632, i+2032, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+192, i+2464, i+1664, i+2064, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+224, i+2496, i+1696, i+2096, 1);
	//
    // AI
    for (i = 0; i < 32; i++)
        PLC_AIO_define(DNP_AI_SEL, i,    i+200, 1, 0, 0, 16000);
    for (i = 0; i < 32; i++)
        PLC_AIO_define(DNP_AI_SEL, i+32, i+232, 1, 0, 0, 16000);
    for (i = 0; i < 32; i++)
        PLC_AIO_define(DNP_AI_SEL, i+64, i+264, 1, 0, 0, 16000);
	//
    PLC_AIO_define(DNP_AI_SEL, 48, 248, 1, 0, -300, 15700);
	//
    // AO
    for (i = 0; i < 32; i++)
        PLC_AIO_define(DNP_AO_SEL, i,    i+400, 1, 0, 0, 16000);
    for (i = 0; i < 32; i++)
        PLC_AIO_define(DNP_AO_SEL, i+32, i+432, 1, 0, 0, 16000);
    for (i = 0; i < 32; i++)
        PLC_AIO_define(DNP_AO_SEL, i+64, i+464, 1, 0, 0, 16000);
    for (i = 0; i < 32; i++)
        PLC_AIO_define(DNP_AO_SEL, i+96, i+496, 1, 0, 0, 16000);
}
//
//
//


//
// 독산변전소 
//
/****************************************************************/
void byun_jun_so_doksan()
/****************************************************************/
{
    int i = 0;
    
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i,     i+128, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+32,  i+192, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+64,  i+256, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+96,  i+320, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+128, i+384, 0, 0, 1);
    
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i,     i+3100, i+1600, i+1900, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+32,  i+3132, i+1632, i+1932, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+64,  i+3164, i+1664, i+1964, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+96,  i+2800, i+2200, i+2500, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+128, i+2832, i+2232, i+2532, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+160, i+2864, i+2264, i+2564, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+192, i+960,  i+1600, i+1900, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+224, i+1088, i+1664, i+1964, 0);
    
    for (i = 0; i < 64;i++)
        PLC_AIO_define(DNP_AI_SEL, i, i+300, 1, 0, 0, 16000);
    
    for (i = 0; i < 96; i++)
        PLC_AIO_define(DNP_AO_SEL, i, i+500, 1, 0, 0, 16000);
}
//
//


//
//
// 녹번변전소 
//
/****************************************************************/
void byun_jun_so_nokbern()
/****************************************************************/
{
    int i = 0;
    
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i,      i+128, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+32,   i+160, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+64,   i+192, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+96,   i+224, 0, 0, 1);
	for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+128,  i+256, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+160,  i+288, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+192,  i+320, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+224,  i+352, 0, 0, 1);
    
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+256,  i+0,   0, 0, 1);

	//출력변수2, ON, OFF
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i,      i+2800, i+400, i+800, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+32,   i+2832, i+432, i+832, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+64,   i+2864, i+464, i+864, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+96,   i+2896, i+496, i+896, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+128,  i+2928, i+528, i+928, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+160,  i+2960, i+560, i+960, 0);

	//모드,	 ON, OFF
	for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+192, i+2400, i+1600, i+2000, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+224, i+2432, i+1632, i+2032, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+256, i+2464, i+1664, i+2064, 1);
	for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+288, i+2496, i+1696, i+2096, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+320, i+2528, i+1728, i+2128, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+352, i+2560, i+1760, i+2160, 1);
	
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+390,  i+2592, i+1792, i+2192, 0); 

    for (i = 0; i < 64;i++)
        PLC_AIO_define(DNP_AI_SEL, i, i+200, 1, 0, 0, 16000);
        
	PLC_AIO_define(DNP_AI_SEL, 32 , 232, 1, 0, -300, 15700);

    for (i = 0; i < 128; i++)
        PLC_AIO_define(DNP_AO_SEL, i, i+400, 1, 0, 0, 16000);
}
//
//
//


//
// 5. 봉천변전소 - start
//
/****************************************************************/
void byun_jun_so_bongchun()
/****************************************************************/
{
    int i = 0;

    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DI_SEL, i,    i+160, 0, 0, 1);
    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DI_SEL, i+32, i+256, 0, 0, 1);
    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DI_SEL, i+64, i+304, 0, 0, 1);
    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DI_SEL, i+96, i+352, 0, 0, 1);
    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DI_SEL, i+128, i+480, 0, 0, 1);
    for(i=0;i<8 ;i++)
        PLC_DIO_define(DNP_DI_SEL, i+160, i+16 , 0, 0, 1);

  
    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DO_SEL, i,   i+16208, i*2+800, i*2+801, 1);
    PLC_DIO_define(DNP_DO_SEL,  0,   16208, 800, 801, 1);
    PLC_DIO_define(DNP_DO_SEL,  1,   16209, 802, 803, 1);
    PLC_DIO_define(DNP_DO_SEL,  3,   16211, 806, 807, 1);
    PLC_DIO_define(DNP_DO_SEL,  4,   16212, 808, 809, 1);
    PLC_DIO_define(DNP_DO_SEL,  6,   16214, 812, 813, 1);
    PLC_DIO_define(DNP_DO_SEL,  7,   16215, 814, 815, 1);
    PLC_DIO_define(DNP_DO_SEL, 11,   16219, 694, 695, 1);
    PLC_DIO_define(DNP_DO_SEL, 12,   16220, 697, 698, 1);
    PLC_DIO_define(DNP_DO_SEL, 13,   16221, 868, 869, 1);



    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DO_SEL, i+32,	 i+16400, i+832, i+833, 1);
    PLC_DIO_define(DNP_DO_SEL, 32,   16400, 832, 833, 1);
    PLC_DIO_define(DNP_DO_SEL, 34,   16402, 834, 835, 1);
    PLC_DIO_define(DNP_DO_SEL, 36,   16404, 836, 837, 1);
    PLC_DIO_define(DNP_DO_SEL, 38,   16406, 838, 839, 1);
    PLC_DIO_define(DNP_DO_SEL, 40,   16408, 840, 841, 1);
    PLC_DIO_define(DNP_DO_SEL, 42,   16410, 842, 843, 1);
    PLC_DIO_define(DNP_DO_SEL, 44,   16412, 844, 845, 1);
    PLC_DIO_define(DNP_DO_SEL, 46,   16414, 846, 847, 1);
    PLC_DIO_define(DNP_DO_SEL, 48,   16416, 848, 849, 1);
    PLC_DIO_define(DNP_DO_SEL, 50,   16418, 850, 851, 1);
    PLC_DIO_define(DNP_DO_SEL, 52,   16420, 852, 853, 1);
    PLC_DIO_define(DNP_DO_SEL, 54,   16422, 854, 855, 1);
    PLC_DIO_define(DNP_DO_SEL, 56,   16424, 856, 857, 1);
    PLC_DIO_define(DNP_DO_SEL, 58,   16426, 858, 859, 1);
    PLC_DIO_define(DNP_DO_SEL, 60,   16428, 860, 861, 1);
    PLC_DIO_define(DNP_DO_SEL, 62,   16430, 862, 863, 1);



    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DO_SEL, i+64,	 i+16448, i+864, i+865, 1);
    PLC_DIO_define(DNP_DO_SEL, 64,   16448, 864, 865, 1);
    PLC_DIO_define(DNP_DO_SEL, 66,   16450, 866, 867, 1);
    PLC_DIO_define(DNP_DO_SEL, 76,   16460, 878, 879, 1);
    PLC_DIO_define(DNP_DO_SEL, 78,   16462, 882, 883, 1);
    PLC_DIO_define(DNP_DO_SEL, 80,   16464, 886, 887, 1);
    PLC_DIO_define(DNP_DO_SEL, 82,   16466, 890, 891, 1);
    PLC_DIO_define(DNP_DO_SEL, 84,   16468, 894, 895, 1);
    PLC_DIO_define(DNP_DO_SEL, 86,   16470, 898, 899, 1);

    
    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DO_SEL, i+96,	 i+640, i+638, i+639, 1);
    PLC_DIO_define(DNP_DO_SEL, 98,     642, 640, 641, 1);
    PLC_DIO_define(DNP_DO_SEL,101,     645, 643, 644, 1);
    PLC_DIO_define(DNP_DO_SEL,104,     648, 646, 647, 1);
    PLC_DIO_define(DNP_DO_SEL,107,     651, 649, 650, 1);
    PLC_DIO_define(DNP_DO_SEL,110,     654, 652, 653, 1);
    PLC_DIO_define(DNP_DO_SEL,113,     657, 655, 656, 1);
    PLC_DIO_define(DNP_DO_SEL,116,     660, 658, 659, 1);
    PLC_DIO_define(DNP_DO_SEL,119,     663, 661, 662, 1);
    PLC_DIO_define(DNP_DO_SEL,122,     666, 664, 665, 1);
    PLC_DIO_define(DNP_DO_SEL,125,     669, 667, 668, 1);

    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DO_SEL, i+128,	 i+672, i+670, i+671, 1);
    PLC_DIO_define(DNP_DO_SEL,128,     672, 670, 671, 1);
    PLC_DIO_define(DNP_DO_SEL,131,     675, 673, 674, 1);
    PLC_DIO_define(DNP_DO_SEL,134,     678, 676, 677, 1);
    PLC_DIO_define(DNP_DO_SEL,146,     690, 688, 689, 1);
    PLC_DIO_define(DNP_DO_SEL,149,     693, 691, 692, 1);
    PLC_DIO_define(DNP_DO_SEL,152,     696, 694, 695, 1);
    PLC_DIO_define(DNP_DO_SEL,155,     699, 697, 698, 1);
    PLC_DIO_define(DNP_DO_SEL,158,     702, 700, 701, 1);


    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DO_SEL, i+160,	 i+704, i+702, i+703, 1);
    PLC_DIO_define(DNP_DO_SEL,161,     705, 703, 704, 1);
    PLC_DIO_define(DNP_DO_SEL,164,     708, 706, 707, 1);
    PLC_DIO_define(DNP_DO_SEL,167,     711, 709, 710, 1);
    PLC_DIO_define(DNP_DO_SEL,170,     714, 712, 713, 1);
    PLC_DIO_define(DNP_DO_SEL,173,     717, 715, 716, 1);
    PLC_DIO_define(DNP_DO_SEL,176,     720, 718, 719, 1);
    PLC_DIO_define(DNP_DO_SEL,179,     723, 721, 722, 1);
    PLC_DIO_define(DNP_DO_SEL,182,     726, 724, 725, 1);
    PLC_DIO_define(DNP_DO_SEL,185,     729, 727, 728, 1);
    PLC_DIO_define(DNP_DO_SEL,188,     732, 730, 731, 1);
    PLC_DIO_define(DNP_DO_SEL,191,     735, 733, 734, 1);


    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DO_SEL, i+192,	 i+736, i+734, i+735, 1);
    PLC_DIO_define(DNP_DO_SEL,194,     738, 736, 737, 1);
    PLC_DIO_define(DNP_DO_SEL,197,     741, 739, 740, 1);
    PLC_DIO_define(DNP_DO_SEL,200,     744, 742, 743, 1);
    PLC_DIO_define(DNP_DO_SEL,203,     747, 745, 746, 1);
    PLC_DIO_define(DNP_DO_SEL,206,     750, 748, 749, 1);
    PLC_DIO_define(DNP_DO_SEL,209,     753, 751, 752, 1);
    PLC_DIO_define(DNP_DO_SEL,215,     759, 757, 758, 1);
    PLC_DIO_define(DNP_DO_SEL,221,     765, 763, 764, 1);
    
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+224, 768, 766, 767, 1);    
    PLC_DIO_define(DNP_DO_SEL,227,     771, 769, 770, 1);
    PLC_DIO_define(DNP_DO_SEL,233,     777, 775, 776, 1);
    PLC_DIO_define(DNP_DO_SEL,239,     783, 781, 782, 1);
    PLC_DIO_define(DNP_DO_SEL,245,     789, 787, 788, 1);
    PLC_DIO_define(DNP_DO_SEL,251,     795, 793, 794, 1);
    PLC_DIO_define(DNP_DO_SEL,254,     798, 796, 797, 1);


    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DO_SEL, i+256,	i+800, i+798, i+799, 1);
    PLC_DIO_define(DNP_DO_SEL,257,     801, 799, 800, 1);
    

    for(i=0;i<48;i++)
        PLC_DIO_define(DNP_DO_SEL, i+288,	i+576, i+1224, i+1524, 1);

    for(i=0;i<32;i++)
        PLC_DIO_define(DNP_DO_SEL, i+336,	i+208, i+802, i+803, 1);
    PLC_DIO_define(DNP_DO_SEL,338,     210, 804, 805, 1);
    PLC_DIO_define(DNP_DO_SEL,341,     213, 810, 811, 1);
    PLC_DIO_define(DNP_DO_SEL,344,     216, 816, 817, 1);
    PLC_DIO_define(DNP_DO_SEL,345,     217, 818, 819, 1);
    PLC_DIO_define(DNP_DO_SEL,346,     218, 691, 692, 1);
    PLC_DIO_define(DNP_DO_SEL,359,     231, 902, 903, 1);
    PLC_DIO_define(DNP_DO_SEL,360,     232, 904, 905, 1);
    PLC_DIO_define(DNP_DO_SEL,361,     233, 906, 907, 1);

    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+368, 456, 872, 873, 1);
    PLC_DIO_define(DNP_DO_SEL,370,     458, 874, 875, 1);    
    PLC_DIO_define(DNP_DO_SEL,384,     472, 870, 871, 1);
    PLC_DIO_define(DNP_DO_SEL,385,     473, 872, 873, 1);
        
    
    for(i=0;i<64;i++)
        PLC_AIO_define(DNP_AI_SEL, i,  i+70, 1, 0, 0, 16000);
    for(i=0;i<40;i++)
        PLC_AIO_define(DNP_AI_SEL, i+78,  i+500, 1, 0, 0, 16000);
    for(i=0;i<2;i++)
        PLC_AIO_define(DNP_AI_SEL, i+118,  i+735, 1, 0, 0, 16000);
    
    PLC_AIO_define(DNP_AI_SEL, 8,  78, 1, 0, -300, 15700);
    
    for(i=0;i<49;i++)
        PLC_AIO_define(DNP_AO_SEL, i,  i+700, 1, 0, 0, 16000);
    for(i=0;i<12;i++)
        PLC_AIO_define(DNP_AO_SEL, i+37,  i+739, 1, 0, 0, 16000);
    for(i=0;i<8;i++)
        PLC_AIO_define(DNP_AO_SEL, i+49,  i+142, 1, 0, 0, 16000);
}
//
//
//


//
// DEUNG_CHON 
//
/****************************************************************/
void byun_jun_so_deungchon()
/****************************************************************/
{
    int i = 0;
    
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i,      i+64, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+32,   i+96, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+64,   i+128, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+96,   i+160, 0, 0, 1);
	for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+128,  i+192, 0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+160,  i+224, 0, 0, 1);

    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i,      i+3040, i+640, i+1040, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+32,   i+3072, i+672, i+1072, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+64,   i+3104, i+704, i+1104, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+96,   i+3136, i+736, i+1136, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+128,  i+2640, i+1840, i+2240, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+160,  i+2672, i+1872, i+2272, 1);
	for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+192, i+2704, i+1904, i+2304, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+224, i+2736, i+1936, i+2336, 1);

    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+256, i+2768, i+1968, i+2368, 1);        


    for (i = 0; i < 64;i++)
        PLC_AIO_define(DNP_AI_SEL, i, i+300, 1, 0, 0, 16000);
	PLC_AIO_define(DNP_AI_SEL, 8 , 308, 1, 0, -300, 15700);

    for (i = 0; i < 96; i++)
        PLC_AIO_define(DNP_AO_SEL, i, i+497, 1, 0, 0, 16000);
 
}
//
//
//


//
//	GWANGMYEONG Substation
//
/****************************************************************/
void Substation_GWANG_MYEONG()
/****************************************************************/
{
    int i = 0;

    for (i = 0; i < 64; i++)
        PLC_DIO_define(DNP_DI_SEL, i+32,	i+160, 	0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+96,	i+224, 	0, 0, 1);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+128,	i+256, 	0, 0, 1);
	for (i = 0; i < 64; i++)
        PLC_DIO_define(DNP_DI_SEL, i+160,	i+288, 	0, 0, 1);
    
    // 가상주소
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i,		i, 		0, 0, 1);
 	
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i,     	i+350, 	i+1000, i+1300, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+32,  	i+384,	i+1032, i+1332, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+64,   	i+576,	i+1224, i+1524, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+96,   	i+2200,	i+1600, i+1900, 0);
    for (i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+128,  	i+2232,	i+1632, i+1932, 0);
	
    for (i = 0; i < 96;i++)
        PLC_AIO_define(DNP_AI_SEL, i, i+300, 0.1, 0, 0, 16000);
#if 1
    for (i = 0; i < 32; i++)
        PLC_AIO_define(DNP_AO_SEL, i,     i+500, 1, 0, 0, 16000);
    for (i = 0; i < 32; i++)
        PLC_AIO_define(DNP_AO_SEL, i+32,  i+600, 0.001, 0, -500, 16000);
    for (i = 0; i < 32; i++)
        PLC_AIO_define(DNP_AO_SEL, i+64,  i+700, 0.1, 0, -500, 16000);
//    for (i = 0; i < 32; i++)
//        PLC_AIO_define(DNP_AO_SEL, i+96,  i+732, 0.1, 0, -500, 16000);
//    for (i = 0; i < 32; i++)
//    	PLC_AIO_define(DNP_AO_SEL, i+128, i+764, 0.1, 0, -500, 16000);                 
    for (i = 0; i < 23; i++)
        PLC_AIO_define(DNP_AO_SEL, i+96,  i+732, 0.1, 0, -500, 16000);
    for (i = 0; i < 41; i++)
    	PLC_AIO_define(DNP_AO_SEL, i+119, i+755, 1, 0, -500, 16000);                 	
#else
    for (i = 0; i < 32; i++)
        PLC_AIO_define(DNP_AO_SEL, i,     i+500, 1, 0, 0, 16000);
    for (i = 0; i < 32; i++)
        PLC_AIO_define(DNP_AO_SEL, i+32,  i+600, 0.001, 0, -500, 16000);
    for (i = 0; i < 32; i++)
        PLC_AIO_define(DNP_AO_SEL, i+64,  i+701, 0.1, 0, -500, 16000);
//    for (i = 0; i < 32; i++)
//        PLC_AIO_define(DNP_AO_SEL, i+96,  i+732, 0.1, 0, -500, 16000);
//    for (i = 0; i < 32; i++)
//    	PLC_AIO_define(DNP_AO_SEL, i+128, i+764, 0.1, 0, -500, 16000);                 
    for (i = 0; i < 23; i++)
        PLC_AIO_define(DNP_AO_SEL, i+96,  i+733, 0.1, 0, -500, 16000);
    for (i = 0; i < 41; i++)
    	PLC_AIO_define(DNP_AO_SEL, i+119, i+756, 1, 0, -500, 16000);                 	
#endif    	
}
//
//
//


//
//	GWANGMYEONG Substation
//
/****************************************************************/
void Substation_SAMSUNG()
{
    int i = 0;
    
    for(i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i, i+64, 0, 0, 1);
    for(i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+32, i+96, 0, 0, 1);
    for(i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+64, i+128, 0, 0, 1);
    for(i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DI_SEL, i+96, i+160, 0, 0, 1);                        

    for(i = 0; i < 64; i++)
        PLC_DIO_define(DNP_DI_SEL, i+128, i+0, 0, 0, 1);                        
    
    for(i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i, i+3040, i+640, i+1040, 0);
    for(i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+32, i+3072, i+672, i+1072, 0);
    for(i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+64, i+3104, i+704, i+1104, 0);
    for(i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+96, i+3136, i+736, i+1136, 0);
    for(i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+128, i+2640, i+1840, i+2240, 1);
    for(i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+160, i+2672, i+1872, i+2272, 1);
    for(i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+192, i+2704, i+1904, i+2304, 1);
    for(i = 0; i < 32; i++)
        PLC_DIO_define(DNP_DO_SEL, i+224, i+2736, i+1936, i+2336, 1);        

    for(i = 0; i < 16; i++)
        PLC_DIO_define(DNP_DO_SEL, i+256, i+2864, i+2064, i+2464, 1);        
   
    for(i = 0; i < 32;i++)
        PLC_AIO_define(DNP_AI_SEL, i, i+300, 1, 0, 0, 16000);
    for(i = 0; i < 16;i++)
        PLC_AIO_define(DNP_AI_SEL, i+32, i+332, 1, 0, 0, 16000);        
    
    for(i = 0; i < 32;i++)
        PLC_AIO_define(DNP_AO_SEL, i, i+500, 1, 0, 0, 16000);
    for(i = 0; i < 32;i++)
        PLC_AIO_define(DNP_AO_SEL, i+32, i+532, 1, 0, 0, 16000);
    for(i = 0; i < 31;i++)
        PLC_AIO_define(DNP_AO_SEL, i+64, i+564, 1, 0, 0, 16000);                
}
//
//
//


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/****************************************************************/
void PLC_point_define(void)
/****************************************************************/
{
    int i = 0;
    
    for (i = 0; i < MAX_DNP_DI_POINT; i++)
        PLC_DIO_define(DNP_DI_SEL, i, 0, 0, 0, 0);
    
    for (i = 0; i < MAX_DNP_DO_POINT; i++)
        PLC_DIO_define(DNP_DO_SEL, i, 0, 0, 0, 0);
    
    for (i = 0; i < MAX_DNP_AI_POINT; i++)
        PLC_AIO_define(DNP_AI_SEL, i, 0, 0, 0, 0, 0);
    
    for (i = 0; i < MAX_DNP_AO_POINT; i++)
        PLC_AIO_define(DNP_AO_SEL, i, 0, 0, 0, 0, 0);

    // 교대변전소
    //printf("byun_jun_so_kyodae \n");
    //byun_jun_so_kyodae();
    //

    // 독산변전소 
    //printf("byun_jun_so_doksan \n");
    //byun_jun_so_doksan();
    //

    // 녹번변전소 
    //printf("byun_jun_so_nokbern \n");
    //byun_jun_so_nokbern();
    //

    // 녹번변전소 
    //printf("byun_jun_so_bongchun \n");
	//byun_jun_so_bongchun();
	
	//DEUNG_CHON
	//printf("byun_jun_so_deungchon \n");
	//byun_jun_so_deungchon();

	//GWANGMYEONG Substation
	//printf("GWANG_MYEONG Substation 2010.03.16 \n");
	//Substation_GWANG_MYEONG();

	printf("SAMSUNG Substation 2010.09.15 \n");
	Substation_SAMSUNG();
}
//
//
//
/***************************i*************************************/
void setValue3iS(int type, long point, float val)
/***************************i*************************************/
{
	int i = 0;
	
	for (i = 0; i < 1024; i++)
	{
		if (plcData[i].addr == point && plcData[i].type == type)
		{
			if (g_fExPtbl[plcData[i].pcm][plcData[i].pno] == val)
				return;
			//printf("pSet(%d, %d, %f)\n", plcData[i].pcm, plcData[i].pno, val);
			pSet(plcData[i].pcm, plcData[i].pno, val);
			return;
		}
	}	
	return;
}

//
//
//
/***************************i*************************************/
void MakeHeader(char *pData, char command, char datatype, char blocknumber)
/****************************************************************/
{
	memcpy(pData, "LGIS-GLOFA", 10);
	pData[LOC_SOURCEOFFRAME] = 0x33;
	LinkId = (LinkId+1) % MAX_LINKID;
	pData[LOC_LINKID] = LinkId & 0xFF;
	pData[LOC_LINKID+1] = (LinkId & 0xFF00) >> 8;
    //
    pData[LOC_COMMAND] = command;
    pData[LOC_DATATYPE] = datatype;
    pData[LOC_BLOCKNUMBER] = blocknumber;
}


/****************************************************************/
int ReadDIOBlock(int plc_sockfd, long addr, long count, long point, int DIO_sel)
/****************************************************************/
{
	char		chSend[4096];
	char		chRecv[4096];
	//int			nErrorCode;
	int			nSendByte, nSentByte, nLength;
	int			nRecvByte;
	int			i, j, NoOverFlag, NoOverFlagNo;
	char		chAddr[10];
	int         bRet=1;
	//int			nCount;
	long        lAddr, lCount, lPoint;
	int 		nWhileCnt = 0;
	//
	DNP_DIO_Point *DNP_p;
	//
	memset(chSend, 0, sizeof(chSend));
	MakeHeader(chSend, 0x54, 0x14, 0x01);
	//
	lAddr = addr;
	lCount = count;
	lPoint = point;
	//
	//printf("DEBUG: ReadDIOBlock addr %d, count %d, point %d \n", addr, count, point);
	//printf("DEBUG: ReadDIOBlock laddr %d, lcount %d, lpoint %d \n", lAddr, lCount, lPoint);
	//
	nSendByte = HEADER_SIZE; //header size
	memset(chAddr, 0, sizeof(chAddr));
	chAddr[0] = '%';
	sprintf(&chAddr[1], "MB%d", (int) lAddr/8);
	//
	chSend[LOC_VARLENG] = strlen(chAddr) & 0xFF;
	chSend[LOC_VARLENG+1] = (strlen(chAddr) & 0xFF00) >> 8;
	//
	nSendByte += 10;  // command ~ variable_length 10bytes
	//
	memcpy(&chSend[nSendByte], chAddr, strlen(chAddr));
	nSendByte += strlen(chAddr);
	//
	// DataSize
	chSend[nSendByte++] = (lCount/8) & 0xFF;
	chSend[nSendByte++] = ((lCount/8) & 0xFF00) >> 8;
	//
	// DataLength
	chSend[LOC_LENGTH] = (nSendByte - HEADER_SIZE) & 0xff;
	chSend[LOC_LENGTH+1] = ((nSendByte - HEADER_SIZE) & 0xff00) >> 8;
	//
	// CheckSum
	int checksum = 0;
	for(i=0; i<LOC_CHECKSUM; i++)
	  checksum = (checksum + chSend[i]) & 0xff;
	//
	chSend[LOC_CHECKSUM] = (unsigned char)checksum;
	//
	// Add break timer for avoiding endless loop
	while(1)
	{
		if (debug_plc_tx)
		{
			printf("DEBUG(%d): ReadDIOBlock send %d bytes!!! \n", 
				nWhileCnt++, nSendByte);
			//
			for(i = 0; i < nSendByte; i++)
			{
				printf("%02X ", chSend[i]);
				if (!((i+1)%16))
					printf("\n");
			}
			printf("\n");
		}
		//
		nSentByte = send(plc_sockfd, chSend, nSendByte, 0);
		//
		// If you want lab test, check usleep timing.
		usleep(300000);
		//
		////////////////////////////////////////////////
		for (i = 0; i < 1024; i++)
			chRecv[i] = 0x00;
		///////////////////////////////////////////////
		nRecvByte = recv(plc_sockfd, chRecv, 1024, 0);
		if(nRecvByte == 0)
		{
			bRet = 0;
			break;
		}
		else if(nRecvByte < 0)
		{
			printf("nRecvByte : %d\n",nRecvByte);
			bRet = 0;
			break;
		}
		else
		{
			if (debug_plc_tx)
			{
				printf("DEBUG: ReadDIOBlock recv %d bytes!!! \n", nRecvByte);
				for(i = 0; i < nRecvByte; i++)
				{
					printf("%02X ", chRecv[i]);
					if (!((i+1)%16))
						printf("\n");
				}
				printf("\n");
			}
			bRet = 1;
			break;
		}
	}
	//
	nLength = 32 + (lCount/8);
	//
	///////////////////////////////////////////////////////////////////////////////        
	NoOverFlag = 0;
	NoOverFlagNo = 0;
	///////////////////////////////////////////////////////////////////////////////        
	//
	///
	lPoint = point;
	///
#ifdef PLC_TYPE_1	
	//  ORG - modified for DAECHI
	// Chunho is here !!!
	// bongchun
	//    // 순화변전소
	//    // 아현변전소 - 이 소스를 사용하면 안됨
	//    // 마포변전소
	// Doksan
	if((nRecvByte == nLength) && (chRecv[26] == 0x00) && (chRecv[27] == 0x00)
	        && memcmp(&chSend[LOC_LINKID], &chRecv[LOC_LINKID], 2) == 0)
#else
	// 서소문변전소
	// DAECHI is here !!!
	// 교대변전소
	if((nRecvByte == (nLength+1)) && (chRecv[26] == 0x00) && (chRecv[27] == 0x00)
		&& memcmp(&chSend[LOC_LINKID], &chRecv[LOC_LINKID], 2) == 0)
#endif		
	{
		for(i=0;i<(lCount/8);i++) // byte 갯수만큼
		{
			for(j=0;j<8;j++) // 1바이트를 => 비트로 분리
			{
				///////////////////////////////////////////////////////////////////////////////        
				if( NoOverFlag )
					j += NoOverFlagNo;
				///////////////////////////////////////////////////////////////////////////////        
				//		  
				if(DIO_sel == DNP_DI_SEL)
					DNP_p = &DNP_di_point[lPoint];
				else
					DNP_p = &DNP_do_point[lPoint];
				//
				if((chRecv[i+32] & BitTable[j])>0)
				{
					DNP_p->val = 1;
					// printk(" [%d] ",lPoint);
				}
				else
					DNP_p->val = 0;
				//
				if( DNP_p->val != DNP_p->PrevVal )
				{
				  //printf("DEBUG: ReadDIOBlock chRecv[i+32] 0x%x, point %d, lPoint %d, val %d, PreVal %d \n", chRecv[i+32], point, lPoint, DNP_p->val, DNP_p->PrevVal);
					//jong2ry dbg
					/*
				  printf("ReadDIOBlock[%d] point %d, lPoint %d, val %d, PreVal %d \n", 
						DIO_sel,
				  	point, 
				  	lPoint, 
				  	DNP_p->val, 
				  	DNP_p->PrevVal);
				  	*/
				  DNP_p->Unsol = 1;
				}
				//  
				DNP_p->PrevVal = DNP_p->val;
				setValue3iS(DIO_sel, lPoint, (float)DNP_p->val);
				lPoint++;
				//
#if 0        
				///////////////////////////
				if (DIO_sel == DNP_DO_SEL)
				  printf("[%d-%d] ", j, lPoint);
				///////////////////////////
#endif
				//
				if(DIO_sel == DNP_DI_SEL)
				{
					if (DNP_di_point[lPoint].PLCaddr > DNP_di_point[lPoint - 1].PLCaddr)
						j += (DNP_di_point[lPoint].PLCaddr - DNP_di_point[lPoint - 1].PLCaddr);
					else
						break;
				}
				else
				{
					if (DNP_do_point[lPoint].PLCaddr > DNP_do_point[lPoint - 1].PLCaddr)
						j += (DNP_do_point[lPoint].PLCaddr - DNP_do_point[lPoint - 1].PLCaddr);
					else
						break;
				}
				//
				if( j > 8 )
				{
					NoOverFlag = 1;
					NoOverFlagNo = j - 8;
				}
				else
					NoOverFlag = 0;
				//
				j--;
			}
		}
	}
	else if(nRecvByte < 0)
	{
		printf("nRecvBytes : %d\n",nRecvByte);
		bRet=0;
	}
	else
	{
		printf("DEBUG: Read DIO Error !!! nRecvBytes : %d\n",nRecvByte);
		for(i=0;i<nRecvByte;i++)
			printf("[%02X]", chRecv[i]);
		printf("\n");

		printf("DIO>>> nRecvByte %d, nLength %d, chRecv[26] 0x%x 0x%x, chSend 0x%x 0x%x, chRecv 0x%x 0x%x \n",
				nRecvByte, nLength, 
				chRecv[26], chRecv[27], 
				chSend[LOC_LINKID], chSend[LOC_LINKID+1], 
				chRecv[LOC_LINKID], chRecv[LOC_LINKID+1]);

		bRet = 0;
	}
	//
	return bRet;
}

/****************************************************************/
int ReadAIOBlock(int plc_socket, long addr, long count, long point, int AIO_sel)
/****************************************************************/
{
	char		chSend[1000];
	char		chRecv[2000];
	//int			nErrorCode;
	int			nSendByte, nSentByte, nLength;
	int			nRecvByte;
	int			i;
	char		chAddr[10];
	int         bRet = 1;
	//int			nCount;
	int 		nWhileCnt = 0;
	//
    long lAddr, lCount, lPoint;
    //
    short sVal;
    float fVal;
    //
    DNP_AIO_Point *DNP_p;
    //
    //printf("PLCd: %s() plc_socket %d, addr %d, count %d, point %d, AIO_sel %d \n", __FUNCTION__, plc_socket, addr, count, point, AIO_sel);
    memset(chSend, 0, sizeof(chSend));
    //
    MakeHeader(chSend, 0x54, 0x14, 0x01);
    //
    lAddr = addr;
    lCount = count;
    lPoint = point;
    //
    nSendByte = HEADER_SIZE; //header size
    memset(chAddr, 0, sizeof(chAddr));
    chAddr[0] = '%';
    sprintf(&chAddr[1], "MB%d", (int)lAddr*2);
	//
	chSend[LOC_VARLENG] = strlen(chAddr) & 0xFF;
	chSend[LOC_VARLENG+1] = (strlen(chAddr) & 0xFF00) >> 8;
	//
	nSendByte += 10;  // command ~ variable_length 10bytes
	//
	memcpy(&chSend[nSendByte], chAddr, strlen(chAddr));
	nSendByte += strlen(chAddr);
	//
    //DataSize
	chSend[nSendByte++] = (lCount*2) & 0xFF;
	chSend[nSendByte++] = ((lCount*2) & 0xFF00) >> 8;
	//
    //DataLength
	chSend[LOC_LENGTH] = (nSendByte - HEADER_SIZE) & 0xff;
	chSend[LOC_LENGTH+1] = ((nSendByte - HEADER_SIZE) & 0xff00) >> 8;
	//
    //CheckSum
	int checksum = 0;
    for(i = 0; i < LOC_CHECKSUM; i++)
        checksum = (checksum + chSend[i]) & 0xff;
    //
    chSend[LOC_CHECKSUM] = (unsigned char)checksum;
    //
    // printk("DEBUG: send %d bytes!!!\n",nSentByte);
    //
    // Add break timer for avoiding endless loop
	nWhileCnt = 0;
    while(1)
    {
     	if (debug_plc_tx)
		{
			printf("DEBUG(%d): ReadAIOBlock send %d bytes!!! \n", 
				nWhileCnt++, nSendByte);
			
			for(i = 0; i < nSendByte; i++)
			{
				printf("%02X ", chSend[i]);
				if (!((i+1)%16))
					printf("\n");
			}
			printf("\n");
		}
		//
		//
		nSentByte = send(plc_socket, chSend, nSendByte, 0);
        ////////////////////////////////////////////////
		for (i = 0; i < 1024; i++)
			chRecv[i] = 0x00;
		///////////////////////////////////////////////
		//
		// If you want lab test, check usleep timing.
		usleep(300000);
		//
        nRecvByte = recv(plc_socket, chRecv, 500, 0); // CHECK_ME 255 is small, but 500 is not correct
        //
        if(nRecvByte == 0)
        {
            bRet = 0;
            break;
        }
        else if(nRecvByte < 0)
        {
            printf("nRecvByte : %d\n",nRecvByte);
            bRet = 0;
            break;
        }
        else
        {
            if (debug_plc_rx)
            {
                printf("DEBUG: ReadAIOBlock recv %d bytes!!! \n",nRecvByte);
                for(i = 0; i < nRecvByte; i++)
                {
                    printf("%02X ", chRecv[i]);
                    if (!((i+1)%16))
                        printf("\n");
                }
                printf("\n");
            }
            bRet = 1;
            break;
        }
    }
	//
    nLength = 32 + (lCount*2);
    //printf("DEBUG: ReadAIOBlock lCount %d, nLength %d \n", lCount, nLength);
	//
#ifdef PLC_TYPE_1
	//  ORG - modified for DAECHI
    // Chunho is here !!!
    // DOKSAN is here !!!
    // bongchun
	//    // 순화변전소
	// Doksan
	if((nRecvByte == nLength) && (chRecv[26] == 0x00) && (chRecv[27] == 0x00)
	    && memcmp(&chSend[LOC_LINKID], &chRecv[LOC_LINKID], 2) == 0)
#else
	// DAECHI is here !!!
    // 교대변전소 
    if ((nRecvByte == (nLength+1)) && (chRecv[26] == 0x00) && (chRecv[27] == 0x00)
        && memcmp(&chSend[LOC_LINKID], &chRecv[LOC_LINKID], 2) == 0)
#endif
	{
        for (i=0;i<(lCount*2);i=i+2)
        {
			if(AIO_sel == DNP_AI_SEL)
          		DNP_p = &DNP_ai_point[lPoint];
          	else
          		DNP_p = &DNP_ao_point[lPoint];
			//
          	memcpy(&sVal, &chRecv[i+32], 2);
          
          	fVal = sVal;
          	fVal = fVal * DNP_p->scale;
          	if(fVal > DNP_p->MaxVal)
          		fVal = DNP_p->MaxVal;
          	else if(fVal < DNP_p->MinVal)
          		fVal = DNP_p->MinVal;

			//jong2ry dbg
			/*
			  printf("ReadAIOBlock[%d] point %d, lPoint %d, fVal %f\n", 
			  	AIO_sel,
			  	point, 
			  	lPoint, 
			  	fVal);
			*/
			//
			// 090812 jong2ry add. Make AO Unsol.
			//if (DNP_p->val != DNP_p->PreVal)
			//{
			//	DNP_p->Unsol = 1;
			//}
			//
          	DNP_p->val = fVal;
          	lPoint++;
          	
          	setValue3iS(AIO_sel, lPoint, fVal);
        }
    }
    else if(nRecvByte < 0)
    {
        printf("nRecvBytes : %d\n",nRecvByte);
        bRet = 0;
    }
    else
    {
        printf("DEBUG: Read AIO Block Error !!! - nRecvBytes : %d\n", nRecvByte);
        for(i = 0; i < nRecvByte; i++)
            printf("[%02X]", chRecv[i]);
        printf("\n");

        printf("AIO>>> nRecvByte %d, nLength %d, chRecv[26] 0x%x 0x%x, chSend 0x%x 0x%x, chRecv 0x%x 0x%x \n",
                      nRecvByte, nLength, 
					chRecv[26], chRecv[27], 
					chSend[LOC_LINKID], chSend[LOC_LINKID+1], 
					chRecv[LOC_LINKID], chRecv[LOC_LINKID+1]);
		//
        bRet = 0;
    }
	//
    return bRet;
}

/****************************************************************/
int WriteDOPoint(int plc_socket, short dnp_pno, unsigned char point)
/****************************************************************/
{
    char chSend[2048];
    char chRecv[2048];
    //int nErrorCode;
    int nSendByte, nSentByte, nLength;
    int nRecvByte;
    int i;
    char chAddr[10];
    int bRet=1;
    long lAddr, lPoint;
    short pno;
    int val_ptr;
    DNP_DIO_Point *DNP_p;
	//
    printf("\n");
    printf("DEBUG: WriteDOPoint plc_socket %d, dnp_pno %d, point %d \n", plc_socket, dnp_pno, point);
    memset(chSend, 0, sizeof(chSend));
    MakeHeader(chSend, 0x58, 0x00, 0x01);
	//
    pno = dnp_pno;
    if(point > 0)
        lPoint = 1;
    else
        lPoint = 0;
	//
    DNP_p = &DNP_do_point[pno];
    //
    if(lPoint == 1)
        lAddr = DNP_p->ONaddr;
    else
        lAddr = DNP_p->OFFaddr;
	//
    nSendByte = HEADER_SIZE;              // header size 20
    memset(chAddr, 0, sizeof(chAddr));
    chAddr[0] = '%';
    sprintf(&chAddr[1], "MX%d", (int)lAddr);
	//
    printf("DEBUG: WriteDOPoint dnp_pno %d, lAddr %d \n", dnp_pno, (int)lAddr);
	//
    chSend[LOC_VARLENG] = strlen(chAddr) & 0xFF;
    chSend[LOC_VARLENG+1] = (strlen(chAddr) & 0xFF00) >> 8;
	//
    nSendByte += 10;    // command ~ variable_length 10bytes
	//
    memcpy(&chSend[nSendByte], chAddr, strlen(chAddr));
    nSendByte += strlen(chAddr);
	//
    // DataSize
    chSend[nSendByte++] = 1;
    chSend[nSendByte++] = 0;
	//
	val_ptr = nSendByte;
    chSend[nSendByte++] = 1;    // (byte)lPoint;
	//    
    // DataLength
    chSend[LOC_LENGTH] = (nSendByte - HEADER_SIZE) & 0xff;
    chSend[LOC_LENGTH+1] = ((nSendByte - HEADER_SIZE) & 0xff00) >> 8;
	//    
    // CheckSum
    int checksum = 0;
    for (i = 0; i < LOC_CHECKSUM; i++)
        checksum = (checksum + chSend[i]) & 0xff;
	//    
    chSend[LOC_CHECKSUM] = (unsigned char)checksum;
	//
    // for (i = 0; i < nSendByte; i++)
    //     printk("[%X]", chSend[i]);
    // printk("\n");
	//
    // printk("DEBUG: send %d bytes!!!\n", nSentByte);
    //
    // Add break timer for avoiding endless loop
    while(1)
    {
        nSentByte = send(plc_socket, chSend, nSendByte, 0);
		//
		usleep(900000);
		//
        //nRecvByte = recv(plc_socket, chRecv, 255, 0);
        nRecvByte = recv(plc_socket, chRecv, 1024, 0);
		//
        if (nRecvByte == 0)
        {
            bRet = 0;
            break;
        }
        else if (nRecvByte < 0)
        {
            printf("nRecvByte : %d\n", nRecvByte);
            bRet = 0;
            break;
        }
        else
        {
            // printk("DEBUG: recv %d bytes!!!\n",nRecvByte);
            // for(i=0;i<nRecvByte;i++)
            //  printk(" [%X] ",chRecv[i]);
            // printk("\n");
            bRet = 1;
            break;
        }
    }
	//
    nLength = 30;
	//
#ifdef PLC_TYPE_1	
    //  ORG - modified for DAECHI
	// Doksan
      if((nRecvByte == nLength) && (chRecv[26] == 0x00) && (chRecv[27] == 0x00)
        && memcmp(&chSend[LOC_LINKID], &chRecv[LOC_LINKID], 2) == 0)
#else		
    if((nRecvByte == (nLength+1)) && (chRecv[26] == 0x00) && (chRecv[27] == 0x00)
        && memcmp(&chSend[LOC_LINKID], &chRecv[LOC_LINKID], 2) == 0)
#endif		
    {
		bRet = 1;
        //printf("DEBUG : Write DO Point(1) = nRecvByte %d, nLength %d \n", nRecvByte, nLength);
    }
    else
    {
        // debug code
        printf("DEBUG: Write DO Point: nRecvByte %d, nLength %d \n", nRecvByte, nLength);
        // debug code
        //
        bRet = 0;
        return bRet;
    }
	//
    // write same point 0
    chSend[val_ptr] = 0;
    //
    // CheckSum
    checksum = 0;
    for (i = 0; i < LOC_CHECKSUM; i++)
        checksum = (checksum + chSend[i]) & 0xff;
    //
    chSend[LOC_CHECKSUM] = (unsigned char)checksum;
    //
    // Add break timer for avoiding endless loop
    while(1)
    {
        nSentByte = send(plc_socket, chSend, nSendByte, 0);
		usleep(900000);
		//  
        nRecvByte = recv(plc_socket, chRecv, 255, 0);
        //
        if (nRecvByte == 0)
        {
            bRet = 0;
            break;
        }
        else if (nRecvByte < 0)
        {
            printf("nRecvByte : %d\n", nRecvByte);
            bRet = 0;
            break;
        }
        else
        {
            bRet = 1;
            break;
        }
    }
    //
    nLength = 30;
    //
    // debug code
    // printf("DO Write>>> nRecvByte %d, nLength %d \n", nRecvByte, nLength);
    // debug code
	//
#ifdef PLC_TYPE_1	
	//  ORG - modified for DAECHI  
	// Doksan
    if((nRecvByte == nLength) && (chRecv[26] == 0x00) && (chRecv[27] == 0x00)
        && memcmp(&chSend[LOC_LINKID], &chRecv[LOC_LINKID], 2) == 0)
#else
	if((nRecvByte == (nLength+1)) && (chRecv[26] == 0x00) && (chRecv[27] == 0x00)
        && memcmp(&chSend[LOC_LINKID], &chRecv[LOC_LINKID], 2) == 0)
#endif		
    {
        bRet = 1;
    }
    else
    {
        // debug code
        printf("DEBUG: Write DO Point(2) : nRecvByte %d, nLength %d \n", nRecvByte, nLength);
        // debug code
		//  
        bRet = 0;
    }
    //
    return bRet;
}

/****************************************************************/
int WriteAOPoint(int plc_socket, short dnp_pno, float point)
/****************************************************************/
{
    char chSend[2048];
    char chRecv[2048];
    //int nErrorCode;
    int nSendByte, nSentByte, nLength;
    int nRecvByte;
    int i;
    char chAddr[10];
    int bRet = 1;
    long lAddr;
    short lPoint;
    short pno;
    float fVal;
    DNP_AIO_Point *DNP_p;
	//
	memset(chSend, 0, sizeof(chSend));
    MakeHeader(chSend, 0x58, 0x02, 0x01);
    //
    pno = dnp_pno;
    // lPoint = point;
	//
    DNP_p = &DNP_ao_point[pno];
	//
    //lAddr = DNP_p->PLCaddr;  //org
    lAddr = DNP_p->PLCaddr - 1;			//??????????????????????????????????????????????????????????
    fVal = point;
    fVal = fVal / DNP_p->scale;	
    lPoint = fVal;
    //
    printf("AO Write>>> lAddr %d, point %f, lPoint %d \n", (int)lAddr, point, (int)lPoint);
	//
    nSendByte = HEADER_SIZE;            // header size
    memset(chAddr, 0, sizeof(chAddr));
    chAddr[0] = '%';
    sprintf(&chAddr[1], "MW%d", (int)lAddr);
    //
    chSend[LOC_VARLENG] = strlen(chAddr) & 0xFF;
    chSend[LOC_VARLENG+1] = (strlen(chAddr) & 0xFF00) >> 8;
    //
    nSendByte += 10;        // command ~ variable_length 10bytes
	//
	memcpy(&chSend[nSendByte], chAddr, strlen(chAddr));
	nSendByte += strlen(chAddr);
	//
    // DataSize
    chSend[nSendByte++] = 2;
    chSend[nSendByte++] = 0;
	//
    chSend[nSendByte++] = lPoint & 0xff;
    chSend[nSendByte++] = (lPoint & 0xff00) >> 8;
    //
    // DataLength
    chSend[LOC_LENGTH] = (nSendByte - HEADER_SIZE) & 0xff;
    chSend[LOC_LENGTH+1] = ((nSendByte - HEADER_SIZE) & 0xff00) >> 8;
    //
    //CheckSum
    int checksum = 0;
    for (i = 0; i < LOC_CHECKSUM; i++)
        checksum = (checksum + chSend[i]) & 0xff;
    //
    chSend[LOC_CHECKSUM] = (unsigned char)checksum;
	//
    // for (i = 0; i < nSendByte; i++)
    //    printk("[%d]", chSend[i]);
    // printk("\n");
    // return 1;
	//
    // printk("DEBUG: send %d bytes!!!\n",nSentByte);
    //
    // Add break timer for avoiding endless loop
    while(1)
    {
        nSentByte = send(plc_socket, chSend, nSendByte, 0);
		usleep(900000);
        //
        nRecvByte = recv(plc_socket, chRecv, 1024, 0);
        //
        if (nRecvByte == 0)
        {
            bRet = 0;
            break;
        }
        else if (nRecvByte < 0)
        {
            printf("nRecvByte : %d\n", nRecvByte);
            bRet = 0;
            break;
        }
        else
        {
            //printk("DEBUG: recv %d bytes!!!\n",nRecvByte);
            //for(i=0;i<nRecvByte;i++)
            //printk(" [%X] ",chRecv[i]);
            //printk("\n");
            bRet = 1;
            break;
        }
    }
	//
    nLength = 30;
    //
    // debug code
     printf("AO Write>>> nRecvByte %d, nLength %d \n", nRecvByte, nLength);
    // debug code
	//
#ifdef PLC_TYPE_1	
    //  ORG - modified for DAECHI  
	// Doksan
    if((nRecvByte == nLength) && (chRecv[26] == 0x00) && (chRecv[27] == 0x00)
        && memcmp(&chSend[LOC_LINKID], &chRecv[LOC_LINKID], 2) == 0)
#else    
	if((nRecvByte == (nLength+1)) && (chRecv[26] == 0x00) && (chRecv[27] == 0x00)
        && memcmp(&chSend[LOC_LINKID], &chRecv[LOC_LINKID], 2) == 0)
#endif		
    {
        bRet = 1;
    }
    else
    {
        printf("DEBUG: AO write error !!! \n");
        printf("Write AO Point(1) : nRecvByte %d, nLength %d \n", nRecvByte, nLength);
        bRet = 0;
    }
    //
    return bRet;
}


// jong2ry plc
/****************************************************************/
int get_plc_message_queue(point_info *pPoint, int *type, int *index)
/****************************************************************/
{
	int i = 0;
	int result = ERROR;
	int checkData = ERROR;

	pthread_mutex_lock(&plcQ_mutex);
	result = getq(&plc_message_queue, pPoint);
	pthread_mutex_unlock(&plcQ_mutex);

	if(result == SUCCESS)
	{
		printf("result.. pPoint->pcm = %d, pPoint->pno = %d\n", pPoint->pcm, pPoint->pno);	
		
		for (i = 0; i < 1024; i++)
		{
			if (plcData[i].pcm == pPoint->pcm  
					&& plcData[i].pno == pPoint->pno)	
			{
				*type = plcData[i].type;
				*index = plcData[i].addr;
				checkData = SUCCESS;
				printf("check.. pcmd = %d, pno = %d\n", plcData[i].pcm, plcData[i].pno);
			}
		}
		
		return checkData;
	}
	else
		return ERROR;
}

// jong2ry plc
void get_3iStoPlcData(void)
{
	int i = 0;
	int type = 0;
	int index = 0;
    DNP_DIO_Point *dio_p;
    DNP_AIO_Point *aio_p;	
	point_info myPoint;	

	for (i = 0; i < 512; i++)
	{	
		if (get_plc_message_queue(&myPoint, &type, &index) == SUCCESS)
		{
			printf("Set...[%d]%d, %d, %f\n", type, myPoint.pcm, myPoint.pno, myPoint.value);
			if (type == DNP_DO_SEL)
			{
				dio_p = &DNP_do_point[index];
				dio_p->WrFlag = 1;
				dio_p->WrVal = myPoint.value;
				
				g_iChkControl = 1;
			}
			else if (type == DNP_AO_SEL)
			{
				printf("AO Write ...%d,  %d, %f\n", type, index, myPoint.value);
				aio_p = &DNP_ao_point[index];
				aio_p->WrFlag = 1;
				aio_p->val = myPoint.value;				
				aio_p->PrevVal = myPoint.value;				
				
				g_iChkControl = 1;
			}
		}	
	}
}


/****************************************************************/
int PLC_write(int plc_socket)
/****************************************************************/
{
    int i = 0;
    DNP_DIO_Point *dio_p;
    DNP_AIO_Point *aio_p;


	get_3iStoPlcData();
	
    for (i = 0; i < MAX_DNP_DO_POINT; i++)
    {
        dio_p = &DNP_do_point[i];

#if 0
        if (i == 56)
        {
            printf("DEBUG, plc_write i %d, WrFlag %d, WrVal %d \n", i, dio_p->WrFlag, dio_p->WrVal);
        }
#endif
    
        if(dio_p->WrFlag)
        {
            if(WriteDOPoint(plc_socket, i, dio_p->WrVal))
            {
                printf("DEBUG: WriteDOPoint pno %d \n", i);
                //setValue3iS(DNP_DO_SEL, i, (float)dio_p->WrVal);
                dio_p->WrFlag = 0;
            }
            else
            {
                printf("DO [%d] write failed !!! \n", i);
                dio_p->WrFlag = 0;
                return -1;	
            }
        }
    }
  
    for (i = 0; i < MAX_DNP_AO_POINT; i++)
    {
        aio_p = &DNP_ao_point[i];
        
        if(aio_p->WrFlag)
        {
            printf("DEBUG: WriteAOPoint pno %d \n", i);
            if ( WriteAOPoint(plc_socket, i, aio_p->PrevVal) ) {
	            aio_p->WrFlag = 0;
	            aio_p->val = aio_p->PrevVal;    // 20071108 PLC AO Unsol ???
	            aio_p->Unsol = 1;               // 20071108 PLC AO Unsol ???
	        }
	        else {
	        	printf("AO [%d] write failed !!! \n", i);
	        	return -1;	
	        }
        }
    }
    
    return 1;
}

//not used... 
/*
void SleepnWriteCheck(int plc_socket)
{
	int i = 0;
	//
	for (i = 0; i < 10 ; i++)
	{
		usleep(100000);
		PLC_write(plc_socket);
	}
}
*/



/****************************************************************/
void *plc_observer_main(void *arg)
/****************************************************************/
{
	//Variable 
	int i, j, k;
	int ret = 0;
	int iChkCount = 0;
	//
	int sock_fd;
	int connect_flag;
	char stationip[16];
	struct sockaddr_in server_addr;
	struct timeval timeo;
	//
	//int checkCount = 0;
	//
	int plc_status;
	int nRtnVal ;
	int plc_connect_flag = 0;
	int plc_disconnect_cnt = 0;
	//
	//Initialize
	//printf("PLC Initialize\n");
	i = j = k = 0;
	//
	sock_fd = -1;
	connect_flag = -1;
	strncpy(stationip, "192.168.2.3", sizeof(stationip));
	memset(&server_addr, 0, sizeof(server_addr));
	timeo.tv_sec = 0;
	timeo.tv_usec = 10000;
	//
	memset(&gRxD, 0x00, sizeof(gRxD));
	memset(&gTxD, 0x00, sizeof(gTxD));
	//	
	plc_status = PLC_CREATE_SOCKET;
	nRtnVal = 0;
	//
	//Ignore broken_pipe signal
	signal(SIGPIPE, SIG_IGN);
	//
	sleep(2);
	//
	printf("Start PLC While Loop - 2010.03.18 Gwang-Myeong\n");
	//
	printf("int = %d, char = %d, short = %d\n", sizeof(int), sizeof(char), sizeof(short));
	//
	// 091102 Jong2ry.
	PLC_point_define();
	//get_3iStoPlcData();		// jong2ry plc
	//
	strncpy(chPlcIp[0], "203.252.236.238", sizeof(chPlcIp[0]));
	strncpy(chPlcIp[1], "203.252.236.239", sizeof(chPlcIp[1]));
	while(1)
	{
//		sleep(1);
//		if (checkCount < 100)	checkCount++;
//		else					checkCount = 0;
//		pSet(1, 255, checkCount);


		for (iChkCount = 0; iChkCount < 1000; iChkCount++) {
			if (sock_fd != -1) {
				ret = PLC_write(sock_fd);
				
				if (ret < 0) {
					close(sock_fd);
					sock_fd = -1;	
				}
			}
		}
		if (sock_fd == -1) {
			connect_flag = DISCONNECTED;
			plc_status = PLC_CREATE_SOCKET;		
			sleep(3);					
		}
			
		//printf("PLC g_iChkControl = %d\n", g_iChkControl);
		
		switch(plc_status)
		{
			case PLC_CREATE_SOCKET:
				// 주의 해서 확인한다...;; 혹시나 문제가 생길지도 모른다...
				// Warning이 나서 수장하였다. 2010.06.08
				//if (sock_fd = socket(AF_INET, SOCK_STREAM, 0) < 0)
				if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0) < 0))
				{
					printf("PLC Socket Creation Error\n");
					break;
				}
				//
				setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo));
				//	
				memset(&server_addr, 0, sizeof(server_addr));
				server_addr.sin_family = AF_INET;
				if (plc_connect_flag == 0)
				{
					memcpy(stationip, &chPlcIp[0], sizeof(stationip));
					plc_connect_flag = 1;
				}
				else
				{
					memcpy(stationip, &chPlcIp[1], sizeof(stationip));
					plc_connect_flag = 0;
				}
				server_addr.sin_addr.s_addr = inet_addr(stationip);
				server_addr.sin_port = htons(PLC_SERVER_PORT);
				//
				printf("Trying to connect PLC, IP [%s], FD [%d]\n", 
						stationip, sock_fd);
				//
				if (connect(sock_fd, (struct sockaddr*)&server_addr, 
							sizeof(server_addr)) == -1)
				{
					printf("Connect Error...Goto Retrying...\n");
					sleep(3);
					close(sock_fd);
					sock_fd = -1;
					connect_flag = DISCONNECTED;
					
					// 091102 Jong2ry. PLC-point-table initialize
					plc_disconnect_cnt++;
					if(plc_disconnect_cnt > 3)
					{
						PLC_point_define();
						printf("PLC point table initialize.\n");
						plc_disconnect_cnt = 0;
					}
					continue;
				}
				else
				{
					printf("Connect Success...\n");
	
					// 091102 Jong2ry. blocking code.
					// Becasue, If connection-process retry, PLC-point-table initialize.
					// PLC Point Initialize
					// PLC_point_define();
					connect_flag = CONNECTED;
					plc_status = PLC_DI_PROC;
					plc_disconnect_cnt = 0;
					break;
				}
				break;
	
			case PLC_DI_PROC:
				//usleep(30000);
				for (iChkCount = 0; iChkCount < 100; iChkCount++) {
					if (sock_fd != -1) {
						ret = PLC_write(sock_fd);
						
						if (ret < 0) {
							close(sock_fd);
							sock_fd = -1;	
						}
					}
				}
				
				if ( sock_fd == -1) {
					connect_flag = DISCONNECTED;
					plc_status = PLC_CREATE_SOCKET;		
					sleep(3);					
					break;
				}
										
				//printf("PLC DI Process\n");
				for(i=0;i<PLC_DI_ADDR_CNT;i++)
				{
					nRtnVal = ReadDIOBlock(
						sock_fd,
						PLC_DI_ADDR[i], 
						PLC_DI_COUNT[i], 
						PLC_DI_POINT[i], 
						DNP_DI_SEL);
					//
					if(nRtnVal == 0)
						break;
				}
				//
				PLC_write(sock_fd);
				//
				if (nRtnVal == 0)
				{
					printf("PLCd: Read DIO block is failed!!! \n");
					plc_status = PLC_CLOSE_SOCKET;
					break;
				}
				else
					plc_status = PLC_DO_PROC;
				break;

			case PLC_DO_PROC:
				//usleep(30000);

				for (iChkCount = 0; iChkCount < 100; iChkCount++) {
					if (sock_fd != -1) {
						ret = PLC_write(sock_fd);
						
						if (ret < 0) {
							close(sock_fd);
							sock_fd = -1;	
						}
					}
				}
				
				if ( sock_fd == -1) {
					connect_flag = DISCONNECTED;
					plc_status = PLC_CREATE_SOCKET;		
					sleep(3);	
					break;				
				}

				if (g_iChkControl > 0) {
					plc_status = PLC_AI_PROC;
					break;
				}
				//printf("PLC DO Process\n");
				for(i=0;i<PLC_DO_ADDR_CNT;i++)
				{
					nRtnVal = ReadDIOBlock(
						sock_fd, 
						PLC_DO_ADDR[i], 
						PLC_DO_COUNT[i], 
						PLC_DO_POINT[i], 
						DNP_DO_SEL);
					//
					if(nRtnVal == 0)
						break;
				}
				//
				PLC_write(sock_fd);
				//
				if(nRtnVal == 0)
				{
					printf("PLCd: Read DO block is failed!!! (%d) \n", i);
					plc_status = PLC_CLOSE_SOCKET;
					break;
				}
				else
					plc_status = PLC_AI_PROC;
				break;

			case PLC_AI_PROC:
				//usleep(30000);
				for (iChkCount = 0; iChkCount < 100; iChkCount++) {
					if (sock_fd != -1) {
						ret = PLC_write(sock_fd);
						
						if (ret < 0) {
							close(sock_fd);
							sock_fd = -1;	
						}
					}
				}
				
				if ( sock_fd == -1) {
					connect_flag = DISCONNECTED;
					plc_status = PLC_CREATE_SOCKET;		
					sleep(3);	
					break;				
				}
				
				//printf("PLC AI Process\n");
				for(i = 0; i < PLC_AI_ADDR_CNT; i++)
				{
					nRtnVal = ReadAIOBlock(
						sock_fd, 
						PLC_AI_ADDR[i],
						PLC_AI_COUNT[i], 
						PLC_AI_POINT[i], 
						DNP_AI_SEL);
					//
					if(nRtnVal == 0)
						break;
				}
				//
				PLC_write(sock_fd);
				//
				if(nRtnVal == 0)
				{
					printf("PLCd: Read AI block is failed!!! \n");
					plc_status = PLC_CLOSE_SOCKET;
					break;
				}
				else
					plc_status = PLC_AO_PROC;
				break;

			case PLC_AO_PROC:
				//usleep(30000);
				for (iChkCount = 0; iChkCount < 100; iChkCount++) {
					if (sock_fd != -1) {
						ret = PLC_write(sock_fd);
						
						if (ret < 0) {
							close(sock_fd);
							sock_fd = -1;	
						}
					}
				}
				
				if ( sock_fd == -1) {
					connect_flag = DISCONNECTED;
					plc_status = PLC_CREATE_SOCKET;		
					sleep(3);
					break;					
				}
				
				if (g_iChkControl > 0) {
					plc_status = PLC_DI_PROC;
					g_iChkControl = 0;
					break;
				}
				
				//printf("PCL AO Process\n");
				for(i = 0; i < PLC_AO_ADDR_CNT; i++)
				{
					nRtnVal = ReadAIOBlock(
						sock_fd, 
						PLC_AO_ADDR[i],
						PLC_AO_COUNT[i], 
						PLC_AO_POINT[i], 
						DNP_AO_SEL);
					//
					if(nRtnVal == 0)
							break;
				}
				//
				
				//for (i = 0 ; i < 30; i++)
				//{
				//	printf("PLCd :: pno = %d, Val = %f\n", i, DNP_ao_point[i].val);
				//}
				//
				PLC_write(sock_fd);
				//
				if(nRtnVal == 0)
				{
					printf("PLCd: Read AO block is failed!!! \n");
					plc_status = PLC_CLOSE_SOCKET;
					break;
				}
				else
					plc_status = PLC_DI_PROC;
				break;


			case PLC_CLOSE_SOCKET:
				printf("PLC Socket Close\n");
				if (connect_flag == CONNECTED)
				{
					close(sock_fd);
					sock_fd = -1;
					connect_flag = DISCONNECTED;
				}
				plc_status = PLC_CREATE_SOCKET;		
				sleep(3);
				break;

			default :
				plc_status = PLC_CLOSE_SOCKET;
				printf("Check.. PLC status\n");
				break;
		}
	}
}
