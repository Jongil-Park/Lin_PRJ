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
#include <sys/time.h>


#include "define.h"
#include "queue_handler.h"
#include "ghp_app.h"
#include "FUNCTION.h"


extern pthread_mutex_t schedule_mutex;
extern point_queue ghp_message_queue;
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];

extern float pGet(int pcm, int pno);
extern int pSet(int pcm, int pno, float value);
extern int RecvUart2(unsigned char *p, int maxlength);
extern int SendUart2(unsigned char *p, int length);
extern void InitUart2(int baudrate);

/** variable *****************************************************/
int unit = 0;
float prePtbl[32][256];

char txbuf[512];		// tx buffer.
char rxbuf[512];		// rx buffer.

int g_unitCnt = 0;		// indoor unit count.
int g_sddcNum = 0;		// sddc number.
int g_dbgShow = 1;		// debug-text show enable/disable
int text_number = 0;	// text_number;

SEND_DATA g_schedFileData;

g_GHP_Info *g_pDongInfo;
#define UART2DEVICE "/dev/s3c2410_serial2"
int g_iUart2Fd;
struct pollfd g_PollEvents;      // 체크할 event 정보를 갖는 struct
int g_iPollState;
int g_CntHaksaInit;


// pcm value.
int GHP_ONOFF_PCM = 0;
int GHP_ONOFF_FAN_PCM = 0;
int GHP_SET_TEMP_PCM = 0;
int GHP_MODE_PCM = 0;
int GHP_TEMP_PCM = 0;
int GHP_GROUP_PCM = 0;
int GHP_GROUP_MODE_PCM = 0;
int GHP_WINDSPEED_PCM = 0;
int GHP_WINDDIRECTION_PCM = 0;        
int GHP_ERROR_PCM = 0;
int GHP_ERROR_NUM_PCM = 0;
int GHP_GROUP_STATUS_PCM = 0;
int GHP_GROUP_DATA_WEEK_PCM = 0;
int GHP_GROUP_DATA1_PCM = 0;
int GHP_GROUP_DATA2_PCM = 0;
int GHP_MODE_STATUS_PCM = 0;
int GHP_RUNTIME_PCM = 0;


void init_poll(void);
void CheckUserControl(void);
void IfaceGhpSelectSleep(int sec,int msec);
int OnReadIn(void);
int OnWriteOut(unsigned char *p, int length);

// ghp_app.c function
void IfaceGhpSelectSleep(int sec,int msec) ;
void Get_Ghp_Unit(void);
void Chk_User_Control(int type);
int Do_User_Control(void);
void CheckUserControl(void);

#include "ghp_init.c"
#include "ghp_message.c"
#include "ghp_comm.c"
#include "ghp_group.c"
#include "ghp_schedule.c"

#include "iface_hong.c"
#include "iface_cnue.c"


/*
slect를 사용해서 TimeWait를 만들었다. 
*/
/*******************************************************************/
void IfaceGhpSelectSleep(int sec,int msec) 
/*******************************************************************/
{
    struct timeval tv;
    tv.tv_sec=sec;
    tv.tv_usec=msec;                
    select(0,NULL,NULL,NULL,&tv);
    return;
}


/****************************************************************/
void Get_Ghp_Unit(void)
/****************************************************************/
{
	int cnt = 0;
	//int i = 0;
	int send_result = 0;
	int result = 0;;
	int text_ptr = 0;
	unsigned char text[256];
	
	memset(text, 0x00, sizeof(text));

	//Start Message
	result = Start_Text(text, TYPE_POLL, text_number);
	text_ptr = result;

	for(cnt = 0; cnt < 4 ;cnt++) {

		if (unit < g_unitCnt)
			unit++;
		else
			unit = 0;
			
		//On_Off Message
		result = Make_Read_Msg(&text[text_ptr], 1, unit);
		text_ptr += result;
		//Mode Message
		result = Make_Read_Msg(&text[text_ptr], 4, unit);
		text_ptr += result;
		//Temp Message
		result = Make_Read_Msg(&text[text_ptr], 2, unit);
		text_ptr += result;	
		//SetTemp Message
		result = Make_Read_Msg(&text[text_ptr], 5, unit);
		text_ptr += result;		
		//WindSpeed Message
		result = Make_Read_Msg(&text[text_ptr], 30, unit);
		text_ptr += result;		
		//WindDirection Message
		result = Make_Read_Msg(&text[text_ptr], 31, unit);
		text_ptr += result;			
	}				
	
	//End Message
	result = End_Text(text, text_ptr);
	text_ptr = result;

	send_result = Send_Selecting(text, text_ptr);
}


/*
static void get_ghp_setting_value(void)
{
	printf("%s()\n", __FUNCTION__);
}
*/

