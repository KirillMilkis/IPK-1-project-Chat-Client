#include "tcp.h"

#define BUFFER_SIZE 1024

#define NOT_TO_SEND 5
#define TO_SEND 6

void parse_err(char* token, userInfo* user, char* rec_msg, size_t rec_msg_size){
    regex_t reegex;
    token = strtok(NULL, " \t\n");

    if (strcmp(token, "FROM") != 0){
        close_connection(user, "Error recieve from Server", 1);
    }

    token = strtok(NULL, " \t\n");
    regcomp(&reegex, "[!-~]", 0);
    int comp1 = regexec(&reegex, token, 0, NULL, 0);

    if(strlen(token) > 20 || comp1 != 0){
        close_connection(user, "Invalid display name in recieve from Server", 1);
    }
    char* rec_disp_name = token;

    token = strtok(NULL, " \t\n");

    if(strcmp(token, "IS") != 0){
        close_connection(user, "Error err recieve from Server", 1);
    }

    token = strtok(NULL, "\r\n");
    regcomp(&reegex, "[ -~]", 0);
    int comp2 = regexec(&reegex, token, 0, NULL, 0);

    if(strlen(token) > 1400 || comp2 != 0){
        close_connection(user, "Invalid message content in receive from Server", 1);
    }
    char* msg_content = token;

    printf("ERR FROM %s: %s\n", rec_disp_name, msg_content);

    return;

}


void parse_reply(char* token, userInfo* user, char* rec_msg, size_t rec_msg_size){
    regex_t reegex;
    token = strtok(NULL, " \t\n");

    int ok;
    if (strcmp(token, "OK") == 0){
        ok = 1;
    } else if(strcmp(token, "NOK") == 0){
        ok = 0;
    } else{
        close_connection(user, "Error reply recieve from Server", 1);
    }

    token = strtok(NULL, " \t\n");

    if(strcmp(token, "IS") != 0){
        close_connection(user, "Error reply recieve from Server 2", 1);
    }

    token = strtok(NULL, "\r\n");
    regcomp(&reegex, "[ -~]", 0);
    int comp1= regexec(&reegex, token, 0, NULL, 0);

    if(strlen(token) > 1400 || comp1 != 0){
        close_connection(user, "Invalid message content from Server", 1);
    }
    char* msg_content = token;

    if(strcmp(msg_content, "Authentication successful.") == 0){
        user->authorized = 1;
    } 
    if(ok){
        printf("Success: %s\n", msg_content);
    } else{
        printf("Failure: %s\n", msg_content);
    }

    return;
}

void parse_common_msg(char* token, userInfo* user, char* rec_msg, size_t rec_msg_size){
    regex_t reegex;
    token = strtok(NULL, " \t\n");

    if(strcmp(token, "FROM") != 0){
        close_connection(user, "Error msg recieve from Server", 1);
    }

    token = strtok(NULL, " \t\n");
    regcomp(&reegex, "[!-~]", 0);
    int comp1 = regexec(&reegex, token, 0, NULL, 0);

    if(strlen(token) > 20 || comp1 != 0){
        close_connection(user, "Incrorrect display name in recieve from Server", 1);
    }
    char* rec_disp_name = token;

    token = strtok(NULL, " \t\n");

    if(strcmp(token, "IS") != 0){
        close_connection(user, "Error msg recieve from Server", 1);
    }

    token = strtok(NULL, "\r\n");
    regcomp(&reegex, "[ -~]", 0);
    int comp2 = regexec(&reegex, token, 0, NULL, 0);

    if(strlen(token) > 1400 || comp2 != 0){
        close_connection(user, "Invalid message content from Server", 1);
    }
    char* msg_content = token;

    printf("%s: %s\n", rec_disp_name, msg_content);

    return;
}

