/*
 * duksan 프로그램을 시작한다. 
 *  	1. Point Table을 복구한다. 
 *  	2. Thread간의 Queue를 초기화 한다. 
 *
 *
 * version :
 *		0.0.1  tykim working.
 *		0.0.2  jong2ry code clean.
 *
 *
 * code 작성시 유의사항
 *		1. global 변수는 'g_' 접두사를 쓴다.
 *		2. 변수를 선언할 때에는 아래와 같은 규칙으로 선언한다. 
 *			int					n
 *			short				w
 *			float				f
 *			char				ch
 *			unsigned char       b
*			unsignnd int		un
 *			unsigned short 		uw 
 *			pointer				p
 *			structure			s
 *		3. 함수명은 소문자로 '_' 기호를 사용해서 생성한다. 
 *		4. 함수와 함수 사이의 간격은 2줄로 한다. 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <asm/io.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <linux/watchdog.h>

/****************************************************************/
//include
#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"
#include "queue_handler.h"														// queue handler

#include "bacnet.h"
#include "main.h"

#include "elba_mgr.h"															// elba manager
#include "cfg_mgr.h"															// cfg manager
#include "modbus_mgr.h"															// modbus server manager															
#include "net32_mgr.h"															// net32 manager
#include "point_observer.h"														// point file observer
#include "point_mgr.h"															// Physical poinat manager
#include "interface_handler.h"
#include "cmd_handler.h"
#include "apg_handler.h"
#include "libinterface_handler.h"
#include "dev_gpio.h"
#include "bacnet.h"
#include "iface_subio.h"
#include "ghp_app.h"
#include "ghp_WebSvr.h"
#include "ghp_ScheduleSvr.h"
#include "plc_observer.h"
#include "iface_hantec.h"
//#include "iface_peak.h"
#include "iface_dbsvr.h"
//#include "log_mgr.h"															// system_log_manager
//#include "iface_msvr.h"
//#include "iface_ccms.h"
//#include "iface_ccms_modem.h"													// interface ccms
//#include "iface_ccms_mgr.h"
//#include "iface_ccms_client.h"
//#include "iface_modem.h"
//#include "iface_sms.h"
//#include "iface_handler.h"
//#include "iface_xgt.h"

#include "wdog_handler.h"
#include "demo_handler.h"
#include "dnp_observer.h"
#include "net32_to_dnp.h"

// share memory
#define  KEY_NUM     9527			// share memory key
#define  SHARE_MEM_SIZE    32786			// point table size

/****************************************************************/
//mutex
pthread_mutex_t elbaQ_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t net32Q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pointTable_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fileAccess_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bacnetQ_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bacnetAccess_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bacnetMessageQ_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t schedule_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t plcQ_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ccmsQ_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t uclientQ_mutex = PTHREAD_MUTEX_INITIALIZER;


/****************************************************************/
//queue
point_queue elba_queue;															// elba queue ( TCP/IP Networks -> elba_thread )
point_queue net32_queue;														// net32 message queue ( net32_thread -> net32 Networks )
point_queue bacnet_message_queue;												// bacnet queue. jong2ry.
point_queue ghp_message_queue;													// ghp interface jong2ry.
point_queue plc_message_queue;													// plc queue. jong2ry.
point_queue apg_queue;															// apg queue. jong2ry.
point_queue ccms_queue;															// ccms queue. jong2ry.
point_queue uclient_queue;															// ccms queue. jong2ry.


/****************************************************************/
//point and node
float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];							// point-table only value
float g_fPreExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];
PTBL_INFO_T *g_pPtbl;															// point-table
int g_nFileAccess[MAX_NET32_NUMBER];											// file access variable
int g_nMyPcm;																	// My Pcm Number
unsigned char g_cNode[MAX_NET32_NUMBER];										// node

// calibration
char *g_chDfnTypeName[] =  {" VR"," DO"," VO"," DI","JPT"," VI"," CI"};			// point-define type name
CAL_INFO_T *g_pCal;																// calibration table
int g_nCalReady = 0;															// calibration start flag
int g_nCalStatus = 0;															// calibration status
int g_nCalGrp = 0;																// calibration group (g0 ~ g3)
int g_nCalType = 0;																// calibration type


/****************************************************************/
// Version string
static char g_chAppVersion[] = "LiN AD-0228 3.3 - 2011.02.28"; 						// Version String.
static int g_nAppVersion = 4;


/****************************************************************/
// Command argument
static cmdinfo g_sCmdArgs[CFG_COMMAND_COUNT];


/****************************************************************/
// DDC MODE variable.
// g_nMultiDdcFlag is 0 ==> Net32 Mode ( USE NET32 )
// g_nMultiDdcFlag is 1 ==> Multi_ddc Mode ( NOT USE NET32 )
int g_nMultiDdcFlag = 0;


/****************************************************************/
// bacnet table. jong2ry.
aoPtbl_t aoPtbl[MAX_OBJECT_INSTANCE];
aiPtbl_t aiPtbl[MAX_OBJECT_INSTANCE];
doPtbl_t doPtbl[MAX_OBJECT_INSTANCE];
diPtbl_t diPtbl[MAX_OBJECT_INSTANCE];
msoPtbl_t msoPtbl[MAX_OBJECT_INSTANCE];
msiPtbl_t msiPtbl[MAX_OBJECT_INSTANCE];
int bacnetObjCnt = 0;


/****************************************************************/
// thread structure
static pthread_t g_sThread[MAIN_CREAT_THREAD_CNT];
pthread_attr_t *rt_attributes;
struct sched_param *rt_param;
				
/****************************************************************/
// Interface value
plcInfo plcData[1024]; 													// plc-interface variable
int g_nModeCCMS = 0;

