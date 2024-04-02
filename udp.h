#ifndef UDP_H
#define UDP_H
#include "client.h"

/* Forward declaration so that there are no errors in function declarations below */
typedef struct userInfo userInfo;

/* In this structure, the client will upload the content to be sent out */
typedef struct msgPacket
{
    char *msg;
    int msgSize;
} msgPacket;

/* Struct which store ids that have been confirmed by the client, actual client id and actual server id which will be confirmed by client*/
typedef struct msgIdStorage
{
    uint16_t client_msg_id;
    uint16_t serv_msg_id;
    size_t confirmed_ids_size;
    int confirmed_ids_count;
    uint16_t *confirmed_ids;
} msgIdStorage;

/**
 * The function call universal function after creating and sending BYE message and free memmory allocated for msg packet 
 * and confirmed ids (only available in UDP)
 * 
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param error: A pointer to the string containing the error message based on where programm called close connection (may be "").
 * @param is_error: A flag indicating whether the function was called due to an error (1) or not (0).
 * 
*/
void udp_close_connection(userInfo* user, char* error, int is_error);

/**
 * The function applies the regular expression pattern to the token string. If the token matches the pattern and does not exceed the maximum length, the function does nothing. 
 * If the token does not match the pattern or exceeds the maximum length, the function call function close_connection() with the error message.
 * 
 * @param field: A pointer to the content field to be checked against the regular expression and size.
 * @param pattern: A pointer to the string containing the regular expression pattern.
 * @param max_length: The maximum length of the token string.
 * @param error_msg: A pointer to the string where any error message will be stored (for calling udp_close_connection() with that argument).
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * 
 */
void udp_regular_exp_check_parsing(char* field, char* pattern, int max_length, char* error_msg, userInfo* user);

/**
 * The function applies the regular expression pattern to the token string. If the token matches the pattern and does not exceed the maximum length, the function does nothing. 
 * If the token does not match the pattern or exceeds the maximum length, the function prints error message to stdin and returns 1.
 * 
 * @param token: A pointer to the token string to be checked against the regular expression and size.
 * @param pattern: A pointer to the string containing the regular expression pattern.
 * @param max_length: The maximum length of the token string.
 * @param upcoming_msg: A pointer to the string where any semi finished msgPacket->msg will be stored.
 * @param error_msg: A pointer to the string where any error message will be stored (to print in error case).
 * 
 */
int udp_regular_exp_check_creating(char* field, char* pattern, int max_length, char* upcoming_msg, char* error_msg);

/**
 * The function is used only to call udp_close_connection() because signal(SIGINT, udp_interrupt_connection) require function to have int sig argument, 
 * but this arh is not important in our case.
 * 
 * @param sig: An integer representing the signal number.
*/
void udp_interrupt_connection(int sig);

/**
 * The function which help to store message paramets in msgPacket structure., increment magPacket->msgSize and in some cases rellocate it.
 * 
 * @param msg_packet: A pointer to the msgPacket structure where the message parameters will be stored.
 * @param field: A pointer to the content field (parameter) to be stored in msgPacket.
 * 
*/
void add_field_msg(msgPacket* msg_packet, char* field);

/**
 * The function which called in cases when we want to close connection and end client execution. It fiils msgPacket with data for BYE message,
 * 
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param msg_packet: A pointer to the msgPacket structure where the message parameters will be stored.
 * @param msg_id: An integer representing the message id on the client side.
 * 
*/
void udp_create_bye_msg(userInfo* user, msgPacket* msg_packet, uint16_t msg_id);

/**
 *  The function uses the input string and the user information to create a UDP chat message packet.  
 *  If the content in buffer (got from user) does not pass the checks, function returns 1. if everything is fine, function returns 0.
 * 
 * @param buffer: A pointer to the buffer where the message from stdin is stored.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param msg_packet: A pointer to the msgPacket structure where the message parameters will be stored.
 * @param msg_id: An integer representing the message id on the client side.
 * 
*/
int udp_create_common_msg(char* buffer, userInfo* user, msgPacket* msg_packet, uint16_t msg_id);

/**
 * The function takes new diaplay name form user's command, check it and change it in user's structure.
 * If the content int input does not pass the checks, function returns 1. if everything is fine, function returns 0.
 * 
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * 
*/
int udp_local_rename(userInfo* user);

/**
 * The function sepparate the user command string and collect content which contain new channel name into msgPacket->msg. 
 * If the content int input does not pass the checks, function returns 1. if everything is fine, function returns 0.
 * 
 * @param buffer: A pointer to the buffer where the command from stdin is stored.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param msg: A pointer to the buffer where the created message will be stored.
 * @param msg_size: The size of the msg buffer.
 * 
*/
int udp_create_join_msg(char* buffer, userInfo* user, msgPacket* msg_packet, uint16_t msg_id);

/**
 * The function sepparate the user command string and collect content which contain username, secret code and dipaly name to authenticate user into msgPakcer->msg. 
 * If the content int input does not pass the checks, function returns 1. if everything is fine, function returns 0.
 * 
 * @param buffer: A pointer to the buffer where the command from stdin is stored.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param msg: A pointer to the buffer where the created message will be stored.
 * @param msg_size: The size of the msg buffer.
 * 
*/
int udp_create_auth_msg(char* buffer, userInfo* user, msgPacket* msg_packet, uint16_t msg_id);

