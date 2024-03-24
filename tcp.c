#include "tcp.h"

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

    regcomp(&reegex, "[ -~]", 0);

    int comp1 = regexec(&reegex, input, 0, NULL, 0);

    if (strlen(input) > 1400 || comp1 != 0){
        printf("Incorrect message content\n");
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

int create_join_msg(char* token, userInfo* user,char* msg, int msg_size){

    regex_t reegex;

    char* channel = strtok(NULL, " ");;

    regcomp(&reegex, "[A-z0-9-]", 0);

    int comp1 = regexec(&reegex, channel, 0, NULL, 0);

    if (strlen(channel) > 20 || comp1 != 0){
        printf("Incorrect format channel\n");
        return 1;
    }
    channel[strlen(channel) - 1] = '\0';

    if(strtok(NULL, " ") != NULL){
        printf("Too many arguments for this command\n");
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
                MSGFORMATCHECK(create_auth_msg(token, user, buffer, buffer_size));
                user->authorized = 1;
                break;
            } else {
                perror("You are not authorized\n");
                break;
            }
        
        case 1:
            if (strcmp(token, "/auth") == 0) {
                printf("You are already authorized\n");
                return 0;
            } else if (strcmp(token, "/join") == 0) {
                MSGFORMATCHECK(create_join_msg(token, user, buffer, buffer_size));
                break;
            } else if (strcmp(token, "/rename") == 0) {
                MSGFORMATCHECK(local_rename(token, user));
                break;
            } else if (strcmp(token, "/help") == 0) {
                printf("555");
                break;
            } else if(strcmp(token, "//") == 0) {
                printf("555");
                perror("Invalid command\n");
                break;
            } else{
                MSGFORMATCHECK(create_common_message(input, user, buffer, buffer_size));
                break;
            }

            default:
                break;

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


    while(poll(polled_fds, nfds, 100000) > 0) { /* error handling elided */
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
