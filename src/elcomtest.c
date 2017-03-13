#define _GNU_SOURCE
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "elcom.h"
#include "network.h"
#include "globals.h"
#include "serialize_data.h"
#include "utils.h"


void testSerializeInternalCalls(void);
void testDeserializeInternalCalls(char const * buffer);

void testSerializeInternalCalls(void) {

    InternalCallsList_t calls_list;
    //Fill with some test data
    for (size_t elevator = 0; elevator < NUM_ELEVATORS; ++elevator) {
        for ( size_t floor = 0; floor < NUM_FLOORS; ++floor ) {
            calls_list[elevator][floor] = (elevator == floor);
        }
    }
    
    char buffer[1024] = {0};
    serializeInternalCallsListIntoBuffer(calls_list, buffer, 1024);
    printf("Serialized internal calls to: %s\n", buffer);
    testDeserializeInternalCalls(buffer);
}

void testDeserializeInternalCalls(char const * buffer) {

    InternalCallsList_t calls_list;

    deserializeInternalCallsListFromBuffer(buffer, calls_list);

    printf("Printing internal calls list\n");
    for (size_t elevator = 0; elevator < NUM_ELEVATORS; ++elevator) {
        for ( size_t floor = 0; floor < NUM_FLOORS; ++floor ) {
            printf("%d ", calls_list[elevator][floor]);
        }
        putc('\n', stdout);
    }

}

void testReadConfFile(void) {
    char buffer[1024] = {0};
    char const * conf_file_name = "elevators.conf";
    FILE * conf_file = fopen(conf_file_name, "r");
    if (!conf_file) {
        fprintf(stderr,"Could not open configuration file\n");
        exit(1);
    }
    //Read config file line by line
    while(fgets(buffer, sizeof(buffer), conf_file)) {
        if (buffer[0] == '/' && buffer[1] == '/') {
           //This line is a comment
           continue;
        }
        char ip[15];
        unsigned int port;
        unsigned int id;
        //Attempt to parse config file
        int ret = sscanf(buffer, "%15[^:]:%u %u", ip, &port, &id);
        if (ret == EOF || ret < 3) {
            fprintf(stderr, "ERROR: Could not read config file\n");
            fprintf(stderr, "Read %d parameters\n", ret);
            fprintf(stderr, "ip:%s\nport:%u\nid:%u\n", ip, port, id);
            exit(1);
        }
    }
    fclose(conf_file);
}

int main(int const argc, char const * const * const argv) {
    /*
    if (argc != (2*NUM_ELEVATORS + 1)) {
        printf("Invalid number of arguments. Expected %d\n", 2*NUM_ELEVATORS);
        exit(1);
    }
    */
    if (argc != 2) {
        printf("Invalid number of arguments. Expected 1\n");
        exit(1);
    }
    unsigned const int my_id = strtol(argv[1], NULL, 10);
    //testReadConfFile();
    elcom_init(my_id);

    char received_msg[1024] = {0};
    size_t received_msg_length;

    char send_msg[1024] = {0};

    Job_t test_job = {my_id - 8000, BUTTON_CALL_UP, false, 0};

    ElevatorStatus_t status_test = {my_id-8000, 0, 0, 0, 0, IDLE, DIRN_STOP};

    OutsideCallsList_t outside_calls_list_test;
    size_t i;
    for ( i = 0; i < NUM_FLOORS; ++i) {
        outside_calls_list_test[i].up = 0;
        outside_calls_list_test[i].down = 0;
        outside_calls_list_test[i].el_id_up = my_id - 8000;
        outside_calls_list_test[i].el_id_down = i;
    }
    
    InternalCallsList_t internal_calls_list_test;
    //Fill with some test data
    for (size_t elevator = 0; elevator < NUM_ELEVATORS; ++elevator) {
        for ( size_t floor = 0; floor < NUM_FLOORS; ++floor ) {
            internal_calls_list_test[elevator][floor] = (elevator == floor);
        }
    }

    while(true) {
        sleep(2);
        printf("Sending...\n");
        //elcom_broadcastJob(test_job);
        //elcom_broadcastElevatorStatus(status_test);
        //elcom_broadcastOutsideCallsList(outside_calls_list_test);
        elcom_broadcastInternalCallsList(internal_calls_list_test);
        //sprintf(send_msg, "Hello from %s:%d. The time is: %d",
        //        my_hostname, my_port, (int)time(NULL));
        //elcom_broadcast(send_msg, 100);
        sleep(2);
    }
    return 0;
}

