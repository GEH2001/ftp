#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>


typedef struct state
{
    int sock_control;     // the control socket for the server
    int sock_data;        // the data socket for the server
} state;

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
    char code[6];  // the size must > 5, see parse_command
    char arg[256];
} command;

/* parse "USER anonymouse" to a command struct */
void parse_command(char *, command *);



/*commands handle functions*/
void response(command *, state *);
void user(command *, state *);
void pass(command *, state *);

