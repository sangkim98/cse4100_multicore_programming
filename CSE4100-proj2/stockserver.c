/*
 * echoserveri.c - An iterative echo server
 */
/* $begin echoserverimain */
#include "csapp.h"
#include "stock_data_structure.h"

typedef struct {
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int maxi;
    int clientfd[FD_SETSIZE];
    rio_t clientrio[FD_SETSIZE];
} pool;

stock_tree_head head;

void echo(int connfd);
void stock_service(pool *p);
void parse_stock_command(char buf[], int command_args[3]);
void init_pool(int listenfd, pool* pool);
void add_client(int connfd, pool *p);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */ // line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];
    static pool pool;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    load_from_txt(&head);

    listenfd = Open_listenfd(argv[1]);
    init_pool(listenfd, &pool);
    while (1)
    {
        pool.ready_set = pool.read_set;
        pool.nready = Select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);

        if(FD_ISSET(listenfd, &pool.ready_set)){
            clientlen = sizeof(struct sockaddr_storage);
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                        client_port, MAXLINE, 0);
            add_client(connfd, &pool);
            printf("Connected to (%s, %s)\n", client_hostname, client_port);
        }
        stock_service(&pool);
    }

    exit(0);
}
/* $end echoserverimain */

void stock_service(pool *p)
{
    int i, n, connfd, command_args[3];
    char buf[MAXLINE];
    rio_t rio;


    for(i = 0; (i <= p->maxi) && (p->nready > 0); i++){
        connfd = p->clientfd[i];
        rio = p->clientrio[i];

        if((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))){
            p->nready--;
            if((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){
                printf("server received %d bytes of command %s", n, buf);

                parse_stock_command(buf, command_args);

                switch (command_args[0]){
                    case Sell:
                        n = sell(&head, command_args[1], command_args[2], buf);
                        Rio_writen(connfd, buf, MAXLINE);
                        break;
                    case Buy:
                        n = buy(&head, command_args[1], command_args[2], buf);
                        Rio_writen(connfd, buf, MAXLINE);
                        break;
                    case Show:
                        n = show(&head, connfd, buf);
                        Rio_writen(connfd, buf, MAXLINE);
                        break;
                    default:
                        n = sprintf(buf, "No such command\n");
                        Rio_writen(connfd, buf, MAXLINE);
                        break;
                }
            }
            else{
                Close(connfd);
                FD_CLR(connfd, &p->read_set);
                p->clientfd[i] = -1;
            }
        }
    }
}

void parse_stock_command(char buf[], int command_args[3]){
    char command[MAXLINE] = {'\0'};

    int i;

    for(i = 0; i < MAXBUF && buf[i] != ' '; i++)

    for(; i < MAXCOMMAND; i++){
        if(buf[i] == ' ' || buf[i] == '\n'){
            break;
        }
        command[i] = buf[i];
    }
    command[i] = '\0';

    if(!strcmp(SELLSTRING, command)){
        command_args[0] = Sell;
        sscanf(buf, "%s %d %d", command, command_args+1, command_args+2);
    }
    else if(!strcmp(BUYSTRING, command)){
        command_args[0] = Buy;
        sscanf(buf, "%s %d %d", command, command_args+1, command_args+2);
    }else if(!strcmp(SHOWSTRING, command)){
        command_args[0] = Show;
    }
    else{
        command_args[0] = Echo;
    }
}

void init_pool(int listenfd, pool *p){
    int i;
    p->maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++)
        p->clientfd[i] = -1;

    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, pool *p){
    int i;
    p->nready--;
    for(i = 0; i < FD_SETSIZE; i++){
        if(p->clientfd[i] < 0){
            p->clientfd[i] = connfd;
            Rio_readinitb(&p->clientrio[i], connfd);

            FD_SET(connfd, &p->read_set);

            if(connfd > p->maxfd)
                p->maxfd = connfd;
            if(i > p->maxi)
                p->maxi = i;
            break;
        }
    }
    if (i == FD_SETSIZE)
        app_error("add_client error: Too many clients");
}