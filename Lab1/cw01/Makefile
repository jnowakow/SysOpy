CC = gcc -Wall

static:
	make clean
	$(CC) -c mylib.c
	ar crs libmylib.a mylib.o

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	