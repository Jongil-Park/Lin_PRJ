#ifndef INTERFACE_HANDLER_H
#define INTERFACE_HANDLER

#define INIT_PROCESS			1
#define SELECT_PROCESS			2
#define CONNECTION_REQUESTED	3
#define HANDLE_INTERFACE		4

#define GET_POINT_INFO			11
#define INTERFACE_PSET			12	
#define INTERFACE_PGET			13
#define INTERFACE_PGET_PLURAL	14
#define INTERFACE_ELBA_CMD_GET	15
#define INTERFACE_UNKNOWN		16
#define GET_NEXT_MESSAGE		17

#define EMPTY					-1
#define MAX_ELBA_COMMAND_COUNT	100

void *interface_handler_main(void* arg);
int do_handle_interface(int fd, fd_set* reads);
int getNextMessage(int* rxmsg_length);
int handlePset(int fd, point_info point);
int handlePget(int fd, point_info point);
int handlePgetPlural(int fd, point_info point);
int handleElbaCmdGet(int fd, point_info point);

#endif
