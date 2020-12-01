# Author:	Johannes Felzmann 11912368
# Modul: 	3coloring
# Date:  	31.20.2020

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: clean all

all: supervisor generator

circularbuffer: circularbuffer.c circularbuffer.h
		$(CC) $(CFLAGS) -o $@ $^ -lrt -pthread

generator: generator.c circularbuffer.o
		$(CC) $(CFLAGS) -o $@ $^ -lrt -pthread

supervisor: supervisor.c circularbuffer.o
		$(CC) $(CFLAGS) -o $@ $^ -lrt -pthread

%.o: %.c
		gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f generator supervisor circularbuffer.o