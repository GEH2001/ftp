/**
 * Basic variables to be used.
*/
#pragma once

#define BSIZE 8192  // buffer size

/* the state of the connection between client and server */
typedef struct state
{
    int sock_control;     // the control socket for the server
    int sock_data;        // the data socket for the server
    int sock_pasv;        // pasv data socket just for listening
    char message[BSIZE];  // response message to client
    int user_ok;          // is user allowed? (only anonymous)
    int is_login;         // is user logged in?
    int mode;             // NORMAL, PASSIVE
    char rn_from[256];     // for RNFR, rename from
    int last_verb;        // The verb of the last command, USER PASS RNFR RNTO etc.
} state;
// TODO: socket should not be inited with 0

/**
 * NORMAL: only one socket for control
 * PASSIVE: PASV create another socket for data transfer
*/
enum mode {
    NORMAL, PASSIVE
};

/* commands from clients, such as: USER anonymous */
typedef struct command {
    // verb
    char code[6];  // the size must > 5, see parse_command
    char arg[256];
} command;


/* Used by cmd_to_id */
typedef enum cmdlist {
    USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, 
    PASV, MKD, CWD, PWD, LIST, RMD, RNFR, RNTO
} cmdlist;

static const char *cmdlist_str[] = {
    "USER", "PASS", "RETR", "STOR", "QUIT", "SYST", "TYPE", "PORT", 
    "PASV", "MKD", "CWD", "PWD", "LIST", "RMD", "RNFR", "RNTO"
};