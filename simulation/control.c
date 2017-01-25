#include "control.h"
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

ElevatorStatus status;
static pthread_mutex_t ctr_mtx;
void (*passOnStatus)(ElevatorStatus);

#define MAX_REQUESTS 20
// col 0 - floor of the request
// col 1 - button pressed when requested
static int requests[MAX_REQUESTS][2];
static unsigned int num_req; // Number of pending requests
void (*passOnRequest)(int, int);

void* startControl(void* args)
{
    struct control_args* arguments = args;
    passOnStatus = arguments->passOnStatusPtr;
    passOnRequest = arguments->passOnRequestPtr;

    num_req = 0;
    memset(requests, 0, MAX_REQUESTS * 2 * sizeof(int));

    status.working = false;
    status.finished = false;
    status.action = IDLE;
    status.current_floor = -1;
    status.next_floor = -1;
    status.direction = 0;

    usleep(1000);

    bool working;
    bool finished;
    int next_floor;
    int num;
    int top_req[2];
    while (1) {
        pthread_mutex_lock(&ctr_mtx);
        working = status.working;
        finished = status.finished;
        next_floor = status.next_floor;
        num = num_req;
        if (num_req > 0) {
            top_req[0] = requests[num_req - 1][0];
            top_req[1] = requests[num_req - 1][1];
        }
        pthread_mutex_unlock(&ctr_mtx);

        if (!working) {
            if (finished) {
                pthread_mutex_lock(&ctr_mtx);
                num_req--;
                num = num_req;
                status.finished = false;
                pthread_mutex_unlock(&ctr_mtx);
            }

            if (num > 0) {
                drv_startJob(top_req[1], top_req[0]);
            }
        } else if (working && top_req[0] != next_floor) {
            drv_startJob(top_req[1], top_req[0]);
        }


        // if (status.current_floor == 2 && status.action == IDLE) {
        //     drv_startJob(BUTTON_CALL_UP, 0);
        // }
        // if (!status.working && status.current_floor != 2) {
        //     drv_startJob(BUTTON_CALL_DOWN, 2);
        // }
        usleep(500);
    } // while

    printf("Control shut down\n");
    pthread_exit(NULL);
    return NULL; // To avoid compiler warnings
} // startControl

void ctr_updateStatus(ElevatorStatus new_status)
{
    pthread_mutex_lock(&ctr_mtx);
    status = new_status;
    printf("Updated status:\nworking %d\nnext %d\n", status.working, status.next_floor);
    pthread_mutex_unlock(&ctr_mtx);
    passOnStatus(new_status);
} // ctr_updateStatus

int findPosition(int button, int floor)
{
    // reg_mtx must be locked when this function is called
    if (num_req == 0) {
        return 0;
    }

    return 0;
} // findPosition

void insertReq(int button, int floor, int pos)
{
    pthread_mutex_lock(&ctr_mtx);
    num_req = 0;
    if (num_req == MAX_REQUESTS || pos < 0 || pos >= MAX_REQUESTS) {
        return;
    }

    for (int i = num_req; i > pos; i--) {
        requests[i][0] = requests[i - 1][0];
        requests[i][1] = requests[i - 1][1];
    }

    requests[pos][0] = floor;
    requests[pos][1] = button;
    num_req++;
    pthread_mutex_unlock(&ctr_mtx);
} // insertReq

// Check for duplicates and out-of-bound values
bool validRequest(int button, int floor)
{
    if (floor < 0 || floor >= N_FLOORS) {
        return false;
    }

    pthread_mutex_lock(&ctr_mtx);

    for (size_t i = 0; i < num_req; i++) {
        if (requests[i][0] == floor && requests[i][1] == button) {
            return false;
        }
    }

    pthread_mutex_unlock(&ctr_mtx);
    return true;
} // validRequest

void ctr_handleRequest(int button, int floor)
{
    printf("Requested floor: %d btn: %d\n", floor, button);

    if (validRequest(button, floor)) {
        int pos = findPosition(button, floor);
        insertReq(button, floor, pos);
    }
} // ctr_handleRequest

void ctr_receiveRequest(int button, int floor)
{
    if (button == BUTTON_COMMAND) {
        ctr_handleRequest(button, floor);
    } else {
        passOnRequest(button, floor);
    }
}
