#include "driver.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "unistd.h"
#include "timer.h"

#define MV_TIMEOUT 400000

static struct {
    int buttons[N_BUTTONS][N_FLOORS];
    int lamps[N_BUTTONS][N_FLOORS];
} input;

// To keep track of which button light to switch of after current job
static int              job_btn;
static ElevatorStatus_t status;
static pthread_mutex_t  status_mtx;
static void             (*updateStatus)(ElevatorStatus_t);
static void             (*sendJob)(Job_t);

void  evalJobProgress(void);
void  checkInputs(void);
bool  drv_startJob(Job_t job);
void* runDriver();

void drv_start(UpdateStatusCallback_t stat_callback, SendJobCallback_t job_callback)
{
    updateStatus = stat_callback;
    sendJob      = job_callback;
    pthread_t driver_thrd;
    pthread_create(&driver_thrd, NULL, runDriver, NULL);
}

void* runDriver()
{
    /***************************************************************************


        Initialize elevator and setup module variable


    ***************************************************************************/
    elev_init(ET_Simulation);
    // elev_init(ET_Comedi);
    elev_set_motor_direction(DIRN_STOP);

    pthread_mutex_lock(&status_mtx);

    memset(input.buttons, 0, N_BUTTONS * N_FLOORS * sizeof(int));
    memset(input.lamps, 0, N_BUTTONS * N_FLOORS * sizeof(int));

    status.working       = false;
    status.available     = true;
    status.finished      = false;
    status.action        = IDLE;
    status.current_floor = elev_get_floor_sensor_signal();
    status.next_floor    = -1;
    status.direction     = 0;

    if (status.current_floor == -1) {
        // Move to floor below if elevator startet in between floors
        elev_set_motor_direction(DIRN_DOWN);
        while (status.current_floor == -1) {
            status.current_floor = elev_get_floor_sensor_signal();
        }
        elev_set_motor_direction(DIRN_STOP);
    }

    elev_set_floor_indicator(status.current_floor);
    int last_floor = status.current_floor;
    pthread_mutex_unlock(&status_mtx);
    usleep(10000);
    updateStatus(status);

    bool            working;
    bool            available;
    int             current_floor;
    unsigned int    timeout_counter = 0;
    ElevatorActions action;
    while (1) {
        /***********************************************************************


            Driver thread's main loop:
            Check progress of current job, check for a physical error and check
            if any button is pressed.


        ***********************************************************************/
        pthread_mutex_lock(&status_mtx);
        working       = status.working;
        current_floor = status.current_floor;
        action        = status.action;
        available     = status.available;
        pthread_mutex_unlock(&status_mtx);

        if (working) evalJobProgress();

        if (current_floor == last_floor && action == MOVING && available) {
            timeout_counter++;

            if (timeout_counter == MV_TIMEOUT) {
                pthread_mutex_lock(&status_mtx);
                status.available = false;
                pthread_mutex_unlock(&status_mtx);
                updateStatus(status);
                timeout_counter = 0;
            }
        } else if (timeout_counter != 0) {
            timeout_counter = 0;
        } else if (!available && last_floor != current_floor) {
            pthread_mutex_lock(&status_mtx);
            status.available = true;
            pthread_mutex_unlock(&status_mtx);
            updateStatus(status);
        }

        last_floor = current_floor;
        checkInputs();

        if (elev_get_stop_signal()) {
            elev_set_motor_direction(DIRN_STOP);
            exit(0);
        }

        usleep(20);
    }     // while

    printf("Driver shut down!\n");
    pthread_exit(NULL);
    return NULL;
} // runDriver

