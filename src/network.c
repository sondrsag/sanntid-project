#define _GNU_SOURCE
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/queue.h>
#include <time.h>
#include <pthread.h>
#include "dyad.h"
#include "network.h"
#include "utils.h"
#include "globals.h"
#include "utils.h"
#include "work_distribution.h"

#define SIZE_BACKLOG NUM_ELEVATORS
#define MAX_MSG_SIZE 1024
//Number of symbols in an IP address including dots and \0
#define SIZE_IP 16
#define DYAD_TIMEOUT 1
#define STREAM_TIMEOUT 0.5

typedef struct elevatorInfo {
    char ip[SIZE_IP];
    uint16_t port;
    unsigned int id;
    dyad_Stream* stream;
} ElevatorInfo_t;

typedef struct Msg_queue_node {
    char message[MAX_MSG_SIZE];
    size_t length;
    int sender_id;
    STAILQ_ENTRY(Msg_queue_node_t) messages;
} Msg_queue_node_t;

STAILQ_HEAD(Msg_queue_head, Msg_queue_node);

typedef struct Msg_queue_head Msg_queue_head_t;


//Queues to hold incoming and outgoing messages
static Msg_queue_head_t incoming_messages_queue;
static Msg_queue_head_t outgoing_messages_queue;

static ElevatorInfo_t elevator_list[NUM_ELEVATORS];

pthread_mutex_t msg_mutex;
pthread_mutex_t module_mutex;
pthread_mutex_t stream_mutex;

static unsigned int my_id;

void msg_queue_addMessage(Msg_queue_head_t * queue, Msg_queue_node_t* node) {
    char const * message = node->message;
    size_t length = node->length;
    if (length > MAX_MSG_SIZE) {
        fprintf(stderr, "Received too long message. Discarding. %d > %d\n", (int)length, MAX_MSG_SIZE);
        printf("Message is: %s\n", message);
        return;
    }
    pthread_mutex_lock(&msg_mutex);
    STAILQ_INSERT_TAIL(queue, node, messages);
    pthread_mutex_unlock(&msg_mutex);
}

void msg_queue_newMessage(Msg_queue_head_t * queue, char const * message, size_t const length,
        int const sender_id) {
    
    Msg_queue_node_t* node;
    node = malloc(sizeof(Msg_queue_node_t));
    if (node) {
        memcpy(node->message, message, length);
        node->length = length;
        node->sender_id = sender_id;
    }
    else {
        fprintf(stderr, "ERROR: Could not allocate memory for message node");
        exit(1);
    }
    msg_queue_addMessage(queue, node);
}


static unsigned int num_connected_streams;

static dyad_Stream* stream_list[NUM_ELEVATORS];

int ip2elId(char const * ip);

static void addStreamToList(dyad_Stream* s) {
    int id = ip2elId(dyad_getAddress(s));
    elevator_list[id].stream = s;
    stream_list[id] = s;
    ++num_connected_streams;
}

static void removeStreamFromList(dyad_Stream* s) {
    //Find index of stream in list
    char const * ip = dyad_getAddress(s);
    int id = ip2elId(ip);
    stream_list[id] = NULL;
    elevator_list[id].stream = NULL;
    --num_connected_streams;
}

int ip2elId(char const * ip) {
    size_t i;
    for ( i = 0; i < NUM_ELEVATORS; ++i) {
        if (strcmp(ip, elevator_list[i].ip) == 0) {
            return i;
        }
    }
    fprintf(stderr,"Unknown address. We're being hacked. Aborting\n");
    fprintf(stderr,"Address is: %s\n", ip);
    exit(1);
    return 0;
}

static void onData(dyad_Event* e) {

    int sender_id = ip2elId(dyad_getAddress(e->stream));

    //Check for concatenated messages
    unsigned int num_messages = (e->size)/MESSAGE_LENGTH;

    for (unsigned int i = 0; i < num_messages; ++i) {
        char* msg_ptr = e->data + i*MESSAGE_LENGTH;
        //Scan past null bytes
        while (*msg_ptr == 0) {
            ++msg_ptr;
        }
        //Allocate message and add to incoming messages queue
        Msg_queue_node_t* node = checkMalloc(sizeof(Msg_queue_node_t));
        memcpy(node->message, msg_ptr, MESSAGE_LENGTH);
        node->length = MESSAGE_LENGTH;
        node->sender_id = sender_id;
        mutex_lock(&msg_mutex);
        STAILQ_INSERT_TAIL(&incoming_messages_queue, node, messages);
        mutex_unlock(&msg_mutex);
    }
}

