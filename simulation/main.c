#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "driver.h"

ElevatorStatus status;

void updateStatus(ElevatorStatus new_status)
{
    printf("Updated status!\n");
    status = new_status;
}

void handleRequest(int button, int floor)
{
    printf("Requested floor: %d btn: %d\n", floor, button);
}

int main() {
    struct driver_args dargs;
    dargs.updateStatusPtr = &updateStatus;
    dargs.jobRequestPtr = &handleRequest;

    pthread_t driver_thrd;
    pthread_create(&driver_thrd, NULL, startDriver, (void*)&dargs);
    while (1) {
        if (status.current_floor == 1 && status.action == IDLE) {
            break;
        }
        if (!status.working && status.current_floor != 3) {
            drvStartJob(BUTTON_CALL_DOWN, 3);
        }
        if (status.current_floor == 3 && status.action == IDLE) {
            drvStartJob(BUTTON_CALL_UP, 1);
        }
    }
    pthread_join(driver_thrd, NULL);
    return 0;
}

/*
// Change direction when we reach top/bottom floor
if (elev_get_floor_sensor_signal() == N_FLOORS - 1) {
    elev_set_motor_direction(DIRN_DOWN);
} else if (elev_get_floor_sensor_signal() == 0) {
    elev_set_motor_direction(DIRN_UP);
}

// Stop elevator and exit program if the stop button is pressed
if (elev_get_stop_signal()) {
    elev_set_motor_direction(DIRN_STOP);
    return 0;
}
*/
