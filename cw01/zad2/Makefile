CC=gcc
CFLAGS=-c -Wall

static:
	$(CC) $(CFLAGS) mylib.c
	ar crs libmylib.a mylib.o
	$(CC) main.o mylib.o -o main

shared:
	$(CC) $(CFLAGS) -fPIC mylib.c
	$(CC) -shared -fPIC -o mylib.so mylib.o
	$(CC)  -L.  -Wl,-rpath=. -o main main.c -lmylib

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main