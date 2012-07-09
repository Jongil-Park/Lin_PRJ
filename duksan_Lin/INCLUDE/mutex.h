#ifndef MUTEX_H
#define MUTEX_H	

#include "define.h"

pthread_cond_t g_condition;  // ���Ǻ����� ���� ������������ �˸��� ���ؼ� ���Ǹ�, (�� ���Ǻ����� Lock ��� ����.)
pthread_mutex_t g_mutex;     // ���������� ������(atomic)���� �ְ�ޱ� ���ؼ��� ���ؽ��� �Բ� ����ؾ� �Ѵ�.


pthread_cond_t net32Cond = PTHREAD_COND_INITIALIZER ;  // ���Ǻ����� ���� ������������ �˸��� ���ؼ� ���Ǹ�, (�� ���Ǻ����� Lock ��� ����.)
pthread_mutex_t net32CondMutex = PTHREAD_MUTEX_INITIALIZER;     // ���������� ������(atomic)���� �ְ�ޱ� ���ؼ��� ���ؽ��� �Բ� ����ؾ� �Ѵ�.

#endif
