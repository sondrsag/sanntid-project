#include "driver.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "unistd.h"
#include "lib/timer.h"

static int btn_inputs[N_BUTTONS][N_FLOORS]; // Status of input buttons
static int btn_lamps[N_BUTTONS][N_FLOORS];  // Status of button lamps
static int job_btn; // To keep track of which buttons light to switch of after current job
static ElevatorStatus status;
//static bool status_updated;
static bool stopped;
static pthread_mutex_t status_mtx;
static void (*updateStatus)(ElevatorStatus);
static void (*jobRequest)(int, int);

void evalJobProgress(void);

void checkInputs(void);

bool drv_startJob(elev_button_type_t button, int floor);

void* startDriver(void* args)
{
    pthread_mutex_lock(&status_mtx);

    elev_init(ET_Simulation);
    elev_set_motor_direction(DIRN_STOP);

    memset(btn_inputs, 0, N_BUTTONS * N_FLOORS * sizeof(int));
    memset(btn_lamps, 0, N_BUTTONS * N_FLOORS * sizeof(int));

    struct driver_args* arguments = args;
    updateStatus = arguments->updateStatusPtr;
    jobRequest = arguments->jobRequestPtr;

    status.working = false;
    status.finished = false;
    status.action = IDLE;
    status.current_floor = elev_get_floor_sensor_signal();
    status.next_floor = -1;
    status.direction = 0;

    if (status.current_floor == -1) {
        elev_set_motor_direction(DIRN_DOWN);
        while (status.current_floor == -1) {
            status.current_floor = elev_get_floor_sensor_signal();
        }
        elev_set_floor_indicator(status.current_floor);
        elev_set_motor_direction(DIRN_STOP);
    }

    pthread_mutex_unlock(&status_mtx);

    // Avoiding function calls during locked mtx with these variables
    bool working;
    bool dir;
    while (1) {
        pthread_mutex_lock(&status_mtx);
        working = status.working;
        dir = status.direction;
        pthread_mutex_unlock(&status_mtx);

        if (working) {
            evalJobProgress();
        }

        checkInputs();

        if (elev_get_stop_signal()) {
            elev_set_motor_direction(DIRN_STOP);
            stopped = true;
        } else if (stopped && !elev_get_stop_signal()) {
            elev_set_motor_direction(dir);
            stopped = false;
        }

        usleep(100);
    } // while

    printf("Driver shut down!\n");
    pthread_exit(NULL);
    return NULL;
} // startDriver

bool drv_startJob(elev_button_type_t button, int floor)
{
    printf("Starting job\n");
    pthread_mutex_lock(&status_mtx);

    if (stopped || floor < 0 || floor >= N_FLOORS) {
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
        job_btn = button;
        // status_updated = true;
        ret = true;
    } else if (floor < status.current_floor) {
        elev_set_motor_direction(DIRN_DOWN);
        status.working = true;
        status.action = MOVING;
        status.next_floor = floor;
        status.direction = DIRN_DOWN;
        job_btn = button;
        // status_updated = true;
        ret = true;
    } else if (floor == status.current_floor) {
        status.working = true;
        status.action = OPEN;
        elev_set_door_open_lamp(1);
        timer_start(3.0);
        // status_updated = true;
        ret = true;
    } // if

    pthread_mutex_unlock(&status_mtx);
    updateStatus(status);

    if (ret) {
        printf("Startet job. Btn: %d Floor: %d\n", button, floor);
    }

    return ret;
} // drv_startJob

void evalJobProgress(void)
{
    pthread_mutex_lock(&status_mtx);
    switch (status.action) {
    case MOVING:
        if (status.current_floor == status.next_floor) {
            elev_set_motor_direction(DIRN_STOP);
            status.action = OPEN;
            status.direction = DIRN_STOP;

            switch (job_btn) {
            case BUTTON_CALL_UP:
                elev_set_button_lamp(BUTTON_CALL_UP, status.current_floor, 0);
                btn_lamps[BUTTON_CALL_UP][status.current_floor] = 0;
                break;

            case BUTTON_CALL_DOWN:
                elev_set_button_lamp(BUTTON_CALL_DOWN, status.current_floor, 0);
                btn_lamps[BUTTON_CALL_DOWN][status.current_floor] = 0;
                break;

            case BUTTON_COMMAND:
                elev_set_button_lamp(BUTTON_COMMAND, status.current_floor, 0);
                btn_lamps[BUTTON_COMMAND][status.current_floor] = 0;
                break;
            }

            elev_set_door_open_lamp(1);
            timer_start(3.0);
            printf("StatUp when opening\n");
            updateStatus(status);
        } // if

        break;

    case OPEN:
        if (timer_timedOut()) {
            elev_set_door_open_lamp(0);
            status.working = false;
            status.finished = true;
            status.action = IDLE;
            // status_updated = true;
            printf("StatUp when closing\n");
            updateStatus(status);
        }

        break;

    case IDLE:
        break;
    } // switch
    pthread_mutex_unlock(&status_mtx);
} // evalJobProgress

void checkButton(elev_button_type_t button, int floor)
{
    if (button == BUTTON_CALL_DOWN && floor == 0) {
        return;
    } else if (button == BUTTON_CALL_UP && floor == N_FLOORS - 1) {
        return;
    }

    if (btn_inputs[button][floor] != elev_get_button_signal(button, floor)) {
        btn_inputs[button][floor] = elev_get_button_signal(button, floor);

        if (btn_inputs[button][floor] != 0) {
            jobRequest(button, floor);
            elev_set_button_lamp(button, floor, 1);
            btn_lamps[button][floor] = 1;
        }
    }
} // checkButton

void checkInputs(void)
{
    pthread_mutex_lock(&status_mtx);
    if (status.current_floor != elev_get_floor_sensor_signal() &&
            elev_get_floor_sensor_signal() != -1) {
        status.current_floor = elev_get_floor_sensor_signal();
        elev_set_floor_indicator(status.current_floor);
    }
    pthread_mutex_unlock(&status_mtx);

    // Check every button for change in input
    for (size_t f = 0; f < N_FLOORS; f++) {
        for (size_t b = 0; b < N_BUTTONS; b++) {
            checkButton(b, f);
        }
    }
} // checkInputs
