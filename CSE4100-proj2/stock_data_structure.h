#ifndef __DATA_STRUCTURE__
#define __DATA_STRUCTURE__

#include "csapp.h"
typedef struct _stock_item
{
    int ID;
    int stocks_left;
    int price;
    int readcnt;

    struct _stock_item *leftp;
    struct _stock_item *rightp;

    sem_t mutex;
} stock_item;

typedef struct _stock_tree_head
{
    int num_stocks;
    stock_item *first_stock_pt;

    sem_t mutex;
} stock_tree_head;

int sell(stock_tree_head *head, int ID, int num_sell);

int buy(stock_tree_head *head, int ID, int num_buy);

int show(stock_tree_head *head);

int show_subfunc(stock_item *travel);

stock_item *find(stock_tree_head *head, int ID);

int add_item(stock_tree_head *head, int ID, int num_stocks, int price);

stock_item *create_stock_item(int ID, int num_stocks, int price);

#endif