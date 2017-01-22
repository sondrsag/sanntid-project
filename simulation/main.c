#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "driver.h"

void updateStatus(ElevatorStatus status)
{
    printf("Updated status!\n");
    printf("floor: %d\ndirection: %d\nnext_floor: %d\n", status.current_floor,
            status.direction, status.next_floor);
}

int main() {
    struct driver_args dargs;
    dargs.updateStatusPtr = &updateStatus;

    pthread_t driver_thrd;
    pthread_create(&driver_thrd, NULL, startDriver, (void*)&dargs);
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
