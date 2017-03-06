#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdint.h>
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
    bool available;

    ElevatorActions        action;
    elev_motor_direction_t direction;
} ElevatorStatus_t;

//used both to comunicate comands from cabin and from outside
typedef struct {
    int                floor;
    elev_button_type_t button;
    bool finished;
    int assignee;
} Job_t;

typedef struct {
    bool up;
    bool down;
    uint8_t el_id_up;
    uint8_t el_id_down;
} FloorCalls_t;

/*
typedef struct {
    int floor;
    int el_id;
} InternalCall_t
*/

//#define NUM_FLOORS 4
//#define NUM_ELEVATORS 3

typedef FloorCalls_t OutsideCallsList_t[NUM_FLOORS]; 
//typedef InternalCall_t InternalCalls[NUM_ELEVATORS*NUM_FLOORS
typedef bool InternalCalls_t[NUM_ELEVATORS][NUM_FLOORS];

#endif /* end of include guard: _GLOBALS_H_ */

//    Contact GitHub API Training Shop Blog About 
//    © 2017 GitHub, Inc. Terms Privacy Security Status Help 

