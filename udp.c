#include "udp.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define RETRY_COUNT 3 
#define TIMEOUT 250
#define BUFFER_SIZE 1024
#define CONFIRMED_IDS_SIZE 1000

#define LOCAL 5
#define SERVER 6

regex_t reegex;


void add_field_msg(msgPacket* msg_packet, char* field){
    msg_packet->msgSize += strlen(field) + 1;
    msg_packet->msg = realloc(msg_packet->msg, msg_packet->msgSize + 1);
    strcpy(&msg_packet->msg[msg_packet->msgSize - strlen(field) - 1], field);
    msg_packet->msg[msg_packet->msgSize - 1] = '\0';

}


int udp_create_chat_msg(char* token, char* buffer, userInfo* user, msgPacket* msg_packet, uint16_t msg_id){

    msg_packet->msgSize = 3;
    msg_packet->msg = malloc(sizeof(char) * msg_packet->msgSize);
    if (msg_packet->msg == NULL){
        udp_close_connection(user, "Error allocation memory", 1);
    }    

    msg_packet->msg[0] = (uint8_t)0x04;
    msg_packet->msg[1] = (msg_id >> 8) & 0xFF; 
    msg_packet->msg[2] = msg_id & 0xFF; 

    char* message = strtok(buffer, "\n");

    regcomp(&reegex, "[ -~]", 0);

    int comp1 = regexec(&reegex, message, 0, NULL, 0);

    if (strlen(message) > 1400 || comp1 != 0){
        free(msg_packet->msg);
        printf("Incorrect message format\n");
        return 1;
    }

    add_field_msg(msg_packet, user->display_name);
    add_field_msg(msg_packet, message);

    return 0;
}


