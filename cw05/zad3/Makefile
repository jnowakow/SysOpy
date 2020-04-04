CC=gcc
CFLAGS=-Wall 


compile: main.c producer.c consumer.c
	$(CC) $(CFLAGS) -o main main.c
	$(CC) $(CFLAGS) -o consumer consumer.c
	$(CC) $(CFLAGS) -o producer producer.c
	
run:
	rm -f fifo
	./main fifo output.txt files/a.txt files/b.txt files/c.txt files/d.txt files/e.txt 

clean:
	rm -f fifo
	rm -f main producer consumer