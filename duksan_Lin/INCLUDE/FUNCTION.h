#ifndef FUNCTION_H
#define FUNCTION_H	

// main.c
//int main(void);
int	queue_init(void);
int file_access_init(void);
int calibration_init(void);
int stat_init(void);
int point_table_init(void);
int point_file_init(void);
int bacnet_file_init(void);
int plc_file_init(void);
void Make_Prompt_Shell(int pcm);
int get_my_pcm(void);
int get_bacnetInfo(void);
int get_subioInfo(void);
int get_dbsvrInfo(void);
int get_lightInfo(void);
int get_net32Info(void);
int get_elbaInfo(void);
int get_apgInfo(void);
int get_mdsvrInfo(void);
int get_ccmsInfo(void);
int get_smsInfo(void);
void bootup_data(void);
void app_version_init(void);


// iface_handler.c
void *iface_handler_main(void *arg);


int BACnet_readprop_main(int argc, char *argv[]);
int BACnet_writeprop_main(int argc, char *argv[]);

int ds_npdu_handler(uint8_t *Buf, uint16_t length);


// net32_mgr.c
void net32_push_queue(point_info *p);
int net32_pop_queue(point_info *p);
//int net32_send_message(int nSockFd, void *p);
void net32_send_message(NET32_T *p);
//int net32_recv_message(int nSockFd, void *p);
int net32_recv_message (NET32_T *p);
void net32_data_handler(void *p) ;
NET32_T *net32_init(void) ;
int net32_open( NET32_T *p ) ;
void net32_save_file(void) ;
void *net32_main(void *arg);


// elba.c
void elba_save_file(void);
void elba_push_queue(point_info *p);
int elba_pop_queue(point_info *p);
void elba_make_tx_message(unsigned char *p, point_info sPoint);
float elba_change_byte_order(float fValue);
int elba_send_message(ELBA_T *p);
unsigned char elba_get_chksum8(void *p, size_t len);
void elba_send_time_message(ELBA_T *p);
int elba_get_station_ipaddr(char* stationip);
ELBA_T *eba_init(void);
void elba_close(ELBA_T *p);
int elba_open(ELBA_T *p);
void elba_parsing (unsigned char *pBuf);
int elba_recv_message(ELBA_T *p);
void *elba_main(void* arg);


// log_mgr.c
void syslog_record(int type);


// point_mrg.c
int pSet(int, int, float);
float pGet(int, int);
void pReq(int, int);
void pDef(void);
void pnt_ismax(point_info *);
void pnt_ismin(point_info *);
int pnt_is_hyst(point_info *);
void pnt_local_pset(point_info *);
void pnt_local_adc(point_info *);
void pnt_local_subio(point_info *);
void *physical_pnt_mgr_main(void* arg);
void pnt_cal_load(void);
void pnt_cal_init(void);
void pnt_cal_save(void);
void pnt_cal_display(void);
void pnt_cal_zero(void);
void pnt_cal_grp(unsigned int n_grp, unsigned int n_type);
void pnt_cal_setvalue(unsigned char *p);
void pnt_loger(int nValue);


// point_observer.c
void file_create_node(void);
void pointObserverSelectSleep(int sec,int msec) ;
int pnt_observer_value_check(void) ;
void httpd_file_check(void);
int getFileAccess(int index);
int releaseFileAccess(int index);
int file_check(void);
int fileStatusCheck(char* filename, int index);
void *point_observer_main(void* arg);


// queue_handler.c
void initq(point_queue *);
void initBacnetq(bacnet_queue *);
int putq(point_queue *, point_info *);
int getq(point_queue *, point_info *);
int queue_full();
int queue_empty();


// dbsvr_mgr.c
DBSVR_T *dbsvr_init(void);
void dbsvr_close(DBSVR_T *p);
int dbsvr_open(DBSVR_T *p);
void dbsvr_make_message(DBSVR_T *p);
int dbsvr_send_message(DBSVR_T *p);
int dbsvr_recv_message(DBSVR_T *p);
void dbsvr_handler(DBSVR_T *p, int nLength);
void *dbsvr_main(void* arg);


// modbus_mgr.c
void mdsvr_log(int type);
int modsvr_handler(int fd, int rxmsg_length);
void *modsvr_main(void* arg);


// iface_hantec.c
void IfaceHT_Sleep(int sec,int msec) ;
int IFaceHT_On_Send(void *p);
int IFaceHT_On_Recv(void *p);
void IFaceHT_On_Recv_Handler(void *pIFaceHT) ;
int IFaceHT_On_Send_Handler(void *pIFaceHT);
int IFaceHT_On_Alive_Handler(void *pIFaceHT);
int IFaceHT_On_Change_SlaveId(void *pIFaceHT);
void IFaceHT_On_Make_Data(void *pIFaceHT);
void IFaceHT_On_Dummy_Handler(void *pIFaceHT) ;
int IFaceHT_On_Observer(void *pIFaceHT);
IfaceHT_T *New_IfaceHT(void);
void IfaceHT_Close(IfaceHT_T *p);
int IfaceHT_Open(IfaceHT_T *p);
void IFaceHT_Guri(void);
void IFaceHT_KimChun(void);
void IFaceHT_OhSong(void);
void IFaceHT_PointSet(void);
int IfaceHT_TimeLine (void);
void IfaceHT_GetConfig(void);
void *IfaceHT_Main(void* arg);


void *userver_main(void* arg);
void *uclient_main(void* arg);
void uclient_push_queue(point_info *p);
int uclient_pop_queue(point_info *p);


//watchdog.c
void *watchdog_main(void* arg);

#endif






