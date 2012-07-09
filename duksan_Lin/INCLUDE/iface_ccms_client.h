#ifndef _IFACE_CCMS_CLIENT_H
#define _IFACE_CCMS_CLIENT_H


void *ccms_client_main(void* arg);
void cclnt_put_queue(point_info *pQueue);
int cclnt_get_queue(point_info *pQueue);
int cclnt_get_status(void);

#endif
