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

//used both to comunicate comands from cabin and from outside
typedef struct {
    int                floor;
    elev_button_type_t button;
    bool finished;
    int assignee;
} job_t;

typedef struct {
    bool up;
    bool down;
    int el_id_up; //ANTON, why do we need el_id here?
	int el_id_down;
} floorstate_t;

#define NUM_FLOORS 4

typedef floorstate_t OutsidecCallsList_t[NUM_FLOORS]; // ANTON, suggestion for the name: OutsidecCallsList

#endif /* end of include guard: _GLOBALS_H_ */

//    Contact GitHub API Training Shop Blog About 
//    © 2017 GitHub, Inc. Terms Privacy Security Status Help 

