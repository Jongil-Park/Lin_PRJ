#ifndef DNP_OBSERVER_H
#define DNP_OBSERVER_H
//
//
//
// dnp data link function code
#define RESET_LINK            0x00
#define REQUEST_CONFIRM       0x03
#define REQUEST_UNCONFIRM     0x04
//
// dnp application layer function code
#define DNP_CONFIRM           0
#define DNP_READ              1
#define DNP_WRITE             2
#define DNP_DOPERATE          5
//
// dnp Object Library
#define BINARY_INPUT          1
#define BINARY_INPUT_CHANGE   2
#define BINARY_OUTPUT         10
#define CONTROL_BLOCK         12
#define COUNTER_OBJECT        20
#define FROZEN_COUNTER        21
#define COUNTER_CHANGE        22
#define COUNTER_CHANGE_EVENT  23
#define ANALOG_INPUT          30
#define FROZEN_ANALOG_INPUT   31
#define ANALOG_CHANGE_EVENT   32
#define FROZEN_ANALOG_EVENT   33
#define ANALOG_OUTPUT         40
#define ANALOG_OUTPUT_BLOCK   41
#define TIME_DATE             50                        
#define TIME_DATE_CTO         51
#define TIME_DELAY            52
#define CLASS_0123            60
#define FILE_OBJECT           70
#define INTERNAL INDICATIONS  80
#define STORAGE_OBJECT        81                        
#define DEVICE_PROFILE        82
#define PRIVATE_REGISTRATION  83
#define APP_IDENTIFIER        90
#define FLOATION_POINT        100
#define BINARY_CODED_DECIMAL  101                        
//
// dnp addr
#define DNP_MY_ADDR       0x01
//
#define ISspace(x) isspace((int)(x))
//
#define SERVER_STRING "Server: DNP-gateway/1.0\r\n"
//
//extern ADDR_INFO	g_INET_ADDR;
//
//char  *SERVER_ADDR = "211.170.97.69";		/* to be tested */
//
#define DNP_SERVER_PORT	4201
//
#define DNP_BUF_SIZE		1024
//
unsigned short dnp_mkcrc16(unsigned char *p, int len);
void mk_lpdu(int client_sock, unsigned char *apdu, int apdu_length);
void mk_tpdu(int client_sock, unsigned char *apdu, int apdu_length);
void mk_unsol(int client_sock);
void mk_unsol_do(int client_sock);
void mk_unsol_ao(int client_sock);
void check_unsol(int client_sock);
void rd_point(int client_sock, unsigned char *apdu, int apdu_length);
void wr_point(int client_sock, unsigned char *apdu, int apdu_length);
void send_confirm(int client_sock);
void rd_apdu(int client_sock, unsigned char *apdu, int cnt);
void dnp_reset();
void send_ACK(int client_sock);
void rd_lpdu(int client_sock, unsigned char *buf, int cnt);
void get_command(int client_sock, char *recv_sock,int msg_length);
void dnp_thread(int new_sock);
void accept_request(void);
void dnp_observer_main(void* arg);
//
//
#endif
