#define MAX_BUFFER_SIZE			1024
#define COMMAND_SERVER_PORT 	1004

#define INIT_PROCESS			0
#define SELECT_PROCESS			1
#define CONNECTION_REQUESTED	2
#define HANDLE_COMMAND			3



















void *command_handler_main(void *arg);
int DnpSet_Proc(int nType, int nPno, int nValue);
int DnpInfo_Proc(int fd, int nType, int nPno);


