CC = gcc

ROOT = .

CFLAGS = -Wall 

.PHONY: all

all: 
	$(CC) $(CFLAGS) -c main.c -o main.o
	$(CC) main.o -lwiringPi -o main

clean:	
	-rm -f *.o
	-rm -f main
