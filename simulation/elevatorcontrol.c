#include "elevatorcontrol.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define MAX_JOBS 20
static job_t jobs[MAX_JOBS];
static unsigned int num_jobs;       // Number of pending jobs
static ElevatorStatus status;
static pthread_mutex_t ectr_mtx;

void (*updateStatus)(ElevatorStatus);   // control module callback
void (*sendJob)(job_t);                 // control module callback

void* runElevatorcontrol();
void ectr_updateStatus(ElevatorStatus);
void ectr_receiveJob(job_t);

void ectr_start(UpdateStatusCallback_t stat_callback,
                SendJobCallback_t      job_callback)
{
    drv_start(&ectr_updateStatus, &ectr_receiveJob);
    sendJob = job_callback;
    updateStatus = stat_callback;
    pthread_t elevatorcontrol_thrd;
    pthread_create(&elevatorcontrol_thrd, NULL, runElevatorcontrol, NULL);
}

void* runElevatorcontrol()
{
    pthread_mutex_lock(&ectr_mtx);
    num_jobs = 0;
    memset(jobs, 0, MAX_JOBS * sizeof(job_t));

    status.working = false;
    status.finished = false;
    status.action = IDLE;
    status.current_floor = -1;
    status.next_floor = -1;
    status.direction = 0;

    pthread_mutex_unlock(&ectr_mtx);

    bool working;
    bool finished;
    int next_floor;
    int num;
    job_t top_job;
    while (1) {
        pthread_mutex_lock(&ectr_mtx);
        working = status.working;
        finished = status.finished;
        next_floor = status.next_floor;
        num = num_jobs;

        if (num_jobs > 0) {
            top_job = jobs[num_jobs - 1];
        }

        pthread_mutex_unlock(&ectr_mtx);

        if (!working) {
            if (finished) {
                pthread_mutex_lock(&ectr_mtx);
                assert(("Finished a job without any registered jobs",
                        num_jobs > 0));
                num_jobs--;
                num = num_jobs;

                if (num_jobs > 0) {
                    top_job = jobs[num_jobs - 1];
                }

                status.finished = false;
                pthread_mutex_unlock(&ectr_mtx);
            }

            if (num > 0) {
                drv_startJob(top_job);
            }
        } else if (working && top_job.floor != next_floor) {
            drv_startJob(top_job);
        }

        usleep(1000);
    } // while

    printf("Control shut down\n");
    pthread_exit(NULL);
    return NULL;
} // runElevatorcontrol

void ectr_updateStatus(ElevatorStatus new_status)
{
    pthread_mutex_lock(&ectr_mtx);
    status = new_status;
    //printf("Updated status:\nworking %d\nnext %d\n", status.working, status.next_floor);
    pthread_mutex_unlock(&ectr_mtx);
    updateStatus(new_status);
} // ectr_updateStatus

bool putFirst(job_t job)
{
    bool ret = false;
    pthread_mutex_lock(&ectr_mtx);
    if (num_jobs == 0) {
        ret = true;
    } else if (status.direction == DIRN_UP && status.next_floor > job.floor &&
               status.current_floor < job.floor && job.button == BUTTON_CALL_UP) {
        ret = true;
    } else if (status.direction == DIRN_DOWN && status.next_floor < job.floor &&
               status.current_floor > job.floor && job.button == BUTTON_CALL_DOWN) {
        ret = true;
    }
    pthread_mutex_unlock(&ectr_mtx);
    if (ret) puts("Put first!");
    return ret;
}

bool goodPos(job_t job, size_t pos)
{
    pthread_mutex_lock(&ectr_mtx);
    int floor_a = jobs[pos].floor;
    int button_a = jobs[pos].button;
    int floor_b = jobs[pos - 1].floor;
    pthread_mutex_unlock(&ectr_mtx);
    bool ret = false;

    if (button_a == BUTTON_CALL_UP && job.button == BUTTON_CALL_UP &&
        job.floor > floor_a && floor_b > job.floor) {
        ret = true;
    } else if (button_a == BUTTON_CALL_UP && job.button == BUTTON_CALL_DOWN &&
               job.floor > floor_a && floor_b < job.floor) {
        ret = true;
    } else if (button_a == BUTTON_CALL_UP && job.button == BUTTON_COMMAND &&
               job.floor > floor_a && (floor_b > job.floor || floor_b < floor_a)) {
        ret = true;
    } else if (button_a == BUTTON_CALL_DOWN && job.button == BUTTON_CALL_DOWN &&
               job.floor < floor_a && floor_b < job.floor) {
        ret = true;
    } else if (button_a == BUTTON_CALL_DOWN && job.button == BUTTON_CALL_UP &&
               job.floor < floor_a && floor_b > job.floor) {
        ret = true;
    } else if (button_a == BUTTON_CALL_DOWN && job.button == BUTTON_COMMAND &&
               job.floor < floor_a && (floor_b < job.floor || floor_b > floor_a)) {
        ret = true;
    } else if (button_a == BUTTON_COMMAND && job.button == BUTTON_COMMAND &&
               job.floor < floor_a && floor_b < job.floor) {
        ret = true;
    } else if (button_a == BUTTON_COMMAND && job.button == BUTTON_COMMAND &&
               job.floor > floor_a && floor_b > job.floor) {
        ret = true;
    }

    return ret;
}

size_t findPosition(job_t job)
{
    pthread_mutex_lock(&ectr_mtx);
    size_t pos = num_jobs;
    pthread_mutex_unlock(&ectr_mtx);

    if(!putFirst(job)) {
        pos--;
        for (pos; pos > 0; pos--) {
            if (goodPos(job, pos)) break;
        }
    }

    return pos;
} // findPosition

void insertJob(job_t job, size_t pos)
{
    assert(("Inserting job at invalid position",
            pos < MAX_JOBS));
    pthread_mutex_lock(&ectr_mtx);
    if (num_jobs == MAX_JOBS) {
        return;
    }

    for (size_t i = num_jobs; i > pos; i--) {
        jobs[i] = jobs[i - 1];
        jobs[i] = jobs[i - 1];
    }

    jobs[pos] = job;
    num_jobs++;
    printf("Inserted job\tBtn %d Floor %d Pos %u Num %d\n",
           job.button, job.floor, pos, num_jobs);
    pthread_mutex_unlock(&ectr_mtx);
} // insertJob

bool validJob(job_t job)
{
    assert(("Job validation: floor out of bounds",
            job.floor >= 0 && job.floor < N_FLOORS));

    pthread_mutex_lock(&ectr_mtx);

    // NOTE, add:
    // When validating an internal request return false if theres already an
    // external request for the same floor.
    bool ret = true;
    for (size_t i = 0; i < num_jobs; i++) {
        if (jobs[i].floor == job.floor && jobs[i].button == job.button) {
            ret = false;
        }
    }

    pthread_mutex_unlock(&ectr_mtx);
    return ret;
} // validJob

void ectr_handleJob(job_t job)
{
    if (validJob(job)) {
        size_t pos = findPosition(job);
        insertJob(job, pos);
    }
} // ectr_handleJob

void ectr_receiveJob(job_t job)
{
    if (job.button == BUTTON_COMMAND) {
        ectr_handleJob(job);
    } else {
        sendJob(job);
    }
}
