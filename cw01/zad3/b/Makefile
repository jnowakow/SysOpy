CC=gcc
CFLAGS=-c -Wall
OPTLEVEL=-O2

static:
	$(CC) $(CFLAGS) $(OPTLEVEL) mylib.c
	ar crs libmylib.a mylib.o
	$(CC) $(CFLAGS) $(OPTLEVEL) main.c
	$(CC) main.o mylib.o -o main

shared: mylib.c mylib.h
	$(CC) $(CFLAGS) $(OPTLEVEL) -fPIC mylib.c
	$(CC) -shared -fPIC -o mylib.so mylib.o
	$(CC) $(OPTLEVEL) -L.  -Wl,-rpath=. -o main main.c -lmylib
clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main

tests_static:
	make static
	./main create_table 4 compare_pairs sa1.txt:sb1.txt sa2.txt:sb2.txt sa3.txt:sb3.txt sa0.txt:sb0.txt remove_block 0 remove_operation 1 0

tests_shared: 
	make shared
	./main create_table 4 compare_pairs sa1.txt:sb1.txt sa2.txt:sb2.txt sa3.txt:sb3.txt sa0.txt:sb0.txt remove_block 0 remove_operation 1 0