/**
 * The function creates feature of the UDP conenction CONFIRM where we accept that client messagewas recieved by message which  contain number of client message.
 * 
 * @param msg_id: An integer representing the last message id on the client side.
 * @param msg_packet: A pointer to the msgPacket structure where the message parameters will be stored.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * 
*/
void udp_create_confirm(uint16_t msg_id, msgPacket* msg_packet, userInfo* user);

/**
 * The function that sends a msgPacket to the server by sendto() function,
 * 
 * @param msg_packet: A pointer to the msgPacket structure where the message parameters is stored.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param res: A pointer to the address information structure.
 * 
*/
void udp_send_msg(msgPacket* msg_packet, userInfo* user, struct addrinfo *res);

/**
 * The function begin separate user's command string and decides which function should be called to process the message.
 * If the user's command does not need to be sent to the server it returns NOT_TO_SEND, else TO_SEND. Function store formatted message in user->prev_msg_packet.
 * 
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param msg_packet: A pointer to the msgPacket structure where the message parameters will be stored.
 * @param buffer: A pointer to the buffer where the message to send will be stored.
 * @param client_msg_id: An integer representing the message id on the client side.
 * 
*/
int udp_format_input(userInfo* user, msgPacket* msg_packet, char* buffer, uint16_t client_msg_id);

/**
 * The function checks if the message id was previously confirmed by the client. If not, it returns 0. If yes, it returns 1.
 * 
 * @param msg_id_storage: A pointer to the msgIdStorage structure where the messages ids are stored.
 * 
*/
int was_id_previously(msgIdStorage* msg_id_storage);

/**
 * The function call was_id_previously() and if it returns 0, it stores the message id in the msgIdStorage structure and returns 0. If it returns 1, it returns 1.
 * If there is not enought space in confirmed_ids array, function reallocates it and increases confirmed_ids_size by 10.
 * 
 * @param msg_id_storage: A pointer to the msgIdStorage structure where the messages ids are stored.
 * 
*/
int id_check(msgIdStorage* msg_id_storage);

/**
 * The function processes an incoming message REPLY from the server. It checks the message format and if everything's fine, it prints it in right format.
 * 
 * @param buffer: A pointer to the buffer where the message from the server is stored.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param byterecv: An integer representing the number of bytes received.`
 * @param msg_id_storage: A pointer to the msgIdStorage structure where the messages ids are stored.
 * 
 */
void udp_parse_reply(char* buffer, userInfo* user, int byterecv, msgIdStorage* msg_id_storage);
/**
 * The function processes an incoming message CONFIRM from the server. It checks the message format and if everything's fine, it  NOT prints it.
 * 
 * @param buffer: A pointer to the buffer where the message from the server is stored.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param msg_id_storage: A pointer to the msgIdStorage structure where the messages ids are stored.
 * 
 */
void udp_parse_confirm(char* buffer, userInfo* user, msgIdStorage* msg_id_storage);
/**
 * The function processes an incoming message MSG from the server. It checks the message format and if everything's fine, it prints it in right format.
 * 
 * @param buffer: A pointer to the buffer where the message from the server is stored.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param byterecv: An integer representing the number of bytes received.
 * @param msg_id_storage: A pointer to the msgIdStorage structure where the messages ids are stored.
 * 
 */
void udp_parse_msg(char* buffer, userInfo* user, int byterecv, msgIdStorage* msg_id_storage);
/**
 * The function processes an incoming message BYE from the server. It checks the message format and if everything's fine, it NOT prints it.
 * 
 * @param buffer: A pointer to the buffer where the message from the server is stored.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param byterecv: An integer representing the number of bytes received.
 * @param msg_id_storage: A pointer to the msgIdStorage structure where the messages ids are stored.
 * 
*/
void udp_parse_bye(char* buffer, userInfo* user, int byterecv, msgIdStorage* msg_id_storage);
/**
 * The function processes an incoming message ERROR from the server. It checks the message format and if everything's fine, it prints it in right format.
 * 
 * @param buffer: A pointer to the buffer where the message from the server is stored.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.  
 * @param byterecv: An integer representing the number of bytes received.
 * @param msg_id_storage: A pointer to the msgIdStorage structure where the messages ids are stored.
 * 
*/
void udp_parse_error(char* buffer, userInfo* user, int byterecv, msgIdStorage* msg_id_storage);

/**
 * 
 * The function reads a message from the UDP connection into the buffer. The size of the message should not exceed buffer_size.
 * The function starts divide message into tokens and based on 1 token choose the right function to process the message.
 * 
 * @param buffer: A pointer to the buffer where the received message will be stored.
 * @param buffer_size: The size of the buffer.
 * @param user: A pointer to a userInfo structure containing user information.
 * @param res: A pointer to the address information structure.
 * @param msg_id_storage: A pointer to the msgIdStorage structure where the messages ids are stored.
 * 
 */
void udp_receive_msg(char* buffer, userInfo* user, struct addrinfo* res, msgIdStorage* msg_id_storage);

/**
 * The function NOT creates a UDP connection with the server. It wait some events from user or from server and then call 
 * right function that will communicate with the server
 * 
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param res: A pointer to the address information structure.
 * 
 */
int udp_connection(userInfo* user, struct addrinfo *res);


#endif