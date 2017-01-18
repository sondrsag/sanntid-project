
#include <stdio.h>
#include "elev.h"

int main() {
    stack_t* job_stack;
    int job_input = -1;

    stack_init(job_stack);
    elev_init(ET_Simulation);
    elev_set_motor_direction(DIRN_STOP);

    printf("Press STOP button to stop elevator and exit program.\n");

    while (1) {
        // Check for next job request
        job_input = getNextJob(void);
        if (job_input > -1) {
            push(job_stack, job_input);
        }
    }
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
