all: client server

client: client.c
	gcc -o client client.c

server: server.c utils.c routing.c
	gcc -o server server.c utils.c routing.c -pthread

clean:
	rm client server