CC = gcc
CFLAGS = -Wall -o

all: server.c client.c
	$(CC) $(CFLAGS) server server.c
	$(CC) $(CFLAGS) client client.c
	
clean:
	rm -f server client

