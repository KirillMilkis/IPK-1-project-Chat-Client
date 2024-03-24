#ifndef TCP_H
#define TCP_H

#include "client.h"

typedef struct  userInfo userInfo;

int tcp_connection(userInfo* user, struct addrinfo *res);

#endif