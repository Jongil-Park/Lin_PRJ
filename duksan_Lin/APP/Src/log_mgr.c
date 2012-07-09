/*
 * file 		: elba_mgr.c
 * creator   	: tykim
 *
 *
 * System Log 처리
 *  	1. Application을 종료하기전에 이벤트를 발생시켜 로그를 기록한다.
 *  	2. GCU 디버깅 및 GCU 관리.
 *
 *
 * version :
 *		0.0.1  jong2ry working.
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
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <sys/poll.h>		// use poll event

#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"
#include "queue_handler.h"									// queue handler

#include "modbus_mgr.h"										// elba manager
#include "net32_mgr.h"										// net32 manager


static void syslog_sleep(int sec, int msec) 
// ----------------------------------------------------------------------------
// WAIT TIMER
// Description : use select function for timer.
// Arguments   : sec		Is a second value.
//				 msec		Is a milli-second value. 
// Returns     : none
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = msec * 1000;
    select( 0, NULL, NULL, NULL, &tv );
	return;
}

void syslog_record(int type)
// ----------------------------------------------------------------------------
// WRITE LOG TEXT
// Description : system log를 기록하여 system이 어떻게 운영되는지 확인한다. 
// Arguments   : type		log에 기록되는 text의 type
// Returns     : none
{
	FILE *fp = NULL;
	FILE *fp_bak = NULL;
	FILE *fp_log = NULL;
	time_t     tm_nd;
	struct tm *tm_ptr;
	int filesize = 0;
	int size = 0;
	unsigned char buff[1028];
	
	fp = fopen("/duksan/FILE/system_log.txt", "a+");
	if( fp == NULL ) {
		fprintf( stdout, "SystemLog 1 : system_log.txt file open error\n" );
		fflush( stdout );
		syslog_sleep(5, 0);
		//system("reboot 1000");
		//exit(1);
		system("killall duksan");
		return;		
	}
	
	fseek(fp, 0L, SEEK_END); 
	filesize = ftell( fp );
	//printf("file size = %d\n", filesize);

	if ( filesize > 40960 ) {

		fseek(fp, 0L, SEEK_SET); 

		fp_bak = fopen("/duksan/FILE/system_log.bak", "w");
		if( fp_bak == NULL ) {
			fprintf( stdout, "SystemLog 2 : system_log.txt.bak file open error\n" );
			fflush( stdout );
			syslog_sleep(5, 0);
			//system("reboot 1000");
			//exit(1);
			system("killall duksan");
			return;		
		}
		
		memset( buff, 0x00, sizeof(buff) );
		while( 0 < (size = fread( buff, 1, 1024, fp))) {
			fwrite( buff, 1, size, fp_bak);
			memset( buff, 0x00, sizeof(buff) );
		}  

		fclose(fp);
		fclose(fp_bak);
	
		fp = fopen("/duksan/FILE/system_log.txt", "w");
		if( fp == NULL ) {
			fprintf( stdout, "SystemLog 3 : system_log.txt file open error\n" );
			fflush( stdout );
			//system("reboot 1000");
			//exit(1);
			system("killall duksan");
			return;		
		}
	}

	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);

	// write Communication Error
	switch ( type ) {
	// BootOn Text
	case SYSLOG_BOOT_ON:
		fprintf(fp,	"[System BootOn]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;

	// Net32 Close Text
	case SYSLOG_NET32_FAIL_REBOOT:
		fprintf(fp,	"[Net32 Close]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;

	// Net32 TimeOut Text
	case SYSLOG_NET32_TIMEOUT_REBOOT:
		fprintf(fp,	"[Net32 TIMEOUT]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;

	// Net32 TimeOut Text
	case SYSLOG_NET32_OPEN_ERROR:
		fprintf(fp,	"[Net32 OPEN ERROR]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;
		
	// Net32  Select Error
	case SYSLOG_NET32_SELECT_ERROR:
		fprintf(fp,	"[Net32 SELECT ERROR]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;
		
	// Net32 Accept Error
	case SYSLOG_NET32_ACCEPT_ERROR:
		fprintf(fp,	"[Net32 ACCEPT ERROR]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;			

	// Net32 Accept Error
	case SYSLOG_NET32_CLOSE_ERROR:
		fprintf(fp,	"[Net32 CLOSE ERROR]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;			
				
	// ADC Driver Fail Text
	case SYSLOG_ADCDRV_FAIL_REBOOT:
		fprintf(fp,	"[ADC-DRV Fail]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;

	// elba Connected
	case SYSLOG_CONNECT_ELBA:
		fprintf(fp,	"[ELBA Connected]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;

	// elba Disconnected
	case SYSLOG_DISCONNECT_ELBA:
		fprintf(fp,	"[ELBA Disconnected]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;
		
	// net32 destroy
	case SYSLOG_DESTROY_NET32:
		fprintf(fp,	"[Net32 Destroy]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;	

	// elba destroy
	case SYSLOG_DESTROY_ELBA:
		fprintf(fp,	"[ELBA Destroy]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;				

	// msg_handler destroy
	case SYSLOG_DESTROY_MSG_HANDLER:
		fprintf(fp,	"[MSG Handler Destroy]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;		

	// command handler destroy
	case SYSLOG_DESTROY_CMD_HANDLER:
		fprintf(fp,	"[CMD Handler Destroy]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;	

	// point observer destroy
	case SYSLOG_DESTROY_POINT_OBSERVER:
		fprintf(fp,	"[POINT Observer Destroy]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;		
		
	// point manager destroy
	case SYSLOG_DESTROY_POINT_MANAGER:
		fprintf(fp,	"[POINT Manager Destroy]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;		
		
	// ccms server destroy
	case SYSLOG_DESTROY_CCMS_SERVER:
		fprintf(fp,	"[CCMS Server Destroy]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;													

	// ccms client destroy
	case SYSLOG_DESTROY_CCMS_CLIENT:
		fprintf(fp,	"[CCMS Client Destroy]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;		

	// ccms manager destroy
	case SYSLOG_DESTROY_CCMS_MANAGER:
		fprintf(fp,	"[CCMS Manager Destroy]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;		
		
	// ccms modem server destroy
	case SYSLOG_DESTROY_CCMS_MODEM_SERVER:
		fprintf(fp,	"[CCMS Modem Server Destroy]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;		
	
	// iface modem destroy
	case SYSLOG_DESTROY_IFACE_MODEM:
		fprintf(fp,	"[Iface Modem Destroy]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;			

	// iface sms destroy
	case SYSLOG_DESTROY_IFACE_SMS:
		fprintf(fp,	"[Iface Sms Destroy]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;		

	// iface sms destroy
	case SYSLOG_DESTROY_IFACE_GHP:
		fprintf(fp,	"[Iface GHP Destroy]  %d/%d %d:%d\n", 
				tm_ptr->tm_mon + 1, 
				tm_ptr->tm_mday, 
				tm_ptr->tm_hour, 
				tm_ptr->tm_min);
		break;				
	}

	fclose(fp);

	fp_log = fopen("/httpd/system_log.txt", "w");
	if( fp_log == NULL ) {
		fprintf( stdout, "SystemLog 4 : system_log.txt file open error\n" );
		fflush( stdout );
		return;		
	}
	
	fp_bak = fopen("/duksan/FILE/system_log.bak", "r");
	if( fp_bak == NULL ) {
		fprintf( stdout, "SystemLog 5 : system_log.bak file open error\n" );
		fflush( stdout );
	} 
	else {
		fseek(fp_bak, 0L, SEEK_SET); 
		memset( buff, 0x00, sizeof(buff) );
		while( 0 < (size = fread( buff, 1, 1024, fp_bak))) {
			fwrite( buff, 1, size, fp_log);
			memset( buff, 0x00, sizeof(buff) );
		}  
	
		fclose(fp_bak);	
	}
	
	fp = fopen("/duksan/FILE/system_log.txt", "r");
	if( fp == NULL ) {
		fprintf( stdout, "SystemLog 6 : system_log.txt file open error\n" );
		fflush( stdout );
	} 
	else {
		fseek(fp, 0L, SEEK_SET); 
		memset( buff, 0x00, sizeof(buff) );
		while( 0 < (size = fread( buff, 1, 1024, fp))) {
			fwrite( buff, 1, size, fp_log);
			memset( buff, 0x00, sizeof(buff) );
		}  
	
		fclose(fp);	
	}	
	
	fclose(fp_log);	
}





