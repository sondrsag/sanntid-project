#include "globals.h"
#include <stdio.h>
#include <stdbool.h>

typedef enum {
    JOB   = 0,
    ELEVATOR_STATUS = 1,
    OUTSIDE_CALLS   = 2,
	NOT_RECOGNIZED   = 3
} message_type_t;

message_type_t identify_message_type(char* buffer);

int serialize_job_into_buffer(job_t job,char* buffer, int buffer_size);
int de_serialize_job_from_buffer(char* buffer, job_t *job);

int serialize_ElevatorStatus_into_buffer(ElevatorStatus S,char* buffer, int buffer_size);
int de_serialize_ElevatorStatus_from_buffer(char* buffer, ElevatorStatus *S);

int serialize_OutsideCallsList_into_buffer(floorstate_t *CallsList,int size_CallsList,char* buffer, int buffer_size);
int de_serialize_OutsideCallsList_into_buffer(char* buffer, floorstate_t *CallsList,int size_CallsList);

// ANTON, the one below will not be sent, or not?
//int serialize_floorstate_into_buffer(floorstate_t floor,char* buffer, int buffer_size);
//int de_serialize_floorstate_from_buffer(char* buffer, floorstate_t *floor);
