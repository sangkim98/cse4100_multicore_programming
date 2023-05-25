#include "stock_data_structure.h"

int sell(stock_tree_head *head, int ID, int num_sell)
{
    stock_item *item_found;

    item_found = find(head, ID);

    if (item_found)
    {
        item_found->stocks_left += num_sell;
        return 0;
    }
    else
    {
        int random_price = rand() % 100000;
        add_item(head, ID, num_sell, random_price);
    }
}

int buy(stock_tree_head *head, int ID, int num_buy)
{
    stock_item *stock_to_buy;

    stock_to_buy = find(head, ID);

    if (stock_to_buy == NULL)
    {
        printf("No such stock\n");
        return 1;
    }

    if (stock_to_buy->stocks_left < num_buy)
    {
        printf("Not enough left stock\n");
    }
    else
    {
        stock_to_buy->stocks_left -= num_buy;
        printf("[buy]\n");
    }

    return 0;
}

int show(stock_tree_head *head)
{
    stock_item *first_item;

    first_item = head->first_stock_pt;

    show_subfunc(first_item);
}

int show_subfunc(stock_item *travel)
{
    if (travel)
    {
        show_subfunc(travel->leftp);
        printf("%d %d %d", travel->ID, travel->stocks_left, travel->price);
        show_subfunc(travel->rightp);
    }
}

stock_item *find(stock_tree_head *head, int ID)
{
    stock_item *travel;

    travel = head->first_stock_pt;

    if (!travel || head->num_stocks == 0)
        return NULL;

    while (travel && travel->ID != ID)
    {
        if (ID > travel->ID)
        {
            travel = travel->rightp;
        }
        else
        {
            travel = travel->leftp;
        }
    }

    return travel;
}

int add_item(stock_tree_head *head, int ID, int num_stocks, int price)
{

    stock_item *new_item, *prev, *curr;

    new_item = create_stock_item(ID, num_stocks, price);

    if (!new_item)
    {
        printf("Memory allocation for new item failed\n");
        return 1;
    }

    prev = NULL;
    curr = head->first_stock_pt;

    if (!curr)
    {
        head->first_stock_pt = new_item;
    }
    else
    {
        while (curr)
        {
            prev = curr;
            if (ID > curr->ID)
            {
                curr = curr->rightp;
            }
            else
            {
                curr = curr->leftp;
            }
        }

        if (ID > prev->ID)
        {
            prev->rightp = new_item;
        }
        else
        {
            prev->leftp = new_item;
        }
    }

    head->num_stocks++;

    return 0;
}

stock_item *create_stock_item(int ID, int num_stocks, int price)
{
    stock_item *new_item = (stock_item *)malloc(sizeof(stock_item));
    new_item->ID = ID;
    new_item->leftp = new_item->rightp = NULL;
    new_item->price = price;
    new_item->stocks_left = num_stocks;
}