CC = gcc
CFLAGS = -Og
LDLIBS = -lpthread

PROGS = shellex

all: $(PROGS)

shellex: myshell.c csapp.c
	$(CC) $(CFLAGS) myshell.c csapp.c -lpthread -o shellex
test: testing.c csapp.c
	$(CC) $(CFLAGS) testing.c csapp.c -lpthread -o test

clean:
	rm -rf *~ $(PROGS) test