DNP_DIO_Point DNP_di_point[512];
DNP_DIO_Point DNP_do_point[512];
DNP_AIO_Point DNP_ai_point[512];
DNP_AIO_Point DNP_ao_point[512];
//
int debug_plc_tx;
int debug_plc_rx;

// jong2ry 2o11_0302
int get_thread_opt(char *pThreadName, int nLength);


int main(void)
// ----------------------------------------------------------------------------
// MAIN FUNCTION
{
	int i = 0;
	int th_id = 0;
	int th_status = 0;
	int nIndexThread = 0;
	FILE *fp = NULL;

	struct sched_param thread_param;
	int thread_policy = 0;
	
	time_t     tm_nd;
	struct tm *tm_ptr;	
	int nPreSec = 0;
	int   shm_id;
	void *shm_addr;
   
   struct timespec ts;
   
	int nWatchdogDev;						// watchdog driver
	int nWatchdogTime = 3;					// watchdog timer value
	int nWatchdogInterval;							// watchdog interval value

	memset( g_sThread, 0x00, sizeof(g_sThread) );

	rt_attributes = (pthread_attr_t *) malloc (sizeof (pthread_attr_t));
	rt_param = (struct sched_param *) malloc (sizeof (struct sched_param));
	
	memset(rt_attributes, 0x00, sizeof (pthread_attr_t));
	memset(rt_param, 0x00, sizeof (struct sched_param));

	pthread_attr_init (rt_attributes);

	// jong2ry 2011_0322
	// main thread가 SIG_IGN 신호에 의해 terminate되는 것을 방지합니다. 
	signal(SIGPIPE, SIG_IGN);								// Ignore broken_pipe signal.

	// thread 설정.
	// SCHED_OTHER로 설정한다. 
	// sched_priority는 반드시 0이 되어야 한다. 
    if (pthread_attr_setschedpolicy (rt_attributes, SCHED_OTHER)) 
		printf("Cannot set SCHED_OTHER scheduling attributes for RT thread\n");
    
    if (pthread_attr_setscope (rt_attributes, PTHREAD_SCOPE_SYSTEM)) 
		printf("Cannot set scheduling scope for RT thread\n");

    rt_param->sched_priority = 0;
    if (pthread_attr_setschedparam (rt_attributes, rt_param)) 
		printf("Cannot set scheduling priority for RT thread\n");
	
	pthread_attr_getschedpolicy(rt_attributes, &thread_policy);
	pthread_attr_getschedparam(rt_attributes, &thread_param);

	ts.tv_sec = 0;
	ts.tv_nsec = 500 * M_SEC;
	for (i = 0; i < 6; i++ ) {
		nanosleep(&ts, NULL);
		fprintf(stdout, ".");
		fflush(stdout);	
	}
	fprintf(stdout, "\n");
	fflush(stdout);		
	
	system("echo 1 > /proc/sys/net/ipv4/tcp_tw_recycle");

	app_version_init();
	
	syslog_record(SYSLOG_BOOT_ON);

	printf("\n\n");
	printf("  ######          ##                               \n");
	printf(" ##   ###         ##    ###                       \n");
	printf(" ##     ##        ##  ###                         \n");
	printf(" ##     ## ##  ## ## ##    #####  ####  ######  \n");
 	printf(" ##     ## ##  ## ###     ##     ##  ## ##   ##  \n");
	printf(" ##     ## ##  ## ####     ##### ###### ##   ##   \n");
	printf(" ##    ##  ##  ## ##  ##      ## ##  ## ##   ##   \n");
	printf(" #######   ###### ##    ## ##### ##  ## ##   ##   \n");
	
	// duksan application.
	printf("== Initialize ================================================\n");	
	if ( queue_init() < 0 )			
		return ERROR;
	fprintf(stdout, "Queue Initialize \t\t\t\t [  OK  ]\n");
	fflush(stdout);

	if ( file_access_init() < 0 )		
		return ERROR;
	fprintf(stdout, "File Initialize \t\t\t\t [  OK  ]\n");
	fflush(stdout);

	if ( get_my_pcm() < 0 )			
		return ERROR;
	fprintf(stdout, "Pcm Initialize \t\t\t\t\t [  OK  ]\n");
	fflush(stdout);

	if ( point_file_init() < 0 )		
		return ERROR;
	fprintf(stdout, "Point Initialize \t\t\t\t [  OK  ]\n");
	fflush(stdout);

	//pnt_observer_check();
	if ( calibration_init() < 0 ) 
		return ERROR;
	fprintf(stdout, "Calibration Initialize \t\t\t\t [  OK  ]\n");
	fflush(stdout);

	if ( point_table_init() < 0 )		
		return ERROR;
	fprintf(stdout, "PTBL Initialize \t\t\t\t [  OK  ]\n");
	fflush(stdout);

	if ( -1 == ( shm_id = shmget( (key_t)KEY_NUM, SHARE_MEM_SIZE, IPC_CREAT|0666))) {
		fprintf(stdout, "Share Memory Initialize \t\t\t [  FAIL  ]\n");
		fflush(stdout);	
		return -1;
	} 
	else {
		fprintf(stdout, "Share Memory Initialize \t\t\t [  OK  ]\n");
		fflush(stdout);		
	}
	
	if ( ( void *)-1 == ( shm_addr = shmat( shm_id, ( void *)0, 0))) {
		fprintf(stdout, "Share Memory Add \t\t\t\t [  FAIL  ]\n");
		fflush(stdout);	
		return -1;
	} 
	else {
		fprintf(stdout, "Share Memory Add \t\t\t\t [  OK  ]\n");
		fflush(stdout);		
	}
	//fprintf(stdout, "Stat Initialize \t\t\t\t [  OK  ]
	//fflush(stdout);
	//if ( stat_init() < 0 )		
	//	return ERROR;
	
	if ( bacnet_file_init() < 0 )		
		return ERROR;
	fprintf(stdout, "BACnet File Initialize \t\t\t\t [  OK  ]\n");
	fflush(stdout);
	printf("\n");

	// create thread.
	printf("== Thread ====================================================\n");	
	nIndexThread = 0;
	
	printf("Default policy is %s, priority is %d\n", 
			(thread_policy == SCHED_FIFO ? "FIFO"
			 : (thread_policy == SCHED_RR ? "RR"
				: (thread_policy == SCHED_OTHER ? "OTHER"
					: "unknown"))), thread_param.sched_priority);	
	
	if ( get_thread_opt("net32", 5) == SUCCESS ) {
		fprintf(stdout, "NET32 Mgr \t\t\t\t\t [  OK  ]\n");
		fflush(stdout);
		th_id = pthread_create(
					&g_sThread[nIndexThread], 
					rt_attributes,
					(void*(*)(void*)) net32_main, 
					(void*) &g_sThread[nIndexThread]);
		nIndexThread++;

		// g_nMultiDdcFlag is 0 ==> Net32 Mode ( USE NET32 )
		// g_nMultiDdcFlag is 1 ==> Multi_ddc Mode ( NOT USE NET32 )
		g_nMultiDdcFlag = 0;
	}
	else {
		fprintf(stdout, "NET32 Mgr \t\t\t\t\t [ FAIL ]\n");
		fflush(stdout);

		// g_nMultiDdcFlag is 0 ==> Net32 Mode ( USE NET32 )
		// g_nMultiDdcFlag is 1 ==> Multi_ddc Mode ( NOT USE NET32 )
		g_nMultiDdcFlag = 1;
	}
	sleep(1);

	if ( get_thread_opt("obsvr", 5) == SUCCESS ) {
		fprintf(stdout, "OBSVR Mgr \t\t\t\t\t [  ON  ]\n");
		fflush(stdout);
		th_id = pthread_create(&g_sThread[nIndexThread], 
					rt_attributes,
					(void*) point_observer_main, 
					(void*) &g_sThread[nIndexThread]);
		nIndexThread++;
	}
	else {
		fprintf(stdout, "OBSVR Mgr \t\t\t\t\t [ FAIL ]\n");
		fflush(stdout);
	}
	sleep(1);
	
	if ( get_thread_opt("point", 5) == SUCCESS ) {
		fprintf(stdout, "POINT Mgr \t\t\t\t\t [  ON  ]\n");
		fflush(stdout);		
		th_id = pthread_create(&g_sThread[nIndexThread], 
					rt_attributes,
					(void*) physical_pnt_mgr_main, 
	               	(void*) &g_sThread[nIndexThread]);	
		nIndexThread++;
	}
	else {
		fprintf(stdout, "POINT Mgr \t\t\t\t\t [ FAIL ]\n");
		fflush(stdout);		
	}
	sleep(1);

	if ( get_thread_opt("cmd", 3) == SUCCESS ) {
		fprintf(stdout, "CMD Mgr \t\t\t\t\t [  ON  ]\n");
		fflush(stdout);
		th_id = pthread_create(
					&g_sThread[nIndexThread], 
					rt_attributes,
					(void*) command_handler_main, 
					(void*) &g_sThread[nIndexThread]); 
		nIndexThread++; 	
	}
	else {
		fprintf(stdout, "CMD Mgr \t\t\t\t\t [ FAIL ]\n");
		fflush(stdout);
	}
	sleep(1);

	if ( get_thread_opt("light", 5) == SUCCESS ) {
		fprintf(stdout, "Light Mgr \t\t\t\t\t [  ON  ]\n");
		fflush(stdout);
		th_id = pthread_create( &g_sThread[nIndexThread], 
								rt_attributes,
								(void*(*)(void*)) IfaceHT_Main,
								(void*) &g_sThread[nIndexThread]);
		nIndexThread++;
	}
	else {
		fprintf(stdout, "Light Mgr \t\t\t\t\t [ FAIL ]\n");
		fflush(stdout);
	}
	sleep(1);

	// Interface DBSVR
	if ( get_thread_opt("dbsvr", 5) == SUCCESS ) {
		printf("+ [MAIN] Database Manager Run \n");
		th_id = pthread_create( &g_sThread[nIndexThread], 
								rt_attributes,
								(void*(*)(void*)) dbsvr_main,
								(void*) &g_sThread[nIndexThread]);
		nIndexThread++;
	}	

	// Interface MODBUS
	if ( get_thread_opt("mdsvr", 5) == SUCCESS ) {
		printf("+  [MAIN] MODBUS Server Run\n");
		th_id = pthread_create(
					&g_sThread[nIndexThread], 
					rt_attributes,
					(void*) modsvr_main, 
					(void*) &g_sThread[nIndexThread]);  
		nIndexThread++;
	}
	
	if ( get_thread_opt("subio", 5) == SUCCESS ) {
		printf("+ SUBIO thread created\n");
		th_id = pthread_create(
					&g_sThread[nIndexThread], 
					rt_attributes, 
					(void*(*)(void*)) subio_main, 
					(void*) &g_sThread[nIndexThread]);
		nIndexThread++;
	}

	/*
	if( get_bacnetInfo() == SUCCESS ) {
		printf("+ BACnet readprop thread created\n");
		th_id = pthread_create(
					&g_sThread[nIndexThread], 
					rt_attributes, 
					(void*(*)(void*)) bacnet_readprop_main, 
					(void*) &g_sThread[nIndexThread]);
		nIndexThread++;
	}
	*/

	if ( get_thread_opt("apg", 3) == SUCCESS ) {
		fprintf(stdout, "APG Mgr \t\t\t\t\t [  ON  ]\n");
		fflush(stdout);
		th_id = pthread_create(
					&g_sThread[nIndexThread], 
					rt_attributes,
					(void*) apg_handler_main, 
					(void*) &g_sThread[nIndexThread]);  
		nIndexThread++;
	}
	else {
		fprintf(stdout, "APG Mgr \t\t\t\t\t [ FAIL ]\n");
		fflush(stdout);
	}
	sleep(1);//

	// hong univ ghp interface
	if ( get_thread_opt("t_hong", 6) == SUCCESS ) {
		fprintf(stdout, "GHP Sched \t\t\t\t\t [  ON  ]\n");
		fflush(stdout);
		th_id = pthread_create(&g_sThread[nIndexThread], 
								rt_attributes, 
								(void*(*)(void*)) schedule_main, 
								(void*) &g_sThread[nIndexThread]);
		nIndexThread++;
		sleep(1);

		fprintf(stdout, "GHP Mgr \t\t\t\t\t [  ON  ]\n");
		fflush(stdout);
		th_id = pthread_create(&g_sThread[nIndexThread], 
								rt_attributes, 
								(void*) iface_ghp_main, 
								(void*) &g_sThread[nIndexThread]);
		nIndexThread++;
		sleep(1);
		
		fprintf(stdout, "GHP Web2 \t\t\t\t\t [  ON  ]\n");
		fflush(stdout);
		th_id = pthread_create(&g_sThread[nIndexThread], 
								rt_attributes, 
								(void*(*)(void*)) web2server_main, 
								(void*) &g_sThread[nIndexThread]);
		nIndexThread++;
		sleep(1);
	}

#if 0
/*
	if( get_smsInfo() == SUCCESS ) {
		printf("+ SMS Interface thread created\n");
		th_id = pthread_create(
					&g_sThread[nIndexThread], 
					rt_attributes, 
					(void*(*)(void*)) IfaceSms_Main, 
					(void*) &g_sThread[nIndexThread]);
		nIndexThread++;
	}
	*/
/*
	printf("+ DNP Interface thread created\n");
	th_id = pthread_create(
				&g_sThread[nIndexThread], 
				rt_attributes,
				(void*) dnp_observer_main, 
			   	(void*) &g_sThread[nIndexThread]);  
	nIndexThread++;

	printf("+ Net32toDNP Interface thread created\n");
	th_id = pthread_create(
				&g_sThread[nIndexThread], 
				rt_attributes,
				(void*) net32_to_dnp_main, 
			   	(void*) &g_sThread[nIndexThread]);  
	nIndexThread++;
*/
	/*
	printf("+ Interface_Handler_Main thread created\n");
	th_id = pthread_create(
				&g_sThread[nIndexThread], 
				rt_attributes,
				(void*) ifh_main, 
			   	(void*) &g_sThread[nIndexThread]);  
	nIndexThread++;
	*/


	// interface GHP
	/* 
	// daegu univ
	printf("+ GHP Interface.\n");
	th_id = pthread_create(&g_sThread[nIndexThread], 
							rt_attributes, 
							(void*) iface_ghp_main, 
	 						(void*) &g_sThread[nIndexThread]);
							nIndexThread++;
	*/

	// cnue
	/*
	printf("+ GHP Interface.\n");
	th_id = pthread_create(&g_sThread[nIndexThread], 
							rt_attributes, 
							(void*) iface_ghp_main, 
	 						(void*) &g_sThread[nIndexThread]);
							nIndexThread++;

	printf("+ GHP Web1 Interface.\n");
	th_id = pthread_create(&g_sThread[nIndexThread], 
							rt_attributes, 
							(void*(*)(void*)) webserver_main, 
	 						(void*) &g_sThread[nIndexThread]);
							nIndexThread++;

	printf("+ GHP Schedule Web Interface.\n");
	th_id = pthread_create(&g_sThread[nIndexThread], 
							rt_attributes, 
							(void*(*)(void*)) schedule_main, 
	 						(void*) &g_sThread[nIndexThread]);
							nIndexThread++;
	*/
				

							   	
	// interface PLC
	/*
	if (plc_file_init() < 0)		
		return ERROR;		

	printf("+ Substation PLC Interface.\n");
	th_id = pthread_create( &g_sThread[nIndexThread], 
							rt_attributes, 
							(void*(*)(void*)) plc_observer_main, 
	 						(void*) &g_sThread[nIndexThread]);
							nIndexThread++;
	*/


	/*
	// interface MDTD Server
	printf("+ MDTD Server Interface thread create \n");
	th_id = pthread_create( &g_sThread[nIndexThread], 
							rt_attributes,
							(void*(*)(void*)) Iface_Msvr_Main,
							(void*) &g_sThread[nIndexThread]);
	nIndexThread++;
	*/

	// Interface MODBUS client (Peak control)
	/*
	printf("+ PEAK MODBUS CLIENT Interface\n");
	th_id = pthread_create( &g_sThread[nIndexThread], 
							rt_attributes,
							(void*(*)(void*)) iface_peak_main,
	 						(void*) &g_sThread[nIndexThread]);
							nIndexThread++;
	*/	

	/*
	if ( get_ccmsInfo() == SUCCESS ) {
		// CCMS MODE에서 0번 PCM은 항상 서버로 동작한다. 
		if ( g_nMyPcm == 0) {
			g_nModeCCMS = CCMS_SERVER;
			// Interface CCMS
			printf("+ CCMS Interface\n");
			th_id = pthread_create( &g_sThread[nIndexThread], 
									rt_attributes,
									(void*(*)(void*)) ccms_main,
									(void*) &g_sThread[nIndexThread]);
			nIndexThread++;
	
				// Interface CCMS
			printf("+ CCMS MODEM Interface\n");
			th_id = pthread_create( &g_sThread[nIndexThread], 
									rt_attributes,
									(void*(*)(void*)) ccms_modem_main,
									(void*) &g_sThread[nIndexThread]);
			nIndexThread++;
		}
		else {
			g_nModeCCMS = CCMS_CLIENT;
			// Interface CCMS Client
			printf("+ CCMS Client Interface\n");
			th_id = pthread_create( &g_sThread[nIndexThread], 
									rt_attributes,
									(void*(*)(void*)) ccms_client_main,
									(void*) &g_sThread[nIndexThread]);
			nIndexThread++;
			
			if ( g_nMyPcm == 2) {
				// Interface CCMS Client
				printf("+ CCMS Modem Client Interface\n");
				th_id = pthread_create( &g_sThread[nIndexThread], 
										rt_attributes,
										(void*(*)(void*)) IfaceModem_Main,
										(void*) &g_sThread[nIndexThread]);
				nIndexThread++;		
			}	
		}
	}

	// Interface CCMS Html Manager
	printf("+ CCMS FILE Interface\n");
	th_id = pthread_create( &g_sThread[nIndexThread], 
							rt_attributes,
							(void*(*)(void*)) ccms_mgr_main,
							(void*) &g_sThread[nIndexThread]);
	nIndexThread++;
	*/
#endif

	if ( get_thread_opt("target", 6) == SUCCESS ) {
		fprintf(stdout, "ELBA Mgr \t\t\t\t\t [  ON  ]\n");
		fflush(stdout);
		th_id = pthread_create(
					&g_sThread[nIndexThread], 
					rt_attributes,
					(void*(*)(void*)) elba_main, 
					(void*) &g_sThread[nIndexThread]);
		nIndexThread++;
	}
	else {
		fprintf(stdout, "ELBA Mgr \t\t\t\t\t [ FAIL ]\n");
		fflush(stdout);
		fp = fopen("/duksan/FILE/elba", "w");
		if( fp != NULL ) {
			fprintf(fp,	"Elba Count\n" );
			fprintf(fp,	"+ STATUS : Not Used\n"); 
			fclose(fp);
		}	
	}
	sleep(1);

	// Test Version의 코드이기 때문에 사용하지 않습니다. 
	/*
	fprintf(stdout, "USERVER Mgr \t\t\t\t\t [  ON  ]\n");
	fflush(stdout);
	th_id = pthread_create(
				&g_sThread[nIndexThread], 
				rt_attributes,
				(void*(*)(void*)) userver_main, 
				(void*) &g_sThread[nIndexThread]);
	nIndexThread++;
	sleep(1);
	
	fprintf(stdout, "UCLIENT Mgr \t\t\t\t\t [  ON  ]\n");
	fflush(stdout);
	th_id = pthread_create(
				&g_sThread[nIndexThread], 
				rt_attributes,
				(void*(*)(void*)) uclient_main, 
				(void*) &g_sThread[nIndexThread]);
	nIndexThread++;			
	sleep(1);
	*/
	
	/*
	fprintf(stdout, "WATCHDOG Mgr \t\t\t\t\t [  ON  ]\n");
	fflush(stdout);
	th_id = pthread_create(
				&g_sThread[nIndexThread], 
				rt_attributes,
				(void*(*)(void*)) watchdog_main, 
				(void*) &g_sThread[nIndexThread]);
	nIndexThread++;			
	sleep(1);	
	*/

	printf("\n");
	printf("== Boot =====================================================\n");	
	nIndexThread = 0;
	printf("[MAIN] App Version %s \n", g_chAppVersion);		
	printf("[MAIN] My Id %d \n", g_nMyPcm);		

	// g_nMultiDdcFlag is 0 ==> Net32 Mode ( USE NET32 )
	// g_nMultiDdcFlag is 1 ==> Multi_ddc Mode ( NOT USE NET32 )	
	if( g_nMultiDdcFlag == 0 ) {
		printf("[MAIN] Net32 on \n");		
	}
	else {
		printf("[MAIN] Net32 off \n");		
	}
	
	// net32에 자신이 부팅되었음을 알린다.
	bootup_data();	
	
	ts.tv_sec = 0;
	ts.tv_nsec = 100 * M_SEC;

		
	// open watchdog
	printf("open watchdog\n");
	nWatchdogDev = open( "/dev/watchdog",  O_WRONLY );
	
	//nWatchdogTime = 3;
	//ioctl( nWatchdogDev, WDIOC_SETTIMEOUT, &nWatchdogTime );
	
	// 항상 main thread를 살려 놓기 위하여 추가한다. 
	for(;;) {
		ioctl( nWatchdogDev, WDIOC_KEEPALIVE, NULL );						// Watchdog reload
		
		nWatchdogTime = 3;
		ioctl( nWatchdogDev, WDIOC_SETTIMEOUT, &nWatchdogTime );			// set watchdog
		
		// check watchdog
		if ( ioctl(nWatchdogDev, WDIOC_GETTIMEOUT, &nWatchdogInterval) == 0 ) {
			fprintf(stdout, "Current watchdog interval is %d\n", nWatchdogInterval);
		}		
		
		sched_yield();
		sleep(1);	

		// 1초마다 share memory에 point table의 값을 복사한다.
		time(&tm_nd);
		tm_ptr = localtime(&tm_nd);
		if ( nPreSec != tm_ptr->tm_sec ) {
			nPreSec = tm_ptr->tm_sec;
			pthread_mutex_lock( &pointTable_mutex );
			memcpy((float *)shm_addr, &g_fExPtbl, SHARE_MEM_SIZE );		// 공유메모리 복사
			pthread_mutex_unlock( &pointTable_mutex );
		}			
	}

	pthread_join(g_sThread[0], (void **)th_status);
	//printf("1 th_status = %d \n", th_status);			
	pthread_join(g_sThread[1], (void **)th_status);		
	//printf("2 th_status = %d \n", th_status);			
	pthread_join(g_sThread[2], (void **)th_status);		
	//printf("3 th_status = %d \n", th_status);			
	pthread_join(g_sThread[3], (void **)th_status);		
	//printf("4 th_status = %d \n", th_status);			
	pthread_join(g_sThread[4], (void **)th_status);		
	//printf("5 th_status = %d \n", th_status);			
	pthread_join(g_sThread[5], (void **)th_status);		
	//printf("6 th_status = %d \n", th_status);			
	pthread_join(g_sThread[6], (void **)th_status);		
	//printf("7 th_status = %d \n", th_status);			
	pthread_join(g_sThread[7], (void **)th_status);		
	//printf("8 th_status = %d \n", th_status);			
	pthread_join(g_sThread[8], (void **)th_status);		
	//printf("9 th_status = %d \n", th_status);			
	pthread_join(g_sThread[9], (void **)th_status);		
	//printf("10 th_status = %d \n", th_status);			
	pthread_join(g_sThread[10], (void **)th_status);	
	//printf("11 th_status = %d \n", th_status);			
	pthread_join(g_sThread[11], (void **)th_status);	
	//printf("12 th_status = %d \n", th_status);			
	pthread_join(g_sThread[12], (void **)th_status);	
	//printf("13 th_status = %d \n", th_status);			
	pthread_join(g_sThread[14], (void **)th_status);	
	//printf("14 th_status = %d \n", th_status);			
	pthread_join(g_sThread[15], (void **)th_status);	
	//printf("15 th_status = %d \n", th_status);			
	pthread_join(g_sThread[16], (void **)th_status);	
	//printf("16 th_status = %d \n", th_status);			
		
	printf("Duksan Terminate\n");
	syslog_record(SYSLOG_DESTROY_MAIN);
	sleep(3);
	system("sync");
	system("sync");
	system("sync");	
	system("killall duksan");
		
	return -1;
}

