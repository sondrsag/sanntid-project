
#include <stdio.h>
#include <stdbool.h>
#include "elev.h"
#include "lib/stack.h"

pthread_mutex_t working_mtx;
bool working = false;

int getNextJob(void) { return 0; }

void* doJob(int floor) {
    printf("Doing some work!\n");
    pthread_mutex_lock(&working_mtx);
    working = false;
    pthread_mutex_unlock(&working_mtx);
    return NULL;
}

int main() {
    pthread_t work_thread;
    stack_root_t* job_stack;
    int job_input = -1;

    stackInit(&job_stack);
    pthread_mutex_init(&working_mtx);
    elev_init(ET_Simulation);
    elev_set_motor_direction(DIRN_STOP);

    printf("Press STOP button to stop elevator and exit program.\n");

    while (1) {
        // Check for next job request
        job_input = getNextJob();
        if (job_input > -1) {
            stackPush(job_stack, job_input);
        }

        // Check if there's any jobs left in the job stack
        if (!stackIsEmpty(job_stack)) {
            // Check if the elevator is currently working
            pthread_mutex_lock(&working_mtx);
            if (!working) {
                pthread_mutex_unlock(&working_mtx);
                pthread_join(&work_thread);
                pthread_create(&work_thread, NULL, doJob, stackPop(job_stack));
                working = true;
            } else {
                pthread_mutex_unlock(&working_mtx);
            }
        }

        // Stop elevator and exit program if the stop button is pressed
        if (elev_get_stop_signal()) {
            elev_set_motor_direction(DIRN_STOP);
            return 0;
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
