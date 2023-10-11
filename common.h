#pragma once
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>

#define BSIZE 8192  // buffer size

/* the state of the connection between client and server */
typedef struct state
{
    int sock_control;     // the control socket for the server
    int sock_data;        // the data socket for the server
    int sock_pasv;        // pasv data socket
    // char *message;        // response message to client
    char message[BSIZE];
    int user_ok;          // is user allowed? (only anonymous)
    int is_login;         // is user logged in?
} state;
// TODO: socket should not be inited with 0


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
/* write current state to client */
void write_state(state *);

typedef enum cmdlist {
    USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, 
    PASV, MKD, CWD, PWD, LIST, RMD, RNFR, RNTO
} cmdlist;

static const char *cmdlist_str[] = {
    "USER", "PASS", "RETR", "STOR", "QUIT", "SYST", "TYPE", "PORT", 
    "PASV", "MKD", "CWD", "PWD", "LIST", "RMD", "RNFR", "RNTO"
};

/* cmd to id, "USER" -> USER, unknown -> -1 */
int cmd_to_id(char *);

/* commands from clients, such as: USER anonymous */
typedef struct command {
    // verb
    char code[6];  // the size must > 5, see parse_command
    char arg[256];
} command;

/* parse "USER anonymouse" to a command struct */
void parse_command(char *, command *);


void sock_process(int);
/* Create a socket to listen [port], and return the socket (-1 for error) */
int socket_listen(int port);
/*commands handle functions*/
void cmd_response(command *, state *);
void cmd_user(command *, state *);
void cmd_pass(command *, state *);
void cmd_port(command *, state *); // TODO
void cmd_pasv(command *, state *);
void cmd_cwd(command *, state *);
void cmd_pwd(command *, state *);
void cmd_list(command *, state *);