///******************************************************************/
// file : ghp_ScheduleSvr.h
// date : 2010.03.03.
// author : jong2ry
///******************************************************************/
#ifndef		IFACE_GHP_SCHEDULE_H_
#define		IFACE_GHP_SCHEDULE_H_

#define SCHED_SERVER_PORT					9000

int	schedule_main(void);
int Check_Message(SEND_DATA *pRx);
void schedule_handler(int fd, fd_set* reads);
void SetDate(SEND_DATA *pRx);
void Parsing_Data(SEND_DATA *pRx);

#endif

