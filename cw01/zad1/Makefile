CC=gcc
CFLAGS=-c -Wall

static:
	$(CC) $(CFLAGS) mylib.c
	ar crs libmylib.a mylib.o

shared:
	$(CC) $(CFLAGS) -fPIC mylib.c
	$(CC) -shared -fPIC -o mylib.so mylib.o

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so