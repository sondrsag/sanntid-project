#ifndef _ELCOM_H_
#define _ELCOM_H_
//#include "../simulation/globals.h"
#include "globals.h"

void elcom_broadcastJob(Job_t job);

void elcom_broadcastOutsideCallsList(OutsideCallsList_t outside_calls_list);

void elcom_broadcastElevatorStatus(ElevatorStatus_t status);

void elcom_broadcastInternalCallsList(InternalCallsList_t calls_list);

void elcom_init(unsigned int const my_id);

void elcom_broadcast(char* msg, size_t length);

#endif //_ELCOM_H_


