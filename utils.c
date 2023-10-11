#include "utils.h"

// TODO: Just send one by one
int list_files(char *buf, int buf_size, const char *path) {
    char command[272];
    memset(command, 0, sizeof(command));
    snprintf(command, sizeof(command), "/bin/ls -l %s", path);
    FILE *pipe = popen(command, "r");
    if(!pipe) {
        perror("Error popen()");
        return -1;
    }
    char line[1024];
    int total_bytes = 0;
    // remove the first line of the output: total 124
    fgets(line, sizeof(line), pipe);
    memset(line, 0, sizeof(line));
    while(fgets(line, sizeof(line), pipe) != NULL) {
        int len = strlen(line);
        // `ls -l` will append \n to every file
        line[--len] = '\0';     // remove \n in the tail
        if(total_bytes + len + 2 < buf_size) {
            strcat(buf, line);
            strcat(buf, "\r\n");
            total_bytes += len + 2;
        } else {
            fclose(pipe);
            return -1;  // TODO: This is a server error.
        }
        memset(line, 0, sizeof(line));
    }
    fclose(pipe);
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
    fgets(buf, sizeof(buf), pipe); // remove the first output: total 124
    while(fgets(buf, sizeof(buf), pipe) != NULL) {
        int len = strlen(buf);
        // replace \n with \r\n
        buf[--len] = '\0';
        strncat(buf, "\r\n", 3);
        printf("%s", buf);
        if(write(sock_data, buf, sizeof buf) == -1) {
            printf("Error write(): %s(%d)\n", strerror(errno), errno);
        }
    }
    fclose(pipe);
    return 0;
}