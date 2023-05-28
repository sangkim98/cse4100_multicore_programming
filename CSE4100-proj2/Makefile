CC = gcc
CFLAGS=-O2 -Wall
LDLIBS = -lpthread

all: multiclient stockclient stockserver test_ds

multiclient: multiclient.c csapp.c csapp.h stock_data_structure.c stock_data_structure.h
stockclient: stockclient.c csapp.c csapp.h stock_data_structure.c stock_data_structure.h
stockserver: stockserver.c echo.c csapp.c csapp.h stock_data_structure.c stock_data_structure.h
test_ds: test_ds.c csapp.c csapp.h stock_data_structure.c stock_data_structure.h
	$(CC) $(CFLAGS) test_ds.c csapp.c csapp.h stock_data_structure.c stock_data_structure.h $(LDLIBS) -o test_ds

clean:
	rm -rf *~ multiclient stockclient stockserver test_ds *.o output.log
