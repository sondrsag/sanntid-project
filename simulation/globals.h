#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdbool.h>

typedef enum tag_elev_motor_direction {
    DIRN_DOWN = -1,
    DIRN_STOP = 0,
    DIRN_UP = 1
} elev_motor_direction_t;

typedef enum tag_elev_lamp_type {
    BUTTON_CALL_UP = 0,
    BUTTON_CALL_DOWN = 1,
    BUTTON_COMMAND = 2
} elev_button_type_t;

typedef enum elevator_actions {
    IDLE = 0,
    MOVING = 1,
    OPEN = 2
} ElevatorActions;

typedef struct {
    bool working;
    bool finished;
    ElevatorActions action;
    int current_floor;
    int next_floor;
    elev_motor_direction_t direction;
} ElevatorStatus;

typedef struct {
    int floor;
    elev_button_type_t button;
} job_t;

#endif /* end of include guard: _GLOBALS_H_ */
