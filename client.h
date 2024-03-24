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

int client_socket;

#define MSGFORMATCHECK(func)  \
    do {                  \
        if ((func) == 1){       \
            return 1;     \
        }                       \
    } while (0)                 \


typedef struct userInfo{ 
    char* username;
    char* secret;
    char* display_name;
    int authorized;
} userInfo;

void close_connection(userInfo* user, char* error, int is_error);



#endif