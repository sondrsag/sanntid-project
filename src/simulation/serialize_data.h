#include "globals.h"
#include <stdio.h>
#include <stdbool.h>

typedef enum {
    JOB   = 0,
    ELEVATOR_STATUS,
    OUTSIDE_CALLS,
    INTERNAL_CALLS,
    NOT_RECOGNIZED,
} Message_type_t;

Message_type_t identify_message_type(char* buffer);

int serialize_job_into_buffer(Job_t job,char* buffer, int buffer_size);
int de_serialize_job_from_buffer(char* buffer, Job_t *job);

int serialize_ElevatorStatus_into_buffer(ElevatorStatus_t S,char* buffer, int buffer_size);
int de_serialize_ElevatorStatus_from_buffer(char* buffer, ElevatorStatus_t *S);

int serialize_OutsideCallsList_into_buffer(OutsideCallsList_t outside_calls_list,int size_calls_list,char* buffer, int buffer_size);
int de_serialize_OutsideCallsList_from_buffer(char const * buffer, OutsideCallsList_t CallsList);

int serializeInternalCallsListIntoBuffer(InternalCallsList_t calls_list, char* buffer, size_t buffer_size);
int deserializeInternalCallsListFromBuffer(char const * buffer, InternalCallsList_t calls_list);

// ANTON, the one below will not be sent, or not?
//int serialize_floorstate_into_buffer(floorstate_t floor,char* buffer, int buffer_size);
//int de_serialize_floorstate_from_buffer(char* buffer, floorstate_t *floor);
