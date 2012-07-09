#ifndef APG_HANDLER_H
#define APG_HANDLER_H

#define APG_FILE_SERVER 		"/tmp/apgSvr"				// server fd
#define APG_CMD_PSET			'S'							// command is pSet
#define APG_CMD_PGET			'G'							// command is pGet
#define APG_CMD_PDEF			'D'							// command is pGet
#define APG_CMD_STARTUP			'I'							// startup
#define APG_CMD_NONE			0							// none.

void apg_sleep(int sec, int msec);
APG_T *new_apg(void);
int apg_open(APG_T *p);
void apg_handler(APG_T *p);
void apg_send_msg(APG_T *p, int n_fd);
int apg_wait_select(APG_T *p);
void *apg_handler_main(void);

#endif