int	queue_init(void)
// ----------------------------------------------------------------------------
// QUEUE INITIALIZE
{
	initq(&elba_queue);
	initq(&net32_queue);
	initq(&bacnet_message_queue);
	initq(&ghp_message_queue);
	initq(&plc_message_queue);
	initq(&apg_queue);
	initq(&ccms_queue);
	initq(&uclient_queue);
	//initq(&ccms_modem_queue);
	
	//printf("+ Success queue initialize\n");
	return SUCCESS;
}


int file_access_init(void)
// ----------------------------------------------------------------------------
// FILE ACCESS VARIABLE INITIALIZE
{
	int i = 0;

	for (i = 0; i < MAX_NET32_NUMBER; i++) 
		g_nFileAccess[i] = 0;

	//printf("+ Success file-access initialize\n");
	return SUCCESS;
}


int calibration_init(void)
// ----------------------------------------------------------------------------
// CALIBRATION TABLE INITIALIZE
// Description : calibration structurer initialize.
// Arguments   : none
// Returns     : SUCCESS		always return SUCCESS
{
	FILE *pf;
	int n_cal_size = 0;
	
	// 32개의 물리적 포인트를 가지고 있기 때문에 * 32를 한다. 
	n_cal_size = sizeof(CAL_INFO_T) * 32;
	
	g_pCal = malloc(n_cal_size);	
	memset( g_pCal, 0x00, n_cal_size );

	if ( ( pf = fopen("/duksan/DATA/PointCalibration.dat", "r") ) == NULL ) {

		if ( ( pf = fopen("/duksan/DATA/PointCalibration.dat", "w") ) == NULL ) {
			fprintf( stderr, "[ERROR] PointCalibration.dat Open Error\n");
			fflush(stderr);
			return ERROR;
		}
		else {
			fwrite( g_pCal, n_cal_size, 1, pf );
		}

		fclose(pf);
		//fprintf( stderr, "+ Success point-calibration initialize (size %d)\n", n_cal_size );
		//fflush(stderr);
		return SUCCESS;
	}
	else {

		if ( fread( g_pCal, n_cal_size, 1, pf ) == 1 ) {
			fclose(pf);
			//fprintf( stderr, "+ Success point-calibration initialize (size %d)\n", n_cal_size );
			//fflush(stderr);
			return SUCCESS;
		} 
		else {
			fclose(pf);
			fprintf( stderr, "[ERROR] PointCalibration.dat read Error\n");
			fflush(stderr);
			return ERROR;
		}
	}
}

