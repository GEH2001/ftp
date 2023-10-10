#include "common.h"

void server(int port, char *root);

int main(int argc, char **argv) {
	
	// TODO: check whether is port from argv available or not
	server(6789, ".");
	// server(34074, ".");

	exit(0);
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