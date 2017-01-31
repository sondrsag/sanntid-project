#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdbool.h>

typedef enum {
    DIRN_DOWN = -1,
    DIRN_STOP = 0,
    DIRN_UP   = 1
} elev_motor_direction_t;

typedef enum {
    BUTTON_CALL_UP   = 0,
    BUTTON_CALL_DOWN = 1,
    BUTTON_COMMAND   = 2
} elev_button_type_t;

typedef enum {
    IDLE   = 0,
    MOVING = 1,
    OPEN   = 2
} ElevatorActions;

typedef struct {
    bool working;
    bool finished;
    int  current_floor;
    int  next_floor;

    ElevatorActions        action;
    elev_motor_direction_t direction;
} ElevatorStatus;

typedef struct {
    int                floor;
    elev_button_type_t button;
} job_t;

typedef struct {
    bool up;
    bool down;
    int el_id;
} floorstate_t;

#define NUM_FLOORS 4

typedef floorstate_t systemstate_t[NUM_FLOORS];

#endif /* end of include guard: _GLOBALS_H_ */
