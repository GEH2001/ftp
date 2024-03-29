/**
 * Basic variables to be used.
*/
#pragma once

#define BSIZE 8192  // buffer size

/* the state of the connection between client and server */
typedef struct state
{
    int sock_control;       // the control socket for the server
    int sock_data;          // the data socket for the server
    char message[BSIZE];    // response message to client
    int user_ok;            // is user allowed? (only anonymous)
    int is_login;           // is user logged in?
    int mode;               // default standard(port)
    int pt_addr[6];         // PORT address(h1,h2,h3,h4,p1,p2)
    int sock_pasv;          // created by PASV, listening for data connection
    char rn_from[256];      // for RNFR, rename from
    int last_verb;          // for RNTO, the verb of the last command, USER PASS RNFR RNTO etc.
    int rest_pos;           // for REST, restart position for RETR and STOR
    char fpath[256];         // for thread RETR and STOR, file path
} state;


/**
 * PASSIVE: Passive mode,   server(20) -> client(>1024)
 * STANDARD: Default mode,  server(>1024) <- client(>1024) 
*/
enum mode {
    STANDARD, PASSIVE
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
    PASV, MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, ABOR, REST
} cmdlist;

static const char *cmdlist_str[] = {
    "USER", "PASS", "RETR", "STOR", "QUIT", "SYST", "TYPE", "PORT", 
    "PASV", "MKD", "CWD", "PWD", "LIST", "RMD", "RNFR", "RNTO", "ABOR", "REST"
};