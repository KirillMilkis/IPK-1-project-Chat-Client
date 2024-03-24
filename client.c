#include "client.h"


void close_connection(userInfo* user, char* error, int is_error){ 

    printf("Closing the connection\n");

    free(user->username);
    free(user->secret);
    free(user->display_name);

    if(close(client_socket) == -1){
        perror("Failed to close the socket");
        exit(EXIT_FAILURE);
    }

    if(is_error){
        perror(error);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

void interrupt_connection(int sig){

    printf("Closing the connection\n");

    if(shutdown(client_socket, SHUT_WR) == -1){
        perror("Failed to close the connection");
        exit(EXIT_FAILURE);
    }

    if(close(client_socket) == -1){
        perror("Failed to close the socket");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
   

int main(int argc, char *argv[]) {
   
    char *port = NULL;
    char *addr = NULL;
    char *protocol = NULL;

    int opt;   
    while((opt = getopt(argc, argv, ":t:s:p:")) != -1){
        switch(opt){
            case 't':
                if (strcmp(optarg, "tcp") == 0 || strcmp(optarg, "udp") == 0){
                    protocol = optarg;
                    printf("t: %s\n", optarg);
                } else {
                    printf("Invalid protocol: %s\n", optarg);
                }
                break;
            case 's':
                addr = optarg;
                printf("s: %s\n", optarg);
                break;
            case 'p':
                if (atoi(optarg) != 0){
                    port = optarg;
                } else {
                    printf("Invalid port: %s\n", optarg);
                }
                printf("p: %s\n", optarg);
                break;
            case ':':
                printf("Argument	Value	      Possible values	   Meaning or expected program behaviour"
                          "-t	 User provided	     tcp or udp	        Transport protocol used for connection"
                          "-s	 User provided	IP address or addr	        Server IP or addr"
                          "-p	     4567	           uint16	              Server port"
                          "-d	      250	           uint16	          UDP confirmation timeout"
                          "-r	       3	            uint8	         Maximum number of UDP retransmissions"
                          "-h			                                  Prints program help output and exits\n");
                break;
            case '?':
                printf("unknown option: %c\n", opt);
                break;
        }


    }

    // Set up server address
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(addr, port, &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    client_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    printf("%d\n", client_socket);

    signal(SIGINT, interrupt_connection);

    userInfo user;

    user.username = malloc(sizeof(char) * 20);
    if(user.username == NULL){
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    user.secret = malloc(sizeof(char) * 120);
    if(user.secret == NULL){
        perror("Failed to allocate memory");
        free(user.username);
        exit(EXIT_FAILURE);
    }
    user.display_name = malloc(sizeof(char) * 128);
    if(user.display_name == NULL){
        perror("Failed to allocate memory");
        free(user.username);
        free(user.secret);
        exit(EXIT_FAILURE);
    }
    
    //Connect to the server
    if (strcmp(protocol, "tcp") == 0){
        tcp_connection(&user, res);

    } else if(strcmp(protocol, "udp") == 0){
        udp_connection(&user, res);
    }
    
    close_connection(&user, "", 0);

     freeaddrinfo(res);

    return 0;

}
