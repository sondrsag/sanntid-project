#include "communication.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static ElevatorStatus elev_status;
void (*handleRequest)(int, int);

void* startCommunication(void* args)
{
    struct communication_args* arguments = args;
    handleRequest = arguments->ctr_handleRequestPtr;

    usleep(100); // All module threads sleeps 100us after initializing
    return args; // To avoid compiler warnings
}

void cmc_updateElevStatus(ElevatorStatus new_status)
{
    elev_status = new_status;
}

void cmc_updateNetStatus()
{
}

void cmc_receiveRequest(int button, int floor)
{
    printf("communication returns request!\n");
    handleRequest(button, floor);
}
