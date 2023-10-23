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
        perror("Error: getsockname()");
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
            return -2;
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

int send_file(int sock_data, const char *path, int rest_pos) {
    int file_fd = open(path, O_RDONLY);
    if(file_fd == -1) {
        perror("Failed to open file");
        return -1;
    }
    struct stat file_stat;
    if(fstat(file_fd, &file_stat) == -1) {
        perror("Failed to get file status");
        close(file_fd);
        return -1;
    }
    // check if this is a file
    if(S_ISREG(file_stat.st_mode) == 0) {
        close(file_fd);
        return -2;
    }
    // file_stat.st_size
    // rest: seek position
    if (lseek(file_fd, rest_pos, SEEK_SET) == -1) {
        perror("Error lseek() inside send_file()");
        close(file_fd);
        return -4;
    }
    char buf[BSIZE];
    memset(buf, 0, BSIZE);

    ssize_t bytes_read = 0, bytes_written = 0, total = 0;
    while((bytes_read=read(file_fd, buf, BSIZE)) > 0) {
        bytes_written = write(sock_data, buf, bytes_read);
        total += bytes_written;
        if(bytes_written < 0) {
            perror("Failed to write to socket");
            close(file_fd);
            return -3;
        }
        memset(buf, 0, BSIZE);
    }
    close(file_fd);
    if(bytes_read == -1) { // read errors
        return -4;
    }

    return 0;
}

void *send_file_thread(void* _st) {
    state* st = (state*) _st;
    int err_code = send_file(st->sock_data, st->fpath, st->rest_pos);
    close_safely(st->sock_data);
    switch (err_code)
    {
    case 0:
        sprintf(st->message, "226 Transfer complete.");
        break;
    case -1:
        sprintf(st->message, "550 Invalid path.");
        break;
    case -2:
        sprintf(st->message, "550 Not a common file, maybe directory.");
        break;
    case -3:
        sprintf(st->message, "426 TCP connection was broken.");
        break;
    case -4:
        sprintf(st->message, "451 Server had a trouble reading the file.");           
        break;
    default:
        sprintf(st->message, "550 Something wrong.");
        break;
    }
    
    write_state(st);
    free(st);   // must
    pthread_exit(0);
}

int create_data_conn(state *st) {
    if(st->mode == PASSIVE) {   // PASV
        int connfd;
        if((connfd=accept(st->sock_pasv, NULL, NULL)) == -1) {
            perror("create_data_conn(): Error accept()");
            return -1;
        }
        close_safely(st->sock_pasv); // stop accepting new connections
        st->sock_data = connfd;
    }

    if(st->mode == STANDARD) {  // PORT
        int port = st->pt_addr[4] * 256 + st->pt_addr[5];
        char ip[256];
        memset(ip, 0, sizeof ip);
        sprintf(ip, "%d.%d.%d.%d", st->pt_addr[0], st->pt_addr[1], st->pt_addr[2], st->pt_addr[3]);
        int sockfd;
        struct sockaddr_in addr;
        if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
            perror("Error socket() within create_data_conn()");
            return -1;
        }
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
            perror("Error inet_pton() within create_data_conn()");
            return -1;
        }
    	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("Error connect() within create_data_conn()");
            return -1;
	    }
        st->sock_data = sockfd;
    }

    return 0;
}


void close_safely(int sock_fd) {
    // stdin=0, stdout=1, stderr=2
    if(sock_fd != 0 && sock_fd != 1 && sock_fd != 2) {
        close(sock_fd);
    }
}

int recv_file(int sock_data, const char *path, int rest_pos) {
    // parse file name, "./send.txt" -> "send.txt"
    const char *filename = strrchr(path, '/');
    if(filename != NULL) {
        filename++;
    } else {
        filename = path;
    }
    int file_fd = -1;
    if(rest_pos == 0) {
        // O_WRONLY: allow write and read;  O_CREAT: create if file not exist;
        // O_TRUNC: clear if file exist; O_APPEND: write to the end of the file(auto seek end)
        file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    } else {
        // handle REST: just append to the file
        file_fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0666);
    }
    if(file_fd == -1) {
        perror("Error open() inside recv_file()");
        return -1;
    }
    // not needed, because open mode is APPEND
    // lseek(file_fd, 0, SEEK_END);
    char buf[BSIZE];
    memset(buf, 0, BSIZE);
    ssize_t bytes_read = 0, bytes_written = 0;
    while ((bytes_read = read(sock_data, buf, BSIZE)) > 0)
    {
        bytes_written = write(file_fd, buf, bytes_read);
        if(bytes_written == -1) {
            perror("Error write() inside recv_file()");
            return -1;
        }
        memset(buf, 0, BSIZE);
    }
    if(bytes_read == -1) {
        perror("Error read() inside recv_file()");
        return -2;
    }
    close(file_fd);
    return 0;
}

void* recv_file_thread(void* _st) {
    state* st = (state*) _st;
    int err_code = recv_file(st->sock_data, st->fpath, st->rest_pos);
    close_safely(st->sock_data);
    switch (err_code)
    {
    case 0:
        sprintf(st->message, "226 Successfully stored.");
        break;
    case -1:
        sprintf(st->message, "451 Server had a trouble saving the file.");
        break;
    case -2:
        sprintf(st->message, "426 TCP connection was broken.");
        break;
    default:
        sprintf(st->message, "552 Something wrong.");
        break;
    }
    write_state(st);
    free(st);
    pthread_exit(0);
}