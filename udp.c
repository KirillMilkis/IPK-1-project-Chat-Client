#include "udp.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024
#define CONFIRMED_IDS_SIZE 1000

#define NOT_TO_SEND 5
#define TO_SEND 6

struct addrinfo *udp_res;

/* The function of connection termination has its own function here because in addition to deleting all allocated objects it is necessary to send BYE */
void udp_close_connection(userInfo* user, char* error, int is_error){
    if(user->authorized){
        msgPacket msg_packet;
        udp_create_bye_msg(user, &msg_packet, user->user_msg_id_storage->client_msg_id);
        udp_send_msg(&msg_packet, user, udp_res);
    }
    if(user->prev_msg_packet != NULL){
        free(user->prev_msg_packet->msg);
    }
    if(user->user_msg_id_storage->confirmed_ids != NULL){
        free(user->user_msg_id_storage->confirmed_ids);
    }
    close_connection(user, error, is_error);
    
}
/* Code block for testing individual server key words */ 
void udp_regular_exp_check_parsing(char* field, char* pattern, int max_length, char* error_msg, userInfo* user){
    regex_t reegex;
    regcomp(&reegex, pattern, 0);
    int comp = regexec(&reegex, field, 0, NULL, 0);
    if (strlen(field) > max_length || comp != 0){
       udp_close_connection(user, error_msg, 1);
    }
}

int udp_regular_exp_check_creating(char* field, char* pattern, int max_length, char* upcoming_msg, char* error_msg){
    regex_t reegex;
    regcomp(&reegex, pattern, 0);
    int comp = regexec(&reegex, field, 0, NULL, 0);
    if (strlen(field) > max_length || comp != 0){
        free(upcoming_msg);
        printf("ERR: %s\n", error_msg);
        return 1;
    }
    return 0;
}

/* Function to end the connection with Сtrl+C signal*/
void udp_interrupt_connection(int sig){

    udp_close_connection(user, "", 0);

}

/* Function that adds a field with a parameter to the integer array to be sent as msgPacket */
void add_field_msg(msgPacket* msg_packet, char* field){
    msg_packet->msgSize += strlen(field) + 1;
    msg_packet->msg = realloc(msg_packet->msg, msg_packet->msgSize + 1);
    strcpy(&msg_packet->msg[msg_packet->msgSize - strlen(field) - 1], field);
    msg_packet->msg[msg_packet->msgSize - 1] = '\0';

}

/*  1 byte       2 bytes
+--------+--------+--------+
|  0xFF  |    MessageID    |
+--------+--------+--------+
*/
void udp_create_bye_msg(userInfo* user, msgPacket* msg_packet, uint16_t msg_id){
    msg_packet->msgSize = 3;
    msg_packet->msg = malloc(sizeof(char) * msg_packet->msgSize);
    if (msg_packet->msg == NULL){
        free(msg_packet->msg);
        udp_close_connection(user, "Error allocation memory", 1);
    }    

    msg_packet->msg[0] = (uint8_t)0xFF;
    msg_packet->msg[1] = (msg_id >> 8) & 0xFF; 
    msg_packet->msg[2] = msg_id & 0xFF; 

    return;
}

/*  1 byte       2 bytes
+--------+--------+--------+-------~~------+---+--------~~---------+---+
|  0x04  |    MessageID    |  DisplayName  | 0 |  MessageContents  | 0 |
+--------+--------+--------+-------~~------+---+--------~~---------+---+
*/
int udp_create_common_msg(char* buffer, userInfo* user, msgPacket* msg_packet, uint16_t msg_id){

    /* Allocation memory for 3 bytes MessageType and Message ID */
    msg_packet->msgSize = 3;
    msg_packet->msg = malloc(sizeof(char) * msg_packet->msgSize);
    if (msg_packet->msg == NULL){
        free(msg_packet->msg);
        udp_close_connection(user, "Error allocation memory", 1);
    }    
    /* Assigning important values to these 3 bytes */
    msg_packet->msg[0] = (uint8_t)0x04;
    msg_packet->msg[1] = (msg_id >> 8) & 0xFF; 
    msg_packet->msg[2] = msg_id & 0xFF; 

    /* If the function strtok returns null, the user command is incorrect, too short.*/
    char* message;
    if((message = strtok(buffer, "\n")) == NULL){
        printf("ERR: Too few arguments for this command\n");
        return NOT_TO_SEND;
    }
    /* Length check and regular expression сheck*/
    if (udp_regular_exp_check_creating(message, "[ -~]", 1400, msg_packet->msg, "Incorrect message format") == 1){
        return NOT_TO_SEND;
    }

    /* Adding more parameters to the package because we don't know their length, so it is probematic.*/
    add_field_msg(msg_packet, user->display_name);
    add_field_msg(msg_packet, message);

    return 0;
}

