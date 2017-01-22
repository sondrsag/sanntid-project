#include "driver.h"
#include <stdio.h>
#include <pthread.h>
#include "lib/timer.h"

static ElevatorStatus status;
static bool status_updated;
static bool stopped;
static pthread_mutex_t status_mtx;

void evalJobProgress(void);

void checkInputs(void);

void* openDoors(void);

void startDriver(void* args)
{
    elev_init(ET_Simulation);
    elev_set_motor_direction(DIRN_STOP);

    struct driver_args* arguments = args;

    void (*updateStatus)(ElevatorStatus) = arguments->updateStatusPtr;

    status.working = false;
    status.action = IDLE;
    status.current_floor = elev_get_floor_sensor_signal();
    status.next_floor = -1;
    status.direction = 0;
    status_updated = false;

    if (status.current_floor == -1) {
        elev_set_motor_direction(DIRN_DOWN);
        status.working = true;
        status.action = MOVING;
        status.next_floor = 0;
        status.direction = DIRN_DOWN;
        status_updated = true;
    }

    while (1) {
        pthread_mutex_lock(&status_mtx);

        if (status.working) {
            evalJobProgress();
        }

        checkInputs();

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

void checkInputs(void)
{
    // status_mtx is locked before this func is called
    if (status.current_floor != elev_get_floor_sensor_signal() &&
        elev_get_floor_sensor_signal() != -1) {
        printf("Checking\n");
        status.current_floor = elev_get_floor_sensor_signal();
    }
}
