#ifndef _UTILS_H_
#define _UTILS_H_
#include <pthread.h>

typedef pthread_mutex_t Mutex_t;


//The following are borrowed from "The Little Book of Semaphores"
//http://greenteapress.com/semaphores/LittleBookOfSemaphores.pdf
Mutex_t * mutex_make(void);

void mutex_lock(Mutex_t * mutex);

void mutex_unlock(Mutex_t * mutex);

void * checkMalloc(size_t size);

void perrorExit(char const * const error_msg);

int str2int(char const * str);

#endif //_UITLS_H_
