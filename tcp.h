#ifndef TCP_H
#define TCP_H

#include "client.h"

/* Forward declaration so that there are no errors in function declarations below*/
typedef struct userInfo userInfo;

/**
 * The function applies the regular expression pattern to the token string. If the token matches the pattern and does not exceed the maximum length, the function does nothing. 
 * If the token does not match the pattern or exceeds the maximum length, the function call function close_connection() with the error message.
 * 
 * @param token: A pointer to the token string to be checked against the regular expression and size.
 * @param pattern: A pointer to the string containing the regular expression pattern.
 * @param max_length: The maximum length of the token string.
 * @param error_msg: A pointer to the string where any error message will be stored (for calling close_connection() with that argument).
 * @param user: A pointer to a userInfo structure containing user information.
 * 
 */
void tcp_regular_exp_check_parsing(char* token, char* pattern, int max_length, char* error_msg, userInfo* user);

/**
 * The function applies the regular expression pattern to the token string. If the token matches the pattern and does not exceed the maximum length, the function does nothing. 
 * If the token does not match the pattern or exceeds the maximum length, the function prints error message to stdin and returns 1
 * 
 * @param token: A pointer to the token string to be checked against the regular expression and size.
 * @param pattern: A pointer to the string containing the regular expression pattern.
 * @param max_length: The maximum length of the token string.
 * @param error_msg: A pointer to the string where any error message will be stored (to print in error case).
 * 
 */
int tcp_regular_exp_check_creating(char* token, char* pattern, int max_length, char* error_msg);

/**
 * The function processes an incoming message ERR from the server. It checks the message format and if everything's fine, it prints it.
 * 
 * @param token: A pointer to the token string to be checked against the regular expression.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * 
 */
void tcp_parse_err(char* token, userInfo* user);

/**
 * The function processes an incoming message REPLY from the server. It checks the message format and if everything's fine, it prints it.
 * 
 * @param token: A pointer to the token string to be checked against the regular expression.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * 
 */
void tcp_parse_reply(char* token, userInfo* user);

/**
 * The function processes an incoming common chat message from the server. It checks the message format and if everything's fine, it prints it.
 * 
 * @param token: A pointer to the token string to be checked against the regular expression.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * 
 */
void tcp_parse_common_msg(char* token, userInfo* user);

/**
 * The function reads a message from the TCP connection into the buffer. The size of the message should not exceed buffer_size.
 * Tрe function starts divide message into tokens and based on 1 token choose the right function to process the message.
 * 
 * @param buffer: A pointer to the buffer where the received message will be stored.
 * @param buffer_size: The size of the buffer.
 * @param user: A pointer to a userInfo structure containing user information.
 * 
 */
void tcp_receive_msg(char* buffer, size_t buffer_size, userInfo* user);

/**
 * The function uses the input string and the user information to create a TCP chat message. The created message is stored in the msg buffer. 
 * If the content int input does not pass the checks, function returns 1. if everything is fine, function returns 0.
 * 
 * @param input: A pointer to the input string that contain user's command.
 * @param user: A pointer to a userInfo structure containing user information.
 * @param msg: A pointer to the buffer where the created message will be stored.
 * @param msg_size: The size of the msg buffer.
 * 
 */
int tcp_create_common_message(char* input, userInfo* user, char* msg, int msg_size);
/**
 * The function takes new diaplay name form user's command, check it and change it in user's structure.
 * If the content int input does not pass the checks, function returns 1. if everything is fine, function returns 0.
 * 
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * 
*/
int tcp_local_rename(userInfo* user);
/**
 * The function sepparate the user command string and collect contant which contain new channel name. 
 * If the content int input does not pass the checks, function returns 1. if everything is fine, function returns 0.
 * 
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param msg: A pointer to the buffer where the created message will be stored.
 * @param msg_size: The size of the msg buffer.
 * 
*/
int tcp_create_join_msg(userInfo* user,char* msg, int msg_size);
/**
 * The function sepparate the user command string and collect contant which contain username, secret code and dipaly name to authenticate user. 
 * If the content int input does not pass the checks, function returns 1. if everything is fine, function returns 0.
 * 
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param msg: A pointer to the buffer where the created message will be stored.
 * @param msg_size: The size of the msg buffer.
 * 
*/
int tcp_create_auth_msg(userInfo* user, char* msg, int msg_size);
/**
 * The function begin separate user's command string and decides which function should be called to process the message.
 * If the user's command does not need to be sent to the server it returns NOT_TO_SEND, else TO_SEND.
 * 
 * @param buffer: A pointer to the buffer where the message to send will be stored.
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * 
*/
int tcp_format_input(char* buffer, userInfo* user);
/**
 * The function uses the user information and the address information to establish a TCP connection. 
 * The function returns nothing because end of the connection is handled by the close_connection() function in different situations
 * 
 * @param user: A pointer to the userInfo structure containing some information about actual connection state.
 * @param res: A pointer to a struct addrinfo that contains information about the address to which the connection will be established.
 * 
 */
void tcp_connection(userInfo* user, struct addrinfo *res);

#endif