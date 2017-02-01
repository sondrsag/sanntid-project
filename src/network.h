#ifndef _NETWORK_H_
#define _NETWORK_H_
#include "stdint.h"

void net_broadcast(char* data);

void net_connect(char* hostname, uint16_t port);

char* net_getMessage();

//Elcom needs to know who is connect in order to figure out who is master
//connections_t is some list of connected nodes
//connections_t net_getConnectedNodes(void);

#endif //_NETWORK_H_
