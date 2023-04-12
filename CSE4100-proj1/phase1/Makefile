CC = gcc
CFLAGS = -Og
LDLIBS = -lpthread

PROGS = shellex

all: $(PROGS)

shellex: shellex.c csapp.c

test: testing.c csapp.c
	$(CC) $(CFLAGS) testing.c csapp.c -lpthread -o test

clean:
	rm -rf *~ $(PROGS) test