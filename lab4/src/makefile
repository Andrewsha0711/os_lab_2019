CC=gcc
CFLAGS=-I.

all: parallel_min_max utils.o find_min_max.o process_memory

parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

process_memory : process_memory.c
	$(CC) process_memory.c -o process_memory.o

clean :
	rm utils.o find_min_max.o sequential_min_max parallel_min_max task5 process_memory
