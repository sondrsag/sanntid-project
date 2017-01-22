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

typedef struct {
    bool working;
    ElevatorActions action;
    int current_floor;
    int next_floor;
    int direction;
} ElevatorStatus;

struct driver_args {
    void (*updateStatusPtr)(ElevatorStatus);
    void (*jobRequestPtr)(int, int);
};

// Initializes environment variables and runs the while(1) loop.
// Args points to a struct of function pointers passed from the
// control module.
void* startDriver(void* args);

// drv = driver
// returns false if failed to start job
bool drvStartJob(elev_button_type_t btn, int floor);

#endif /* end of include guard: _DRIVER_H_ */
