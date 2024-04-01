#ifndef TCP_H
#define TCP_H

#include "client.h"

extern uint16_t udp_timeout;
extern uint8_t max_retransmissions;

typedef struct userInfo userInfo;

int tcp_connection(userInfo* user, struct addrinfo *res);

#endif