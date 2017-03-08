#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <stdlib.h>
#include "globals.h"
//#include "network.h"

typedef void (*HandleJobCallback_t)(Job_t);

void work_distribution_start(HandleJobCallback_t jobCallback); //used by main

/* functions for local elevator module*/
void wd_updateLocalElevStatus(ElevatorStatus_t new_status); 
void wd_receiveJob(Job_t job); 

/*function for communication module*/
//void wd_receiveCallsListFromPrimary(FloorState_t* newOutsideCallsList); 
void wd_receiveCallsListFromPrimary(OutsideCallsList_t newOutsideCallsList); 
void wd_HandleInternalCallsAfterRestart();
void wd_updateElevStatus(ElevatorStatus_t new_status, int assignee_id);

#endif /* end of include guard: _CONTROL_H_ */
