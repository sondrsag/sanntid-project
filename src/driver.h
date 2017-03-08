#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdlib.h>
#include <stdbool.h>
#include "elev.h"
#include "globals.h"

typedef void (*UpdateStatusCallback_t)(ElevatorStatus_t);
typedef void (*SendJobCallback_t)(Job_t);

// drv = driver
void drv_start(UpdateStatusCallback_t stat_callback,
               SendJobCallback_t      job_callback);

// returns false if failed to start job
bool drv_startJob(Job_t job);

#endif /* end of include guard: _DRIVER_H_ */
