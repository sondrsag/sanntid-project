#define _GNU_SOURCE
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "elcom.h"
#include "network.h"
#include "serialize_data.h"
#include "globals.h"
#include <stdbool.h>
#include "work_distribution.h"

static bool elcom_just_started;

static void printMessage(char* message, size_t length) {
    size_t i;
    for ( i = 0; i < length; ++i) {
        putc(message[i], stdout);
    }
    putc('\n', stdout);
}

void printJob(Job_t job) {
    printf(("Floor: %d\n"
            "Button: %d\n"
            "Finished?: %d\n"
            "Assigned to: %d\n"),
           job.floor, job.button, job.finished, job.assignee);
}

void printElevatorStatus(ElevatorStatus_t status) {
    printf(("Working: %d\n"
            "Finished?: %d\n"
            "Current floor: %d\n"
            "Next floor: %d\n"
            "Availble: %d\n"
            "Action: %d\n"
            "Direction: %d\n"),
           status.working, status.finished, status.current_floor,
           status.next_floor, status.available, status.action, status.direction);
}

void printOutsideCallsList(OutsideCallsList_t list) {
    for ( size_t i = 0; i < NUM_FLOORS; ++i ) {
        printf("Floor %zd\n", i);
        printf(("Up: %d\n"
                "Down: %d\n"
                "Id up: %d\n"
                "Id down: %d\n"), list[i].up, list[i].down,
               list[i].el_id_up, list[i].el_id_down);
    }
}

void printInternalCallsList(InternalCallsList_t list) {
    printf("Printing internal calls_list\n");
    for ( size_t elevator = 0; elevator < NUM_ELEVATORS; ++elevator ) {
        for ( size_t floor = 0; floor < NUM_FLOORS; ++floor ) {
            printf("%d ", (int)list[elevator][floor]);
        }
        putc('\n', stdout);
    }
}

static void deserializeMessageAndDistribute(Message_type_t message_type,
                                            char*          message,
                                            size_t         message_length,
                                            int            sender_id) {
    if (message_type == JOB) {
        Job_t job;
        int   ret = de_serialize_job_from_buffer(message, &job);
        if (ret == -1) {
            fprintf(stderr, ("Could not deserialize message into Job."
                             "Malformed or missing data. Discarding"));
            fprintf(stderr, "Message was:\n");
            printMessage(message, message_length);
            return;
        }
        wd_receiveJob_from_elcom(job);
    }
    else if (message_type == ELEVATOR_STATUS) {
        ElevatorStatus_t status;
        int              ret = de_serialize_ElevatorStatus_from_buffer(message, &status);
        if (ret == -1) {
            fprintf(stderr, ("Could not deserialize message into ElevatorStatus."
                             "Malformed or missing data. Discarding"));
            fprintf(stderr, "Message was:\n");
            printMessage(message, message_length);
            return;
        }

        wd_updateElevStatus(status, sender_id);
    }
    else if (message_type == OUTSIDE_CALLS) {
        OutsideCallsList_t list;

        int ret = de_serialize_OutsideCallsList_from_buffer(message, list);
        if (ret == -1) {
            fprintf(stderr, ("Could not deserialize message into external calls list."
                             "Malformed or missing data. Discarding"));
            fprintf(stderr, "Message was:\n");
            printMessage(message, message_length);
            return;
        }

        if (sender_id == net_getMasterId()) {
            wd_receiveCallsListFromPrimary(list);
        }
    }
    else if (message_type == INTERNAL_CALLS) {
        InternalCallsList_t calls_list;

        int ret = deserializeInternalCallsListFromBuffer(message, calls_list);
        if (ret) {
            fprintf(stderr, ("Could not deserialize internal calls list message."
                             "Discarding.\n"
                             "Message was:\n"));
            printMessage(message, message_length);
            return;
        }

        if (elcom_just_started == true) {
            wd_HandleInternalCallsAfterRestart(calls_list);
            elcom_just_started = false;
        }
    } else {
        fprintf(stderr, "Unable to interpret message, discarding.\nMessage was: ");
        printMessage(message, message_length);
    }
} /* deserializeMessageAndDistribute */

static void* workerThread() {
    char received_msg[1024];

    size_t received_msg_length;

    int sender_id;

    elcom_just_started = true;


    sleep(2);
    while (true) {
        usleep(1000);
        while (net_getMessage(received_msg, &received_msg_length, &sender_id) == 0) {
            if (received_msg_length != MESSAGE_LENGTH) {
                fprintf(stderr, ("ERROR: Received message of invalid length %zd!=%d. "
                                 "Too lazy to handle. Discarding.\n"),
                        received_msg_length, MESSAGE_LENGTH);
                continue;
            }

            Message_type_t message_type = identify_message_type(received_msg);
            deserializeMessageAndDistribute(message_type,
                                            received_msg,
                                            received_msg_length,
                                            sender_id);
        }
    }
    return NULL;
} /* workerThread */


void elcom_broadcastJob(Job_t job) {
    char buf[MESSAGE_LENGTH] = {0};
    serialize_job_into_buffer(job, buf, sizeof(buf));
    net_broadcast(buf, sizeof(buf));
}

void elcom_broadcastOutsideCallsList(OutsideCallsList_t outside_calls_list) {
    char buf[MESSAGE_LENGTH] = {0};

    serialize_OutsideCallsList_into_buffer(outside_calls_list,
                                           sizeof(OutsideCallsList_t),
                                           buf,
                                           sizeof(buf));

    net_broadcast(buf, sizeof(buf));
}

void elcom_broadcastInternalCallsList(InternalCallsList_t calls_list) {
    char buf[MESSAGE_LENGTH] = {0};
    serializeInternalCallsListIntoBuffer(calls_list, buf, sizeof(buf));
    net_broadcast(buf, sizeof(buf));
}

void elcom_broadcastElevatorStatus(ElevatorStatus_t status) {
    char buf[MESSAGE_LENGTH] = {0};
    serialize_ElevatorStatus_into_buffer(status, buf, sizeof(buf));
    net_broadcast(buf, sizeof(buf));
}

void elcom_broadcast(char* msg, size_t length) {
    net_broadcast(msg, length);
}


void elcom_init(unsigned int const my_id) {
    net_init(my_id);

    //Start worker thread
    pthread_t thread_elcom;
    pthread_create(&thread_elcom, NULL, workerThread, NULL);
}
