#ifndef _NETWORK_H_
#define _NETWORK_H_
#include <stdint.h>
#include <stdlib.h>

void net_broadcast(char* data, size_t length);

void net_connect(char* hostname, uint16_t port);

int net_getMessage(char* target, size_t* received_msg_length, int * sender_id);

void net_init(char* ips_and_ports[]);

void net_listen(char* my_hostname, uint16_t my_port);

//Elcom needs to know who is connect in order to figure out who is master
//connections_t is some list of connected nodes
//void net_getConnectedIps(char* ip_buf[]);

#endif //_NETWORK_H_
