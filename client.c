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

typedef struct{
    char* username;
    char* secret;
    char* display_name;
    int authorized;
} userInfo;

int client_socket;

uint16_t udp_msg_num;

void close_connection(userInfo* user, char* error, int is_error){ 

    printf("Closing the connection\n");

    free(user);

    if(close(client_socket) == -1){
        perror("Failed to close the socket");
        exit(EXIT_FAILURE);
    }

    if(is_error){
        perror(error);
        exit(EXIT_FAILURE);
    }
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


int parse_err(char* rec_msg, char* token, userInfo* user){
    regex_t reegex;
    token = strtok(NULL, " \t\n");

    if (strcmp(token, "FROM") != 0){
        close_connection(user, "Error recieve from Server", 1);
    }

    token = strtok(NULL, " \t\n");
    regcomp(&reegex, "[!-~]", 0);
    int comp1 = regexec(&reegex, token, 0, NULL, 0);

    if(strlen(token) > 20 || comp1 != 0){
        close_connection(user, "Invalid DisplayName in recieve from Server", 1);
    }
    char* rec_disp_name = token;

    token = strtok(NULL, " \t\n");

    if(strcmp(token, "IS") == 0){
        close_connection(user, "Error recieve from Server", 1);
    }

    token = strtok(NULL, " \t\n");
    regcomp(&reegex, "[ -~]", 0);
    int comp2 = regexec(&reegex, token, 0, NULL, 0);

    if(strlen(token) > 1400 || comp2 != 0){
        close_connection(user, "Invalid MessageContent", 1);
    }
    char* msg_content = token;

    snprintf(rec_msg, sizeof(rec_msg), "ERR FROM %s: %s\n", msg_content, rec_disp_name);

    return 0;

}

/* TODO: Parse_reply*/

/* TODO: Parse_auth*/

/* TODO: Parse_join*/

/* TODO: Parse_msg*/

/* TODO: Parse_confirm*/

int parse_output(char* buffer, userInfo* user, int client_sokcet){

    char tokenized_buffer[1024];
    memset(tokenized_buffer, 0, sizeof(tokenized_buffer));
    strcpy(tokenized_buffer, buffer);
    char* token = strtok(buffer, " \t\n");

    if(strcmp(token, "ERR") == 0){
        parse_err(buffer, token, user);
    } else if(strcmp(token, "REPLY") == 0){
        printf("%s", buffer);
    } else if(strcmp(token, "AUTH") == 0){
        printf("%s", buffer);
    } else if(strcmp(token, "JOIN") == 0){
        printf("%s", buffer);
    } else if(strcmp(token, "MSG") == 0){
        printf("%s", buffer);
    } else if(strcmp(token, "BYE") == 0){
        printf("%s", buffer);
    } else if(strcmp(token, "CONFIRM") == 0){
        printf("%s", buffer);
    } 

    return 0;

}


int create_common_message(char* input, userInfo* user,char* msg, int msg_size){
    regex_t reegex;

    if (regcomp(&reegex, "[ -~]", 0) != 0){
        close_connection(user, "Failed to compile the reg expression", 1);
    }

    int comp1 = regexec(&reegex, input, 0, NULL, 0);

    if (strlen(input) > 1400 || comp1 != 0){
        close_connection(user, "Incorrect format message", 1);
    }

    snprintf(msg, msg_size, "MSG FROM %s IS %s\r\n",user->display_name, input);

    return 0;
}



int local_rename(char* token, userInfo* user){
    regex_t reegex;

    char* new_display_name = strtok(NULL, " ");;

    if (regcomp(&reegex, "[!-~]", 0) != 0){
        close_connection(user, "Failed to compile the reg expression", 1);
    }

    int comp1 = regexec(&reegex, new_display_name, 0, NULL, 0);

    if (strlen(new_display_name) > 20 || comp1 != 0){
        close_connection(user, "Incorrect format display-name", 1);
    }
    new_display_name[strlen(new_display_name) - 1] = '\0';

    strcpy(user->display_name, new_display_name);

    return 0;
}

int create_join_msg(char* token, userInfo* user,char* msg, int msg_size){

    regex_t reegex;

    char* channel = strtok(NULL, " ");;

    if (regcomp(&reegex, "[A-z0-9-]", 0) != 0){
        close_connection(user, "Failed to compile the reg expression", 1);
    }

    int comp1 = regexec(&reegex, channel, 0, NULL, 0);

    if (strlen(channel) > 20 || comp1 != 0){
        close_connection(user, "Incorrect format channel", 1);
    }
    channel[strlen(channel) - 1] = '\0';

    snprintf(msg, msg_size, "JOIN %s AS %s\r\n", channel, user->display_name);

    return 0;
}


int create_auth_msg(char* token, userInfo* user, char* msg, int msg_size){

    regex_t reegex;
    int cmd_length = 0;

    char* username = strtok(NULL, " ");;

    if (regcomp(&reegex, "[A-z0-9-]", 0) != 0){
        close_connection(user, "Failed to compile the reg expression", 1);
    }

    int comp1 = regexec(&reegex, username, 0, NULL, 0);

    if (strlen(username) > 20 || comp1 != 0){
        close_connection(user, "Incorrect username format", 1);
    }

    char* secret = strtok(NULL, " ");

    int comp2 = regexec(&reegex, secret, 0, NULL, 0);

    if (strlen(secret) > 120 || comp2 != 0){
        close_connection(user, "Incorrect secret format", 1);
    }

    char* display_name = strtok(NULL, " ");

    int comp3 = regexec(&reegex, display_name, 0, NULL, 0);

    if (strlen(display_name) > 128 || comp3 != 0){
        close_connection(user, "Incorrect display name format", 1);
    }
    display_name[strlen(display_name) - 1] = '\0';
    
    snprintf(msg, msg_size, "AUTH %s AS %s USING %s\r\n", username, display_name, secret);

    strcpy(user->username, username);
    strcpy(user->secret, secret);
    strcpy(user->display_name, display_name);
    return 0;
}


int send_input(char* buffer, int buffer_size, userInfo* user, int client_socket){

    char input[1024];
    memset(input, 0, sizeof(input));

    fgets(input, sizeof(input), stdin);

    char tokenize_input[1024];
    memset(tokenize_input, 0, sizeof(tokenize_input));
    strcpy(tokenize_input, input); 

    char* token = strtok(tokenize_input, " \t\n");

    memset(buffer, 0, sizeof(&buffer));

    switch(user->authorized){

        case 0:
            if (strcmp(token, "/auth") == 0){
                create_auth_msg(token, user, buffer, buffer_size);
                user->authorized = 1;
            } else {
                printf("555");
                perror("You are not authorized");
            }
            break;
        
        case 1:
            if (strcmp(token, "/auth") == 0) {
                printf("You are already authorized");
                return 0;
            } else if (strcmp(token, "/join") == 0) {
                create_join_msg(token, user, buffer, buffer_size);
            } else if (strcmp(token, "/rename") == 0) {
                local_rename(token, user);
            } else if (strcmp(token, "/help") == 0) {
                printf("555");
            } if(strncmp(token, "//", 1) == 0) {
                printf("555");
                perror("Invalid command");
            } 
            else{
                create_common_message(input, user, buffer, buffer_size);
            }


    }

    

    if (send(client_socket, buffer, strlen(buffer), 0) < 0){
        close_connection(user, "Failed to send the message", 1);
    }

    return 0;

}


int tcp_connection(userInfo* user, struct addrinfo *res){
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int nfds = 2;
    struct pollfd *polled_fds;

    polled_fds = calloc(nfds,sizeof(struct pollfd));

    polled_fds[0].fd = STDIN_FILENO;
    polled_fds[0].events = POLLIN;
    polled_fds[1].fd = client_socket;
    polled_fds[1].events = POLLIN;


        if (connect(client_socket, res->ai_addr, res->ai_addrlen) == -1) {
            perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");


    while(poll(polled_fds, nfds, 10000) > 0) { /* error handling elided */
        if(polled_fds[0].revents & POLLIN) {
            // read data from stdin and send it over the socket
            send_input(buffer, sizeof(buffer), user, client_socket);
            
        }
        if(polled_fds[1].revents & POLLIN && user->authorized == 1) {
            // chat data received
            memset(buffer, 0, sizeof(buffer));

            if (recv(client_socket, buffer, sizeof(buffer), 0) < 0){
                perror("Recieve failed");
                exit(EXIT_FAILURE);
            }

            printf("Server: %s\n", buffer);

            if (strcmp(buffer, "BYE\r\n") == 0){
                close_connection(user, "", 0);
            }
            
        }
        if(polled_fds[1].revents & (POLLERR | POLLHUP)) {
            close(client_socket);
            // socket was closed
        }
    }

    return 0;

}


char* udp_create_auth_msg(char* token, char* buffer, userInfo* user){
    regex_t reegex;
    char* msg_packet = malloc(sizeof(char) * 3);

    msg_packet[0] = (uint8_t)0x02;
    msg_packet[1] = udp_msg_num;

    char* username = strtok(NULL, " ");;

    regcomp(&reegex, "[A-z0-9-]", 0);

    int comp1 = regexec(&reegex, username, 0, NULL, 0);

    if (strlen(username) > 20 || comp1 != 0){
        close_connection(user, "Incorrect username format", 1);
    }

    size_t msg_old_size = sizeof(msg_packet);
    msg_packet = realloc(msg_packet, sizeof(msg_packet) + sizeof(username) + 1);
    msg_packet[msg_old_size] = *username;
    msg_packet[msg_old_size + 1] = '0';

    char* secret = strtok(NULL, " ");

    int comp2 = regexec(&reegex, secret, 0, NULL, 0);

    if (strlen(secret) > 120 || comp2 != 0){
        close_connection(user, "Incorrect secret format", 1);
    }

    msg_old_size = sizeof(msg_packet);
    msg_packet = realloc(msg_packet, sizeof(msg_packet) + sizeof(secret) + 1);
    msg_packet[msg_old_size] = *secret;
    msg_packet[msg_old_size + 1] = '0';

    char* display_name = strtok(NULL, " ");

    int comp3 = regexec(&reegex, display_name, 0, NULL, 0);

    if (strlen(display_name) > 128 || comp3 != 0){
        close_connection(user, "Incorrect display name format", 1);
    }
    display_name[strlen(display_name) - 1] = '\0';

    msg_old_size = sizeof(msg_packet);
    msg_packet = realloc(msg_packet, sizeof(msg_packet) + sizeof(display_name) + 1);
    msg_packet[msg_old_size] = *display_name;
    msg_packet[msg_old_size + 1] = '0';

    strcpy(user->username, username);
    strcpy(user->secret, secret);
    strcpy(user->display_name, display_name);

    return msg_packet;

    return 0;
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
        close_connection(&user, "", 0);

    } else if(strcmp(protocol, "udp")){
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

        // char msg_packet[1026];
        // memset(msg_packet, 0, sizeof(msg_packet));

        fgets(buffer, sizeof(buffer), stdin);
        
        char* token = strtok(buffer, " \t\n");
        // while(1){
        if(strcmp(buffer, "/auth") == 0){
            char* msg_packet = udp_create_auth_msg(token, buffer, &user);
        }

        int byte = sendto(client_socket, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen);
        if (byte < 0){
            perror("Failed to send the message");
            exit(EXIT_FAILURE);
        }

    }

     freeaddrinfo(res);

    return 0;

}
