#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <stdlib.h>
#include "globals.h"
//#include "network.h"

typedef void (*handleRequestCallback_t)(job_t);

void* ctr_start(void* args);

void ctr_updateElevStatus(ElevatorStatus new_status);

void ctr_receiveRequest(job_t job);

#endif /* end of include guard: _CONTROL_H_ */
