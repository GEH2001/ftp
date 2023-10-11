all: client server

client: client.c
	gcc -o client client.c

server: server.c common.c utils.c
	gcc -o server server.c common.c utils.c

main: main.c common.c utils.c
	gcc -o main main.c common.c utils.c

clean:
	rm client server main