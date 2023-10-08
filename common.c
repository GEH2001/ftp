#include "common.h"

void write_state(state *st) {
    write(st->sock_control, st->message, strlen(st->message));
}

int cmd_to_id(char *cmd) {
    int total = sizeof(cmdlist_str) / sizeof(cmdlist_str[0]);
    for(int i = 0; i < total; i++) {
        if(strncmp(cmd, cmdlist_str[i], sizeof(cmd)) == 0) {
            return i;
        }
    }
    return -1;
}

void parse_command(char *cmdstr, command *cmd) {
    char *token = strtok(cmdstr, " ");  // split by " "
    if(token) {
        strncpy(cmd->code, token, sizeof(cmd->code));
        cmd->code[sizeof(cmd->code) - 1] = '\0';
    }
    token = strtok(NULL, " ");
    if(token) {
        strncpy(cmd->arg, token, sizeof(cmd->arg));
        cmd->arg[sizeof(cmd->arg) - 1] = '\0';
    }

    // sscanf(cmdstr, "%s %s", cmd->code, cmd->arg); 
    /*
    If use sscanf, here is a bug that code not end with '\0'.
    For example, cmdstr = "usrnae hello".
    Because the size of code is 6, and code is just "usrnae" but not end with '\0'.
    So when you print code, you will get "usrnaehello" because it won't stop until '\0'.
    */
}


void response(command *cmd, state *st) {
    switch (cmd_to_id(cmd->code))
    {
    case USER: cmd_user(cmd, st); break;
    
    default:
        st->message = "?Invalid command.\n";
        write_state(st);
        break;
    }
}

/*Handle USER*/
void cmd_user(command *cmd, state *st) {
    if(strncmp(cmd->arg, "anonymous", sizeof(cmd->arg)) == 0) {
        st->user_ok = 1;
        st->message = "331 Guest login ok, send your complete e-mail address as password.\n";
    } else {
        st->message = "530 This FTP server is anonymous only.\n";
    }
    write_state(st);
}

/*Handle PASS*/
void cmd_pass(command *cmd, state *st) {
    if(st->user_ok) {
        if(st->is_login) {
            st->message = "203 Do not login repeatedly.\n";
        } else {
            st->is_login = 1;
            st->message = "230 Login Successful.\n";
        }
    } else {
        st->message = "503 You should use USER command before PASS.\n";
    }
    write_state(st);
}