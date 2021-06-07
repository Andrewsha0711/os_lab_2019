CC=gcc
CFLAGS=-I.
all: sources client server

client : sources.o sources.h
	$(CC) -o client sources.c sources.h client.c -pthread $(CFLAGS)

server : sources.o sources.h
	$(CC) -o server sources.c sources.h server.c -pthread $(CFLAGS)

sources : sources.h
	$(CC) -o sources.o -c sources.c

clean :
	rm -f client server sources.o