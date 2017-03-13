#ifndef _NETWORK_H_
#define _NETWORK_H_
#include <stdint.h>
#include <stdlib.h>


void net_broadcast(char* data, size_t length);

void net_connect(char const * hostname, uint16_t const port);

int net_getMessage(char* target, size_t* received_msg_length, int * sender_id);

void net_init(unsigned int const my_id);

void net_listen(char const * my_hostname, uint16_t my_port);

int net_getMasterId(void);

//Elcom needs to know who is connect in order to figure out who is master
//connections_t is some list of connected nodes
//void net_getConnectedIps(char* ip_buf[]);

#endif //_NETWORK_H_
