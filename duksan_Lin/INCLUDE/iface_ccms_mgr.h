#ifndef _IFACE_CCMS_MGR_H
#define _IFACE_CCMS_MGR_H

void		cfile_makefile_control(void);
void		cfile_makefile_status(CCMS_T *pTcp, CCMS_MODEM_T *pModem);
void 		ccms_mgr_sleep(int sec, int msec);
void 		*ccms_mgr_main(void* arg);

#endif
