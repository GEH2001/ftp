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
    case MKD: cmd_mkd(cmd, st); break;
    case RMD: cmd_rmd(cmd, st); break;
    case RNFR: cmd_rnfr(cmd, st); break;
    case RNTO: cmd_rnto(cmd, st); break;
    default:
        sprintf(st->message, "?Invalid command.");
        write_state(st);
        break;
    }
    st->last_verb = cmd_to_id(cmd->code);
}

void cmd_user(command *cmd, state *st) {
    if(!st->user_ok) {
        if(strncmp(cmd->arg, "anonymous", sizeof cmd->arg) == 0) {
            st->user_ok = 1;
            sprintf(st->message, "331 Guest ok, send your complete e-mail address as password using PASS.");
        } else {
            sprintf(st->message, "530 This FTP server is anonymous only.");
        }
    } else {
        if(st->is_login) {
            sprintf(st->message, "230 Already logged in.");
        } else {
            sprintf(st->message, "331 Send your e-mail as password using PASS.");
        }
    }
    write_state(st);
}


void cmd_pass(command *cmd, state *st) {
    if(st->user_ok) {
        if(st->is_login) {
            sprintf(st->message, "203 Do not login repeatedly.");
        } else {
            st->is_login = 1;
            sprintf(st->message, "230 Login Successful.");
        }
    } else {
        sprintf(st->message, "503 You should use USER command before PASS.");
    }
    write_state(st);
}


void cmd_pasv(command *cmd, state *st) {
    if(st->is_login) {
        if(strlen(cmd->arg) != 0) {
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

void cmd_mkd(command *cmd, state *st) {
    if(st->is_login) {
        if(mkdir(cmd->arg, 0777) == 0) {
            char *path = realpath(cmd->arg, NULL);
            if(path != NULL) {
                sprintf(st->message, "257 Successfully created: \"%s\"", path);
            } else {
                sprintf(st->message, "250 Successfully created.");
            }
        } else {
            sprintf(st->message, "550 Failed to create directory.");
        }
    } else {
        sprintf(st->message, "530 Permission denied. First login with USER and PASS.");
    }
    write_state(st);
}

void cmd_rmd(command *cmd, state *st) {
    if(st->is_login) {
        if(rmdir(cmd->arg) == 0) {
            sprintf(st->message, "250 Successfully removed.");
        } else {
            sprintf(st->message, "550 Failed to remove the directory.");
        }
    } else {
        sprintf(st->message, "530 Permission denied. First login with USER and PASS.");
    }
    write_state(st); 
}

void cmd_rnfr(command *cmd, state *st) {
    if(st->is_login) {
        // TODO: Unsafe operation
        if(is_file_visiable(cmd->arg)) {
            memset(st->rn_from, 0, sizeof st->rn_from); // clear the buf
            sprintf(st->rn_from, "%s", cmd->arg);
            sprintf(st->message, "350 File to be renamed.");
        } else {
            sprintf(st->message, "550 File not exist.");
        }
    } else {
        sprintf(st->message, "530 Permission denied. First login with USER and PASS.");
    }
    write_state(st); 
}

void cmd_rnto(command *cmd, state *st) {
    if(st->is_login) {
        if(st->last_verb == RNFR) { // Only when the last command is FNFR will this condition satisfiy.
            if(access(cmd->arg, F_OK) != -1) {
                sprintf(st->message, "550 New pathname already exists.");
            } else {
                if(rename(st->rn_from, cmd->arg) == 0) {
                    sprintf(st->message, "250 Rename successfully.");
                } else {
                    sprintf(st->message, "553 Failed to rename.");
                }
            }
        } else {
            sprintf(st->message, "503 RNTO must come immediately after RNFR.");
        }
    } else {
        sprintf(st->message, "530 Permission denied. First login with USER and PASS.");
    }
    write_state(st); 
}