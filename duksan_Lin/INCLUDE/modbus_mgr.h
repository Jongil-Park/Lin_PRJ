#ifndef		_MODBUS_SVR_H
#define		_MODBUS_SVR_H	


#define CMD_READ                	0x03                
#define CMD_WRITE               	0x10
#define CMD_SINGLE_WRITE        	0x06

#define MDSVR_INIT_SOCK				0
#define MDSVR_ACCEPT_SOCK			1
#define MDSVR_HANDLER_SOCK			2

#define  MDSVR_LOG_BOOTON					0
#define  MDSVR_LOG_CONNECTION_OK			1
#define  MDSVR_LOG_CONNECTION_FAIL			2
#define  MDSVR_LOG_CONNECTION_CLOSE			3
#define  MDSVR_LOG_BIND_ERROR				4
#define  MDSVR_LOG_LISTEN_ERROR				5


#endif