/* Local command where user rename his display name on the server */
int udp_local_rename(userInfo* user){

    char* new_display_name;
    if((new_display_name = strtok(NULL, " ")) == NULL){
        printf("ERR: Too few arguments for this command\n");
        return NOT_TO_SEND;
    }
    if (udp_regular_exp_check_creating(new_display_name, "[A-z0-9-]", 20, new_display_name, "Incorrect display name format") == 1){
        return NOT_TO_SEND;
    }
    new_display_name[strlen(new_display_name) - 1] = '\0';

    /* A user command must consist of a certain number of word. Testing for this */
    if(strtok(NULL, " ") != NULL){
        printf("ERR: Too many arguments for this command\n");
        return NOT_TO_SEND;
    }

    strcpy(user->display_name, new_display_name);

    return 0;
}

/*  1 byte       2 bytes
+--------+--------+--------+-----~~-----+---+-------~~------+---+
|  0x03  |    MessageID    |  ChannelID | 0 |  DisplayName  | 0 |
+--------+--------+--------+-----~~-----+---+-------~~------+---+
*/
int udp_create_join_msg(char* buffer, userInfo* user, msgPacket* msg_packet, uint16_t msg_id){
    
    msg_packet->msgSize = 3;
    msg_packet->msg = malloc(sizeof(char) * msg_packet->msgSize);
    if (msg_packet->msg == NULL){
        free(msg_packet->msg);
        udp_close_connection(user, "Error allocation memory", 1);
    }    

    msg_packet->msg[0] = (uint8_t)0x03;
    msg_packet->msg[1] = (msg_id >> 8) & 0xFF; 
    msg_packet->msg[2] = msg_id & 0xFF; 

    char* channel;
    if((channel = strtok(NULL, " ")) == NULL){
        printf("ERR: Too few arguments for this command\n");
        return NOT_TO_SEND;
    }
    if(udp_regular_exp_check_creating(channel, "[A-z0-9-]", 20, msg_packet->msg, "Incorrect channel format") == 1){
        return NOT_TO_SEND;
    }

    if(strtok(NULL, " ") != NULL){
        free(msg_packet->msg);
        printf("ERR: Too many arguments for this command\n");
        return NOT_TO_SEND;
    }

    add_field_msg(msg_packet, channel);
    add_field_msg(msg_packet, user->display_name);

    return 0;
}


