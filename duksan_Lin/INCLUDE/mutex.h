#ifndef MUTEX_H
#define MUTEX_H	

#include "define.h"

pthread_cond_t g_condition;  // 조건변수는 단지 상태정보만을 알리기 위해서 사용되며, (※ 조건변수는 Lock 기능 없음.)
pthread_mutex_t g_mutex;     // 상태정보를 원자적(atomic)으로 주고받기 위해서는 뮤텍스와 함께 사용해야 한다.


pthread_cond_t net32Cond = PTHREAD_COND_INITIALIZER ;  // 조건변수는 단지 상태정보만을 알리기 위해서 사용되며, (※ 조건변수는 Lock 기능 없음.)
pthread_mutex_t net32CondMutex = PTHREAD_MUTEX_INITIALIZER;     // 상태정보를 원자적(atomic)으로 주고받기 위해서는 뮤텍스와 함께 사용해야 한다.

#endif