/*
int stat_init(void)
// ----------------------------------------------------------------------------
// STAT INITIALIZE
// Description : stat값을 초기화 한다. 
// Arguments   : none
// Returns     : SUCCESS		always return SUCCESS
{
	int n_size = 0;

	n_size = sizeof(CHK_STATUS_T);
	g_pStatus = malloc(n_size);	
	memset( g_pStatus, 0x00, n_size );

	return SUCCESS;
}
*/

int point_table_init(void)
// ----------------------------------------------------------------------------
// POINT TABLE INITIALIZE
// Description : point-table structurer initialize.
// Arguments   : none
// Returns     : SUCCESS		always return SUCCESS
{
	FILE *pf;
	int i = 0;
	int n_ptbl_size = 0;
	n_ptbl_size = sizeof(PTBL_INFO_T) * MAX_POINT_NUMBER;
	
	g_pPtbl = malloc(n_ptbl_size);	
	memset( g_pPtbl, 0x00, n_ptbl_size );

	if ( ( pf = fopen("/duksan/DATA/PointDefine.dat", "r") ) == NULL ) {

		for ( i = 0; i < MAX_POINT_NUMBER; i++ ) {
			g_pPtbl[i].n_type = DFN_PNT_VR;
			g_pPtbl[i].n_adc = 0;
			g_pPtbl[i].f_min = -999999;
			g_pPtbl[i].f_max = 999999;
			g_pPtbl[i].f_offset = 0;
			g_pPtbl[i].f_scale = 1;
			g_pPtbl[i].f_hyst = 1;
			pthread_mutex_lock( &pointTable_mutex );
			g_pPtbl[i].f_val = g_fExPtbl[g_nMyPcm][i];
			pthread_mutex_unlock( &pointTable_mutex );
			g_pPtbl[i].f_preval = 0;
		}

		if ( ( pf = fopen("/duksan/DATA/PointDefine.dat", "w") ) == NULL ) {
			fprintf( stderr, "[ERROR] PointDefine.dat Open Error\n");
			fflush(stderr);
			return ERROR;
		}
		else {
			fwrite( g_pPtbl, n_ptbl_size, 1, pf );
		}

		fclose(pf);
		//fprintf( stderr, "+ Success point-table initialize (size %d)\n", n_ptbl_size);
		//fflush(stderr);
		return SUCCESS;
	}
	else {

		if ( fread( g_pPtbl, n_ptbl_size, 1, pf ) == 1 ) {
			// 기존에 저장된 값을 복구한다. 
			for ( i = 0; i < MAX_POINT_NUMBER; i++ ) {
				pthread_mutex_lock( &pointTable_mutex );
				g_pPtbl[i].f_preval = g_fExPtbl[g_nMyPcm][i];
				g_pPtbl[i].f_val = g_fExPtbl[g_nMyPcm][i];
				pthread_mutex_unlock( &pointTable_mutex );
			}
			
			fclose(pf);

			//fprintf( stderr, "+ Success point-table read (size %d)\n", n_ptbl_size);
			//fflush(stderr);
			return SUCCESS;
		} 
		else {
			fclose(pf);

			fprintf( stderr, "[ERROR] PointDefine.dat read Error\n");
			fflush(stderr);
			return ERROR;
		}
	}
}