/*  1 byte       2 bytes
+--------+--------+--------+-----~~-----+---+-------~~------+---+----~~----+---+
|  0x02  |    MessageID    |  Username  | 0 |  DisplayName  | 0 |  Secret  | 0 |
+--------+--------+--------+-----~~-----+---+-------~~------+---+----~~----+---+
*/
int udp_create_auth_msg(char* buffer, userInfo* user, msgPacket* msg_packet, uint16_t msg_id){

    msg_packet->msgSize = 3;
    msg_packet->msg = malloc(sizeof(char) * msg_packet->msgSize);
    if (msg_packet->msg == NULL){
        free(msg_packet->msg);
        udp_close_connection(user, "Error allocation memory", 1);
    }    

    msg_packet->msg[0] = (uint8_t)0x02;
    msg_packet->msg[1] = (msg_id >> 8) & 0xFF; 
    msg_packet->msg[2] = msg_id & 0xFF; 

    
    char* username;
    if ((username = strtok(NULL, " ")) == NULL){
        free(msg_packet->msg);
        printf("ERR: Too few arguments for this command\n");
        return NOT_TO_SEND;
    }
    if(udp_regular_exp_check_creating(username, "[A-z0-9-]", 20, msg_packet->msg, "Incorrect username format") == 1){
        return NOT_TO_SEND;
    }

    char* secret;
    if ((secret = strtok(NULL, " ")) == NULL){
        free(msg_packet->msg);
        printf("ERR: Too few arguments for this command\n");
        return NOT_TO_SEND;
    }
    if(udp_regular_exp_check_creating(secret, "[A-z0-9-]", 128, msg_packet->msg, "Incorrect secret format") == 1){
        return NOT_TO_SEND;
    }

    char* display_name;
    if ((display_name = strtok(NULL, " \n")) == NULL){
        free(msg_packet->msg);
        printf("ERR: Too few arguments for this command\n");
        return NOT_TO_SEND;
    
    }
    display_name[strlen(display_name) - 1] = '\0';
    if(udp_regular_exp_check_creating(display_name, "[A-z0-9-]", 20, msg_packet->msg, "Incorrect display name format") == 1){
        return NOT_TO_SEND;
    }

    if(strtok(NULL, " ") != NULL){
        free(msg_packet->msg);
        printf("ERR: Too many arguments for this command\n");
        return NOT_TO_SEND;
    }

    add_field_msg(msg_packet, username);
    add_field_msg(msg_packet, display_name);
    add_field_msg(msg_packet, secret);

    strcpy(user->username, username);
    strcpy(user->secret, secret);
    strcpy(user->display_name, display_name);

    return 0;
}

/*  1 byte       2 bytes
+--------+--------+--------+
|  0x00  |  Ref_MessageID  |
+--------+--------+--------+
*/
void udp_create_confirm(uint16_t msg_id, msgPacket* msg_packet, userInfo* user){

    msg_packet->msgSize = 3;
    msg_packet->msg = malloc(sizeof(char) * msg_packet->msgSize);
    if (msg_packet->msg == NULL){
        free(msg_packet->msg);
        udp_close_connection(user, "Error allocation memory", 1);
    }

    msg_packet->msg[0] = (uint8_t)0x00;
    msg_packet->msg[1] = (msg_id >> 8) & 0xFF; 
    msg_packet->msg[2] = msg_id & 0xFF; 

}


void udp_send_msg(msgPacket* msg_packet, userInfo* user, struct addrinfo *res){
    int bytesend = sendto(client_socket, msg_packet->msg, msg_packet->msgSize, 0, res->ai_addr, res->ai_addrlen);
    if (bytesend < 0){
        udp_close_connection(user, "Failed to send the message", 1);
    }
}

/* Fu*/
int udp_format_input(userInfo* user, msgPacket* msg_packet, char* buffer, uint16_t client_msg_id){
    
    fgets(buffer, BUFFER_SIZE, stdin);
    /* Tokenize buffer is creating to keep a whole unbroken by function strtok() message from the server.*/
    char tokenized_buffer[1024];
    memset(tokenized_buffer, 0, sizeof(tokenized_buffer));
    strcpy(tokenized_buffer, buffer);

    /* Take the first token on the basis of which we will determine right message to start collecting for sending. */
    char* token;
    if((token = strtok(tokenized_buffer, " \t\n")) == NULL){
        return NOT_TO_SEND;
    }

    /* Implementation of two states when user is authorized and can send all types of messages to server 
        and when he can send only /auth for his authorization, /help (FSM in task)*/
    switch(user->authorized){
        case 0:
            if(strcmp(token, "/auth") == 0){
                MSGFORMATCHECK(udp_create_auth_msg(buffer, user, msg_packet, client_msg_id));
                user->serv_confirm_request = 1;
                user->reply_request = 1;
            } else if(strcmp(token, "/help") == 0){
                printf("Commands:\n/auth <username> <display-name> <secret> - authenticate with the server\n/join <channel> - join a channel\n/rename <new-display-name> - change your display name\n");
                return NOT_TO_SEND;
            }
            break;
        case 1:
            if(strcmp(token, "/auth") == 0){
                printf("You are already authorized\n");
                return 0;
            } else if(strcmp(token, "/join") == 0){
                MSGFORMATCHECK(udp_create_join_msg(buffer, user, msg_packet, client_msg_id));
                user->serv_confirm_request = 1;
                user->reply_request = 1;
                break;
            } else if(strcmp(token, "/rename") == 0){
                MSGFORMATCHECK(udp_local_rename(user));
                return NOT_TO_SEND;
            } else if(strcmp(token, "/help") == 0){
                printf("Commands:\n/auth <username> <display-name> <secret> - authenticate with the server\n/join <channel> - join a channel\n/rename <new-display-name> - change your display name\n");
                return NOT_TO_SEND;
            } else if(strncmp(token, "/", 1) == 0){
                printf("invalid command\n");
                return NOT_TO_SEND;
            } else{
                MSGFORMATCHECK(udp_create_common_msg(buffer, user, msg_packet, client_msg_id));
                user->serv_confirm_request = 1;
                break;
            }

        default:
            break;

    }

        user->prev_msg_packet = msg_packet;

        return TO_SEND;
    }

