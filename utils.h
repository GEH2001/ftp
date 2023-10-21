/**
 * Utility file
*/
#pragma once

#include "basic.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <fcntl.h> // open
#include <sys/stat.h> // struct stat

#include <pthread.h>

/* write current state to client */
void write_state(state *);

/**
 * Parse "USER anonymouse" to a command struct
 * Remove \r\n before the function
*/
void parse_command(char *, command *);

/* cmd to id, "USER" -> USER, unknown -> -1 */
int cmd_to_id(char *);

/* Create a socket to listen [port], and return the socket (-1 for error) */
int socket_listen(int port);

/* Return a random port for PASV mode */
int gen_port();

/* Return 1 if port is available, else 0 */
int port_available(int port);

/**
 * Get the IP address associated with the given socket
 * 
 * @param sockfd The socket file descriptor.
 * @param ip A buffer to store IP (length must be 4 or over).
 * @return -1 if failed, else 0
*/
int get_ip(int sockfd, int *ip);

/**
 * Using popen() to Obtain a List of Files from the ls Utility.
 * Write through the data socket.
*/
int write_list_files(int sock_data, const char *path);

/* For LIST: Return 1 if the file or directory visible, else 0 */
int is_file_visiable(const char *path);

/** Send a file to the client over data connection
 *  @param rest_pos restart position from REST
 */
int send_file(int sock_data, const char *path, int rest_pos);

/**
 * Thread function to send file
*/
void *send_file_thread(void* _st);

/** Create data connection for PORT & PASV
 *  Return -1 for errors, otherwise 0.
 *  Set st->sock_data to the data socket built by PORT or PASV.
 */
int create_data_conn(state *st);

/* Safely close the socket in case the standard stream is closed */
void close_safely(int sock_fd);

/** Read contents from sock_data and write it into path.
 *  Return -1 for errors, otherwise 0
 *  @param rest_pos restart position from REST
 */
int recv_file(int sock_data, const char *path, int rest_pos);

/* Thread function to recv file */
void* recv_file_thread(void* _st);