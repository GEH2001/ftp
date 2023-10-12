#include "utils.h"

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