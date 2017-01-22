#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdlib.h>
#include <stdbool.h>
#include "elev.h"

typedef enum elevator_actions {
    IDLE = 0,
    MOVING = 1,
    OPEN = 2
} ElevatorActions;

typedef struct elevator_status {
    bool working;
    ElevatorActions action;
    int current_floor;
    int next_floor;
    int direction;
} ElevatorStatus;

struct driver_args {
    void (*updateStatusPtr)(ElevatorStatus);
    void (*jobRequestPtr)(int);
};

// Initializes environment variables and runs the while(1) loop.
// updateStatus(..) points to a function in the control module.
void startDriver(void* args);

// drv = driver
// returns false if failed to start job
bool drvStartJob(int floor);

#endif /* end of include guard: _DRIVER_H_ */
