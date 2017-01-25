#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdlib.h>
#include <stdbool.h>
#include "elev.h"
#include "globals.h"

struct driver_args {
    void (*updateStatusPtr)(ElevatorStatus);
    void (*jobRequestPtr)(int, int);
};

typedef void (*UpdateStatusCallback_t)(ElevatorStatus);
typedef void (*JobRequestCallback_t)(int, int);

// Initializes environment variables and runs the while(1) loop.
// Args points to a struct of function pointers passed from the
// control module.
void drv_start(UpdateStatusCallback_t stat_callback, JobRequestCallback_t job_callback);

// drv = driver
// returns false if failed to start job
bool drv_startJob(job_t job);

#endif /* end of include guard: _DRIVER_H_ */
