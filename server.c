/**
 * Entry of the server app.
*/
#include "basic.h"
#include "utils.h"
#include "routing.h"

void sock_process(int connfd);
void server(int port, char *root);

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

void server(int port, char *root) {
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
		
		// TODO: multi-thread
		sock_process(sock_control);
	}
	close(sock_listen);
	// TODO: Can not close if the program exit in while(1) above.
	// TODO: atexit(close all socket)
}


int main(int argc, char **argv) {
	
	// TODO: check whether is port from argv available or not
	server(6789, ".");
	// server(34074, ".");

	exit(0);
}