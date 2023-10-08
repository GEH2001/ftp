#include "common.h"

void server(int port, char *root);

int main(int argc, char **argv) {
	
	server(6789, ".");

	exit(0);
}

// int main(int argc, char **argv) {
// 	int listenfd, connfd;		//监听socket和连接socket不一样，后者用于数据传输
// 	struct sockaddr_in addr;
// 	char sentence[8192];
// 	int p;
// 	int len;

// 	//创建socket
// 	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
// 		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
// 		return 1;
// 	}

// 	//设置本机的ip和port
// 	memset(&addr, 0, sizeof(addr));
// 	addr.sin_family = AF_INET;
// 	addr.sin_port = 6789;
// 	addr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听"0.0.0.0"

// 	//将本机的ip和port与socket绑定
// 	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
// 		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
// 		return 1;
// 	}

// 	//开始监听socket
// 	if (listen(listenfd, 10) == -1) {
// 		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
// 		return 1;
// 	}

// 	//持续监听连接请求
// 	while (1) {
// 		//等待client的连接 -- 阻塞函数
// 		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
// 			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
// 			continue;
// 		}
		
// 		//榨干socket传来的内容
// 		p = 0;
// 		while (1) {
			
// 			int n = read(connfd, sentence + p, 8191 - p);
// 			printf("n=%d\n", n);
// 			if (n < 0) {
// 				printf("Error read(): %s(%d)\n", strerror(errno), errno);
// 				close(connfd);
// 				continue;
// 			} else if (n == 0) {	//EOF: client closed the socket
// 				break;
// 			} else {
// 				p += n;
// 				if (sentence[p - 1] == '\n') {
// 					break;
// 				}
// 			}
// 		}
// 		//socket接收到的字符串并不会添加'\0'
// 		sentence[p - 1] = '\0';
// 		len = p - 1;
		
// 		// //字符串处理
// 		// for (p = 0; p < len; p++) {
// 		// 	sentence[p] = toupper(sentence[p]);
// 		// }

// 		// //发送字符串到socket
//  		// p = 0;
// 		// while (p < len) {
// 		// 	int n = write(connfd, sentence + p, len + 1 - p);
// 		// 	if (n < 0) {
// 		// 		printf("Error write(): %s(%d)\n", strerror(errno), errno);
// 		// 		return 1;
// 	 	// 	} else {
// 		// 		p += n;
// 		// 	}			
// 		// }

// 		printf("From Client: %s\n", sentence);

// 		command cmd;
// 		state st;
// 		st.sock_control = connfd;
// 		parse_command(sentence, &cmd);
// 		cmd_response(&cmd, &st);

// 		close(connfd);
// 	}

// 	close(listenfd);
// }


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
	addr.sin_port = port;
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