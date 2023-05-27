#include "stock_data_structure.h"

int sell(stock_tree_head *head, int ID, int num_sell, char *buf)
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

    return 0;
}

int buy(stock_tree_head *head, int ID, int num_buy, char *buf)
{
    stock_item *stock_to_buy;

    stock_to_buy = find(head, ID);

    if (stock_to_buy == NULL)
    {
        sprintf(buf, "%s", "No such stock\n");
        return 14;
    }

    if (stock_to_buy->stocks_left < num_buy)
    {
        sprintf(buf, "%s", "Not enough left stock\n");
        return 22;
    }
    else
    {
        stock_to_buy->stocks_left -= num_buy;
        sprintf(buf, "%s", "[buy]\n");
        return 6;
    }

    return 0;
}

int show(stock_tree_head *head, int connfd, char *buf)
{
    stock_item *first_item;

    first_item = head->first_stock_pt;

    show_subfunc(first_item, connfd, buf);

    return 0;
}

void show_subfunc(stock_item *travel, int connfd, char *buf)
{
    if (travel)
    {
        show_subfunc(travel->leftp, connfd, buf);
        sprintf(buf, "%d %d %d\n", travel->ID, travel->stocks_left, travel->price);
        Rio_writen(connfd, buf, strlen(buf));
        show_subfunc(travel->rightp, connfd, buf);
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

    return new_item;
}

int save_to_txt(stock_tree_head *head)
{
    FILE *fp;

    fp = fopen(FILENAME, "w");

    if (!fp)
    {
        printf("File not opened\n");
        return 1;
    }

    save_to_txt_subfunc(head->first_stock_pt, fp);

    fclose(fp);

    return 0;
}

void save_to_txt_subfunc(stock_item *travel, FILE *fp)
{
    if (travel)
    {
        save_to_txt_subfunc(travel->leftp, fp);
        fprintf(fp, "%d %d %d\n", travel->ID, travel->stocks_left, travel->price);
        save_to_txt_subfunc(travel->rightp, fp);
    }
}

int load_from_txt(stock_tree_head *head)
{
    FILE *fp = fopen(FILENAME, "r");
    int ID, num_stocks, price;

    if (!fp)
    {
        printf("File %s do not exist\n", FILENAME);
        return 1;
    }

    head->first_stock_pt = NULL;
    head->num_stocks = 0;

    while (fscanf(fp, "%d %d %d\n", &ID, &num_stocks, &price))
    {
        add_item(head, ID, num_stocks, price);
    }

    return 0;
}