#ifndef _ELCOM_H_
#define _ELCOM_H_
#include "../simulation/globals.h"

void elcom_broadcastJob(job_t job);

void elcom_broadcastState(systemstate_t state);

int elcom_numJobsReceived(void);

//elStatuses_t elcom_getElStatus();

//job_t elcom_getJob(void);

#endif //_ELCOM_H_


