#ifndef PLC_OBSERVER_H
#define PLC_OBSERVER_H
//
//
//
#define PLC_CREATE_SOCKET		0
#define PLC_NOT_CONNECTED		1
#define PLC_CONNETED			2
#define PLC_DI_PROC				3
#define PLC_DO_PROC				4
#define PLC_AI_PROC				5
#define PLC_AO_PROC				6
#define PLC_CLOSE_SOCKET				7
//
#define PLC_BUFFER_SIZE			2048
//
#define PLC_SERVER_PORT			2004
//
#define MAX_LINKID              65535
//
#define HEADER_SIZE			    20
#define LOC_LINKID			    14
#define LOC_SOURCEOFFRAME       13
#define LOC_LENGTH			    16
#define LOC_CHECKSUM		    19
//
#define LOC_COMMAND		        20
#define LOC_DATATYPE		    22
#define LOC_BLOCKNUMBER         26
#define LOC_VARLENG	            28
//
#define DNP_DI_SEL      1
#define DNP_DO_SEL      2
#define DNP_AI_SEL      3
#define DNP_AO_SEL      4
//
void PLC_DIO_define(int DIO_sel, short pno, short sAddr, short OnAddr, short OffAddr, unsigned char EnUnsol);
void PLC_AIO_define(int AIO_sel, short pno, short sAddr, float sScale, short sOffset, int sMinVal, int sMaxVal);
void byun_jun_so_kyodae(void);
void PLC_point_define(void);
void MakeHeader(char *pData, char command, char datatype, char blocknumber);
int ReadDIOBlock(int plc_sockfd, long addr, long count, long point, int DIO_sel);
int ReadAIOBlock(int plc_socket, long addr, long count, long point, int AIO_sel);
int WriteDOPoint(int plc_socket, short dnp_pno, unsigned char point);
int WriteAOPoint(int plc_socket, short dnp_pno, float point);
int PLC_write(int plc_socket);
void SleepnWriteCheck(int plc_socket);
void *plc_observer_main(void *arg);
//
//
//
#endif
