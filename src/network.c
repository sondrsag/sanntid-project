#include <stdint.h>
#include <stdbool.h>
//#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "dyad.h"

#define SIZE_BACKLOG 4

static unsigned int num_connected_streams;

static dyad_Stream* stream_list[NUM_ELEVATORS];

static void addStreamToList(dyad_Stream* s) {
    stream_list[num_connected_streams] = s;
    ++num_connected_streams;
}

static void removeStreamFromList(dyad_Stream* s) {
    //Find index of stream in list
    int i;
    for (i = 0; i < num_connected_streams; ++i) {
        if ( (strcmp(dyad_getAddress(s), dyad_getAddress(stream_list[i])) == 0)
        &&   (dyad_getPort(s) == dyad_getPort(stream_list[i])) ) {
            break;    
        }
    }
    if (i >= num_connected_streams) {
        printf("Stream not found in stream_list. Exiting\n");
        exit(1);
    }
    --num_connected_streams;
    //Move last stream into vacated slot
    stream_list[i] = stream_list[num_connected_streams];
    stream_list[num_connected_streams] = NULL;
}

static void onData(dyad_Event* e) {
    printf("Received data from %s:%d: %s\n", dyad_getAddress(e->stream),
            dyad_getPort(e->stream), e->data);
}

static void onClose(dyad_Event* e) {
    printf("Connection from %s:%d closed\n", dyad_getAddress(e->stream), dyad_getPort(e->stream));
    removeStreamFromList(e->stream);
}

static void onConnect(dyad_Event* e) {
    addStreamToList(e->stream);
    printf("Connected to %s:%d\n", dyad_getAddress(e->stream), dyad_getPort(e->stream));
}



static void onAccept(dyad_Event* e) {
    addStreamToList(e->remote);
    printf("Accepted connection from %s:%d\n",dyad_getAddress(e->remote), dyad_getPort(e->remote));
    dyad_addListener(e->remote, DYAD_EVENT_CLOSE, onClose, NULL);
    dyad_addListener(e->remote, DYAD_EVENT_DATA, onData, NULL);
}  


static void* workerThread() {
    while (true) {
        //stateMachine();
        sleep(1);
        dyad_update();
        //broadcastTest("Hello world");
        printf("Number of connected streams: %d\n", num_connected_streams);
        int i;
        for ( i = 0; i < num_connected_streams; ++i) {
            printf("Connected to: %s:%d\n",
                dyad_getAddress(stream_list[i]), dyad_getPort(stream_list[i]));
        }
    }
    //printf("No streams left. Shutting down\n");
    dyad_shutdown();
    pthread_exit(NULL);
    return NULL;
}

void net_listen(char* my_hostname, uint16_t my_port) {
    printf("Listening for connections to %s:%d ...\n", my_hostname, my_port);
    dyad_Stream* s = dyad_newStream();
    dyad_listenEx(s, my_hostname, my_port, SIZE_BACKLOG);
    dyad_addListener(s, DYAD_EVENT_ACCEPT, onAccept, NULL);
    dyad_addListener(s, DYAD_EVENT_DATA, onData, NULL);
    return;
}

void net_init(char* my_hostname, uint16_t my_port) {
    dyad_init();
    dyad_setUpdateTimeout(0);
    net_listen(my_hostname, my_port);
    pthread_t thread_dyad;
    pthread_create(&thread_dyad, NULL, workerThread, NULL);
}



static int tryConnect(dyad_Stream* s, char* hostname, int port) {
    dyad_connect(s, hostname, port);
    sleep(1);
    //We must update manually because the dyad worker thread may not be running
    dyad_update();
    if (dyad_getState(s) == DYAD_STATE_CONNECTED) {
        return 0;
    }
    return -1;
}

void net_connect(char* hostname, uint16_t port) {
    dyad_Stream* s = dyad_newStream();
    dyad_addListener(s, DYAD_EVENT_CONNECT, onConnect, NULL);
    dyad_addListener(s, DYAD_EVENT_CLOSE, onClose, NULL);
    dyad_addListener(s, DYAD_EVENT_DATA, onData, NULL);
    printf("Trying to connect to %s:%d ...\n", hostname, port);
    int ret = tryConnect(s, hostname, port);
    if (ret) {
        printf("No server found\n");
    }
    else {
        printf("Server found\n");
    }
}

void net_broadcast(char* data, size_t length) {
    printf("Broadcasting message: %s\n", data);
    int i;
    for (i = 0; i < num_connected_streams; ++i) {
        dyad_write(stream_list[i], data, length);
    }
}

char* net_getMessage(void) {
    return "test";
}

/*
connections_t net_getConnectedNodes(void) {
    return 0;
}
*/
