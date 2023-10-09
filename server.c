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

	// init a socket
	if((sock_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		exit(1);
	}
	// ip & port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = port;	// TODO: htons(port)
	addr.sin_addr.s_addr = htonl(INADDR_ANY);	// ip 0.0.0.0
	// bind
	if (bind(sock_listen, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		exit(1);
	}
	// listen
	if (listen(sock_listen, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		exit(1);
	}

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
}