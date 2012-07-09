#ifndef		DEFINE_H
#define		DEFINE_H

#include <sys/un.h> 
//
// move TYPE.h
//#define BYTE	unsigned char 
//#define WORD	unsigned short 
//
#define MAX_BUFFER_SIZE					4096
//
#define MAX_NET32_NUMBER        		32
#define MAX_POINT_NUMBER        		256
#define MAX_PHY_POINT_CNT				32
//
#define MAX_QUEUE_SIZE					4096
//
#define SUCCESS							1
#define ERROR				   			-1
#define FAIL							-1	
//
#define MAX_ENV_NUMBER					20

#define U_SEC							1000
#define M_SEC							1000000
//
//move PORT.h
//#define INTERFACE_SERVER_PORT         	2080
//#define COMMAND_SERVER_PORT			  	2090
//#define IFACE_SERVER_PORT			  	2070
//
#define ENABLE							1
#define DISABLE							0
//
#define CFG_VALUE_SIZE					16			// config.dat value size
#define CFG_COMMAND_SIZE				32			// config.dat command size
#define CFG_COMMAND_COUNT				16			// config.dat command count

// point-define¿« type	
#define DFN_PNT_VR						0
#define DFN_PNT_DO						1
#define DFN_PNT_VO						2
#define DFN_PNT_DI2S					3
#define DFN_PNT_JPT						4
#define DFN_PNT_VI						5
#define DFN_PNT_CI						6
#define DFN_PNT_ID						7

#define MAX_DNP_DI_POINT 				512
#define MAX_DNP_DO_POINT 				512
#define MAX_DNP_AI_POINT 				512
#define MAX_DNP_AO_POINT 				512
//
#define CONNECTED						1
#define DISCONNECTED					0
//

// multi ddc value
#define MULTI_DDC_ON					1
#define MULTI_DDC_OFF					0

// hysterisys value
#define PNT_OVER_HYST					1
#define PNT_UNDER_HYST					0

// calibration status
#define CAL_NONE						0
#define CAL_VO_ZERO						1
#define CAL_ZERO						2
#define CAL_VI							3
#define CAL_CI							4
#define CAL_VO							5
#define CAL_JPT							6

// subio  define....
#define SUBIO_POINT_COUNT				8
#define SUBIO_CMD_DO					0x0F			// DO COMMAND FUNCTION CODE (MODBUS)
#define SUBIO_CMD_VO					0x10			// AO COMMAND FUNCTION CODE (MODBUS)
#define SUBIO_CMD_ADI					0x03			// ADI COMMAND FUNCTION CODE (MODBUS)
#define SUBIO_CMD_START_ADDR			0x0000
//#define SUBIO_CMD_DEFINE_ADDR			0x0008			// big endian
#define SUBIO_CMD_DEFINE_ADDR			0x0800			// little endian
//#define SUBIO_CMD_DATA_CNT			0x0008			// big endian
#define SUBIO_CMD_DATA_CNT				0x0800			// little endian
#define SUBIO_CMD_DO_BYTE_CNT			0x02
#define SUBIO_CMD_AO_BYTE_CNT			0x10

// subio crc mask
#define SUBIO_MASK_BYTE					0x00FF

// subio board type define
#define  SUBIO_TYPE_DO     				1
#define  SUBIO_TYPE_VO     				2
#define  SUBIO_TYPE_ADI     			3
#define  SUBIO_TYPE_NONE				0

// subio group number
#define  SUBIO_GRP_0     				0
#define  SUBIO_GRP_1     				1
#define  SUBIO_GRP_2     				2
#define  SUBIO_GRP_3    	 			3

// subio pcm number
#define  SUBIO_GRP_0_PNO     			32
#define  SUBIO_GRP_1_PNO     			40
#define  SUBIO_GRP_2_PNO     			48
#define  SUBIO_GRP_3_PNO     			56

// subio type define
#define SUBIO_ADI_DO  					0x01
#define SUBIO_ADI_DI  					0x02
#define SUBIO_ADI_AO  					0x03
#define SUBIO_ADI_CI  					0x04
#define SUBIO_ADI_VI  					0x05
#define SUBIO_ADI_JPT1000  				0x06

// System Log Type
#define SYSLOG_BOOT_ON  					1
#define SYSLOG_NET32_FAIL_REBOOT  			2
#define SYSLOG_ADCDRV_FAIL_REBOOT  			3
#define SYSLOG_CONNECT_ELBA  				4
#define SYSLOG_DISCONNECT_ELBA  			5
#define SYSLOG_NET32_TIMEOUT_REBOOT  		6
#define SYSLOG_NET32_OPEN_ERROR				7
#define SYSLOG_NET32_SELECT_ERROR			8
#define SYSLOG_NET32_ACCEPT_ERROR			9
#define SYSLOG_NET32_CLOSE_ERROR			10
#define SYSLOG_DESTROY_MAIN					100
#define SYSLOG_DESTROY_NET32  				101
#define SYSLOG_DESTROY_ELBA  				102
#define SYSLOG_DESTROY_MSG_HANDLER 			103
#define SYSLOG_DESTROY_CMD_HANDLER 			104
#define SYSLOG_DESTROY_POINT_OBSERVER		105
#define SYSLOG_DESTROY_POINT_MANAGER		106
#define SYSLOG_DESTROY_CCMS_SERVER			107
#define SYSLOG_DESTROY_CCMS_CLIENT			108
#define SYSLOG_DESTROY_CCMS_MANAGER			109
#define SYSLOG_DESTROY_CCMS_MODEM_SERVER	110
#define SYSLOG_DESTROY_IFACE_MODEM			111
#define SYSLOG_DESTROY_IFACE_SMS			112
#define SYSLOG_DESTROY_IFACE_GHP			113


#define NET32_BUF_SIZE			64

/* same definition (s3c2410.c) */
#define NET32_TYPE_STATE		1			// net32 point type
#define NET32_TYPE_REPORT		2			// net32 point type
#define NET32_TYPE_COMMAND		3			// net32 point type
#define NET32_TYPE_REQUIRE		4			// net32 point type
#define NET32_TYPE_TOKEN		5			// net32 point type


// calibration type
#define CMD_CAL_TYPE_VI					2
#define CMD_CAL_TYPE_CI					3
#define CMD_CAL_TYPE_JPT				4
#define CMD_CAL_TYPE_VO					5
#define CMD_CAL_TYPE_JPT_LO				6
#define CMD_CAL_TYPE_JPT_HI				7


// library buffer size
#define	LIB_BUF_SIZE					4095						// buffer size

// library message type					
#define LIB_REQ_MSG_TYPE				'R'
#define LIB_SET_MSG_TYPE				'S'
#define LIB_DEF_MSG_TYPE				'D'
#define LIB_INFO_MSG_TYPE				'I'
#define LIB_ACK_MSG_TYPE				'A'
#define LIB_FAIL_MSG_TYPE				'F'


//Queue Message
#define QUEUE_FULL				2
#define QUEUE_EMPTY				3

// NET32/IP message type
#define UCLIENT_CMD_ACK			'a'
#define UCLIENT_CMD_MSG			'm'

#endif