/* Enumerating the array with previous ids. */
int was_id_previously(msgIdStorage* msg_id_storage){
    /* If it is a message with id 0, then it is not necessary to search the array because by default the array elements are equal to 0. */
    if (msg_id_storage->serv_msg_id == 0) {
        return 0;
    }

    for (int i = 0; i < msg_id_storage->confirmed_ids_size; i++) {
        if (msg_id_storage->serv_msg_id == msg_id_storage->confirmed_ids[i]) {
            return 1;
        }
    }

    return 0;
}

int id_check(msgIdStorage* msg_id_storage){

    if (was_id_previously(msg_id_storage)) {
        return 1;
    } else {
        /* If the array with ids is not initialized, then we allocate memory for it. */
        if (msg_id_storage->confirmed_ids == NULL) {
            msg_id_storage->confirmed_ids = calloc(10, sizeof(uint16_t));
            msg_id_storage->confirmed_ids_size = 10;
        }
        /* If we run out of space in the array with ids, we increase it by 10. */
        if (msg_id_storage->confirmed_ids_size <= msg_id_storage->confirmed_ids_count) {
            uint16_t* new_array = realloc(msg_id_storage->confirmed_ids, (msg_id_storage->confirmed_ids_size + 10) * sizeof(uint16_t));
            if (new_array == NULL) {
                return -1;
            }
            msg_id_storage->confirmed_ids = new_array;
            msg_id_storage->confirmed_ids_size += 10;
        }
        /* Add new id value */
        msg_id_storage->confirmed_ids[msg_id_storage->confirmed_ids_count++] = msg_id_storage->serv_msg_id;
        return 0;
    }
}

