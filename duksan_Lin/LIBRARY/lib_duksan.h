#ifndef LIB_APG_HANDLER_H
#define LIB_APG_HANDLER_H

#define LIB_PCM(X)				(X>>8) 						// get pcm number
#define LIB_PNO(X)				(X&0xFF) 					// get pno number

#if 0
//#define LIB_PCM(X)				(X>>8) 						// get pcm number
//#define LIB_PNO(X)				(X&0xFF) 					// get pno number

#define FILE_SERVER 			"/tmp/apgSvr"				// server fd

#define	LIB_BUF_SIZE			4095						// buffer size

#define LIB_PSET_REQUEST		'S'							// command is pSet
#define LIB_PGET_REQUEST		'G'							// command is pGet
#define LIB_PDEF_REQUEST		'D'							// command is pGet
#define LIB_STARTUP_REQUEST		'I'							// startup

#define byte					unsigned char				// type define..
#define word					unsigned short				// type define..

#define INIT_RESUME		0									// point define command.. none으로 처리한다. 

// Library type value. 
/* 
 Reference code.. from define.h
 
// point-define의 type	
#define DFN_PNT_VR						0
#define DFN_PNT_DO						1
#define DFN_PNT_VO						2
#define DFN_PNT_DI2S					3
#define DFN_PNT_JPT						4
#define DFN_PNT_VI						5
#define DFN_PNT_CI						6
#define DFN_PNT_ID						7
*/
#define LIB_PDEF_VR				0
#define LIB_PDEF_DO				1
#define LIB_PDEF_VO				2
#define LIB_PDEF_DI2S			3
#define LIB_PDEF_JPT1000		4
#define LIB_PDEF_VI				5
#define LIB_PDEF_CI				6

// Library send type
#define LIB_SEND_TYPE_NONE		0							// send type is 'pset' or 'pget'
#define LIB_SEND_TYPE_STATUP	1							// send type is 'startup'
#define LIB_SEND_TYPE_PDEF		2							// send type is 'pdef'

// Library data format
typedef struct {
	unsigned char		c_cmd;
	unsigned char		c_pcm;
	unsigned short		w_pno;
	float				f_val;	
} __attribute__ ((packed)) APG_DATA_FORMAT_T;

// Library point define data format
typedef struct {
	unsigned char		c_cmd;
	unsigned char		c_pcm;
	unsigned short		w_pno;
	float				f_type;	
	float				f_hyst;	
	float				f_scale;	
	float				f_offset;	
	float				f_min;	
	float				f_max;	
} __attribute__ ((packed)) APG_PDEF_FORMAT_T;
#endif


#endif