void receive_msg(char* buffer,size_t buffer_size, userInfo* user){


    if (recv(client_socket, buffer, buffer_size, 0) < 0){
            perror("Recieve failed");
            exit(EXIT_FAILURE);
        }

    char tokenized_buffer[1024];
    memset(tokenized_buffer, 0, sizeof(tokenized_buffer));
    strcpy(tokenized_buffer, buffer);
    char* token = strtok(tokenized_buffer, " \t\n");

    switch(user->authorized){
        case 0:
            if(strcmp(token, "REPLY") == 0){
                parse_reply(token, user, buffer, BUFFER_SIZE);
                user->reply_request = 0;
            }
            break;
        case 1:
            if(strcmp(token, "REPLY") == 0){
                parse_reply(token, user, buffer, BUFFER_SIZE);
                user->reply_request = 0;
            } else if(strcmp(token, "MSG") == 0){
                parse_common_msg(token, user, buffer, BUFFER_SIZE);
            } else if(strcmp(token, "BYE") == 0){
                close_connection(user, "", 0);
            } else if(strcmp(token, "AUTH") == 0 || strcmp(token, "JOIN")){
                ;
            } else{
                close_connection(user, "Invalid message from server", 1);
            }
            break;

    }


    return;

}


int create_common_message(char* input, userInfo* user,char* msg, int msg_size){
    regex_t reegex;

    regcomp(&reegex, "[ -~]", 0);

    int comp1 = regexec(&reegex, input, 0, NULL, 0);

    if (strlen(input) > 1400 || comp1 != 0){
        printf("ERR: Invalid message content\n");
        return 1;
    }
    input[strlen(input) - 1] = '\0';

    snprintf(msg, msg_size, "MSG FROM %s IS %s\r\n",user->display_name, input);

    return 0;
}



int local_rename(char* token, userInfo* user){
    regex_t reegex;

    char* new_display_name = strtok(NULL, " ");;

    regcomp(&reegex, "[!-~]", 0);

    int comp1 = regexec(&reegex, new_display_name, 0, NULL, 0);

    if (strlen(new_display_name) > 20 || comp1 != 0){
        printf("ERR: Incorrect format display name\n");
        return 1;
    }
    new_display_name[strlen(new_display_name) - 1] = '\0';

    if(strtok(NULL, " ") != NULL){
        printf("ERR: Too many arguments for this command\n");
        return 1;
    }

    strcpy(user->display_name, new_display_name);

    return 0;
}

int create_join_msg(char* token, userInfo* user,char* msg, int msg_size){

    regex_t reegex;

    char* channel = strtok(NULL, " ");;

    regcomp(&reegex, "[A-z0-9-]", 0);

    int comp1 = regexec(&reegex, channel, 0, NULL, 0);

    if (strlen(channel) > 20 || comp1 != 0){
        printf("ERR: Incorrect format channel\n");
        return 1;
    }
    channel[strlen(channel) - 1] = '\0';

    if(strtok(NULL, " ") != NULL){
        printf("ERR: Too many arguments for this command\n");
        return 1;
    }

    snprintf(msg, msg_size, "JOIN %s AS %s\r\n", channel, user->display_name);

    return 0;
}


int create_auth_msg(char* token, userInfo* user, char* msg, int msg_size){

    regex_t reegex;
    int cmd_length = 0;

    char* username = strtok(NULL, " ");

    regcomp(&reegex, "[A-z0-9-]", 0);

    int comp1 = regexec(&reegex, username, 0, NULL, 0);

    if (strlen(username) > 20 || comp1 != 0){
        printf("ERR: Incorrect username format\n");
        return 1;
    }

    char* secret = strtok(NULL, " ");

    int comp2 = regexec(&reegex, secret, 0, NULL, 0);

    if (strlen(secret) > 128 || comp2 != 0){
        printf("ERR: Incorrect secret format\n");
        return 1;
    }

    char* display_name = strtok(NULL, " ");

    int comp3 = regexec(&reegex, display_name, 0, NULL, 0);

    if (strlen(display_name) > 20 || comp3 != 0){
        printf("ERR: Incorrect display name format\n");
        return 1;
    }
    display_name[strlen(display_name) - 1] = '\0';

    if(strtok(NULL, " ") != NULL){
        printf("Too many arguments for this command\n");
        return 1;
    }
    
    snprintf(msg, msg_size, "AUTH %s AS %s USING %s\r\n", username, display_name, secret);

    strcpy(user->username, username);
    strcpy(user->secret, secret);
    strcpy(user->display_name, display_name);

    return 0;
}


