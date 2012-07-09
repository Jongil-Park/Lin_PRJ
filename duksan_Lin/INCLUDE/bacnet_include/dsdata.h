#ifdef DS_DATA_H
#define DS_DATA_H

#include <stdint.h>

#define MAX_DEVICE_INSTANCE			512
#define MAX_OBJECT_INSTANCE			256

typedef struct {
	unsigned int ObjectInstance;
	float Value;
} aoPtbl_t;


typedef struct {
	unsigned int ObjectInstance;
	float Value;
} aiPtbl_t;


typedef struct {
	unsigned int ObjectInstance;
	unsigned int Value;
i} doPtbl_t;


typedef struct {
	unsigned int ObjectInstance;
	unsigned int Value;
} diPtbl_t;



//aoPtbl_t aoPtbl[MAX_DEVICE_INSTANCE][MAX_OBJECT_INSTANCE];
//aiPtbl_t aiPtbl[MAX_DEVICE_INSTANCE][MAX_OBJECT_INSTANCE];
//doPtbl_t doPtbl[MAX_DEVICE_INSTANCE][MAX_OBJECT_INSTANCE];
//diPtbl_t diPtbl[MAX_DEVICE_INSTANCE][MAX_OBJECT_INSTANCE];

#endif
