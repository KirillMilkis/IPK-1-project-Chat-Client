#include "udp.h"

uint16_t usp_send_msg_num;

#define RETRY_COUNT 3 
#define TIMEOUT 250


int udp_create_chat_msg(char* token, char* buffer, userInfo* user, char* msg_packet, int* msg_size){
    regex_t reegex;
    *msg_size = 3;
    msg_packet = malloc(sizeof(char) * *msg_size);

    msg_packet[0] = (uint8_t)0x04;
    msg_packet[1] = usp_send_msg_num;

    char* message = strtok(NULL, "\n");

    regcomp(&reegex, "[ -~]", 0);

    int comp1 = regexec(&reegex, message, 0, NULL, 0);

    if (strlen(message) > 1400 || comp1 != 0){
        printf("Incorrect message format\n");
        return 1;
    }

    *msg_size += strlen(user->display_name);
    msg_packet = realloc(msg_packet, *msg_size + 1);
    strcpy(&msg_packet[2], user->display_name);
    msg_packet[*msg_size] = '\0';

    *msg_size += strlen(message);
    msg_packet = realloc(msg_packet, *msg_size + 1);
    strcpy(&msg_packet[*msg_size - strlen(user->display_name)], message);
    msg_packet[*msg_size] = '\0';


    return 0;
}


udp_local_rename(char* token, userInfo* user){
    regex_t reegex;

    char* new_display_name = strtok(NULL, " ");;

    regcomp(&reegex, "[!-~]", 0);

    int comp1 = regexec(&reegex, new_display_name, 0, NULL, 0);

    if (strlen(new_display_name) > 20 || comp1 != 0){
        printf("Incorrect format display-name\n");
        return 1;
    }
    new_display_name[strlen(new_display_name) - 1] = '\0';

    if(strtok(NULL, " ") != NULL){
        printf("Too many arguments for this command\n");
        return 1;
    }

    strcpy(user->display_name, new_display_name);

    return 0;
}

int udp_create_join_msg(char* token, char* buffer, userInfo* user, char* msg_packet, int* msg_size){
    regex_t reegex;
    *msg_size = 3;
    msg_packet = malloc(sizeof(char) * *msg_size);

    msg_packet[0] = (uint8_t)0x02;
    msg_packet[1] = usp_send_msg_num;

    char* channel = strtok(NULL, " ");

    regcomp(&reegex, "[A-z0-9-]", 0);

    int comp1 = regexec(&reegex, channel, 0, NULL, 0);

    if (strlen(channel) > 20 || comp1 != 0){
        printf("Incorrect channel format\n");
        return 1;
    }

    if(strtok(NULL, " ") != NULL){
        printf("Too many arguments for this command\n");
        return 1;
    }

    *msg_size += strlen(channel);
    msg_packet = realloc(msg_packet, *msg_size + 1);
    strcpy(&msg_packet[2], channel);
    msg_packet[*msg_size] = '\0';

    *msg_size += strlen(user->display_name); 
    msg_packet = realloc(msg_packet, *msg_size + 1);
    strcpy(&msg_packet[*msg_size - strlen(channel)], user->display_name);
    msg_packet[*msg_size] = '\0';

    return 0;
}


int udp_create_auth_msg(char* token, char* buffer, userInfo* user, char* msg_packet, int* msg_size){
    regex_t reegex;
    *msg_size = 3;
    msg_packet = malloc(sizeof(char) * *msg_size);

    msg_packet[0] = (uint8_t)0x02;
    msg_packet[1] = usp_send_msg_num;

    char* username = strtok(NULL, " ");

    regcomp(&reegex, "[A-z0-9-]", 0);

    int comp1 = regexec(&reegex, username, 0, NULL, 0);

    if (strlen(username) > 20 || comp1 != 0){
        printf("Incorrect username format\n");
        return 1;
    }

    char* secret = strtok(NULL, " ");

    int comp2 = regexec(&reegex, secret, 0, NULL, 0);

    if (strlen(secret) > 120 || comp2 != 0){
        printf("Incorrect secret format\n");
        return 1;
    }

    char* display_name = strtok(NULL, " ");

    int comp3 = regexec(&reegex, display_name, 0, NULL, 0);

    if (strlen(display_name) > 128 || comp3 != 0){
        printf("Incorrect display name format\n");
        return 1;
    }
    display_name[strlen(display_name) - 1] = '\0';

    if(strtok(NULL, " ") != NULL){
        printf("Too many arguments for this command\n");
        return 1;
    }

    *msg_size += strlen(username);
    msg_packet = realloc(msg_packet, *msg_size + 1);
    strcpy(&msg_packet[2], username);
    msg_packet[*msg_size] = '\0';

    *msg_size += strlen(display_name);
    msg_packet = realloc(msg_packet, *msg_size + 1);
    strcpy(&msg_packet[*msg_size - strlen(display_name)], display_name);
    msg_packet[*msg_size] = '\0';

    *msg_size += strlen(secret);
    msg_packet = realloc(msg_packet, *msg_size + 1);
    strcpy(&msg_packet[*msg_size - strlen(secret) + 1], secret);
    msg_packet[*msg_size] = '\0';


    strcpy(user->username, username);
    strcpy(user->secret, secret);
    strcpy(user->display_name, display_name);
    
    return msg_packet;

    return 0;
}



