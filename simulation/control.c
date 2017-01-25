#include "control.h"
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static ElevatorStatus elev_status;
void (*handleJob)(job_t); // elevatorcontrol module callback

void* runControl();

void ctr_start(HandleJobCallback_t jobCallback)
{
    handleJob = jobCallback;
    pthread_t communication_thrd;
    pthread_create(&communication_thrd, NULL, runControl, NULL);
}

void* runControl()
{
    return NULL;
}

void ctr_updateElevStatus(ElevatorStatus new_status)
{
    elev_status = new_status;
}

void ctr_updateNetStatus()
{
}

void ctr_receiveJob(job_t job)
{
    handleJob(job);
}
