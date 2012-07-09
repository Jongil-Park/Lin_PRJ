#ifndef APG_API_H
#define APG_API_H	

#include <stdio.h>

extern char _sec;
extern char _minute;

#define byte			unsigned char
#define word			unsigned short

#define INIT_RESUME		0									// point define command.. none으로 처리한다. 

#define b4800			0	
#define b9600			1
#define b19200			2
#define b115200			3

// libraray 에 있는 함수.
/*
extern void lib_sleep(int sec, int msec) ;
extern void initq(point_queue *p);
extern int queue_full(void);
extern int queue_empty(void);
extern int push_lib_queue(point_queue* p1, point_info *p2) ;
extern int pop_lib_queue(point_queue *p1, point_info *p2) ;
extern LIB_T *lib_init(void) ;
extern int lib_open(LIB_T *p) ;
extern void lib_close(LIB_T *p);
extern int lib_recevie_message(LIB_T *p, unsigned char  *pchBuf);
extern void lib_parsing(unsigned char *pchBuf);
extern void lib_handler(LIB_T *p, unsigned char *pchTemp);
extern void lib_send_set_message(LIB_T *p, point_info *pPoint);
extern void lib_send_def_message(LIB_T *p, point_info *pPoint);
extern void lib_send_info_message(LIB_T *p, point_info *pPoint);
extern int main(void);
*/

extern int lib_uart2_open(int nBaudrate);
extern int lib_uart2_send(unsigned char *pTx, unsigned int nLength, int nDebug);
extern int lib_uart2_recv(unsigned char *pRx, unsigned int nLength, int nDebug);
extern int lib_uart2_close(int nBaudrate);

extern void iPset(int n_data, float f_val);
extern float iPget(int n_data) ;

extern void PdefDo2s(byte pcm, byte pno, byte opt);
extern void PdefVo(byte pcm, byte pno, float f_hyst, float f_scale, float f_offset, float f_min, float f_max);
extern void PdefJpt1000(byte pcm, byte pno, float f_hyst, float f_offset, float f_min, float f_max);
extern void PdefDi2s(byte pcm, byte pno);
extern void PdefVi(byte pcm, byte pno, float f_hyst, float f_scale, float f_offset, float f_min, float f_max);
extern void PdefCi(byte pcm, byte pno, float f_hyst, float f_scale, float f_offset, float f_min, float f_max);
extern void PdefVr(byte pcm, byte pno, float f_hyst, float f_min, float f_max);

#endif
