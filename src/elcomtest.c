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


static int str2int(char* str);

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

int main(int argc, char* argv[]) {
    if (argc != (2*NUM_ELEVATORS + 1)) {
        printf("Invalid number of arguments. Expected %d\n", 2*NUM_ELEVATORS);
        exit(1);
    }
    char* my_hostname = argv[1];
    char* port_ptr = strchr(argv[1], ':') + 1;
    int my_port = str2int(port_ptr);

    char** ips_and_ports = &argv[1];

    elcom_init(ips_and_ports);

    char received_msg[1024] = {0};
    size_t received_msg_length;

    char send_msg[1024] = {0};

    Job_t test_job = {my_port - 8000, BUTTON_CALL_UP, false, 0};

    ElevatorStatus_t status_test = {my_port-8000, 0, 0, 0, 0, IDLE, DIRN_STOP};

    OutsideCallsList_t outside_calls_list_test;
    size_t i;
    for ( i = 0; i < NUM_FLOORS; ++i) {
        outside_calls_list_test[i].up = 0;
        outside_calls_list_test[i].down = 0;
        outside_calls_list_test[i].el_id_up = my_port - 8000;
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

static int str2int(char* str) {
    int sum = 0;
    int i = 0;
    //First find end of string
    while (str[i+1] != '\0') {
        ++i;
    }
    int exp = 0;
    for (; i>=0; --i) {
        sum += (str[i] - '0' )*pow(10, exp++);
    }
    return sum;
}

