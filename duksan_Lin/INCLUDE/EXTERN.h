#ifndef EXTERN_H
#define EXTERN_H	

// main.c
extern pthread_mutex_t elbaQ_mutex;
extern pthread_mutex_t net32Q_mutex;
extern pthread_mutex_t pointTable_mutex;
extern pthread_mutex_t fileAccess_mutex;
extern pthread_mutex_t bacnetQ_mutex;
extern pthread_mutex_t bacnetAccess_mutex;
extern pthread_mutex_t bacnetMessageQ_mutex;
extern pthread_mutex_t schedule_mutex;
extern pthread_mutex_t plcQ_mutex;
extern pthread_mutex_t ccmsQ_mutex;

extern point_queue elba_queue;										// elba queue ( TCP/IP Networks -> elba_thread )
extern point_queue net32_queue;										// net32 message queue ( net32_thread -> net32 Networks )
extern point_queue bacnet_message_queue;							// bacnet queue. jong2ry.
extern point_queue ghp_message_queue;								// ghp interface jong2ry.
extern point_queue plc_message_queue;								// plc queue. jong2ry.
extern point_queue apg_queue;										// apg queue. jong2ry.
extern point_queue ccms_queue;										// ccms queue. jong2ry.

extern char *g_chDfnTypeName[];										// point-define type name
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];			// point-table only value
extern float g_fPreExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];		// previous point-table vaule
extern PTBL_INFO_T *g_pPtbl;										// point-table
extern CAL_INFO_T *g_pCal;											// calibration table
extern CHK_STATUS_T *g_pStatus;										// status structure
extern int g_nCalReady;												// calibration start flag
extern int g_nCalStatus;											// calibration status
extern int g_nCalGrp;												// calibration group (g0 ~ g3)
extern int g_nCalType;												// calibration type
extern int g_nFileAccess[MAX_NET32_NUMBER];							// file access variable
extern int g_nMyPcm;												// My Pcm Number

extern char g_chAppVersion[]; 										// Version String.
extern int g_nAppVersion;											// Version Number

extern cmdinfo g_sCmdArgs[CFG_COMMAND_COUNT];					// command argument 

extern int g_nMultiDdcFlag;											// net32 mode flag

extern aoPtbl_t aoPtbl[MAX_OBJECT_INSTANCE];
extern aiPtbl_t aiPtbl[MAX_OBJECT_INSTANCE];
extern doPtbl_t doPtbl[MAX_OBJECT_INSTANCE];
extern diPtbl_t diPtbl[MAX_OBJECT_INSTANCE];
extern msoPtbl_t msoPtbl[MAX_OBJECT_INSTANCE];
extern msiPtbl_t msiPtbl[MAX_OBJECT_INSTANCE];
extern int bacnetObjCnt = 0;

extern pthread_t g_sThread[MAIN_CREAT_THREAD_CNT];

extern plcInfo plcData[1024]; 										// plc-interface variable
extern int g_nModeCCMS = 0;

#endif






