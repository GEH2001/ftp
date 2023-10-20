/**
 * Entry of the server app.
*/
#include "basic.h"
#include "utils.h"
#include "routing.h"
#include "pthread.h"

void sock_process(int connfd);
void *conn_handler(void *sock_desc);
void server(int port);

/* Process connected socket for coming client */
void sock_process(int connfd) {
    command cmd;
    state st;
    char buf[BSIZE];
    memset(&cmd, 0, sizeof(cmd));
    memset(&st, 0, sizeof(st));
    memset(buf, 0, sizeof(buf));
    st.sock_data = st.sock_pasv = -1;   // 0 is stdin, may cause some errors
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

/** Handle connection for each client.
 *  @todo Consider thread synchronization to prevent resource competition.
 */
void *conn_handler(void *sock_desc) {
    int connfd = *(int *)sock_desc;
    command cmd;
    state st;
    char buf[BSIZE];
    memset(&cmd, 0, sizeof(cmd));
    memset(&st, 0, sizeof(st));
    memset(buf, 0, sizeof(buf));
    st.sock_data = st.sock_pasv = -1;   // 0 is stdin, may cause some errors
    st.sock_control = connfd;

    // welcome
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

    printf("client closed the connection.\n");

    close(connfd);
}

void server(int port) {
	int sock_listen, sock_control;
	struct sockaddr_in addr;

	sock_listen = socket_listen(port);
	if(sock_listen == -1) return;

	while(1) {
		// wait for connect, accept can block
		if ((sock_control = accept(sock_listen, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
		}
		// sock_process(sock_control);
        pthread_t thread_id;
        if(pthread_create(&thread_id, NULL, conn_handler, (void*)&sock_control) < 0) {
            perror("Error pthread_create() inside server()");
            return;
        }
        // pthread_join(thread_id, NULL);   // this can block
        pthread_detach(thread_id);  // mark thread as detach, auto recycle thread resources when thread program end
	}
	close(sock_listen);
	// TODO: Can not close if the program exit in while(1) above. atexit or signal may help
}


int main(int argc, char **argv) {
	// parse port and root from args
    int port = 0;
    char *root = NULL;
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-port") == 0) {
            if(i + 1 < argc) {
                port = atoi(argv[++i]);
            } else {
                printf("usage: -port n\n");
                return -1;
            }
        } else if (strcmp(argv[i], "-root") == 0) {
            if(i + 1 < argc) {
                root = argv[++i];
            } else {
                printf("usage: -root /path\n");
                return -1;
            }
        } else {
            printf("usage: -port n -root /path\n");
            return -1;
        }
    }

    if(port == 0) {
        port = 21;
        printf("port is set to default 21\n");
    } else {
        if(!port_available(port)) {
            printf("port %d is being used, set to default 21", port);
        } else {
            printf("port is set to %d\n", port);
        }
    }
    
    if(root == NULL) {
        root = "/tmp";
    }
    if(chdir(root) == -1) {
        perror("Failed to change working dir");
    } else {
        printf("Working dir is set to %s\n", root);
    }

	server(port);

	exit(0);
}