int udp_local_rename(userInfo* user){

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



int udp_create_join_msg(char* token, char* buffer, userInfo* user, msgPacket* msg_packet, uint16_t msg_id){
    
    msg_packet->msgSize = 3;
    msg_packet->msg = malloc(sizeof(char) * msg_packet->msgSize);
    if (msg_packet->msg == NULL){
        udp_close_connection(user, "Error allocation memory", 1);
    }    

    msg_packet->msg[0] = (uint8_t)0x03;
    msg_packet->msg[1] = (msg_id >> 8) & 0xFF; 
    msg_packet->msg[2] = msg_id & 0xFF; 

    char* channel = strtok(NULL, " ");

    regcomp(&reegex, "[A-z0-9-]", 0);

    int comp1 = regexec(&reegex, channel, 0, NULL, 0);

    if (strlen(channel) > 20 || comp1 != 0){
        free(msg_packet->msg);
        printf("Incorrect channel format\n");
        return 1;
    }

    if(strtok(NULL, " ") != NULL){
        free(msg_packet->msg);
        printf("Too many arguments for this command\n");
        return 1;
    }

    add_field_msg(msg_packet, channel);
    add_field_msg(msg_packet, user->display_name);

    return 0;
}


int udp_create_auth_msg(char* token, char* buffer, userInfo* user, msgPacket* msg_packet, uint16_t msg_id){

    msg_packet->msgSize = 3;
    msg_packet->msg = malloc(sizeof(char) * msg_packet->msgSize);
    if (msg_packet->msg == NULL){
        udp_close_connection(user, "Error allocation memory", 1);
    }    

    msg_id = 0;

    msg_packet->msg[0] = (uint8_t)0x02;
    msg_packet->msg[1] = (msg_id >> 8) & 0xFF; 
    msg_packet->msg[2] = msg_id & 0xFF; 
    
    char* username = strtok(NULL, " ");

    regcomp(&reegex, "[A-z0-9-]", 0);

    int comp1 = regexec(&reegex, username, 0, NULL, 0);

    if (strlen(username) > 20 || comp1 != 0){
        free(msg_packet->msg);
        printf("Incorrect username format\n");
        return 1;
    }

    char* secret = strtok(NULL, " ");

    int comp2 = regexec(&reegex, secret, 0, NULL, 0);

    if (strlen(secret) > 120 || comp2 != 0){
        free(msg_packet->msg);
        printf("Incorrect secret format\n");
        return 1;
    }

    char* display_name = strtok(NULL, " \n");

    int comp3 = regexec(&reegex, display_name, 0, NULL, 0);

    if (strlen(display_name) > 128 || comp3 != 0){
        free(msg_packet->msg);
        printf("Incorrect display name format\n");
        return 1;
    }
    display_name[strlen(display_name)] = '\0';

    if(strtok(NULL, " ") != NULL){
        free(msg_packet->msg);
        printf("Too many arguments for this command\n");
        return 1;
    }

    add_field_msg(msg_packet, username);
    add_field_msg(msg_packet, display_name);
    add_field_msg(msg_packet, secret);

    strcpy(user->username, username);
    strcpy(user->secret, secret);
    strcpy(user->display_name, display_name);

    return 0;
}

void udp_create_confirm(uint16_t msg_id, msgPacket* msg_packet, userInfo* user){

    msg_packet->msgSize = 3;
    msg_packet->msg = malloc(sizeof(char) * msg_packet->msgSize);
    if (msg_packet->msg == NULL){
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


int udp_format_msg(struct addrinfo *res, userInfo* user, msgPacket* msg_packet, char* buffer, int bytesend, uint16_t client_msg_id){
    
    fgets(buffer, BUFFER_SIZE, stdin);
    char tokenized_buffer[1024];
    memset(tokenized_buffer, 0, sizeof(tokenized_buffer));
    strcpy(tokenized_buffer, buffer); 
    char* token = strtok(tokenized_buffer, " \t\n");

    printf("User auth: %d\n", user->authorized);

    switch(user->authorized){
        case 0:
            if(strcmp(token, "/auth") == 0){
                MSGFORMATCHECK(udp_create_auth_msg(token, buffer, user, msg_packet, client_msg_id));
                user->serv_confirm_request = 1;
                user->reply_request = 1;
            }
            break;
        case 1:
            if(strcmp(token, "/auth") == 0){
                printf("You are already authorized\n");
                return 0;
            } else if(strcmp(token, "/join") == 0){
                MSGFORMATCHECK(udp_create_join_msg(token, buffer, user, msg_packet, client_msg_id));
                user->serv_confirm_request = 1;
                user->reply_request = 1;
                break;
            } else if(strcmp(token, "/rename") == 0){
                MSGFORMATCHECK(udp_local_rename(user));
                return LOCAL;
            } else if(strcmp(token, "/help") == 0){
                printf("Commands:\n/auth <username> <display-name> <secret> - authenticate with the server\n/join <channel> - join a channel\n/rename <new-display-name> - change your display name\n");
                return LOCAL;
            } else if(strcmp(token, "//") == 0){
                printf("invalid command\n");
                return LOCAL;
            } else{
                printf("MSG FORM CLIENT\n");
                MSGFORMATCHECK(udp_create_chat_msg(token, buffer, user, msg_packet, client_msg_id));
                user->serv_confirm_request = 1;
                break;
            }

        default:
            break;

    }

        user->prev_msg_packet = msg_packet;

        return SERVER;
    }

int was_id_previously(msgIdStorage* msg_id_storage) {
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

int id_check(msgIdStorage* msg_id_storage) {
    if (was_id_previously(msg_id_storage)) {
        return 1;
    } else {
        if (msg_id_storage->confirmed_ids == NULL) {
            msg_id_storage->confirmed_ids = calloc(10, sizeof(uint16_t));
            msg_id_storage->confirmed_ids_size = 10;
        }

        if (msg_id_storage->confirmed_ids_size <= msg_id_storage->confirmed_ids_count) {
            uint16_t* new_array = realloc(msg_id_storage->confirmed_ids, (msg_id_storage->confirmed_ids_size + 10) * sizeof(uint16_t));
            if (new_array == NULL) {
                return -1;
            }
            msg_id_storage->confirmed_ids = new_array;
            msg_id_storage->confirmed_ids_size += 10;
        }

        msg_id_storage->confirmed_ids[msg_id_storage->confirmed_ids_count++] = msg_id_storage->serv_msg_id;
        return 0;
    }
}

void udp_parse_reply(char* buffer, int buffer_size, userInfo* user, int byterecv, msgIdStorage* msg_id_storage){
    msg_id_storage->serv_msg_id = ((uint16_t)buffer[1] << 8);
    msg_id_storage->serv_msg_id += (uint16_t)buffer[2];

    printf("MSG ID: %u\n", msg_id_storage->serv_msg_id);

    if (id_check(msg_id_storage) == 1){
        printf("Reply already confirmed\n");
        return;
    }
    
    uint8_t result = (uint8_t)buffer[3];

    uint16_t reply_msg_id = ((uint16_t)buffer[4] << 8);
    reply_msg_id += (uint16_t)buffer[5];

    if (reply_msg_id != msg_id_storage->client_msg_id){
        udp_close_connection(user, "Unexpected msg confirm", 1);
    }  

    char messageContent[byterecv - 6];
    strncpy(messageContent, buffer + 6, byterecv - 6);
    messageContent[byterecv - 6] = '\0';

    if(strcmp(messageContent, "Authentication successful.") == 0){
        user->authorized = 1;
    } 

    if(result){
        printf("Success: %s\n", messageContent);
    } else{
        printf("Failure: %s\n", messageContent);
    }
    
}

void udp_parse_confirm(char* buffer, int buffer_size, userInfo* user, msgIdStorage* msg_id_storage){

    uint16_t confirmed_msg_num = ((uint16_t)buffer[1] << 8);
    confirmed_msg_num += (uint16_t)buffer[2];

    if (confirmed_msg_num != msg_id_storage->client_msg_id){
         udp_close_connection(user, "Failed to confirm the message", 1);
    } 

}

void udp_parse_msg(char* buffer, int buffer_size, userInfo* user, int byterecv, msgIdStorage* msg_id_storage){
    
    msg_id_storage->serv_msg_id = ((uint16_t)buffer[1] << 8);
    msg_id_storage->serv_msg_id += (uint16_t)buffer[2];

    printf("MSG ID: %u\n", msg_id_storage->serv_msg_id);

    if (id_check(msg_id_storage) == 1){
       return;
    }

    int field_length = 0;
    int index = 3;
    while (buffer[index] != '\0') {
        field_length++;
        index++;
    }

    char display_name[field_length + 1]; 
    strncpy(display_name, &buffer[3], field_length);
    display_name[field_length] = '\0'; 

    index++;

    field_length = 0;
    while (buffer[index] != '\0') {
        field_length++;
        index++;
    }

    char msg_content[field_length + 1]; 
    strncpy(msg_content, &buffer[index - field_length], field_length);
    msg_content[field_length] = '\0'; 

    if ((byterecv - (index + 1)) > 0){
        udp_close_connection(user, "Unexpected msg format", 1);
    }


    printf("%s: %s\n", display_name, msg_content);

}

void udp_parse_join(char* buffer, int buffer_size, userInfo* user, int byterecv, msgIdStorage* msg_id_storage){
    
    msg_id_storage->serv_msg_id = ((uint16_t)buffer[1] << 8);
    msg_id_storage->serv_msg_id += (uint16_t)buffer[2];

    printf("MSG ID: %u\n", msg_id_storage->serv_msg_id);

    if (id_check(msg_id_storage) == 1){
       return;
    }

    int index = 3;
    int field_length = 0;
    while (buffer[index] != '\0'){
        field_length++;
        index++;
    }
    char channel_id[field_length];
    strncpy(channel_id, &buffer[3], field_length);
    channel_id[field_length] = '\0';

    index++;
    field_length = 0;
    while(buffer[index] != '\0'){
        field_length++;
        index++;
    }
    char display_name[field_length + 1];
    strncpy(display_name, &buffer[index - field_length], field_length);
    display_name[field_length] = '\0';

    if ((byterecv - (index + 1)) > 0){
        udp_close_connection(user, "Unexpected join msg format", 1);
    }


    printf("Server: %s has joined %s\n", display_name, channel_id);
}

void udp_parse_bye(char* buffer, int buffer_size, userInfo* user, int byterecv, msgIdStorage* msg_id_storage){
    msg_id_storage->serv_msg_id = ((uint16_t)buffer[1] << 8);
    msg_id_storage->serv_msg_id += (uint16_t)buffer[2];

    if (id_check(msg_id_storage) == 1){
       return;
    }

    if(byterecv > 3){
        udp_close_connection(user, "Unexpected bye msg format", 1);
    }

    printf("Server has disconnected\n");
}

void udp_parse_error(char* buffer, int buffer_size, userInfo* user, int byterecv, msgIdStorage* msg_id_storage){

    msg_id_storage->serv_msg_id = ((uint16_t)buffer[1] << 8);
    msg_id_storage->serv_msg_id += (uint16_t)buffer[2];

    printf("MSG ID: %u\n", msg_id_storage->serv_msg_id);

    if(id_check(msg_id_storage) == 1){
        return;
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

    index++;

    field_length = 0;
    while (buffer[index] != '\0') {
        field_length++;
        index++;
    }

    char msg_content[field_length + 1]; 
    memcpy(msg_content, &buffer[index - field_length], field_length);
    msg_content[field_length] = '\0';

    if ((byterecv - (index + 1)) > 0)
    {
        udp_close_connection(user, "Unexpected error msg format", 1);
    }
    
    printf("ERR FROM %s: %s\n", display_name, msg_content);
}


int udp_receive_msg(char* buffer, int buffer_size, userInfo* user, struct addrinfo* res, msgIdStorage* msg_id_storage){

    int byterecv;
    byterecv = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, res->ai_addr, &res->ai_addrlen);
    if (byterecv < 0) {
        udp_close_connection(user, "Failed to receive the message", 1);
    }

    uint8_t msg_type = (uint8_t)buffer[0];

    printf("MSG TYPE: %d\n", msg_type);

    switch(msg_type){
        case 0x00:
            if (user->serv_confirm_request == 1){
                udp_parse_confirm(buffer, buffer_size, user, msg_id_storage);
                user->serv_confirm_request = 0;
            } else{
                udp_close_connection(user, "Unexpected confirm message", 1);
            }
            break;
        case 0x01:
            if (user->reply_request == 1){ 
                udp_parse_reply(buffer, buffer_size, user, byterecv, msg_id_storage);
                user->reply_request = 0;
                user->client_confirm_request = 1;

            } else{  
                udp_close_connection(user, "Unexpected reply message", 1)
            }
            break;
        case 0x03:
            udp_parse_join(buffer, buffer_size, user, byterecv, msg_id_storage);
            user->client_confirm_request = 1;
            break;
        case 0x04:
            udp_parse_msg(buffer, buffer_size, user, byterecv, msg_id_storage);
            user->client_confirm_request = 1;
            break;
        case 0xFE:
            udp_parse_error(buffer, buffer_size, user, byterecv, msg_id_storage);
            user->client_confirm_request = 1;
            break;
        case 0xFF:
            udp_parse_bye(buffer, buffer_size, user, byterecv, msg_id_storage);
            user->client_confirm_request = 1;
            break;
        default:
            break;


    }  
    return 0; 
}


int udp_connection(userInfo* user, struct addrinfo *res){
    char buffer[1024];
    int buffer_size = sizeof(buffer);
    memset(buffer, 0, buffer_size);

    int bytesend;
    int byterecv;

    struct msgIdStorage msg_id_storage = {.client_msg_id = -1};

    int nfds = 2;

    polled_fds = calloc(nfds,sizeof(struct pollfd)); 

    polled_fds[0].fd = STDIN_FILENO;
    polled_fds[0].events = POLLIN;
    polled_fds[1].fd = client_socket;
    polled_fds[1].events = POLLIN;

    struct timeval start_time, current_time;
    int timer_running = 0;
    int retry_num = 0;

    user->authorized = 0;
    user->reply_request = 0;
    user->serv_confirm_request = 0;

    while(1){

        int ready_sockets = poll(polled_fds, nfds, 0);
        
    
        if(polled_fds[0].revents & POLLIN && timer_running == 0 && user->reply_request == 0 && user->serv_confirm_request == 0){

            msg_id_storage.client_msg_id++;

            memset(buffer, 0, buffer_size);
            
            msgPacket msg_packet;
            int format_msg_result = udp_format_msg(res, user, &msg_packet, buffer, bytesend, msg_id_storage.client_msg_id);
            if(format_msg_result != SERVER){
                continue;
            }
            
            udp_send_msg(&msg_packet, user, res);

            gettimeofday(&start_time, NULL);
            timer_running = 1;

            continue;

        } else if(polled_fds[1].revents & POLLIN) {

            memset(buffer, 0, buffer_size);

            udp_receive_msg(buffer, buffer_size, user, res, &msg_id_storage);

            msgPacket msg_packet;

            if (user->client_confirm_request == 1){
                udp_create_confirm(msg_id_storage.serv_msg_id, &msg_packet, user);
                udp_send_msg(&msg_packet, user, res);
            }
            
            
        } else if(polled_fds[1].revents & (POLLERR | POLLHUP)) {
            udp_close_connection(user, "Poll error", 1);
           
        }

        if(user->serv_confirm_request == 0 && timer_running == 1){
            timer_running = 0;
            retry_num = 0;
        }

        if(user->serv_confirm_request == 1 && timer_running == 1){
            gettimeofday(&current_time, NULL);

            long elapsed_time_ms = (current_time.tv_sec - start_time.tv_sec) * 1000 +
                       (current_time.tv_usec - start_time.tv_usec) / 1000 +
                       ((current_time.tv_usec - start_time.tv_usec) % 1000 >= 500 ? 1 : 0);
        
            if (elapsed_time_ms > TIMEOUT){
                printf("Timeout\n");
                retry_num++;
                if (retry_num == RETRY_COUNT){
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