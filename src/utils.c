#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

int str2int(char const * str) {
    int sum = 0;
    int i = 0;
    //First find end of string
    while (str[i+1] != '\0') {
        ++i;
    }
    int exp = 0;
    for (; i>=0; --i) {
        sum += (str[i] - '0' )*pow(10, exp++);
    }
    return sum;
}

//The following are borrowed from "The Little Book of Semaphores"
//http://greenteapress.com/semaphores/LittleBookOfSemaphores.pdf
Mutex_t * mutex_make(void) {
    Mutex_t * mutex = checkMalloc(sizeof(Mutex_t));
    int ret = pthread_mutex_init(mutex, NULL);
    if (ret != 0) {
        perrorExit("Failed to make mutex\n");
    }
    return mutex;
}

void mutex_lock(Mutex_t * mutex) {
    int ret = pthread_mutex_lock(mutex);
    if (ret != 0) {
        perrorExit("Failed to lock mutex\n");
    }
}

void mutex_unlock(Mutex_t * mutex) {
    int ret = pthread_mutex_unlock(mutex);
    if (ret != 0) {
        perrorExit("Failed to unlock mutex\n");
    }
}

void * checkMalloc(size_t size) {
    void * const ptr = malloc(size);
    if (ptr == NULL) {
        perrorExit("Malloc failed\n");
    }

    return ptr;
}

void perrorExit(char const * const error_msg) {
    perror(error_msg);
    exit(1);
}


