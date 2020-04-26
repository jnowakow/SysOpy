CC = gcc
CFLAGS = -Wall -o
C = 2
P = 2
S = 2

all: main.c collector.c packer.c sender.c
	$(CC) $(CFLAGS) main main.c
	$(CC) $(CFLAGS) collector collector.c
	$(CC) $(CFLAGS) packer packer.c
	$(CC) $(CFLAGS) sender sender.c

run:
	./main $(C) $(P) $(S)
clean:
	rm -f collector packer sender main