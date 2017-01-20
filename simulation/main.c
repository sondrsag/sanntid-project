#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "elev.h"
#include "lib/stack.h"

pthread_mutex_t working_mtx;
bool working = false;

int getNextJob(void) {
    if (elev_get_button_signal(BUTTON_CALL_UP, 0) ||
        elev_get_button_signal(BUTTON_CALL_DOWN, 0)) {
        return 0;
    }
    if (elev_get_button_signal(BUTTON_CALL_UP, 1) ||
        elev_get_button_signal(BUTTON_CALL_DOWN, 1)) {
        return 1;
    }
    if (elev_get_button_signal(BUTTON_CALL_UP, 2) ||
        elev_get_button_signal(BUTTON_CALL_DOWN, 2)) {
        return 2;
    }
    if (elev_get_button_signal(BUTTON_CALL_UP, 3) ||
        elev_get_button_signal(BUTTON_CALL_DOWN, 3)) {
        return 3;
    }
    return -1;
}

void* doJob(int floor) {
    printf("Doing some work!\n");
    pthread_mutex_lock(&working_mtx);
    working = false;
    pthread_mutex_unlock(&working_mtx);
    pthread_exit(NULL);
    return NULL;
}

int main() {
    pthread_t work_thread;
    stack_root_t* job_stack;
    int job_input = -1;
    int last_job_in = -2;

    stackInit(&job_stack);
    pthread_mutex_init(&working_mtx, NULL);
    elev_init(ET_Simulation);
    elev_set_motor_direction(DIRN_STOP);

    printf("Press STOP button to stop elevator and exit program.\n");

    while (1) {
        // Stop elevator and exit program if the stop button is pressed
        if (elev_get_stop_signal()) {
            elev_set_motor_direction(DIRN_STOP);
            break;
        }
    }
    printf("stack length: %d\n", job_stack->length);
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
