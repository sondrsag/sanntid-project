#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "network.h"

//#define NUM_ELEVATORS 2

int str2int(char* str) {
    int sum = 0;
    int i   = 0;
    //First find end of string
    while (str[i + 1] != '\0') {
        ++i;
    }
    int exp = 0;
    for (; i >= 0; --i) {
        sum += (str[i] - '0') * pow(10, exp++);
    }
    return sum;
}

int main(int argc, char* argv[]) {
    if (argc != (2 * NUM_ELEVATORS + 1)) {
        printf("Invalid number of arguments. Expected %d\n", 2 * NUM_ELEVATORS);
        exit(1);
    }
    char* my_hostname = argv[1];
    int   my_port     = str2int(argv[2]);
    char* hostname;
    int   port;
    int   i = 3;
    while ( i < argc ) {
        hostname = argv[i++];
        port     = str2int(argv[i++]);
        net_connect(hostname, port);
    }
    net_init(my_hostname, my_port);
    //net_listen(my_hostname, my_port);
    char receivedMsg[1024];
    char sendMsg[1024];
    while (true) {
        sleep(1);
        sprintf(sendMsg, "Hello from %s:%d. The time is: %d\n",
                my_hostname, my_port, (int)time(NULL));
        net_broadcast(sendMsg, 1024);
        while (!net_getMessage(receivedMsg)) {
            printf("Received message: %s\n", receivedMsg);
        }
        //net_getMessage(msg);
        //if (*msg) {
        //}
    }
    return 0;
} /* main */
