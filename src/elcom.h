#ifndef _ELCOM_H_
#define _ELCOM_H_
//#include "../simulation/globals.h"
#include "globals.h"

void elcom_broadcastJob(Job_t job);

void elcom_broadcastOutsideCallsList(OutsideCallsList_t outside_calls_list);

void elcom_broadcastElevatorStatus(ElevatorStatus_t status);

void elcom_broadcastInternalCallsList(InternalCallsList_t calls_list);

void elcom_init(char* ips_and_ports[]);

int elcom_numJobsReceived(void);

void elcom_broadcast(char* msg, size_t length);

//elStatuses_t elcom_getElStatus();

//job_t elcom_getJob(void);

#endif //_ELCOM_H_


