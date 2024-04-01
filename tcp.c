#include "tcp.h"

#define BUFFER_SIZE 1024

#define NOT_TO_SEND 5
#define TO_SEND 6

/* Code block for testing individual server key words*/
void tcp_regular_exp_check_parsing(char* token, char* pattern, int max_length, char* error_msg, userInfo* user){
    regex_t reegex;
    regcomp(&reegex, pattern, 0);
    int comp = regexec(&reegex, token, 0, NULL, 0);
    if (strlen(token) >  max_length|| comp != 0){
        close_connection(user, error_msg, 1);
    }
}


int tcp_regular_exp_check_creating(char* token, char* pattern, int max_length, char* error_msg){
    regex_t reegex;
    regcomp(&reegex, pattern, 0);
    int comp = regexec(&reegex, token, 0, NULL, 0);
    if (strlen(token) >  max_length|| comp != 0){
        printf("ERR: %s\n", error_msg);
        return 1;
    }
    return 0;
}
/* Code block for parsing incoming messages from Server*/
/* ERR FROM {DisplayName} IS {MessageContent}\r\n */
void tcp_parse_err(char* token, userInfo* user){
    regex_t reegex;
    if((token = strtok(NULL, " \t\n")) == NULL){
        close_connection(user, "Error recieve from Server", 1);
    }
    if (strcmp(token, "FROM") != 0){
        close_connection(user, "Error recieve from Server", 1);
    }

    if((token = strtok(NULL, " \t\n")) == NULL){
        close_connection(user, "Error recieve from Server", 1);
    }
    tcp_regular_exp_check_parsing(token, "[!-~]", 20, "Invalid display name in recieve from Server", user);
    char* rec_disp_name = token;

    if((token = strtok(NULL, " \t\n")) == NULL){
        close_connection(user, "Error recieve from Server", 1);
    }
    if(strcmp(token, "IS") != 0){
        close_connection(user, "Error err recieve from Server", 1);
    }

    if((token = strtok(NULL, "\r\n")) == NULL){
        close_connection(user, "Error recieve from Server", 1);
    }
    tcp_regular_exp_check_parsing(token, "[ -~]", 1400, "Invalid message content in recieve from Server", user);
    char* msg_content = token;

    printf("ERR FROM %s: %s\n", rec_disp_name, msg_content);

    return;

}

