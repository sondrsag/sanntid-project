#define _GNU_SOURCE
#include <stdint.h>
#include <stdbool.h>
//#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/queue.h>
#include <time.h>
#include <pthread.h>
#include "dyad.h"
#include "network.h"
#include "msg_queue.h"
#include "utils.h"
#include "globals.h"

#define SIZE_BACKLOG NUM_ELEVATORS
#define MAX_MSG_SIZE 1024
//Number of symbols in an IP address including dots and
#define SIZE_IP 15
#define DYAD_TIMEOUT 1

//Queues to hold incoming and outgoing messages
static Msg_queue_head_t received_messages_head;
static Msg_queue_head_t send_messages_head;

//This list holds IP-adresses of elevators. Add 1 for \0
char elevator_ip_list[NUM_ELEVATORS][SIZE_IP + 1];

pthread_mutex_t msg_mutex;
pthread_mutex_t stream_mutex;

void msg_queue_addMessage(Msg_queue_head_t * queue, Msg_queue_node_t* node) {
    char const * message = node->message;
    size_t length = node->length;
    if (length > MAX_MSG_SIZE) {
        fprintf(stderr, "Received too long message. Discarding. %d > %d\n", (int)length, MAX_MSG_SIZE);
        printf("Message is: %s\n", message);
        return;
    }
    /*
    printf("Received message %s\n", message);
    printf("Adding to queue\n");
    */
    pthread_mutex_lock(&msg_mutex);
    STAILQ_INSERT_TAIL(queue, node, messages);
    /*
    printf("Messages in queue: \n");
    STAILQ_FOREACH(en, &messages_head, entries) {
        printf("%s\n", en->message);
    }
    printf("End messages in queue. \n");
    */
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

static void addIpToIpList(char const * ip, int id) {
    if ( (id < 0) || ( id >= NUM_ELEVATORS)) {
        fprintf(stderr, "ERROR: id must be an integer in interval [0,NUM_ELEVATORS). It was: %d\n", id);
        exit(1);
    }
    strcpy(elevator_ip_list[id], ip);
}

int ip2elId(char const * ip) {
    size_t i;
    for ( i = 0; i < NUM_ELEVATORS; ++i) {
        if (strcmp(ip, elevator_ip_list[i]) == 0) {
            return i;
        }
    }
    fprintf(stderr,"Message received from unknown address. We're being hacked. Aborting\n");
    exit(1);
    return 0;
}

static void onData(dyad_Event* e) {
    printf("Received data from %s:%d: %s\n", dyad_getAddress(e->stream),
           dyad_getPort(e->stream), e->data);
    /*
    printf("Received data from %s:%d:", dyad_getAddress(e->stream), dyad_getPort(e->stream));
    int i;
    for ( i = 0; i < e->size; ++i) {
        putc(e->data[i], stdout);
    }
    putc('\n',stdout);
    printf("Message length: %d\n", e->size);
    */

    //TODO: Add elevator id to message
    int id = ip2elId(dyad_getAddress(e->stream));
    printf("IP is: %s\n", dyad_getAddress(e->stream));
    printf("Id is: %d\n", id);
    msg_queue_newMessage(&received_messages_head, e->data, e->size, id);

    //msg_queue_addMessage(&received_messages_head, node);
}

static void onClose(dyad_Event* e) {
    printf("Connection from %s:%d closed\n", dyad_getAddress(e->stream), dyad_getPort(e->stream));
    removeStreamFromList(e->stream);
}

static void onConnect(dyad_Event* e) {
    addStreamToList(e->stream);
    printf("Connected to %s:%d\n", dyad_getAddress(e->stream), dyad_getPort(e->stream));
}

static void onError(dyad_Event* e) {
    printf("Dyad error: %s\n", e->data);
    exit(1);
}

static void onAccept(dyad_Event* e) {
    addStreamToList(e->remote);
    printf("Accepted connection from %s:%d\n",dyad_getAddress(e->remote), dyad_getPort(e->remote));
    dyad_addListener(e->remote, DYAD_EVENT_CLOSE, onClose, NULL);
    dyad_addListener(e->remote, DYAD_EVENT_DATA, onData, NULL);
    dyad_addListener(e->remote, DYAD_EVENT_ERROR, onError, NULL);
}  


static void popAndBroadcast(Msg_queue_head_t * queue) {
    pthread_mutex_lock(&msg_mutex);
    if (STAILQ_EMPTY(queue)) {
        pthread_mutex_unlock(&msg_mutex);
        return;
    }
    Msg_queue_node_t* node = STAILQ_FIRST(&send_messages_head);
    int i;
    for (i = 0; i < num_connected_streams; ++i) {
        dyad_write(stream_list[i], node->message, node->length);
    }
    STAILQ_REMOVE_HEAD(queue, messages);
    pthread_mutex_unlock(&msg_mutex);
    free(node);
}

static void* workerThread() {
    while (true) {
        popAndBroadcast(&send_messages_head);
        dyad_update();
        //Sleep a bit so that the process doesn't consume too much cpu time
        usleep(1);
    }
    dyad_shutdown();
    pthread_exit(NULL);
    return NULL;
}

void net_init(char* ips[]) {
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
    //Init messagequeue
    STAILQ_INIT(&received_messages_head);
    STAILQ_INIT(&send_messages_head);
    //Init dyad
    dyad_init();
    dyad_setUpdateTimeout(0);
    //Connect to other elevators
    size_t i;
    //Increment by 2 to skip the id
    for ( i = 2; i < 2*NUM_ELEVATORS; i += 2) {
        char* hostname = ips[i];
        //Find port
        char* port_ptr = strchr(ips[i], ':');
        if (port_ptr == NULL) {
            //No port number specified
            fprintf(stderr, "No port number specified. Address should be on form hostname:port\n");
            exit(1);
        }
        //Replace : with \0 so that net_connect will not include port as part of hostname
        *port_ptr = '\0';

        //Add 1 to skip ':' char
        uint16_t port = str2int(port_ptr + 1);
        net_connect(hostname, port);

        //Add ip to list of ips with id
        int id = str2int(ips[i+1]);
        addIpToIpList(hostname, id);
    }
    //Listen for incoming connections
    char* my_hostname = ips[0];
    char* port_ptr = strchr(ips[0], ':');
    int my_port = str2int(port_ptr + 1);
    *port_ptr = '\0';

    //Add ip to list of ips with id
    int id = str2int(ips[1]);
    addIpToIpList(my_hostname, id);

    for (i = 0; i < NUM_ELEVATORS; ++i){
        printf("%s\n", elevator_ip_list[i]);
    }

    net_listen(my_hostname, my_port);

    //Start worker thread
    pthread_t thread_dyad;
    pthread_create(&thread_dyad, NULL, workerThread, NULL);
}



static int tryConnect(dyad_Stream* s, char* hostname, int port) {
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

void net_listen(char* my_hostname, uint16_t my_port) {
    printf("Listening for connections to %s:%d ...\n", my_hostname, my_port);
    dyad_Stream* s = dyad_newStream();
    dyad_listenEx(s, my_hostname, my_port, SIZE_BACKLOG);
    dyad_addListener(s, DYAD_EVENT_ACCEPT, onAccept, NULL);
    dyad_addListener(s, DYAD_EVENT_DATA, onData, NULL);
    dyad_addListener(s, DYAD_EVENT_ERROR, onError, NULL);
}

void net_connect(char* hostname, uint16_t port) {
    dyad_Stream* s = dyad_newStream();
    dyad_addListener(s, DYAD_EVENT_CONNECT, onConnect, NULL);
    dyad_addListener(s, DYAD_EVENT_ERROR, onError, NULL);
    printf("Trying to connect to %s:%d ...\n", hostname, port);
    int ret = tryConnect(s, hostname, port);
    if (ret) {
        printf("No server found\n");
        dyad_close(s);
    }
    else {
        printf("Server found\n");
        dyad_addListener(s, DYAD_EVENT_CLOSE, onClose, NULL);
        dyad_addListener(s, DYAD_EVENT_DATA, onData, NULL);
    }
}

void net_broadcast(char* data, size_t length) {
    //printf("Broadcasting message: %s\n", data);
    //printf("Broadcasting...\n");
    //sender_id is set to zero because it is irrelevant in this context.
    //It will be set by receiving node
    msg_queue_newMessage(&send_messages_head, data, length, 0);
    //msg_queue_addMessage(&send_messages_head, node);
}

int net_getMessage(char* target, size_t* received_msg_length, int* sender_id) {
    pthread_mutex_lock(&msg_mutex);
    if (STAILQ_EMPTY(&received_messages_head)) {
        pthread_mutex_unlock(&msg_mutex);
        return -1;
    }
    Msg_queue_node_t* node = STAILQ_FIRST(&received_messages_head);
    memcpy(target, node->message, node->length);
    *received_msg_length = node->length;
    *sender_id = node->sender_id;
    STAILQ_REMOVE_HEAD(&received_messages_head, messages);
    free(node);
    pthread_mutex_unlock(&msg_mutex);
    return 0;
}

void net_getConnectedIps(char* ip_buf[]) {
    /*
    int i;
    pthread_mutex_lock(&stream_mutex);
    for ( i = 0; i < num_connected_streams; ++i) {
        strcpy(ip_buf[i], dyad_getAddress(stream_list[i]));
    }
    pthread_mutex_unlock(&stream_mutex);
    */
}
