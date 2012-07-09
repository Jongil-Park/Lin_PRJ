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


#include "define.h"
#include "queue_handler.h"
#include "iface_cnue.h"


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

#include "cnue_init.c"
#include "cnue_message.c"
#include "cnue_comm.c"
#include "cnue_group.c"
#include "cnue_schedule.c"




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

#if 0
void write_ghp_data(void)
{
	FILE *fp;
	int i = 0;
	int datasize = 0;

	if((fp = fopen("ClassAC.dat", "w")) != NULL) {
		datasize = sizeof(g_GHP_Info);
		for( i = 0; i < 256; i++) 
			fwrite(&AC_Dong_Info[i], datasize, 1, fp);
	}
	fclose(fp);		

	if((fp = fopen("ClassB.dat", "w")) != NULL) {
		datasize = sizeof(g_GHP_Info);
		for( i = 0; i < 256; i++) 
			fwrite(&B_Dong_Info[i], datasize, 1, fp);
	}
	fclose(fp);

	if((fp = fopen("ClassD.dat", "w")) != NULL) {
		datasize = sizeof(g_GHP_Info);
		for( i = 0; i < 256; i++) 
			fwrite(&D_Dong_Info[i], datasize, 1, fp);
	}
	fclose(fp);
	
	if((fp = fopen("ClassE.dat", "w")) != NULL) {
		datasize = sizeof(g_GHP_Info);
		for( i = 0; i < 256; i++) 
			fwrite(&E_Dong_Info[i], datasize, 1, fp);
	}
	fclose(fp);

	if((fp = fopen("ClassF.dat", "w")) != NULL) {
		datasize = sizeof(g_GHP_Info);
		for( i = 0; i < 256; i++) 
			fwrite(&F_Dong_Info[i], datasize, 1, fp);
	}
	fclose(fp);	

	if((fp = fopen("ClassG.dat", "w")) != NULL) {
		datasize = sizeof(g_GHP_Info);
		for( i = 0; i < 256; i++) 
			fwrite(&G_Dong_Info[i], datasize, 1, fp);
	}
	fclose(fp);

	if((fp = fopen("ClassK.dat", "w")) != NULL) {
		datasize = sizeof(g_GHP_Info);
		for( i = 0; i < 256; i++) 
			fwrite(&Duruarm_Kisuksa_Info[i], datasize, 1, fp);
	}
	fclose(fp);
}
#endif

/****************************************************************/
int iface_ghp_main(void)
/****************************************************************/
{
	//int i = 0;
	//unsigned char tempVal = 0;
    //	int j = 0;
	int st = 0;
	int result = 0;
	//DEL point_info point;
	//int pollingCount = 0;

	IfaceGhpSelectSleep(3, 0);
	
	/* initialize ghp interface */
	init_data();
	clear_tx_buf();		// clear tx buffer.
	clear_rx_buf();		// clear rx buffer.
	set_pcm_number();	// set pcm nummber.
	init_uart();		// init uart2 channel.
	init_poll();
	get_sddc();			// get sddc number. 
	get_unit_cnt();		// get indoor unit count. 
	get_ghp_data();		// get ghp-point data.
	init_queue();
	
	g_CntHaksaInit = 0;

	/*
	 Test Code.
	 반드시 현장에서 지워야 한다. 
	*/
	while( 1 ) {
		IfaceGhpSelectSleep(3, 0);
	}

	/*
	uart2 통신에 의한 Delay가 발생하는 상황을 재현하고자 한다.
	Test가 끝나면 반드시 주석으로 막아 실행되지 않도록 한다.
	*/
#if 0	
	printf("Test Code\n");
	memset(txbuf, 0, sizeof(txbuf));
	for (i = 0; i < 256; i++) {
		txbuf[i] = tempVal++;
	}

	printf("init poll\n");
	init_poll();
	while(1)
	{
		result = OnReadIn();
		if ( 0 < result ) {
			printf("Rx (%d)\n", result);
			/*
			for (i = 0; i < result; i++) {
			printf("%x ", rxbuf[i]);
			}
			printf("\n\n");
			*/
			printf("Write\n");
			OnWriteOut(rxbuf, result);
			printf("scehdule\n");
			Check_Schedule(g_unitCnt);
		}
		
	}
#endif	
	
#if 0
	while( 1 ) {
		IfaceGhpSelectSleep(1, 0);
		Chk_Group_Control();

		// 전체 정지 모드 
		Set_AllStopMode(g_unitCnt);

		// 실내기 온도 제한 
		Set_Temperature(g_unitCnt);
	}
#endif

 	//super loop
	st = ST_GET_POLLING;
 	do{

		// 전체 정지 모드 
		Set_AllStopMode(g_unitCnt);
		
		// 실내기 온도 제한 
		Set_Temperature(g_unitCnt);
		
		Get_RunUnit(g_unitCnt);
				
		// 각 동의 층별제어 및 실내기 타입별 온도제어를 한다.
		Chk_Group_Control();	
			
		// jong2ry test.
		// It is scheduler function.
		Check_Schedule(g_unitCnt);
		//sleep(1);
		//continue;
		
		
		Set_AllHaksaMode(g_unitCnt);

		switch(st)
		{
			case ST_GET_POLLING:
				//if(g_dbgShow) printf("ST_GET_POLLING\n");

				result = Send_Polling();
				//printf("result = %d\n", result);
				switch(result)
				{
					case POLL_REQUIRE_STARTUP:	
						st = ST_STARTUP;	
						break;
					
					case POLL_EMPTY_DATA:
					case POLL_OK:		
						if (Send_Polling() == POLL_REQUIRE_STARTUP) {
							st = ST_STARTUP;	
							break;
						}
						else {
							st = ST_CHK_USER_CONTROL;
							break;
						}
					
					default: 
						//if(g_dbgShow) printf(">>> POLLING ERROR\n");
						st = ST_CHK_USER_CONTROL;
						break;
				}
				break;

			case ST_CHK_USER_CONTROL:
				CheckUserControl();
				st = ST_GET_USER_CONTROL;
				break;

			case ST_GET_USER_CONTROL:
				//if(g_dbgShow) printf("ST_GET_USER_CONTROL\n");

				while(Do_User_Control()) {

					if (Send_Polling() == POLL_REQUIRE_STARTUP) {
						st = ST_STARTUP;	
						break;
					}
	
					// 전체 정지 모드 
					Set_AllStopMode(g_unitCnt);
					
					// 실내기 온도 제한 
					Set_Temperature(g_unitCnt);
					
					Get_RunUnit(g_unitCnt);
							
					// 각 동의 층별제어 및 실내기 타입별 온도제어를 한다.
					Chk_Group_Control();						
				}


				st = ST_GET_GHP_UNIT;
				break;

			case ST_GET_GHP_UNIT:
				//if(g_dbgShow) printf("ST_GET_GHP_UNIT\n");
				Get_Ghp_Unit();

				if (Send_Polling() == POLL_REQUIRE_STARTUP) {
					st = ST_STARTUP;	
					break;
				}   

				//if(g_dbgShow) printf("\n\n");	
				st = ST_GET_POLLING;
				break;

			case ST_STARTUP:
				//if(g_dbgShow) printf("ST_STARTUP\n");
				Startup_SDDC();
				st = ST_GET_POLLING;
				break;

			default:
				//if(g_dbgShow) printf("default\n");
				st = ST_GET_POLLING;
				break;
		}
	}while(1);
	
	//exit(1);
	system("killall duksan");
}



