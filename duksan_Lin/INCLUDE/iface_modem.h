#ifndef   _IFACE_MODEM_
#define   _IFACE_MODEM_

void *IfaceModem_Main(void* arg);
void modem_put_queue(point_info *pQueue);
int modem_get_queue(point_info *pQueue);

#endif



