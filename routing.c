#include "routing.h"

void cmd_response(command *cmd, state *st) {
    switch (cmd_to_id(cmd->code))
    {
    case USER: cmd_user(cmd, st); break;
    case PASS: cmd_pass(cmd, st); break;
    case PASV: cmd_pasv(cmd, st); break;
    case CWD: cmd_cwd(cmd, st); break;
    case PWD: cmd_pwd(cmd, st); break;
    case LIST: cmd_list(cmd, st); break;
    default:
        // st->message = "?Invalid command.\n";
        sprintf(st->message, "?Invalid command.");
        write_state(st);
        break;
    }
}

void cmd_user(command *cmd, state *st) {
    if(!st->user_ok) {
        if(strncmp(cmd->arg, "anonymous", sizeof cmd->arg) == 0) {
            st->user_ok = 1;
            // st->message = "331 Guest ok, send your complete e-mail address as password using PASS.\n";
            sprintf(st->message, "331 Guest ok, send your complete e-mail address as password using PASS.");
        } else {
            // st->message = "530 This FTP server is anonymous only.\n";
            sprintf(st->message, "530 This FTP server is anonymous only.");
        }
    } else {
        if(st->is_login) {
            // st->message = "230 Already logged in.\n";
            sprintf(st->message, "230 Already logged in.");
        } else {
            // st->message = "331 Send your e-mail as password using PASS.\n";
            sprintf(st->message, "331 Send your e-mail as password using PASS.");
        }
    }
    write_state(st);
}


void cmd_pass(command *cmd, state *st) {
    if(st->user_ok) {
        if(st->is_login) {
            // st->message = "203 Do not login repeatedly.\n";
            sprintf(st->message, "203 Do not login repeatedly.");
        } else {
            st->is_login = 1;
            // st->message = "230 Login Successful.\n";
            sprintf(st->message, "230 Login Successful.");
        }
    } else {
        // st->message = "503 You should use USER command before PASS.\n";
        sprintf(st->message, "503 You should use USER command before PASS.");
    }
    write_state(st);
}


void cmd_pasv(command *cmd, state *st) {
    if(st->is_login) {
        if(strlen(cmd->arg) != 0) {
            // st->message = "504 PASV parameters are prohibited.\n";
            sprintf(st->message, "504 PASV parameters are prohibited. %s", cmd->arg);
        } else {
            // close previous passive socket
            // TODO: previous port socket ?
            close(st->sock_pasv);
            int port = gen_port();
            int p1 = port / 256;
            int p2 = port % 256;
            int ip[4] = {0, 0, 0, 0};
            get_ip(st->sock_control, ip); // TODO: return -1
            int sockfd = socket_listen(port);   // TODO: return -1
            st->sock_pasv = sockfd;
            st->mode = PASSIVE;
            sprintf(st->message, "227 =%d,%d,%d,%d,%d,%d", ip[0], ip[1], ip[2], ip[3], p1, p2);
        }
    } else {
        // st->message = "530 Permission denied. First login with USER and PASS.\n";
        sprintf(st->message, "530 Permission denied. First login with USER and PASS.");
    }
    write_state(st);
}


void cmd_cwd(command *cmd, state *st) {
    if(st->is_login) {
        if(chdir(cmd->arg) == 0) {
            sprintf(st->message, "250 Directory successfully changed.");
        } else {
            sprintf(st->message, "550 Failed to change directory.");
        }
    } else {
        sprintf(st->message, "530 Permission denied. First login with USER and PASS.");
    }
    write_state(st);
}

void cmd_pwd(command *cmd, state *st) {
    if(st->is_login) {
        char buf[264];
        memset(buf, 0, sizeof(buf));
        if(getcwd(buf, 256) == NULL) {
            perror("Error getcwd()");
            sprintf(st->message, "Server error"); // TODO: status code
        } else {
            sprintf(st->message, "257 \"%s\"", buf);
        }
    } else {
        sprintf(st->message, "530 Permission denied. First login with USER and PASS.");        
    }
    write_state(st);
}


void cmd_list(command* cmd, state *st) {
    if(st->is_login) {
        if(st->mode == PASSIVE) {
            char buf[BSIZE];
            int connfd;
            if((connfd = accept(st->sock_pasv, NULL, NULL)) == -1) {
                perror("Error accept()");
                sprintf(st->message, "Server error");   // TODO: Server error
                write_state(st);
                return;
            }
            close(st->sock_pasv); // stop listening for new connection
            sprintf(st->message, "150 Here comes the directory listing."); // mask
            write_state(st);
            if(write_list_files(connfd, cmd->arg) == -1) {
                sprintf(st->message, "451 Server error reading the directory.");
                write_state(st);
                return;
            } // TODO: 426 error
            sprintf(st->message, "226 Directory send OK.");
            close(connfd);
            st->mode = NORMAL;
        } else {
            sprintf(st->message, "425 Use PASV or PORT to establish a data connection.");
        }
    } else {
        sprintf(st->message, "530 Permission denied. First login with USER and PASS.");
    }
    write_state(st);
}