all: default

default:
	gcc ./mcm_server/server.c -o server.out
	gcc client.c ./mcm_clients/mcm_client.c -o client.out -lpthread

clean:
	rm -f server.out client.out
