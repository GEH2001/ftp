#include "utils.h"

void write_state(state *st) {
    st->message[BSIZE - 4] = '\0'; // There must be space for "\r\n"
    strncat(st->message, "\r\n", 3);
    if(write(st->sock_control, st->message, strlen(st->message)) == -1) {
        printf("Error write(): %s(%d)\n", strerror(errno), errno);
    }
    // memset(st->message, 0, BSIZE);  // clear the reponse message buffer
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

int gen_port() {
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

int write_list_files(int sock_data, const char *path) {
    char command[272];
    memset(command, 0, sizeof(command));
    snprintf(command, sizeof(command), "/bin/ls -l %s", path);
    FILE *pipe = popen(command, "r");
    if(!pipe) {
        perror("Error popen()");
        return -1;
    }
    char buf[512];
    // fgets(buf, sizeof(buf), pipe); // remove the first output: total 124, bug: if path is just a file.
    memset(buf, 0, sizeof buf);
    while(fgets(buf, sizeof(buf), pipe) != NULL) {
        if(strncmp(buf, "total", 5) == 0) continue;
        int len = strlen(buf);
        // replace \n with \r\n
        buf[--len] = '\0';
        strncat(buf, "\r\n", 3);
        if(write(sock_data, buf, sizeof buf) == -1) {
            printf("Error write(): %s(%d)\n", strerror(errno), errno);
            return -1;
        }
        memset(buf, 0, sizeof buf); 
        // clear the buffer before next, otherwise there will be some error like: WARNING! 3 bare linefeeds received in ASCII mode.
    }
    fclose(pipe);
    return 0;
}


int is_file_visiable(const char *path) {
    // check if the file exist using access()
    if(access(path, F_OK) != -1) {
        const char *basename = strrchr(path, '/'); // the last '/'
        // input: "./main.c", "main.c", ".", "..", "./.gitignore", "/home/geh"
        if(basename) {
            basename++;
        } else {
            basename = path;
        }
        
        // Usually, invisible files start with '.' or '..' 
        if(basename[0] != '.') {
            return 1;
        }
    }
    return 0;
}