static void onClose(dyad_Event* e) {
    unsigned int const id = ip2elId(dyad_getAddress(e->stream));
    ElevatorStatus_t status = {0, 0, 0, 0, 0, IDLE, DIRN_STOP};
    wd_updateElevStatus(status, id);
    printf("Connection from %s:%d closed\n", dyad_getAddress(e->stream), dyad_getPort(e->stream));
    removeStreamFromList(e->stream);
}

static void onConnect(dyad_Event* e) {
    addStreamToList(e->stream);
    printf("Connected to %s:%d\n", dyad_getAddress(e->stream), dyad_getPort(e->stream));
    dyad_setTimeout(e->stream, STREAM_TIMEOUT);

    //Let work distribution know that this elevator is available
    unsigned int const id = ip2elId(dyad_getAddress(e->stream));
    ElevatorStatus_t status = {0, 0, 0, 0, true, IDLE, DIRN_STOP};
    wd_updateElevStatus(status, id);
}

static void onError(dyad_Event* e) {
    printf("Dyad error: %s\n", e->msg);
    dyad_removeAllListeners(e->stream, DYAD_EVENT_CLOSE);
}

static void onAccept(dyad_Event* e) {
/*
    addStreamToList(e->remote);
    printf("Accepted connection from %s:%d\n",dyad_getAddress(e->remote), dyad_getPort(e->remote));
    dyad_addListener(e->remote, DYAD_EVENT_CLOSE, onClose, NULL);
    dyad_addListener(e->remote, DYAD_EVENT_DATA, onData, NULL);
    dyad_addListener(e->remote, DYAD_EVENT_ERROR, onError, NULL);
    dyad_setTimeout(e->remote, STREAM_TIMEOUT);

    //Let work distribution know that this elevator is available
    unsigned int const id = ip2elId(dyad_getAddress(e->remote));
    ElevatorStatus_t status = {0, 0, 0, 0, true, IDLE, DIRN_STOP};
    wd_updateElevStatus(status, id);
*/
}  

static void connectToPeers() {
    for (unsigned int id = 0; id < NUM_ELEVATORS; ++id) {
        if (id == my_id) continue;
        if (elevator_list[id].stream == NULL) {
            elevator_list[id].stream = dyad_newStream();
        }
        char* ip = elevator_list[id].ip;
        uint16_t port = elevator_list[id].port;
        dyad_Stream* stream = elevator_list[id].stream;
        int stream_state = dyad_getState(stream);
        if (stream_state != DYAD_STATE_CONNECTED && stream_state != DYAD_STATE_CONNECTING ) {
            dyad_connect(stream, ip, port);
        }
    }
}

static void popAndBroadcast(Msg_queue_head_t * queue) {

    mutex_lock(&msg_mutex);
    if (STAILQ_EMPTY(queue)) {
        pthread_mutex_unlock(&msg_mutex);
        return;
    }
    Msg_queue_node_t* node = STAILQ_FIRST(&outgoing_messages_queue);
    for (unsigned int i = 0; i < NUM_ELEVATORS; ++i) {
        if (stream_list[i] == NULL) continue;
        dyad_write(stream_list[i], node->message, node->length);
    }

    STAILQ_REMOVE_HEAD(queue, messages);
    mutex_unlock(&msg_mutex);
    free(node);
}

static void* workerThread() {
    while (true) {
        connectToPeers();
        popAndBroadcast(&outgoing_messages_queue);
        //Sleep a bit so that the process doesn't consume too much cpu time
        usleep(1);
        dyad_update();
    }
    dyad_shutdown();
    pthread_exit(NULL);
    return NULL;
}

void populateElevatorList(void) {
    char buffer[1024] = {0};
    char const * conf_file_name = "network_config.conf";
    FILE * conf_file = fopen(conf_file_name, "r");
    if (!conf_file) {
        fprintf(stderr,"Could not open configuration file\n");
        exit(1);
    }

    unsigned int el_configs_read = 0;
    while (el_configs_read < NUM_ELEVATORS) {
        char* res = fgets(buffer, sizeof(buffer), conf_file);
        if (!res) {
            fprintf(stderr,"Error when reading config file. Unexpected end of file\n");
        }
        if (buffer[0] == '/' && buffer[1] == '/') {
           //The parsed line is a comment
           continue;
        }
        char ip[15] = {0};
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
        elevator_list[id].id = id;
        elevator_list[id].port = port;
        strcpy(elevator_list[id].ip, ip);
        ++el_configs_read;
    }
    fclose(conf_file);
}

static void setMyId(unsigned int const _my_id) {
    pthread_mutex_lock(&module_mutex);
    my_id = _my_id;
    pthread_mutex_unlock(&module_mutex);
}