int send_input(char* buffer, userInfo* user, int client_socket){

    char input[1024];
    memset(input, 0, sizeof(input));

    fgets(input, BUFFER_SIZE, stdin);

    signal(SIGINT, interrupt_connection);

    char tokenized_buffer[1024];
    memset(tokenized_buffer, 0, sizeof(tokenized_buffer));
    strcpy(tokenized_buffer, input); 

    char* token = strtok(tokenized_buffer, " \t\n");

    memset(buffer, 0, sizeof(&buffer));

    switch(user->authorized){
            
        case 0:
            if (strcmp(token, "/auth") == 0){
                MSGFORMATCHECK(create_auth_msg(token, user, buffer, BUFFER_SIZE));
                user->reply_request = 1;
                return TO_SEND;
            } else {
                printf("You are not authorized\n");
                return NOT_TO_SEND;
            }
        
        case 1:
            if (strcmp(token, "/auth") == 0) {
                printf("You are already authorized\n");
                return NOT_TO_SEND;
            } else if (strcmp(token, "/join") == 0) {
                MSGFORMATCHECK(create_join_msg(token, user, buffer, BUFFER_SIZE));
                user->reply_request = 1;
                return TO_SEND;
            } else if (strcmp(token, "/rename") == 0) {
                MSGFORMATCHECK(local_rename(token, user));
                return NOT_TO_SEND;
            } else if (strcmp(token, "/help") == 0) {
                printf("Commands:\n/auth <username> <display-name> <secret> - authenticate with the server\n/join <channel> - join a channel\n/rename <new-display-name> - change your display name\n");
                return NOT_TO_SEND;
            } else if(strncmp(token, "/", 1) == 0) {
                printf("invalid command\n");
                return NOT_TO_SEND;
            } else{
                MSGFORMATCHECK(create_common_message(input, user, buffer, BUFFER_SIZE));
                return TO_SEND;
            }

        default:
            break;

    }
    

    return NOT_TO_SEND;

}


int tcp_connection(userInfo* user, struct addrinfo *res){
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int nfds = 2;

    polled_fds = calloc(nfds,sizeof(struct pollfd));

    polled_fds[0].fd = STDIN_FILENO;
    polled_fds[0].events = POLLIN;
    polled_fds[1].fd = client_socket;
    polled_fds[1].events = POLLIN;


    if (connect(client_socket, res->ai_addr, res->ai_addrlen) == -1) {
        close_connection(user, "Failed to connect to server", 1);
    }

    printf("Connected to server\n");

    user->reply_request = 0;
    user->authorized = 0;

    while(poll(polled_fds, nfds, -1) > 0) { 
        if(polled_fds[0].revents & POLLIN && user->reply_request == 0) {
            int send_res = send_input(buffer, user, client_socket);
            if (send_res == NOT_TO_SEND){
                continue;
            }
            if (send(client_socket, buffer, BUFFER_SIZE, 0) < 0){
                close_connection(user, "Failed to send the message", 1);
            }
        }

        if(polled_fds[1].revents & POLLIN) {
            memset(buffer, 0, sizeof(buffer));

            receive_msg(buffer, BUFFER_SIZE, user);
            
        }
        if(polled_fds[1].revents & (POLLERR | POLLHUP)) {
            close(client_socket);
        }
    }

    return 0;

}
