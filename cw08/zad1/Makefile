CC=gcc
CFLAGS=-Wall -o

compile: main.c
	$(CC) $(CFLAGS) main main.c -lpthread

run:
	./main 2 sign mountain.ascii.pgm result.txt

tests:
	./main 1 block mountain.ascii.pgm result.txt
	./main 1 interleaved mountain.ascii.pgm result.txt
	./main 1 sign mountain.ascii.pgm result.txt

	./main 2 block mountain.ascii.pgm result.txt
	./main 2 interleaved mountain.ascii.pgm result.txt
	./main 2 sign mountain.ascii.pgm result.txt

	./main 4 block mountain.ascii.pgm result.txt
	./main 4 interleaved mountain.ascii.pgm result.txt
	./main 4 sign mountain.ascii.pgm result.txt

	./main 8 block mountain.ascii.pgm result.txt
	./main 8 interleaved mountain.ascii.pgm result.txt
	./main 8 sign mountain.ascii.pgm result.txt


clean:
	rm -f main