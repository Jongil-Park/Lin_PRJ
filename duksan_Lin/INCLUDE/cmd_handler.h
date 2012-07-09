#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H

#define INIT_PROCESS					1
#define SELECT_PROCESS					2
#define CONNECTION_REQUESTED			3
#define HANDLE_COMMAND					4

#define MAX_BUFFER						512

#define GET_COMMAND_TYPE				11
#define COMMAND_PSET					12
#define COMMAND_PSET_SUCCESS			13
#define COMMAND_PGET_MULTI_POINT		14
#define COMMAND_PGET_SUCCESS			15
#define COMMAND_DEBUG_TCP				16
#define COMMAND_DEBUG_NTX				17
#define COMMAND_DEBUG_NRX				18
#define COMMAND_STATUS					19 
#define COMMAND_PREQ					20
#define COMMAND_MTR						21
#define COMMAND_NODE					22
#define COMMAND_VIEW					23
#define COMMAND_APSET					24
#define COMMAND_PDEF					25
#define COMMAND_CAL						26
#define COMMAND_DEBUG_SUBIO				27
#define COMMAND_RPSET					28
#define COMMAND_HSET					29
#define COMMAND_HGET					30
#define COMMAND_DINFO					31
#define COMMAND_PNT_LOG					32
#define COMMAND_DEBUG_IFACE				33




void *command_handler_main(void* arg);

#endif