/* Code block for parsing incoming messages from Server*/
/*   1 byte       2 bytes       1 byte       2 bytes
+--------+--------+--------+--------+--------+--------+--------~~---------+---+
|  0x01  |    MessageID    | Result |  Ref_MessageID  |  MessageContents  | 0 |
+--------+--------+--------+--------+--------+--------+--------~~---------+---+
*/
void udp_parse_reply(char* buffer, userInfo* user, int byterecv, msgIdStorage* msg_id_storage){

    msg_id_storage->serv_msg_id = ((uint16_t)buffer[1] << 8);
    msg_id_storage->serv_msg_id += (uint16_t)buffer[2];

    int id_check_result = id_check(msg_id_storage);
    if (id_check_result == 1){
        printf("Reply already confirmed\n");
        return;
    } else if(id_check_result == -1){
        udp_close_connection(user, "Allocation error", 1);
    }
    
    uint8_t result = (uint8_t)buffer[3];

    uint16_t reply_msg_id = ((uint16_t)buffer[4] << 8);
    reply_msg_id += (uint16_t)buffer[5];

    if (reply_msg_id != msg_id_storage->client_msg_id){
        udp_close_connection(user, "Unexpected msg confirm", 1);
    }  

    char msg_content[byterecv - 6];
    strncpy(msg_content, buffer + 6, byterecv - 6);
    msg_content[byterecv - 6] = '\0';

    udp_regular_exp_check_parsing(msg_content, "[ -~]", 1400, "Incorrect message format from Server", user);

    if(strcmp(msg_content, "Authentication successful.") == 0){
        user->authorized = 1;
    } 

    if(result){
        printf("Success: %s\n", msg_content);
    } else{
        printf("Failure: %s\n", msg_content);
    }

    return;
    
}
/*
  1 byte       2 bytes
+--------+--------+--------+
|  0x00  |  Ref_MessageID  |
+--------+--------+--------+
*/
void udp_parse_confirm(char* buffer, userInfo* user, msgIdStorage* msg_id_storage){

    /* Record the ID of the message that was confirmed */
    uint16_t confirmed_msg_num = ((uint16_t)buffer[1] << 8);
    confirmed_msg_num += (uint16_t)buffer[2];

    /* Check if it is the message we expected*/
    if (confirmed_msg_num != msg_id_storage->client_msg_id){
         udp_close_connection(user, "Failed to confirm the message", 1);
    } 

    return;

}
/*
  1 byte       2 bytes
+--------+--------+--------+-------~~------+---+--------~~---------+---+
|  0x04  |    MessageID    |  DisplayName  | 0 |  MessageContents  | 0 |
+--------+--------+--------+-------~~------+---+--------~~---------+---+
*/
void udp_parse_msg(char* buffer, userInfo* user, int byterecv, msgIdStorage* msg_id_storage){

    /* Record message ID*/
    msg_id_storage->serv_msg_id = ((uint16_t)buffer[1] << 8);
    msg_id_storage->serv_msg_id += (uint16_t)buffer[2];

    /* Сhecking whether this message has been accepted before */
    int id_check_result = id_check(msg_id_storage);
    if (id_check_result == 1){
       return;
    /* Id array can be allocated with errors */
    } else if(id_check_result == -1){
        udp_close_connection(user, "Allocation error", 1);
    }

    /* Since we do not know the lengths of the parameters , writing by searching for the first separating element. */
    int field_length = 0;
    int index = 3;
    while (buffer[index] != '\0') {
        field_length++;
        index++;
    }

    char display_name[field_length + 1]; 
    strncpy(display_name, &buffer[3], field_length);
    display_name[field_length] = '\0'; 

    udp_regular_exp_check_parsing(display_name, "[A-z0-9-]", 20, "Incorrect display name format from Server", user);

    index++;

    field_length = 0;
    while (buffer[index] != '\0') {
        field_length++;
        index++;
    }

    char msg_content[field_length + 1]; 
    strncpy(msg_content, &buffer[index - field_length], field_length);
    msg_content[field_length] = '\0'; 

    udp_regular_exp_check_parsing(msg_content, "[ -~]", 1400, "Incorrect message format from Server", user);

    /* Check for right numbers of parameters*/
    if ((byterecv - (index + 1)) > 0){
        udp_close_connection(user, "Unexpected msg format", 1);
    }

    printf("%s: %s\n", display_name, msg_content);

    return;

}

void udp_parse_bye(char* buffer, userInfo* user, int byterecv, msgIdStorage* msg_id_storage){

    msg_id_storage->serv_msg_id = ((uint16_t)buffer[1] << 8);
    msg_id_storage->serv_msg_id += (uint16_t)buffer[2];

    int id_check_result = id_check(msg_id_storage);
    if (id_check_result == 1){
       return;
    } else if(id_check_result == -1){
        udp_close_connection(user, "Allocation error", 1);
    }

    if(byterecv > 3){
        udp_close_connection(user, "Unexpected bye msg format", 1);
    }

    return;

    
}

