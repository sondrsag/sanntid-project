#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdbool.h>

typedef enum elevator_actions {
    IDLE = 0,
    MOVING = 1,
    OPEN = 2
} ElevatorActions;

typedef struct {
    bool working;
    bool finished;  // Used to clear job when finished
    ElevatorActions action;
    int current_floor;
    int next_floor;
    int direction;
} ElevatorStatus;

#endif /* end of include guard: _GLOBALS_H_ */
