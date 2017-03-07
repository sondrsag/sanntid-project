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
    BUTTON_COMMAND   = 2 //think of changing name to cab_button or similar
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
    bool available; //added 2017.02.28, Anton

    ElevatorActions        action;
    elev_motor_direction_t direction;
} ElevatorStatus_t;

typedef struct {
    int                floor;
    elev_button_type_t button;
    bool finished;
    int assignee;
} Job_t;

typedef struct {
    bool up;
    bool down;
    int el_id_up; //added 2017.02.28, Anton
    int el_id_down; //added 2017.02.28, Anton 
} FloorState_t;

#define NUM_FLOORS 4
#define NUM_ELEVATORS 3 // added 2017.02.28, Anton
#define TIME 1.0  //Anton new to be used in all modules for synchronisation


//typedef floorstate_t systemstate_t[NUM_FLOORS]; //OLD version of massive for OutsideCallsList
//typedef FloorState_t OutsideCallsList_t[NUM_FLOORS]; // ANTON, suggestion for the name: OutsidecCallsList
//typedef InternalCall_t InternalCalls[NUM_ELEVATORS*NUM_FLOORS //OLD version of massive


#endif /* end of include guard: _GLOBALS_H_ */