void udp_parse_error(char* buffer, userInfo* user, int byterecv, msgIdStorage* msg_id_storage){

    msg_id_storage->serv_msg_id = ((uint16_t)buffer[1] << 8);
    msg_id_storage->serv_msg_id += (uint16_t)buffer[2];


    int id_check_result = id_check(msg_id_storage);
    if(id_check_result == 1){
        return;
    } else if(id_check_result == -1){
        udp_close_connection(user, "Allocation error", 1);
    }

    int index = 3;
    int field_length = 0;
    while (buffer[index] != '\0') {
        field_length++;
        index++;
    }

    char display_name[field_length + 1]; 
    memcpy(display_name, &buffer[3], field_length);
    display_name[field_length] = '\0';

    udp_regular_exp_check_parsing(display_name, "[A-z0-9-]", 20, "Incorrect display name format from Server", user);

    index++;

    field_length = 0;
    while (buffer[index] != '\0') {
        field_length++;
        index++;
    }

    char msg_content[field_length + 1]; 
    memcpy(msg_content, &buffer[index - field_length], field_length);
    msg_content[field_length] = '\0';

    udp_regular_exp_check_parsing(msg_content, "[ -~]", 1400, "Incorrect message format from Server", user);

    if ((byterecv - (index + 1)) > 0)
    {
        udp_close_connection(user, "Unexpected error msg format", 1);
    }
    
    printf("ERR FROM %s: %s\n", display_name, msg_content);

    return;
}


void udp_receive_msg(char* buffer, userInfo* user, struct addrinfo* res, msgIdStorage* msg_id_storage){

    int byterecv;
    byterecv = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, res->ai_addr, &res->ai_addrlen);
    if (byterecv < 0) {
        udp_close_connection(user, "Failed to receive the message", 1);
    }

    uint8_t msg_type = (uint8_t)buffer[0];

    /* Implementation of two states when user is authorized and can apply all types of messages from server 
        and when he can apply reply for his auth command, BYE, ERR (FSM in task)*/
    switch(user->authorized){
                /* CONFIRM */
        case 0: if (msg_type == 0x00){
                    if (user->serv_confirm_request == 1){
                        udp_parse_confirm(buffer, user, msg_id_storage);
                        user->serv_confirm_request = 0;
                    } else{
                        udp_close_connection(user, "Unexpected confirm message", 1);
                    }
                    break;
                /* REPLY */
                }else if(msg_type == 0x01){
                    if (user->reply_request == 1){ 
                        udp_parse_reply(buffer, user, byterecv, msg_id_storage);
                        user->reply_request = 0;
                        user->client_confirm_request = 1;

                    } else{  
                        udp_close_connection(user, "Unexpected reply message", 1);
                    }
                    break;
                /* ERR */
                } else if (msg_type == 0xFE) {
                    udp_parse_error(buffer, user, byterecv, msg_id_storage);
                    user->client_confirm_request = 1;
                    udp_close_connection(user, "Recieved error before auth", 1);
                    break;
                /* BYE */
                } else if (msg_type == 0xFF) {
                    udp_parse_bye(buffer, user, byterecv, msg_id_storage);
                    msgPacket msg_packet;
                    udp_create_confirm(msg_id_storage->serv_msg_id, &msg_packet, user);
                    udp_send_msg(&msg_packet, user, res);
                    udp_close_connection(user, "", 0);
                    break;
                /* ELSE */
                } else{
                    udp_close_connection(user, "Unexpected message", 1);
                     break;
                }

        case 1:
            /* CONFIRM */
            if (msg_type == 0x00) {
                if (user->serv_confirm_request == 1) {
                    udp_parse_confirm(buffer, user, msg_id_storage);
                    user->serv_confirm_request = 0;
                } else {
                    udp_close_connection(user, "Unexpected confirm message", 1);
                }
                break;
            /* REPLY */
            } else if (msg_type == 0x01) {
                if (user->reply_request == 1) {
                    udp_parse_reply(buffer, user, byterecv, msg_id_storage);
                    user->reply_request = 0;
                    user->client_confirm_request = 1;
                } else {
                    udp_close_connection(user, "Unexpected reply message", 1);
                }
                break;
            /* MSG */
            } else if (msg_type == 0x04) {
                udp_parse_msg(buffer, user, byterecv, msg_id_storage);
                user->client_confirm_request = 1;
                break;
            /* ERR */
            } else if (msg_type == 0xFE) {
                udp_parse_error(buffer, user, byterecv, msg_id_storage);
                user->client_confirm_request = 1;
                break;
            /* BYE */
            } else if (msg_type == 0xFF) {
                printf("Server disconnected\n");
                udp_parse_bye(buffer, user, byterecv, msg_id_storage);
                msgPacket msg_packet;
                udp_create_confirm(msg_id_storage->serv_msg_id, &msg_packet, user);
                udp_send_msg(&msg_packet, user, res);
                udp_close_connection(user, "", 0);
                break;
            /* ELSE */
            } else{
                udp_close_connection(user, "Unexpected message", 1);
                break;
            }

    }  
    return; 
}


