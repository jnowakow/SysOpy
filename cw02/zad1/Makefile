CC=gcc
CFLAGS=-Wall

compile: main.c
	$(CC) $(CFLAGS) -o main main.c

clean:
	rm dane daneLib daneSys main

runTest:
	make compile
	#Tests for 1 byte lines

	./main generate data 50000 1
	./main copy data dataLib 50000 1 lib
	./main copy data dataSys 50000 1 sys
	./main sort dataLib 50000 1 lib
	./main sort dataSys 50000 1 sys

	./main generate data 60000 1
	./main copy data dataLib 60000 1 lib
	./main copy data dataSys 60000 1 sys
	./main sort dataLib 60000 1 lib
	./main sort dataSys 60000 1 sys

	#Tests for 4 bytes lines

	./main generate data 100000 4
	./main copy data dataLib 100000 4 lib
	./main copy data dataSys 100000 4 sys
	./main sort dataLib 100000 4 lib
	./main sort dataSys 100000 4 sys

	./main generate data 500000 4
	./main copy data dataLib 500000 4 lib
	./main copy data dataSys 500000 4 sys
	./main sort dataLib 500000 4 lib
	./main sort dataSys 500000 4 sys

	#Tests for 512 bytes lines

	./main generate data 100000 512
	./main copy data dataLib 100000 512 lib
	./main copy data dataSys 100000 512 sys
	./main sort dataLib 100000 512 lib
	./main sort dataSys 100000 512 sys

	./main generate data 500000 512
	./main copy data dataLib 500000 512 lib
	./main copy data dataSys 500000 512 sys
	./main sort dataLib 500000 512 lib
	./main sort dataSys 500000 512 sys

	#Tests for 1024 bytes lines

	./main generate data 100000 1024
	./main copy data dataLib 100000 1024 lib
	./main copy data dataSys 100000 1024 sys
	./main sort dataLib 100000 1024 lib
	./main sort dataSys 100000 1024 sys

	./main generate data 500000 1024
	./main copy data dataLib 500000 1024 lib
	./main copy data dataSys 500000 1024 sys
	./main sort dataLib 500000 1024 lib
	./main sort dataSys 500000 1024 sys

	#Tests for 4096 bytes lines

	./main generate data 100000 4096
	./main copy data dataLib 100000 4096 lib
	./main copy data dataSys 100000 4096 sys
	./main sort dataLib 100000 4096 lib
	./main sort dataSys 100000 4096 sys

	./main generate data 200000 4096
	./main copy data dataLib 200000 4096 lib
	./main copy data dataSys 200000 4096 sys
	./main sort dataLib 200000 4096 lib
	./main sort dataSys 200000 4096 sys

	#Tests for 8192 bytes lines

	./main generate data 100000 8192
	./main copy data dataLib 100000 8192 lib
	./main copy data dataSys 100000 8192 sys
	./main sort dataLib 100000 8192 lib
	./main sort dataSys 100000 8192 sys

	./main generate data 200000 8192
	./main copy data dataLib 200000 8192 lib
	./main copy data dataSys 200000 8192 sys
	./main sort dataLib 200000 8192 lib
	./main sort dataSys 200000 8192 sys
	


test4:
	./main generate data $(COUNT) $(BYTES)
	./main copy data dataLib $(COUNT) $(BYTES) lib
	./main copy data dataSys $(COUNT) $(BYTES) sys
	./main sort dataLib $(COUNT) $(BYTES) lib
	./main sort dataSys $(COUNT) $(BYTES) sys
