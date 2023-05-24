#ifndef __DATA_STRUCTURE__
#define __DATA_STRUCTURE__

typedef struct _stock_item{
    int ID;
    int stocks_left;
    int price;
    int readcnt;

    struct _stock_item* leftp;
    struct _stock_item* rightp;

    sem_t mutex;
} stock_item;

int sell(stock_item* head, int ID, int num_sell);

int buy(stock_item* head, int ID, int num_buy);

int show(stock_item* head);

int find(stock_item* head, int ID);

#endif