all: client server

client: client.c
	gcc -o client client.c

server: server.c utils.c routing.c
	gcc -o server server.c utils.c routing.c

main: main.c common.c utils.c
	gcc -o main main.c common.c utils.c

clean:
	rm client server main