#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <stdlib.h>
#include "driver.h"
#include "globals.h"

struct control_args {
    void (*passOnStatusPtr)(ElevatorStatus);
    void (*passOnRequestPtr)(int, int);
};

typedef void (*sendRequestCallback_t)(int, int);

// Initializes environment variables and runs the while(1) loop.
// Args points to a struct of function pointers passed from the
// control module.
// ectr : elevatorcontrol
void* ectr_start(void* args);

// NOTE: fjern fra header
void ectr_updateStatus(ElevatorStatus new_status);

// Determines when the requested job should be executed
void ectr_handleRequest(int button, int floor);
//void ctr_handleJob(job_t job);

// NOTE: fjern fra header ----------------------------------------
// Send external requests to the communication module and handle internal requests at once
void ectr_receiveRequest(int button, int floor);
//void ctr_receiveJob(job_t job);

#endif /* end of include guard: _CONTROL_H_ */
