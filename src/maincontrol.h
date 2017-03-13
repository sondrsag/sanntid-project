#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <stdlib.h>
#include "globals.h"
//#include "network.h"

typedef void (*HandleJobCallback_t)(job_t);

void ctr_start(HandleJobCallback_t jobCallback);

void ctr_updateElevStatus(ElevatorStatus new_status);

void ctr_receiveJob(job_t job);

#endif /* end of include guard: _CONTROL_H_ */
