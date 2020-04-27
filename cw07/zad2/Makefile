CC = gcc
CFLAGS = -Wall -o
C = 2
P = 2
S = 2

all: main.c collector.c packer.c sender.c
	$(CC) $(CFLAGS) main main.c -lrt -lpthread
	$(CC) $(CFLAGS) collector collector.c -lrt -lpthread
	$(CC) $(CFLAGS) packer packer.c -lrt -lpthread
	$(CC) $(CFLAGS) sender sender.c -lrt -lpthread

run:
	./main $(C) $(P) $(S)
clean:
	rm -f collector packer sender main