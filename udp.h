#ifndef UDP_H
#define UDP_H
#include "client.h"

typedef struct userInfo userInfo;

int udp_connection(userInfo* user, struct addrinfo *res);

#endif