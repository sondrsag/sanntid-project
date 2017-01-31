#include "elcom.h"
#include "network.h"

static char* serializeJob(job_t job);
static char* serializeState(systemstate_t state);

void elcom_broadcastJob(job_t job) {
    //pre-allocated buffer
    char* job_string = serializeJob(job);
    net_broadcast(job_string);
}

void elcom_broadcastState(systemstate_t state) {
    //pre-allocated buffer
    char* state_string = serializeState(state);
    net_broadcast(state_string);
}

int elcom_numJobsReceived(void) {
    return 0;
}

/*
elStatuses_t elcom_getElStatus(void) {

}
*/
/*
job_t elcom_getJob(void) {
    
}
*/

static char* serializeJob(job_t job) {
    job = job;
    return "test";
}

static char* serializeState(systemstate_t state) {
    state = state;
    return "test";
}
