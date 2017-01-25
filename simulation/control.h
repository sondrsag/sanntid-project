#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <stdlib.h>
#include "driver.h"
#include "globals.h"

struct control_args {
    void (*passOnStatusPtr)(ElevatorStatus);
    void (*passOnRequestPtr)(int, int);
};

// Initializes environment variables and runs the while(1) loop.
// Args points to a struct of function pointers passed from the
// control module.
void* startControl(void* args);

void ctr_updateStatus(ElevatorStatus new_status);

// Determines when the requested job should be executed
void ctr_handleRequest(int button, int floor);

// Send external requests to the communication module and handle internal requests at once
void ctr_receiveRequest(int button, int floor);

#endif /* end of include guard: _CONTROL_H_ */
