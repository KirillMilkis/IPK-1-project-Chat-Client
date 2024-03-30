#ifndef UDP_H
#define UDP_H
#include "client.h"

typedef struct userInfo userInfo;

int udp_connection(userInfo* user, struct addrinfo *res);

typedef struct msgPacket{
    char *msg;
    int msgSize;
} msgPacket;

typedef struct msgIdStorage{
    uint16_t client_msg_id;
    uint16_t serv_msg_id;
    size_t confirmed_ids_size;
    uint16_t confirmed_ids[1000];
} msgIdStorage;

#endif