int udp_connection(userInfo* user, struct addrinfo *res){

    /* Buffer for through which to exchange information with server*/
    char buffer[1024];
    int buffer_size = sizeof(buffer);
    memset(buffer, 0, buffer_size);

    udp_res = res;

    int bytesend;
    int byterecv;

    /* Storage that will record all ids */
    struct msgIdStorage msg_id_storage = {.client_msg_id = -1};
    user->user_msg_id_storage = &msg_id_storage;

    /* The program will know what to do in case of Сtrl+C*/
    signal(SIGINT, udp_interrupt_connection);

    /* Preparations to use function poll which will notice program if some event has happened */
    int nfds = 2;

    polled_fds = calloc(nfds,sizeof(struct pollfd)); 

    polled_fds[0].fd = STDIN_FILENO;
    polled_fds[0].events = POLLIN;
    polled_fds[1].fd = client_socket;
    polled_fds[1].events = POLLIN;

    struct timeval start_time, current_time;
    int timer_running = 0;
    int retry_num = 0;

    /* Flags that will allow or disallow sending a message to the server in a certain state. */
    user->authorized = 0;
    user->reply_request = 0;
    user->serv_confirm_request = 0;

    while(1){

        int ready_sockets = poll(polled_fds, nfds, 0);
        
        /*  Stdin receive msg */
        if(polled_fds[0].revents & POLLIN && timer_running == 0 && user->reply_request == 0 && user->serv_confirm_request == 0){

            msg_id_storage.client_msg_id++;

            memset(buffer, 0, buffer_size);
            
            msgPacket msg_packet;
            int format_msg_result = udp_format_input(user, &msg_packet, buffer, msg_id_storage.client_msg_id);
            if(format_msg_result != TO_SEND){
                continue;
            }
            
            udp_send_msg(&msg_packet, user, res);
            /* Start timer for waiting confirm message */
            gettimeofday(&start_time, NULL);
            timer_running = 1;

            continue;
        /* Socket see msg from server */
        } else if(polled_fds[1].revents & POLLIN) {

            memset(buffer, 0, buffer_size);

            udp_receive_msg(buffer, user, res, &msg_id_storage);

            msgPacket msg_packet;

            /* Send an confirmation to the server that we have received its message*/
            if (user->client_confirm_request == 1){
                udp_create_confirm(msg_id_storage.serv_msg_id, &msg_packet, user);
                udp_send_msg(&msg_packet, user, res);
            } 
            
            
        } else if(polled_fds[1].revents & (POLLERR | POLLHUP)) {
            udp_close_connection(user, "Poll error", 1);
        }
        /* if the client has received the necessary we turn off timer and can send msg or wait for receive in some states */
        if(user->serv_confirm_request == 0 && timer_running == 1){
            timer_running = 0;
            retry_num = 0;
        }

        /* Here we count the time that has passed since the moment of sending and if the time is up, we make an attempt 
            to send the same information to the server. If the attempts are over, we terminate the connection.*/
        if(user->serv_confirm_request == 1 && timer_running == 1){
            gettimeofday(&current_time, NULL);

            long elapsed_time_ms = (current_time.tv_sec - start_time.tv_sec) * 1000 +
                       (current_time.tv_usec - start_time.tv_usec) / 1000 +
                       ((current_time.tv_usec - start_time.tv_usec) % 1000 >= 500 ? 1 : 0);
        
            if (elapsed_time_ms > udp_timeout){
                printf("Send timeout %d\n", retry_num);
                retry_num++;
                if (retry_num == max_retransmissions){
                    free(user->prev_msg_packet->msg);
                    udp_close_connection(user, "Failed to send the message", 1);
                } else{
                    gettimeofday(&start_time, NULL);
                    udp_send_msg(user->prev_msg_packet, user, res);
                }

            }
    
        }
    }
    return 0;

}        