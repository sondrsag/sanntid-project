#ifndef _ELEVATORCONTROL_H_
#define _ELEVATORCONTROL_H_

#include <stdlib.h>
#include "driver.h"
#include "globals.h"

// ectr = elevatorcontrol
void ectr_start(UpdateStatusCallback_t stat_callback,
                SendJobCallback_t      job_callback);

void ectr_handleJob(Job_t job);

void ectr_updateFinishedJob(Job_t job);

#endif /* end of include guard: _ELEVATORCONTROL_H_ */
