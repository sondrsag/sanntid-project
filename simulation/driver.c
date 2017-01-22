#include "driver.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "lib/timer.h"

// The values of input buttons.
static int btn_inputs[N_FLOORS][N_BUTTONS];
static ElevatorStatus status;
static bool status_updated;
static bool stopped;
static pthread_mutex_t status_mtx;

void evalJobProgress(void);

void checkInputs(void (*jobRequest)(int, int));

void* startDriver(void* args)
{
    elev_init(ET_Simulation);
    elev_set_motor_direction(DIRN_STOP);

    memset(btn_inputs, 0, N_FLOORS * N_BUTTONS * sizeof(int));

    struct driver_args* arguments = args;
    void (*updateStatus)(ElevatorStatus) = arguments->updateStatusPtr;
    void (*jobRequest)(int, int) = arguments->jobRequestPtr;

    status.working = false;
    status.action = IDLE;
    status.current_floor = elev_get_floor_sensor_signal();
    status.next_floor = -1;
    status.direction = 0;
    status_updated = true;

    if (status.current_floor == -1) {
        elev_set_motor_direction(DIRN_DOWN);
        status.working = true;
        status.action = MOVING;
        status.next_floor = 0;
        status.direction = DIRN_DOWN;
    }

    while (1) {
        pthread_mutex_lock(&status_mtx);

        if (status.working) {
            evalJobProgress();
        }

        checkInputs(jobRequest);

        if (status_updated) {
            updateStatus(status);
            status_updated = false;
        }

        pthread_mutex_unlock(&status_mtx);

        if (elev_get_stop_signal()) {
            elev_set_motor_direction(DIRN_STOP);
            stopped = true;
        } else if (stopped) {
            pthread_mutex_lock(&status_mtx);
            elev_set_motor_direction(status.direction);
            pthread_mutex_unlock(&status_mtx);
            stopped = false;
        }
    }
}

bool drvStartJob(int floor)
{
    pthread_mutex_lock(&status_mtx);

    if (stopped || status.working) {
        pthread_mutex_unlock(&status_mtx);
        return false;
    }

    bool ret = false;

    if (floor > status.current_floor) {
        elev_set_motor_direction(DIRN_UP);
        status.working = true;
        status.action = MOVING;
        status.next_floor = floor;
        status.direction = DIRN_UP;
        status_updated = true;
        ret = true;
    } else if (floor < status.current_floor) {
        elev_set_motor_direction(DIRN_DOWN);
        status.working = true;
        status.action = MOVING;
        status.next_floor = floor;
        status.direction = DIRN_DOWN;
        status_updated = true;
        ret = true;
    } else if (floor == status.current_floor) {
        status.working = true;
        status.action = OPEN;
        elev_set_door_open_lamp(1);
        timer_start(3.0);
        status_updated = true;
        ret = true;
    }

    pthread_mutex_unlock(&status_mtx);
    return ret;
}

void evalJobProgress(void)
{
    // status_mtx is locked before this func is called
    switch (status.action) {
    case MOVING:
        if (status.current_floor == status.next_floor) {
            elev_set_motor_direction(DIRN_STOP);
            status.action = OPEN;
            status.direction = DIRN_STOP;
            elev_set_door_open_lamp(1);
            timer_start(3.0);
            status_updated = true;
        }

        break;

    case OPEN:
        if (timer_timedOut()) {
            elev_set_door_open_lamp(0);
            status.working = false;
            status.action = IDLE;
            status_updated = true;
        }

        break;
    }
}

void checkButton(elev_button_type_t button, int floor, void (*jobRequest)(int, int))
{
    if (button == BUTTON_CALL_DOWN && floor == 0) {
        return;
    } else if (button == BUTTON_CALL_UP && floor == N_FLOORS - 1) {
        return;
    }

    if (btn_inputs[floor][button] != elev_get_button_signal(button, floor)) {
        btn_inputs[floor][button] = elev_get_button_signal(button, floor);
        if (btn_inputs[floor][button] != 0) {
            jobRequest(button, floor);
        }
    }
}

void checkInputs(void (*jobRequest)(int, int))
{
    // status_mtx is locked before this func is called
    if (status.current_floor != elev_get_floor_sensor_signal() &&
            elev_get_floor_sensor_signal() != -1) {
        status.current_floor = elev_get_floor_sensor_signal();
    }

    for (size_t f = 0; f < N_FLOORS; f++) {
        for (size_t b = 0; b < N_BUTTONS; b++) {
            checkButton(b, f, jobRequest);
        }
    }

}