int point_file_init(void)
// ----------------------------------------------------------------------------
// POINT FILE INITIALIZE
{
	int i, j;
	int pno;
	float value;
	FILE *fp;
	char filename[32];
	char index[2];
	int init_status;
	
	//Initialize
	i = j   = 0;
	pno	 	= 0;
	value   = 0;
	fp      = NULL;
	strncpy(filename, "/duksan/DATA/point00.dat\0", sizeof(filename));
	index[0] = index[1] = 0;
	init_status = MAIN_GET_FILE_NAME;
	
	//mkdir
	mkdir("/duksan/DATA", 00777);
	
	while(1) {
		switch(init_status) {
		case MAIN_GET_FILE_NAME:
			index[0] = (i / 10) + 48;
			index[1] = (i % 10) + 48;
			strncpy(&filename[18], index, sizeof(index));
			init_status = MAIN_FILE_READWRITE_OPEN;
			break;

		case MAIN_FILE_READWRITE_OPEN:
			if((fp = fopen(filename, "r+")) == NULL)
				init_status = MAIN_FILE_CREATE;
			else
				init_status = MAIN_FILE_READ;
			break;

		case MAIN_FILE_CREATE:
			if((fp = fopen(filename, "w")) == NULL) {
				printf("[ERROR] File Open with Option 'w' In main.c\n");
				return ERROR;
			}
			else {
				for(j = 0; j < MAX_POINT_NUMBER; j++) {
					pthread_mutex_lock( &pointTable_mutex );
					g_fExPtbl[i][j] = 0;
					fwrite(&g_fExPtbl[i][j], sizeof(float), 1, fp);
					pthread_mutex_unlock( &pointTable_mutex );
				}
			}
			fclose(fp);
			init_status = MAIN_INCREASE_INDEX;
			break;

		case MAIN_FILE_READ:
			for(j = 0; j < MAX_POINT_NUMBER; j++) {
				if(fread(&value, sizeof(float), 1, fp) == 1) {
					pthread_mutex_lock( &pointTable_mutex );	
					g_fExPtbl[i][j] = value;
					pthread_mutex_unlock( &pointTable_mutex );
				}
				else {
					//file may have been corrupted, so make new one.
					fclose(fp);
					init_status = MAIN_FILE_CREATE;
					break;
				}
			}
			//
			fclose(fp);
			init_status = MAIN_INCREASE_INDEX;
			break;
			
		case MAIN_INCREASE_INDEX:
			i = i + 1;
			//
			if(i < MAX_NET32_NUMBER)
				init_status = MAIN_GET_FILE_NAME;
			else
				init_status = MAIN_FINISH_INIT_PROCESS;
			break;

		case MAIN_FINISH_INIT_PROCESS:
			//printf("+ Success pcm-file-data initialize\n");
			return SUCCESS;
		}
	}

	printf("Failed in %s()\n", __FUNCTION__);
	return ERROR;
}


