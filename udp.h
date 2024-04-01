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
    int confirmed_ids_count;
    uint16_t* confirmed_ids;
} msgIdStorage;

void udp_create_bye_msg(userInfo* user, msgPacket* msg_packet, uint16_t msg_id);
void udp_send_msg(msgPacket* msg_packet, userInfo* user, struct addrinfo *res);


#endif