void net_init(unsigned int const _my_id) {
    //Init mutexes
    if (pthread_mutex_init(&msg_mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        exit(1);
    }
    if (pthread_mutex_init(&stream_mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        exit(1);
    }
     if (pthread_mutex_init(&module_mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        exit(1);
    }

    STAILQ_INIT(&incoming_messages_queue);
    STAILQ_INIT(&outgoing_messages_queue);

    dyad_init();
    dyad_setUpdateTimeout(0);

    setMyId(_my_id);

    populateElevatorList();
    //Connect to other elevators
    for ( unsigned int i = 0; i < NUM_ELEVATORS; ++i) {
        int const id = elevator_list[i].id;
        //Skip self
        if (id == my_id) continue;
        char const * hostname = elevator_list[i].ip;
        uint16_t const port = elevator_list[i].port;
        net_connect(hostname, port);
    }
    //Listen for incoming connections
    char const * my_ip = elevator_list[my_id].ip;
    uint16_t const my_port = elevator_list[my_id].port;

    net_listen(my_ip, my_port);

    //Start worker thread
    pthread_t thread_dyad;
    pthread_create(&thread_dyad, NULL, workerThread, NULL);
}



static int tryConnect(dyad_Stream* s, char const * hostname, int const port) {
    dyad_connect(s, hostname, port);
    clock_t connect_time = 0.5*CLOCKS_PER_SEC; //clock cycles
    clock_t start_time = clock();
    clock_t diff = 0;
    while ( diff < connect_time) {
        //We must update manually because the dyad worker thread may not be running
        dyad_update();
        if (dyad_getState(s) == DYAD_STATE_CONNECTED) {
            return 0;
        }
        diff = clock() - start_time;
    }
    return -1;
}

void net_listen(char const * my_hostname, uint16_t my_port) {
    printf("Listening for connections to %s:%d ...\n", my_hostname, my_port);
    dyad_Stream* s = dyad_newStream();
    dyad_listenEx(s, my_hostname, my_port, SIZE_BACKLOG);
    dyad_addListener(s, DYAD_EVENT_ACCEPT, onAccept, NULL);
    dyad_addListener(s, DYAD_EVENT_DATA, onData, NULL);
    dyad_addListener(s, DYAD_EVENT_ERROR, onError, NULL);
}

void net_connect(char const * hostname, uint16_t const port) {
    dyad_Stream* s = dyad_newStream();
    dyad_addListener(s, DYAD_EVENT_CONNECT, onConnect, NULL);
    dyad_addListener(s, DYAD_EVENT_ERROR, onError, NULL);
    dyad_addListener(s, DYAD_EVENT_CLOSE, onClose, NULL);
    dyad_addListener(s, DYAD_EVENT_DATA, onData, NULL);
    printf("Trying to connect to %s:%d ...\n", hostname, port);
    int ret = tryConnect(s, hostname, port);
    if (ret) {
        printf("No server found\n");
        //dyad_close(s);
    }
    else {
        printf("Server found\n");
    }
}

void net_broadcast(char* data, size_t length) {
    Msg_queue_node_t* node = checkMalloc(sizeof(Msg_queue_node_t));
    memcpy(node->message, data, length);
    node->length = length;
    node->sender_id = 0;
    mutex_lock(&msg_mutex);
    STAILQ_INSERT_TAIL(&outgoing_messages_queue, node, messages);
    mutex_unlock(&msg_mutex);

}

int net_getMessage(char* target, size_t* received_msg_length, int* sender_id) {
    pthread_mutex_lock(&msg_mutex);
    if (STAILQ_EMPTY(&incoming_messages_queue)) {
        pthread_mutex_unlock(&msg_mutex);
        return -1;
    }

    Msg_queue_node_t* node = STAILQ_FIRST(&incoming_messages_queue);

    //Copy data to caller
    memcpy(target, node->message, node->length);
    *received_msg_length = node->length;
    *sender_id = node->sender_id;

    //Remove node from queue
    STAILQ_REMOVE_HEAD(&incoming_messages_queue, messages);
    free(node);

    pthread_mutex_unlock(&msg_mutex);
    return 0;
}

int net_getMasterId(void) {
    pthread_mutex_lock(&module_mutex);
    //Iterate over elevator connections. First available elevator is master
    unsigned int id;
    for ( id = 0; id < NUM_ELEVATORS; ++id ) {
        if (id == my_id ) {
            break;
        }   
        if(stream_list[id] !=NULL) {
            break;
        }
    }
    pthread_mutex_unlock(&module_mutex);
    return id;
}

