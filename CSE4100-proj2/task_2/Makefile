CC = gcc
CFLAGS=-O2 -Wall
LDLIBS = -lpthread

all: multiclient stockclient stockserver

multiclient: multiclient.c csapp.c csapp.h stock_data_structure.c stock_data_structure.h
stockclient: stockclient.c csapp.c csapp.h stock_data_structure.c stock_data_structure.h
stockserver: stockserver.c echo.c csapp.c csapp.h stock_data_structure.c stock_data_structure.h

clean:
	rm -rf *~ multiclient stockclient stockserver *.o
