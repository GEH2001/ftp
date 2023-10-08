#include "common.h"

void write_state(state *st) {
    // TODO: add \r\n
    if(write(st->sock_control, st->message, strlen(st->message)) == -1) {
        printf("Error write(): %s(%d)\n", strerror(errno), errno);
    }
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
    // TODO: remove \r\n
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

/* TODO: use thread to process connected socket */
void sock_process(int connfd) {
    command cmd;
    state st;
    char buf[BSIZE];
    memset(&cmd, 0, sizeof(cmd));
    memset(&st, 0, sizeof(st));
    memset(buf, 0, sizeof(buf));
    st.sock_control = connfd;

    // welcome
    st.message = "220 Anonymous FTP server ready.\n";
    write_state(&st);

    // wait for command, read can block
    while(read(connfd, buf, BSIZE-1)) {
        parse_command(buf, &cmd);
        cmd_response(&cmd, &st);
        memset(buf, 0, sizeof(buf)); // clear buffer
    }
    // TODO: handle EOF & -1

    printf("client closed the connection.\n");

    close(connfd);

}

void cmd_response(command *cmd, state *st) {
    switch (cmd_to_id(cmd->code))
    {
    case USER: cmd_user(cmd, st); break;
    case PASS: cmd_pass(cmd, st); break;
    default:
        st->message = "?Invalid command.\n";
        write_state(st);
        break;
    }
}

/*Handle USER*/
void cmd_user(command *cmd, state *st) {
    if(!st->user_ok) {
        if(strncmp(cmd->arg, "anonymous", strlen("anonymous")) == 0) {  // TODO: fix strlen, use sizeof(arg)
            st->user_ok = 1;
            st->message = "331 Guest ok, send your complete e-mail address as password using PASS.\n";
        } else {
            st->message = "530 This FTP server is anonymous only.\n";
        }
    } else {
        if(st->is_login) {
            st->message = "230 Already logged in.\n";
        } else {
            st->message = "331 Send your e-mail as password using PASS.\n";
        }
    }
    write_state(st);
}


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