#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdlib.h>
#include <stdbool.h>
#include "elev.h"
#include "globals.h"

typedef void (*UpdateStatusCallback_t)(ElevatorStatus);
typedef void (*SendJobCallback_t)(job_t);

// drv = driver
void drv_start(UpdateStatusCallback_t stat_callback,
               SendJobCallback_t      job_callback);

// returns false if failed to start job
bool drv_startJob(job_t job);

#endif /* end of include guard: _DRIVER_H_ */
