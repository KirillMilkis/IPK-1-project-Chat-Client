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



#define MSGFORMATCHECK(func)  \
    do {                  \
        if ((func) == 1){       \
            return 1;     \
        }                       \
    } while (0)                 \


typedef struct{
    char* username;
    char* secret;
    char* display_name;
    int authorized;
} userInfo;