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

/* write current state to client */
void write_state(state *);

/* parse "USER anonymouse" to a command struct */
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


