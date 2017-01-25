#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "globals.h"

struct communication_args {
    void (*ctr_handleRequestPtr)(int, int);
};

void* startCommunication(void* args);

void cmc_updateElevStatus(ElevatorStatus new_status);

void cmc_updateNetStatus();

void cmc_receiveRequest(int button, int floor);

#endif /* end of include guard: _COMMUNICATION_H_ */
