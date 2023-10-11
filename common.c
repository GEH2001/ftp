#include "common.h"
#include "utils.h"

void write_state(state *st) {
    st->message[BSIZE - 4] = '\0'; // There must be space for "\r\n"
    strncat(st->message, "\r\n", 3);
    if(write(st->sock_control, st->message, strlen(st->message)) == -1) {
        printf("Error write(): %s(%d)\n", strerror(errno), errno);
    }
    // memset(st->message, 0, BSIZE);  // clear the reponse message buffer
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


int gen_port() {
    // TODO: check if the port is used or not
    srand(time(NULL));
    
    // between 20000 and 65535
    int lower_bound = 20000;
    int upper_bound = 65535;
    int port;
    do {
        port = rand() % (upper_bound - lower_bound + 1) + lower_bound;
    } while(!port_available(port));
    return port;
}

int port_available(int port) {
    int sockfd;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        perror("Error socket() in port_available()\n");
        return 0;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    // Try binding to the port
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Error bind() in port_available()\n");
        close(sockfd);
        return 0;
    }

    close(sockfd);
    return 1;  // Port is available
}

int get_ip(int sockfd, int *ip) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(addr);
    if(getsockname(sockfd, (struct sockaddr*)&addr, &addr_size) == -1) {
        perror("Error: getsockname()\n");
        return -1;  // failed
    }
    char buf[20];
    memset(buf, 0, 20);
    inet_ntop(AF_INET, &addr.sin_addr, buf, 19);
    // inet_ntoa
    sscanf(buf, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
    return 0;
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
    memset(&st, 0, sizeof(st));  // TODO: st.sock should not be 0, which is stdin
    memset(buf, 0, sizeof(buf));
    st.sock_control = connfd;

    // welcome
    // st.message = "220 Anonymous FTP server ready.\n";
    sprintf(st.message, "220 Anonymous FTP server ready.");
    write_state(&st);

    // wait for command, read can block
    while(read(connfd, buf, BSIZE-1)) {
        printf("%s", buf);
        int len = strlen(buf);
        if(len > 1) {   // remove \r\n
            buf[len-2] = '\0';
            buf[len-1] = '\0';
        }
        parse_command(buf, &cmd);
        cmd_response(&cmd, &st);
        memset(buf, 0, sizeof(buf)); // clear buffer
        memset(&cmd, 0, sizeof(cmd)); // clear cmd
        // Note: Do not clear state(st)
    }
    // TODO: handle EOF & -1

    printf("client closed the connection.\n");

    close(connfd);

}

int socket_listen(int port) {
	int sock_listen;
	struct sockaddr_in addr;

	// init a socket
	if((sock_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	// ip & port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);	// ip 0.0.0.0
	// bind
	if (bind(sock_listen, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	// listen
	if (listen(sock_listen, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
    return sock_listen;
}


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

/*Handle USER*/
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
            // TODO: st.mode
            st->sock_pasv = sockfd;
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

// TODO: use data-sock
void cmd_list(command* cmd, state *st) {
    if(st->is_login) {
        char buf[BSIZE];
        int connfd;
        if((connfd = accept(st->sock_pasv, NULL, NULL)) == -1) {
            perror("Error accept()");
        }
        sprintf(st->message, "150 Here comes the directory listing.");
        write_state(st);
        if(list_files(buf, BSIZE, cmd->arg) == -1) {
            sprintf(st->message, "Server error"); // TODO: status code
        } else {

            // write(connfd, buf, BSIZE);
            // char ansd[100] = "-rw-r--r-- 1 geh geh   177 Oct 11 22:33 utils.h\r\n";
            // write(connfd, ansd, 100);
            // char ansda[100] = "-rw-r--r-- 1 geh geh   256 Oct 11 22:33 util45.h\r\n";
            // write(connfd, ansda, 100);
            write_list_files(connfd, cmd->arg);

            sprintf(st->message, "226 Directory send OK.");
            close(connfd);
            close(st->sock_pasv);
        }
    } else {
        sprintf(st->message, "530 Permission denied. First login with USER and PASS.");
    }
    write_state(st);
}