bacnetFileInfo bacnetFile[65536];

int bacnet_file_init(void)
// ----------------------------------------------------------------------------
// BACNET POINT FILE INITIALIZE
{
	FILE *fp;
	char text[4][12];
	int cnt = 0;
	
	memset(&text, 0x00, sizeof(text));
	for(;;) {
		if((fp = fopen("/duksan/DATA/PointTable.dat", "r")) == NULL) {
			printf("Can't open file /duksan/DATA/PointTable.dat \n");
			sleep(1);
			return ERROR;				
		}	
	
		while(fscanf(fp, "%s %s %s %s\n",
						(char *)&text[0],
						(char *)&text[1],
						(char *)&text[2],
						(char *)&text[3]) != EOF) {
			bacnetFile[cnt].pno = atoi(text[0]);
			bacnetFile[cnt].type = atoi(text[1]);
			bacnetFile[cnt].device = atoi(text[2]);
			bacnetFile[cnt].unit = atoi(text[3]);
			/*
			printf("%d, %d, %d, %d\n", 
						bacnetFile[cnt].pno,
						bacnetFile[cnt].type,
						bacnetFile[cnt].device,
						bacnetFile[cnt].unit);
			*/
			cnt++;
		}

		fclose(fp);
		bacnetObjCnt = cnt;
		return SUCCESS;
	}	
}


