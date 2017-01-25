#include "driver.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "unistd.h"
#include "lib/timer.h"

static struct {
    int buttons[N_BUTTONS][N_FLOORS];
    int lamps[N_BUTTONS][N_FLOORS];
} input;

static int job_btn; // To keep track of which buttons light to switch of after current job
static bool stopped;

static ElevatorStatus status;
static pthread_mutex_t status_mtx;

static void (*updateStatus)(ElevatorStatus); // elevatorcontrol module callback
static void (*sendJob)(job_t); // elevatorcontrol module callback

void evalJobProgress(void);
void checkInputs(void);
bool drv_startJob(job_t job);
void* runDriver();

void drv_start(UpdateStatusCallback_t stat_callback, SendJobCallback_t job_callback)
{
    updateStatus = stat_callback;
    sendJob = job_callback;
    pthread_t driver_thrd;
    pthread_create(&driver_thrd, NULL, runDriver, NULL);
}

void* runDriver()
{
    pthread_mutex_lock(&status_mtx);

    elev_init(ET_Simulation);
    elev_set_motor_direction(DIRN_STOP);

    memset(input.buttons, 0, N_BUTTONS * N_FLOORS * sizeof(int));
    memset(input.lamps, 0, N_BUTTONS * N_FLOORS * sizeof(int));

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
        elev_set_motor_direction(DIRN_STOP);
    }

    elev_set_floor_indicator(status.current_floor);
    pthread_mutex_unlock(&status_mtx);

    // Avoiding function calls during locked mtx with these variables
    bool working;
    int dir;
    while (1) {
        pthread_mutex_lock(&status_mtx);
        working = status.working;
        dir = status.direction;
        pthread_mutex_unlock(&status_mtx);

        if (working) { evalJobProgress(); }

        checkInputs();

        if (elev_get_stop_signal()) {
            elev_set_motor_direction(DIRN_STOP);
            stopped = true;
        } else if (stopped && !elev_get_stop_signal()) {
            elev_set_motor_direction(dir);
            stopped = false;
        }

        usleep(20);
    } // while

    printf("Driver shut down!\n");
    pthread_exit(NULL);
    return NULL;
} // runDriver

bool drv_startJob(job_t job)
{
    assert(("Starting job with floor out of bounds",
            job.floor >= 0 && job.floor < N_FLOORS));

    pthread_mutex_lock(&status_mtx);

    if (stopped) {
        pthread_mutex_unlock(&status_mtx);
        return false;
    }

    bool ret = false;

    if (job.floor > status.current_floor) {
        elev_set_motor_direction(DIRN_UP);
        status.working = true;
        status.action = MOVING;
        status.next_floor = job.floor;
        status.direction = DIRN_UP;
        job_btn = job.button;
        ret = true;
    } else if (job.floor < status.current_floor) {
        elev_set_motor_direction(DIRN_DOWN);
        status.working = true;
        status.action = MOVING;
        status.next_floor = job.floor;
        status.direction = DIRN_DOWN;
        job_btn = job.button;
        ret = true;
    } else if (job.floor == status.current_floor) {
        status.working = true;
        status.action = OPEN;
        elev_set_button_lamp(job.button, job.floor, 0);
        input.lamps[job.button][job.floor] = 0;
        elev_set_door_open_lamp(1);
        timer_start(3.0);
        ret = true;
    } // if

    pthread_mutex_unlock(&status_mtx);
    updateStatus(status);

    if (ret) {
        printf("Startet job. Btn: %d Floor: %d\n", job.button, job.floor);
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
                input.lamps[BUTTON_CALL_UP][status.current_floor] = 0;
                break;

            case BUTTON_CALL_DOWN:
                elev_set_button_lamp(BUTTON_CALL_DOWN, status.current_floor, 0);
                input.lamps[BUTTON_CALL_DOWN][status.current_floor] = 0;
                break;

            case BUTTON_COMMAND:
                elev_set_button_lamp(BUTTON_COMMAND, status.current_floor, 0);
                input.lamps[BUTTON_COMMAND][status.current_floor] = 0;
                break;
            }

            elev_set_door_open_lamp(1);
            timer_start(3.0);
            updateStatus(status);
        } // if

        break;

    case OPEN:
        if (timer_timedOut()) {
            elev_set_door_open_lamp(0);
            status.working = false;
            status.finished = true;
            status.action = IDLE;
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
    if (input.buttons[button][floor] != elev_get_button_signal(button, floor)) {
        input.buttons[button][floor] = elev_get_button_signal(button, floor);

        if (input.buttons[button][floor] != 0) {
            assert(("Invalid button input",
                    (button != BUTTON_CALL_DOWN || floor != 0) &&
                    (button != BUTTON_CALL_UP || floor != N_FLOORS - 1)));

            job_t new_job;
            new_job.floor = floor;
            new_job.button = button;
            sendJob(new_job);
            elev_set_button_lamp(button, floor, 1);
            input.lamps[button][floor] = 1;
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