/* REPLY {"OK"|"NOK"} IS {MessageContent}\r\n */
void tcp_parse_reply(char* token, userInfo* user){
    token = strtok(NULL, " \t\n");

    int ok;
    if (strcmp(token, "OK") == 0){
        ok = 1;
    } else if(strcmp(token, "NOK") == 0){
        ok = 0;
    } else{
        close_connection(user, "Error reply recieve from Server", 1);
    }

    if((token = strtok(NULL, " \t\n")) == NULL){
        close_connection(user, "Error reply recieve from Server", 1);
    } 
    if(strcmp(token, "IS") != 0){
        close_connection(user, "Error reply recieve from Server", 1);
    }


    if((token = strtok(NULL, "\r\n")) == NULL){
        close_connection(user, "Error reply recieve from Server", 1);
    }
    tcp_regular_exp_check_parsing(token, "[ -~]", 1400, "Invalid message content in recieve from Server", user);
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

/* MSG FROM {DisplayName} IS {MessageContent}\r\n */
void tcp_parse_common_msg(char* token, userInfo* user){
    if((token = strtok(NULL, " \t\n")) == NULL){
        close_connection(user, "Error msg recieve from Server", 1);
    };
    if(strcmp(token, "FROM") != 0){
        close_connection(user, "Error msg recieve from Server", 1);
    }

    if((token = strtok(NULL, " \t\n")) == NULL){
        close_connection(user, "Error msg recieve from Server", 1);
    }
    tcp_regular_exp_check_parsing(token, "[!-~]", 20, "Invalid display name in recieve from Server", user);
    char* rec_disp_name = token;

    if((token = strtok(NULL, " \t\n")) == NULL){
        close_connection(user, "Error msg recieve from Server", 1);
    }
    if(strcmp(token, "IS") != 0){
        close_connection(user, "Error msg recieve from Server", 1);
    }

    if((token = strtok(NULL, "\r\n")) == NULL){
        close_connection(user, "Error msg recieve from Server", 1);
    }
    tcp_regular_exp_check_parsing(token, "[ -~]", 1400, "Invalid message content in recieve from Server", user);
    char* msg_content = token;

    printf("%s: %s\n", rec_disp_name, msg_content);

    return;
}

void tcp_receive_msg(char* buffer,size_t buffer_size, userInfo* user){

    if (recv(client_socket, buffer, buffer_size, 0) < 0){
            perror("Recieve failed");
            exit(EXIT_FAILURE);
        }
    /* tokenize buffer is creating to keep a whole unbroken by function strtok() message from the server.*/
    char tokenized_buffer[1024];
    memset(tokenized_buffer, 0, sizeof(tokenized_buffer));
    strcpy(tokenized_buffer, buffer);
    char* token = strtok(tokenized_buffer, " \t\n\r");

    /* Implementation of two states when user is authorized and can apply all types of messages from server 
        and when he can apply only reply for his auth command (FSM in task)*/
    switch(user->authorized){
        case 0:
            if(strcmp(token, "REPLY") == 0){
                tcp_parse_reply(token, user);
                user->reply_request = 0;
            } else if(strcmp(token, "BYE") == 0){
                close_connection(user, "", 0);
            } else if(strcmp(token, "ERR") == 0){
                tcp_parse_err(token, user);
            }
            break;
        case 1:
            if(strcmp(token, "REPLY") == 0){
                tcp_parse_reply(token, user);
                user->reply_request = 0;
            } else if(strcmp(token, "MSG") == 0){
                tcp_parse_common_msg(token, user);
            } else if(strcmp(token, "BYE") == 0){
                close_connection(user, "", 0);
            } else if(strcmp(token, "ERR") == 0){
                tcp_parse_err(token, user);
            } else{
                close_connection(user, "Invalid message from server", 1);
            }
            break;
    }
    return;

}

/* Code block for creating messags to send based on user's command */
/* MSG FROM {DisplayName} IS {MessageContent}\r\n */
int tcp_create_common_message(char* input, userInfo* user,char* msg, int msg_size){

    if(tcp_regular_exp_check_creating(input, "[ -~]", 1400, "Invalid message content") == 1){
        return NOT_TO_SEND;
    }
    input[strlen(input) - 1] = '\0';

    snprintf(msg, msg_size, "MSG FROM %s IS %s\r\n",user->display_name, input);

    return 0;
}

/* Local command where user rename his display name on the server*/
int tcp_local_rename(userInfo* user){

    char* new_display_name;
    if((new_display_name = strtok(NULL, " ")) == NULL){
        printf("Too few arguments for this command\n");
        return NOT_TO_SEND;
    }
    new_display_name[strlen(new_display_name) - 1] = '\0';
    if(tcp_regular_exp_check_creating(new_display_name, "[A-z0-9-]", 20, "Incorrect format display name") == 1){
        return NOT_TO_SEND;
    }

    if(strtok(NULL, " ") != NULL){
        printf("ERR: Too many arguments for this command\n");
        return NOT_TO_SEND;
    }

    strcpy(user->display_name, new_display_name);

    return 0;
}

/* JOIN {Channel} AS {DisplayName}\r\n */
int tcp_create_join_msg(userInfo* user,char* msg, int msg_size){

    char* channel;
    if((channel = strtok(NULL, " ")) == NULL){
        printf("ERR: Too few arguments for this command\n");
        return NOT_TO_SEND;
    }
    channel[strlen(channel) - 1] = '\0';

    if(tcp_regular_exp_check_creating(channel, "[A-z0-9-]", 20, "Incorrect format channel") == 1){
        return NOT_TO_SEND;
    }

    if(strtok(NULL, " ") != NULL){
        printf("ERR: Too many arguments for this command\n");
        return NOT_TO_SEND;
    }

    snprintf(msg, msg_size, "JOIN %s AS %s\r\n", channel, user->display_name);

    return 0;
}

/* AUTH {Username} AS {DisplayName} USING {Secret}\r\n */
int tcp_create_auth_msg(userInfo* user, char* msg, int msg_size){

    char* username;
    if((username = strtok(NULL, " ")) == NULL){
        printf("Too few arguments for this command\n");
        return NOT_TO_SEND;
    }
    if(tcp_regular_exp_check_creating(username, "[A-z0-9-]", 20, "Incorrect format username") == 1){
        return NOT_TO_SEND;
    }

    char* secret;
    if((secret = strtok(NULL, " ")) == NULL){
        printf("Too few arguments for this command\n");
        return NOT_TO_SEND;
    
    }
    if(tcp_regular_exp_check_creating(secret, "[A-z0-9-]", 128, "Incorrect format secret") == 1){
        return NOT_TO_SEND;
    }

    char* display_name;
    if((display_name = strtok(NULL, " \n")) == NULL){
        printf("Too few arguments for this command\n");
        return NOT_TO_SEND;
    }
    if(tcp_regular_exp_check_creating(display_name, "[A-z0-9-]", 20, "Incorrect format display name") == 1){
        return NOT_TO_SEND;
    }

    if(strtok(NULL, " ") != NULL){
        printf("Too many arguments for this command\n");
        return NOT_TO_SEND;
    }
    
    snprintf(msg, msg_size, "AUTH %s AS %s USING %s\r\n", username, display_name, secret);

    strcpy(user->username, username);
    strcpy(user->secret, secret);
    strcpy(user->display_name, display_name);

    return 0;
}


int tcp_format_input(char* buffer, userInfo* user){

    char input[1024];
    memset(input, 0, sizeof(input));

    fgets(input, BUFFER_SIZE, stdin);

    /* Tokenize buffer is creating to keep a whole unbroken by function strtok() message from the server.*/
    char tokenized_buffer[1024];
    memset(tokenized_buffer, 0, sizeof(tokenized_buffer));
    strcpy(tokenized_buffer, input); 

    /* Take the first token on the basis of which we will determine right message to start collecting for sending. */
    char* token;
    if ((token = strtok(tokenized_buffer, " \t\n")) == NULL){
        return NOT_TO_SEND;
    }

    /* Implementation of two states when user is authorized and can send all types of messages to server 
        and when he can send only /auth for his authorization, /help (FSM in task)*/
    switch(user->authorized){
        case 0:
            if (strcmp(token, "/auth") == 0){
                MSGFORMATCHECK(tcp_create_auth_msg(user, buffer, BUFFER_SIZE));
                user->reply_request = 1;
                return TO_SEND;
            } else if (strcmp(token, "/help") == 0) {
                printf("Commands:\n/auth <username> <display-name> <secret> - authenticate with the server\n/join <channel> - join a channel\n/rename <new-display-name> - change your display name\n");
                return NOT_TO_SEND; 
            }  else {
                printf("You are not authorized\n");
                return NOT_TO_SEND;
            }
        
        case 1:
            if (strcmp(token, "/auth") == 0) {
                printf("You are already authorized\n");
                return NOT_TO_SEND;
            } else if (strcmp(token, "/join") == 0) {
                MSGFORMATCHECK(tcp_create_join_msg(user, buffer, BUFFER_SIZE));
                user->reply_request = 1;
                return TO_SEND;
            } else if (strcmp(token, "/rename") == 0) {
                MSGFORMATCHECK(tcp_local_rename(user));
                return NOT_TO_SEND;
            } else if (strcmp(token, "/help") == 0) {
                printf("Commands:\n/auth <username> <display-name> <secret> - authenticate with the server\n/join <channel> - join a channel\n/rename <new-display-name> - change your display name\n");
                return NOT_TO_SEND;
            } else if(strncmp(token, "/", 1) == 0) {
                printf("invalid command\n");
                return NOT_TO_SEND;
            } else{
                MSGFORMATCHECK(tcp_create_common_message(input, user, buffer, BUFFER_SIZE));
                return TO_SEND;
            }

        default:
            break;

    }

    return NOT_TO_SEND;

}


void tcp_connection(userInfo* user, struct addrinfo *res){
    /* Buffer for through which to exchange information with server*/
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int nfds = 2;
    /* The program will know what to do in case of Сtrl+C*/
    signal(SIGINT, interrupt_connection);
    /* Preparations to use function poll which will notice program if some event has happened */
    polled_fds = calloc(nfds,sizeof(struct pollfd));

    polled_fds[0].fd = STDIN_FILENO;
    polled_fds[0].events = POLLIN;
    polled_fds[1].fd = client_socket;
    polled_fds[1].events = POLLIN;


    if (connect(client_socket, res->ai_addr, res->ai_addrlen) == -1) {
        close_connection(user, "Failed to connect to server", 1);
    }
    printf("Connected to server\n");

    /* Some messages (for example join) need to receive repply from server and based on this 
        flag the program prohibits sending other messages until REPLY*/
    user->reply_request = 0;
    user->authorized = 0;
    while(1) { 
        int ready_sockets = poll(polled_fds, nfds, 0);
        /* Stdin receive msg */
        if(polled_fds[0].revents & POLLIN && user->reply_request == 0) {
            memset(buffer, 0, BUFFER_SIZE);
            int send_res = tcp_format_input(buffer, user);
            /* If the message is not to be sent, the program will continiue waiting for another event. */ 
            if (send_res == NOT_TO_SEND){
                continue;
            }
            if (send(client_socket, buffer, BUFFER_SIZE, 0) < 0){
                close_connection(user, "Failed to send the message", 1);
            }
        }
        /* Socket see msg from server */
        if(polled_fds[1].revents & POLLIN) {
            memset(buffer, 0, sizeof(buffer));
            tcp_receive_msg(buffer, BUFFER_SIZE, user);
        }
        if(polled_fds[1].revents & (POLLERR | POLLHUP)) {
            close(client_socket);
        }
    }

}
