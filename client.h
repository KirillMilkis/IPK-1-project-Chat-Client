#ifndef CLIENT_H
#define CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "tcp.h"
#include "udp.h"

extern uint16_t udp_timeout;
extern uint8_t max_retransmissions;
extern int client_socket;
extern struct pollfd *polled_fds;

#define MSGFORMATCHECK(func)  \
    do {                  \
        if ((func) == 1){       \
            return 1;     \
        }                       \
    } while (0)                 \

typedef struct msgPacket msgPacket;
typedef struct msgIdStorage msgIdStorage;

typedef struct userInfo{ 
    char* username;
    char* secret;
    char* display_name;
    int authorized;
    
    int reply_request;
    int serv_confirm_request;
    int client_confirm_request;
    msgPacket* prev_msg_packet;
    msgIdStorage* user_msg_id_storage;
} userInfo;


void interrupt_connection(int sig);
void close_connection(userInfo* user, char* error, int is_error);


#endif