/****************************************************************/
void Chk_User_Control(int type)
/****************************************************************/
{
	//int i= 0;
	int pcm = 0; 
	int pno = 0;
	point_info point;
	
	switch(type) {
		case USER_CONTROL_ONOFF: 	  pcm = GHP_ONOFF_PCM; 			break;	
		case USER_CONTROL_MODE:  	  pcm = GHP_MODE_PCM; 			break;				
		case USER_CONTROL_SETTEMP: 	  pcm = GHP_SET_TEMP_PCM; 		break;	
		case USER_CONTROL_SPEED: 	  pcm = GHP_WINDSPEED_PCM; 		break;	
		case USER_CONTROL_DIRECTION:  pcm = GHP_WINDDIRECTION_PCM; 	break;	
		default: return;									
	}	
		
	for(pno = 0; pno < GHP_UNIT_MAX; pno++)	{

		if(prePtbl[pcm][pno] != g_fExPtbl[pcm][pno]) {

			if(g_dbgShow) printf("Change Point = %d,%d (%f, %f)\n", 
				pcm, 
				pno, 
				prePtbl[pcm][pno], 
				g_fExPtbl[pcm][pno]);
			prePtbl[pcm][pno] = g_fExPtbl[pcm][pno];
			
			point.pcm = pcm;
			point.pno = pno;
			point.value = g_fExPtbl[pcm][pno];
			putq(&ghp_message_queue, &point); 
		}
	}
}


/****************************************************************/
int Do_User_Control(void)
/****************************************************************/
{
	//int i = 0;
	int cnt = 0;
	int result = 0;
	point_info point;
	unsigned char text[256];
	int length = 0;
	int text_ptr = 0;
	int queueCount = 0;
	//int loopCnt = 0;
	
	memset(text, 0x00, sizeof(text));

	//Start Message
	text_ptr = Start_Text(text, TYPE_SEL, text_number);
	while(1) {	
		memset(&point, 0, sizeof(point));

		result = getq(&ghp_message_queue, &point);
		if(result == SUCCESS) {
			printf("get %d, %d %f\n", point.pcm, point.pno, point.value);				
			length = Add_Text((unsigned char *)&text[text_ptr], point);
			text_ptr += length;
			queueCount++;
		}
		else
			break;
		
		cnt++;
		if (cnt > 10)
			break;
	}

	//End Message
	result = End_Text(text, text_ptr);
	text_ptr = result;	
	
	if (cnt > 0) {
#if 0		
		printf("Get User Control data (%d)\n", text_ptr);
		for (i = 0; i < 4; i++)
			printf("%x ", text[i]);
		printf("\n");
		for (i = 4; i < text_ptr - 2; i++)
		{
			if ( text[i] == '$')
				printf("\n");
			printf("%c ", text[i]);
		}
		printf("\n");	
		//for (i = text_ptr - 2; i < text_ptr; i++)
		printf("%x ", text[text_ptr-2]);	
		printf("%x ", text[text_ptr-1]);	
		printf("\n");		
#endif		
		
		// polling에서 Startup 메시지를 얻어오기 위해서 삭제후 Test 한다. 
		//Send_Selecting(text, text_ptr);
		//Send_Polling();
		//Send_Selecting(text, text_ptr);
		Send_Selecting(text, text_ptr);
	}
	return queueCount;
}


void CheckUserControl(void)
{
	if(g_dbgShow) printf("ST_CHK_USER_CONTROL\n");
	Chk_User_Control(USER_CONTROL_ONOFF);
	Chk_User_Control(USER_CONTROL_MODE);
	Chk_User_Control(USER_CONTROL_SETTEMP);
	Chk_User_Control(USER_CONTROL_SPEED);
	Chk_User_Control(USER_CONTROL_DIRECTION);	
}


/****************************************************************/
int iface_ghp_main(void)
/****************************************************************/
{
	IfaceGhpSelectSleep(3, 0);
	
	// initialize ghp interface  	
	clear_tx_buf();						// clear tx buffer.
	clear_rx_buf();						// clear rx buffer.
	init_uart();						// init uart2 channel.
	init_poll();
	init_queue();
	get_sddc();							// get sddc number. 

	
	// hong_univ ghp interface
#if 0
	hong_init_data();
	hong_set_pcm_number();				// 
	hong_get_unit_cnt();				// get indoor unit count. 
	hong_get_ghp_data();
	hong_while();
#endif

	// cnue ghp interface
	// ghpCfg.dat    (A , 1) , (B , 2) , (C , 3)
	cnue_init_data();
	cnue_set_pcm_number();				// 
	cnue_get_unit_cnt();				// get indoor unit count. 
	cnue_get_ghp_data();
	cnue_while();

	return -1;
}