int udp_send_msg(struct addrinfo *res, userInfo* user, char* buffer, int buffer_size, int bytesend){
    memset(buffer, 0, buffer_size);
    fgets(buffer, buffer_size, stdin);

    int msg_packet_size = 0;
    char* msg_packet;

    char* token = strtok(buffer, " \t\n");
    switch(user->authorized){
        case 0:
            if(strcmp(token, "/auth") == 0){
                MSGFORMATCHECK(udp_create_auth_msg(token, buffer, user, msg_packet, &msg_packet_size));
            }
            break;
        case 1:
            if(strcmp(token, "/auth") == 0){
                printf("You are already authorized\n");
                return 0;
            } else if(strcmp(token, "/join") == 0){
                MSGFORMATCHECK(udp_create_list_msg(token, buffer, user, msg_packet, &msg_packet_size));
                break;
            } else if(strcmp(token, "/rename") == 0){
                MSGFORMATCHECK(udp_create_leave_msg(token, buffer, user, msg_packet, &msg_packet_size));
                break;
            } else if(strcmp(token, "/help") == 0){
                printf("555");
                break;
            } else if(strcmp(token, "//") == 0){
                printf("invalid command\n");
                break;
            } else{
                MSGFORMATCHECK(udp_create_chat_msg(token, buffer, user, msg_packet, &msg_packet_size));
                break;
            }

            default:
                break;

    }
       for(uint8_t i = 0; i < msg_packet_size; i++){
            printf("msg_packet %d    %d\n", msg_packet[i], i);
        }
        
        client_socket = socket(AF_INET, SOCK_DGRAM, 0);
        
        bytesend = sendto(client_socket, msg_packet, msg_packet_size + 1, 0, res->ai_addr, res->ai_addrlen);
        
        
        if (bytesend < 0){
            perror("Failed to send the message");
            exit(EXIT_FAILURE);
        }

        return 0;
    }


int udp_parse_confirm(char* buffer, int buffer_size, userInfo* user){
    uint16_t confirmed_msg_num;
    memcpy(&confirmed_msg_num, &buffer[1], 2); //memcpy

    if (confirmed_msg_num == usp_send_msg_num){
        return 0;
    } else {
        close_connection(user, "Failed to confirm the message", 1);
    }

}


int udp_recieve_parser(char* buffer, char* recieved_msg,int buffer_size, userInfo* user){
    int msg_type;

    msg_type = (uint32_t)buffer[0];

    switch(msg_type){
        case 0x00:
            udp_parse_confirm(buffer, buffer_size, user);
            break;
        case 0x03:
            udp_parse_reply(buffer, buffer_size, user);
            break;
        default:
            break;
    }

}


int udp_connection(userInfo* user, struct addrinfo *res){
    char buffer[1024];
    int buffer_size = sizeof(buffer);
    memset(buffer, 0, buffer_size);

    int bytesend;
    int byterecv;

    int nfds = 2;
    struct pollfd *polled_fds;

    polled_fds = calloc(nfds,sizeof(struct pollfd));

    polled_fds[0].fd = STDIN_FILENO;
    polled_fds[0].events = POLLIN;
    polled_fds[1].fd = client_socket;
    polled_fds[1].events = POLLIN;

    int ready_sockets = poll(polled_fds, nfds, -1);
    /* error handling elided */
    if(polled_fds[0].revents & POLLIN) {
        udp_send_msg(res, user, buffer, sizeof(buffer), bytesend);
        
        for(int retry = 0; retry < RETRY_COUNT; retry++){
            int ready_confirm = poll(polled_fds, nfds, TIMEOUT);
            
            if (ready_confirm != 0 && polled_fds[1].revents & POLLIN){
                
                byterecv = recvfrom(client_socket, buffer, buffer_size, 0, res->ai_addr, &res->ai_addrlen);
                if (byterecv < 0) {
                    close_connection(user, "Failed to recieve the message", 1);
                }

                udp_parse_confirm(buffer, buffer_size, user);

                break;
            } else if (ready_confirm != 0 && polled_fds[0].revents & POLLIN){
                printf("Please wait for the confirmation\n");
            }  
            
            if (retry == RETRY_COUNT - 1){
                close_connection(user, "Failed to send the message", 1);
            }
        }


    } else if(polled_fds[1].revents & POLLIN && user->authorized == 1) {
        // chat data received
        memset(buffer, 0, buffer_size);

        byterecv = recvfrom(client_socket, buffer, buffer_size, 0, res->ai_addr, &res->ai_addrlen);
        if (byterecv < 0) {
            perror("ERROR: Recvfrom");
            close(client_socket);
            return EXIT_FAILURE;
        }

        char* recieved_msg[1024];
        udp_recieve_parser(buffer, recieved_msg, buffer_size,  user);

        printf("Server: %s\n", buffer);

        if (strcmp(buffer, "BYE\r\n") == 0){
            close_connection(user, "", 0);
        }
        
    }
    if(polled_fds[1].revents & (POLLERR | POLLHUP)) {
        close(client_socket);
        // socket was closed
    }

    
    
    return 0;
}