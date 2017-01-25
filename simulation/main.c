#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "control.h"
#include "communication.h"

int main() {
    struct driver_args drv_args;
    drv_args.updateStatusPtr = &ctr_updateStatus;
    drv_args.jobRequestPtr = &ctr_receiveRequest;

    struct control_args ctr_args;
    ctr_args.passOnStatusPtr = &cmc_updateElevStatus;
    ctr_args.passOnRequestPtr = &cmc_receiveRequest;

    struct communication_args cmc_args;
    cmc_args.ctr_handleRequestPtr = &ctr_handleRequest;

    pthread_t driver_thrd;
    pthread_t control_thrd;
    pthread_t communication_thrd;
    pthread_create(&driver_thrd, NULL, startDriver, (void*)&drv_args);
    usleep(1000);
    pthread_create(&control_thrd, NULL, startControl, (void*)&ctr_args);
    usleep(1000);
    pthread_create(&communication_thrd, NULL, startCommunication, (void*)&cmc_args);


    pthread_join(communication_thrd, NULL);
    pthread_join(control_thrd, NULL);
    pthread_join(driver_thrd, NULL);

    return 0;
}
