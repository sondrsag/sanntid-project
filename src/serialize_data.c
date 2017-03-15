#include "serialize_data.h"
#include <string.h>

Message_type_t identify_message_type(char* buffer)
{
    Message_type_t for_return;
    char           dummy[10];
    int            result = sscanf(buffer, "%s", dummy);

    if (result != 1) for_return = NOT_RECOGNIZED;

    if (dummy[0] == 'j') for_return = JOB;
    else if (dummy[0] == 'e') for_return = ELEVATOR_STATUS;
    else if (dummy[0] == 'o') for_return = OUTSIDE_CALLS;
    else if (dummy[0] == 'i') for_return = INTERNAL_CALLS;
    else for_return = NOT_RECOGNIZED;

    return for_return;
}

int serialize_job_into_buffer(Job_t job, char* buffer, int buffer_size)
{
    int chksum = job.floor + job.button + job.finished + job.assignee;
    int result = sprintf(buffer,
                         "job %d %d %d %d %d",
                         chksum,
                         job.floor,
                         job.button,
                         job.finished,
                         job.assignee);

    if (result > buffer_size) {return -1; };

    return result;
}

int de_serialize_job_from_buffer(char* buffer, Job_t *job)
{
    int chksum;
    int result = sscanf(buffer,
                        "job %d %d %d %d %d",
                        &chksum,
                        &(job->floor),
                        (int*)&(job->button),
                        (int*)&(job->finished),
                        &(job->assignee));

    if (result != 5) {return -1; }
    int chksum_check = job->floor + job->button + job->finished + job->assignee;
    if (chksum_check != chksum) {return -1; }

    return result;
}

int serialize_ElevatorStatus_into_buffer(ElevatorStatus_t S, char* buffer, int buffer_size)
{
    int chksum = S.working
                 + S.finished
                 + S.current_floor
                 + S.next_floor
                 + S.available
                 + S.action
                 + S.direction;
    int result = sprintf(buffer,
                         "elevator %d %d %d %d %d %d %d %d",
                         chksum,
                         S.working,
                         S.finished,
                         S.current_floor,
                         S.next_floor,
                         S.available,
                         S.action,
                         S.direction);

    if (result > buffer_size) {return -1; };

    return result;
} /* serialize_ElevatorStatus_into_buffer */

int de_serialize_ElevatorStatus_from_buffer(char* buffer, ElevatorStatus_t *S)
{
    int chksum;
    int result = sscanf(buffer,
                        "elevator %d %d %d %d %d %d %d %d",
                        &chksum,
                        (int*)&(S->working),
                        (int*)&(S->finished),
                        &(S->current_floor),
                        &(S->next_floor),
                        (int*)&(S->available),
                        (int*)&(S->action),
                        &(S->direction));

    if (result != 8) {return -1; }

    int chksum_check = S->working
                       + S->finished
                       + S->current_floor
                       + S->next_floor
                       + S->available
                       + S->action
                       + S->direction;

    if (chksum_check != chksum) {return -1; }

    return result;
} /* de_serialize_ElevatorStatus_from_buffer */

int serialize_OutsideCallsList_into_buffer(OutsideCallsList_t calls_list,
                                           int                size_CallsList,
                                           char*              buffer,
                                           int                buffer_size)
{
    int chksum = 0;
    int result = 0;

    if (buffer_size < (size_CallsList * 2 + sizeof("outside"))) {
        printf(("In serialize_OutsideCallsList_into_buffer, "
                "size of allocated buffer is too small, required size %lu"),
               (size_CallsList * sizeof(FloorCalls_t) * 2 + sizeof("Outside")));

        return -1;
    }

    strcpy(buffer, "outside ");

    for (size_t i = 0; i < NUM_FLOORS; ++i) {
        chksum += chksum
                  + calls_list[i].up
                  + calls_list[i].down
                  + calls_list[i].el_id_up
                  + calls_list[i].el_id_down;
        result = sprintf(&buffer[strlen(buffer)],
                         "%d %d %d %d ",
                         (int)(calls_list[i].up),
                         (int)(calls_list[i].down),
                         (int)(calls_list[i].el_id_up),
                         (int)(calls_list[i].el_id_down));
    }

    return result;
} /* serialize_OutsideCallsList_into_buffer */

int de_serialize_OutsideCallsList_from_buffer(char const *       buffer,
                                              OutsideCallsList_t calls_list)
{
    char* buf_ptr = strchr(buffer, ' ') + 1;
    for ( size_t i = 0; i < NUM_FLOORS; ++i ) {
        sscanf(buf_ptr, "%d", (int*)&calls_list[i].up);
        buf_ptr = strchr(buf_ptr, ' ') + 1;
        sscanf(buf_ptr, "%d", (int*)&calls_list[i].down);
        buf_ptr = strchr(buf_ptr, ' ') + 1;
        sscanf(buf_ptr, "%d", (int*)&calls_list[i].el_id_up);
        buf_ptr = strchr(buf_ptr, ' ') + 1;
        sscanf(buf_ptr, "%d", (int*)&calls_list[i].el_id_down);
        buf_ptr = strchr(buf_ptr, ' ') + 1;
    }
    return 0;
}


int serializeInternalCallsListIntoBuffer(InternalCallsList_t calls_list, char* buffer,
                                         size_t buffer_size)
{
    if (buffer_size < (sizeof(InternalCallsList_t) * 2 + sizeof("internal "))) {
        fprintf(stderr, ("Too small buffer size when serializing internal calls list."
                         "Size is %zd"), buffer_size);
        return -1;
    }
    strcpy(buffer, "internal ");
    char* buf_ptr = strchr(buffer, ' ') + 1;
    for (size_t elevator = 0; elevator < NUM_ELEVATORS; ++elevator) {
        for (size_t floor = 0; floor < NUM_FLOORS; ++floor) {
            sprintf(buf_ptr, "%d ", calls_list[elevator][floor]);
            buf_ptr = strchr(buf_ptr, ' ') + 1;
        }
    }
    return 0;
}

int deserializeInternalCallsListFromBuffer(char const *        buffer,
                                           InternalCallsList_t calls_list)
{
    //Skip message identifier
    char* buf_ptr = strchr(buffer, ' ') + 1;
    for (size_t elevator = 0; elevator < NUM_ELEVATORS; ++elevator) {
        for (size_t floor = 0; floor < NUM_FLOORS; ++floor) {
            if (buf_ptr == NULL) {
                printf("buf_ptr in deserializeInternalCallsListFromBuffer points to NULL\n");
                return -1;
            }
            sscanf(buf_ptr, "%d ", (int*)&calls_list[elevator][floor]);

            buf_ptr = strchr(buf_ptr, ' ') + 1;
        }
    }

    return 0;
} /* deserializeInternalCallsListFromBuffer */
