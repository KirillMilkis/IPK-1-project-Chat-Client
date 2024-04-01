#include "client.h"

int client_socket;
struct pollfd *polled_fds;

uint16_t udp_timeout = 250;
uint8_t max_retransmissions = 3;

userInfo* user;


void close_connection(userInfo* user, char* error, int is_error){ 

    printf("Disconnecting...\n");

    if(user != NULL){
        free(user->username);
        free(user->secret);
        free(user->display_name);
    } 

    if(polled_fds != NULL){
        free(polled_fds);
    }

    if(close(client_socket) == -1){
        printf("ERR: Failed to close the socket\n");
        exit(EXIT_FAILURE);
    }

    if(is_error){
        printf("ERR: %s\n", error);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

void interrupt_connection(int sig){

    close_connection(user, "", 0);

}
   

int main(int argc, char *argv[]) {
   
    char port[5] = "4567";
    char *addr = NULL;
    char *protocol = NULL;

    int opt; 
    while((opt = getopt(argc, argv, ":t:s:p:")) != -1){
       switch(opt){
            case 't':
                if (strcmp(optarg, "tcp") == 0 || strcmp(optarg, "udp") == 0){
                    protocol = optarg;
                } else {
                    printf("Invalid protocol: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 's':
                addr = optarg;
                break;
            case 'p':
                if (atoi(optarg) != 0){
                    strcpy(port, optarg);
                } else {
                    printf("Invalid port: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'd':
                if (atoi(optarg) != 0){
                    udp_timeout = atoi(optarg);
                }
                break;
            case 'r':
                if (atoi(optarg) != 0){
                    max_retransmissions = atoi(optarg);
                }
                break;
            case 'h':
                printf("Usage: %s -t <tcp|udp> -s <IP address or addr> -p <4567> [-d <250>] [-r <3>] [-h]\n", argv[0]);
                break;
            case ':':
                printf("Use -h for help\n");
                break;
            case '?':
                printf("Unknown option: %c\n", opt);
                break;
        }


    }

    if(protocol == NULL || addr == NULL){
        printf("Missing required arguments -t -s\n");
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(addr, port, &hints, &res) != 0) {
        printf("ERR: Failed to get address info\n");
        exit(EXIT_FAILURE);
    }

    user = malloc(sizeof(userInfo));
    if(user == NULL){
        printf("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    user->username = malloc(sizeof(char) * 20);
    if(user->username == NULL){
        printf("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    user->secret = malloc(sizeof(char) * 120);
    if(user->secret == NULL){
        printf("Failed to allocate memory");
        free(user->username);
        exit(EXIT_FAILURE);
    }
    user->display_name = malloc(sizeof(char) * 128);
    if(user->display_name == NULL){
        printf("Failed to allocate memory");
        free(user->username);
        free(user->secret);
        exit(EXIT_FAILURE);
    }
    
    if (strcmp(protocol, "tcp") == 0){
        
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1) {
            printf("Error creating socket");
            exit(EXIT_FAILURE);
        }

        tcp_connection(user, res);

    } else if(strcmp(protocol, "udp") == 0){

        client_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (client_socket == -1) {
            printf("Error creating socket");
            exit(EXIT_FAILURE);
        }

        udp_connection(user, res);
    }
    
    close_connection(user, "", 0);

    freeaddrinfo(res);

    return 0;

}