int plc_file_init(void)		
// ----------------------------------------------------------------------------
// PLC POINT FILE INITIALIZE
{
	FILE *fp;
	char text[4][12];
	int cnt = 0;
	
	memset(&text, 0x00, sizeof(text));
	memset(&plcData,  0x00, sizeof(plcData));	
	
	for(;;)	{
		if((fp = fopen("/duksan/DATA/PlcPoint.txt", "r")) == NULL) {
			printf("Can't open file /duksan/DATA/PlcPoint.txt \n");
			sleep(1);
			return ERROR;				
		}	
	
		while(fscanf(fp, "%s %s %s %s\n",
						(char *)&text[0],
						(char *)&text[1],
						(char *)&text[2],
						(char *)&text[3]) != EOF) {
			plcData[cnt].type = atoi(text[0]);
			plcData[cnt].addr = atoi(text[1]);
			plcData[cnt].pcm = atoi(text[2]);
			plcData[cnt].pno = atoi(text[3]);
			cnt++;
		}

		fclose(fp);
		return SUCCESS;
	}	
}


void Make_Prompt_Shell(int pcm)
// ----------------------------------------------------------------------------
// PROMPT FILE INITIALIZE
{
	FILE *fp = NULL;

	if( (fp = fopen("/etc/rc.d/rc.hostname", "w")) == NULL ) {
		printf("[ERROR] File Open with Option 'r'\n");
		return;		
	}
	else {
		fprintf(fp,"#!/bin/sh\n");
		fprintf(fp,"/bin/hostname \'[PCM%02d]\'\n", pcm);
		fprintf(fp,"chmod 777 / \n");
		fclose(fp);
	}

}
	

