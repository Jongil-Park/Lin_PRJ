#ifndef STRUCT_H
#define STRUCT_H
//
//
typedef struct	{
    unsigned char pno;				// n-th point number
    unsigned char Unsol;			// Point status (ERR, OK, ...)
    unsigned char WrFlag;
    float	val;				// Current point value
    float	PrevVal;			// Previous point value
    //
    short   PLCaddr;
    float   scale;
    float   offset;
    //
    int     MinVal;			// minimum point value
    int     MaxVal;         // maximum point value
}	DNP_AIO_Point;

typedef struct	{
    unsigned char	pno;				// n-th point number
    unsigned char	Unsol;			// Point status (ERR, OK, ...)
    unsigned char	EnUnsol;			// Point status (ERR, OK, ...)
    unsigned char  WrFlag;
    unsigned char	val;				// Current point value
    unsigned char	PrevVal;			// Previous point value
    unsigned char	WrVal;			// Write point value
    //
    short   PLCaddr;
    short   ONaddr;
    short   OFFaddr;
}	DNP_DIO_Point;
//
DNP_DIO_Point DNP_di_point[512];
DNP_DIO_Point DNP_do_point[512];
DNP_AIO_Point DNP_ai_point[512];
DNP_AIO_Point DNP_ao_point[512];
//
int debug_plc_tx;
int debug_plc_rx;



#endif
