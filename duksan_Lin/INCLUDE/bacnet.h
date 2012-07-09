#ifndef BACNET_H
#define BACNET_H

#include "queue_handler.h"

#define MAX_DEVICE_INSTANCE			65536
#define MAX_OBJECT_INSTANCE			1048576
#define READ_MAX_DATA				4096

#define BAC_TYPE_AI					0
#define BAC_TYPE_AV					2
#define BAC_TYPE_BI					3
#define BAC_TYPE_BO					4
#define BAC_TYPE_MSI				13
#define BAC_TYPE_MSO				14

#define ST_READ_WRITEPOINT			1
#define ST_READ_READPOINT			2

#define ST_READ_ONOFF				2
#define ST_READ_SETTEMP				19
#define ST_READ_MODE				8
#define ST_READ_TEMP				14
#define ST_READ_ALRAM				15
#define ST_READ_FILTER				5
#define ST_READ_AHU					6

#define ST_WRITE_ONOFF				1
#define ST_WRITE_SETTEMP			13
#define ST_WRITE_MODE				7

#define ST_PCM_ONOFF				0
#define ST_PCM_TEMP					2
#define ST_PCM_SETTEMP				3
#define ST_PCM_MODE					4
#define ST_PCM_ALRAM				5
#define ST_PCM_FILTER				6
#define ST_PCM_AHU					10

#if 0
typedef struct {
	unsigned int pcm;
	unsigned int pno;
	unsigned int objInstance;
	unsigned int objType;
} bacnetFileInfo;
#else
typedef struct {
	unsigned int pno;
	unsigned int type;
	unsigned int device;
	unsigned int unit;
} bacnetFileInfo;
#endif

typedef struct {
	unsigned int DeviceInstance;
	unsigned int ObjectType;
	unsigned int ObjectInstance;
} _ReadData_t;

typedef struct {
	uint16_t ObjectInstance;
	float Value;
} aoPtbl_t;

typedef struct {
	uint16_t ObjectInstance;
	float Value;
} aiPtbl_t;

typedef struct {
	uint16_t ObjectInstance;
	uint8_t Value;
} doPtbl_t;

typedef struct {
	uint16_t ObjectInstance;
	uint8_t Value;
} diPtbl_t;

typedef struct {
	uint16_t ObjectInstance;
	uint8_t Value;
} msoPtbl_t;

typedef struct {
	uint16_t ObjectInstance;
	uint8_t Value;
} msiPtbl_t;



void *bacnet_manager_main(void *arg);
int *bacnet_readprop_main(void *arg);
void bacnet_read(bacnetFileInfo data);
//void bacnet_write(int index, point_info point);
void bacnet_write(int index, point_info point, int pointOffset);

#endif