int get_my_pcm(void)
// ----------------------------------------------------------------------------
// GET MY_PCM NUMBER
{
	
	int i = 0;
	int n_fd = -1;
	int n_ret = 0;
	PNT_DEV_MSG_T *p_msg;
	unsigned char c_buf[12];
	
	// open device
	n_fd = open( DEV_FILENAME, O_RDWR );					

	if ( n_fd < 0) {
		// g_nMyPcm number set zero. and prompt change
		g_nMyPcm = 0;
		Make_Prompt_Shell(g_nMyPcm);
		fprintf(stdout, "[ERROR] GPIO Driver open error\n"); 
		fprintf(stdout, "Set Pcm Number = %d\n", g_nMyPcm); 
		fflush(stdout);
		return SUCCESS;
	}
	
	// initialize
	memset( c_buf, 0x00, sizeof(c_buf) );
	p_msg = (PNT_DEV_MSG_T *) &c_buf;

	// make message
	p_msg->c_pno = 0;
	p_msg->c_type = DFN_PNT_ID;
	p_msg->c_length = (char)sizeof(PNT_DEV_MSG_T);
	p_msg->c_chksum = 0;
	for( i = 0; i < (p_msg->c_length - 1); i++) {
		p_msg->c_chksum -= c_buf[i];
	}

	// wirte message
	write( n_fd, c_buf, 8 );

	// buffer initialize and read message
	memset( c_buf, 0x00, sizeof(c_buf) );
	n_ret = read( n_fd, c_buf, 8 );

	// set g_nMyPcm number. and prompt change
	g_nMyPcm = p_msg->n_val;
	//printf("+ Success g_nMyPcm initialize (%d)\n", g_nMyPcm);
	Make_Prompt_Shell(g_nMyPcm);

	return SUCCESS;				
}


int get_thread_opt(char *pThreadName, int nLength)
// ----------------------------------------------------------------------------
// CHECK THREAD OPTION
{
	FILE* fp;
	int cnt = 0;

	//Initialize
	fp = NULL;
	memset(g_sCmdArgs, 0, sizeof(g_sCmdArgs));
	
	// file open
	if((fp = fopen("/duksan/CONFIG/config.dat", "r")) == NULL) {
		if (fp != NULL)  
			fclose(fp);
		printf("[ERROR] File Open with Option 'r'\n");		

		return ERROR;
	}
	else {
		// search file.
		while( !feof(fp) ) {
			fscanf(fp, "%s %s\n", 
				(char *)&g_sCmdArgs[cnt].name, 
				(char *)&g_sCmdArgs[cnt].value);	
			if (strncmp((char *)&g_sCmdArgs[cnt].name, pThreadName, nLength) == 0) {
				if (strncmp((char *)&g_sCmdArgs[cnt].value, "on", 2) == 0)	{
					if (fp != NULL)  
						fclose(fp);		
					return SUCCESS;							
				}
			}
			else
				cnt++;
		}				
	}
		
	if (fp != NULL)  
		fclose(fp);

	return ERROR;		
}


void bootup_data(void)
// ----------------------------------------------------------------------------
// PHYSICAL VALUE SEND TO NET32 NETWORK
{
	int n_pno;
	point_info point;

	for ( n_pno = 0; n_pno < 32; n_pno++ ) {
		point.pcm = g_nMyPcm;
		point.pno = n_pno;
		point.value = g_pPtbl[n_pno].f_val;
		point.message_type = NET32_TYPE_REPORT;

		net32_push_queue(&point);	
	}
	
	fprintf( stderr, "[MAIN] Send Bootup message\n");
	fflush(stderr);	
}


void app_version_init(void)
// ----------------------------------------------------------------------------
// SET APPLICATION VERSION NUMBER
{
	FILE* fp;

	//Initialize
	fp = NULL;
	
	// file open
	if((fp = fopen("/duksan/FILE/app_version", "w")) == NULL) {
		printf("[MAIN] 'app_version' File Open Error\n");		
	}
	else {
		fprintf(fp, "%d", g_nAppVersion);
		fclose(fp);		
	}
}