bool drv_startJob(Job_t job)
{
    assert(job.floor >= 0 && job.floor < N_FLOORS);
    pthread_mutex_lock(&status_mtx);
    bool ret = false;

    /***************************************************************************


        Determine in which direction to move or if the doors should be opened,
        and send status update to elevatorcontrol


    ***************************************************************************/
    if (job.floor > status.current_floor) {
        elev_set_motor_direction(DIRN_UP);
        status.working    = true;
        status.action     = MOVING;
        status.next_floor = job.floor;
        status.direction  = DIRN_UP;

        job_btn = job.button;
        ret     = true;
    } else if (job.floor < status.current_floor) {
        elev_set_motor_direction(DIRN_DOWN);
        status.working    = true;
        status.action     = MOVING;
        status.next_floor = job.floor;
        status.direction  = DIRN_DOWN;

        job_btn = job.button;
        ret     = true;
    } else if (job.floor == status.current_floor) {
        elev_set_button_lamp(job.button, job.floor, 0);
        input.lamps[job.button][job.floor] = 0;
        elev_set_door_open_lamp(1);
        timer_start(3.0);

        status.working    = true;
        status.action     = OPEN;
        status.next_floor = job.floor;
        ret               = true;
    } // if

    pthread_mutex_unlock(&status_mtx);
    updateStatus(status);

    if (ret) {
        printf("Startet job\tBtn %d Floor %d\n", job.button, job.floor);
    }

    return ret;
} // drv_startJob

void drv_switchLights(Job_t job, int new_val)
{
    assert(new_val == 0 || new_val == 1);
    elev_set_button_lamp(job.button, job.floor, new_val);
    input.lamps[job.button][job.floor] = new_val;
}

void evalJobProgress(void)
{
    /***************************************************************************


        Check if elevator has reached it's destination if it's moving.
        Check if the doors should be closed if they are open.


    ***************************************************************************/
    pthread_mutex_lock(&status_mtx);
    switch (status.action) {
    case MOVING:
        if (status.current_floor == status.next_floor) {
            elev_set_motor_direction(DIRN_STOP);
            status.action    = OPEN;
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
                break;
            }

            elev_set_button_lamp(BUTTON_COMMAND, status.current_floor, 0);
            input.lamps[BUTTON_COMMAND][status.current_floor] = 0;
            elev_set_door_open_lamp(1);
            timer_start(3.0);
            updateStatus(status);
        } // if

        break;

    case OPEN:
        if (timer_timedOut()) {
            elev_set_door_open_lamp(0);
            elev_set_button_lamp(job_btn, status.current_floor, 0);
            status.working  = false;
            status.finished = true;
            status.action   = IDLE;
            updateStatus(status);
            status.finished = false;
        }

        break;

    case IDLE:
        break;
    }     // switch

    pthread_mutex_unlock(&status_mtx);
} // evalJobProgress

void checkButton(elev_button_type_t button, int floor)
{
    /***************************************************************************


        Checks if the button input has changed since last check.
        Tells elevatorcontrol about the call if the changed button is pressed.


    ***************************************************************************/
    if (input.buttons[button][floor] != elev_get_button_signal(button, floor)) {
        input.buttons[button][floor] = elev_get_button_signal(button, floor);

        if (input.buttons[button][floor] != 0) {
            assert((button != BUTTON_CALL_DOWN || floor != 0) &&
                   (button != BUTTON_CALL_UP || floor != N_FLOORS - 1));

            Job_t new_job;
            new_job.floor  = floor;
            new_job.button = button;
            pthread_mutex_lock(&status_mtx);
            updateStatus(status);
            pthread_mutex_unlock(&status_mtx);
            sendJob(new_job);
            elev_set_button_lamp(button, floor, 1);
            input.lamps[button][floor] = 1;
        }
    }
} // checkButton

void checkInputs(void)
{
    pthread_mutex_lock(&status_mtx);
    // Update current floor only when it's not in between floors
    if (status.current_floor != elev_get_floor_sensor_signal() &&
        elev_get_floor_sensor_signal() != -1) {
        status.current_floor = elev_get_floor_sensor_signal();
        elev_set_floor_indicator(status.current_floor);
    }
    pthread_mutex_unlock(&status_mtx);

    for (size_t f = 0; f < N_FLOORS; f++) {
        for (size_t b = 0; b < N_BUTTONS; b++) {
            checkButton(b, f);
        }
    }
} // checkInputs
