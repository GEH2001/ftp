all: client server

client: client.c
	gcc -o client client.c

server: server.c common.c common.h
	gcc -o server server.c common.c

main: main.c common.c common.h
	gcc main.c common.c -o main
clean:
	rm client server main