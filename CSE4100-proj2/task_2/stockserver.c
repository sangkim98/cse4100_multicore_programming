/*
 * echoserveri.c - An iterative echo server
 */
/* $begin echoserverimain */
#include "csapp.h"
#include "stock_data_structure.h"

stock_tree_head head;
sbuf_t sbuf;

void echo(int connfd);
void stock_service(int connfd);
void parse_stock_command(char buf[], int command_args[3]);
void *thread(void *vargp);
void int_handler(int sig);

int main(int argc, char **argv)
{
    int i, listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */ // line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];
    pthread_t tid;
    Signal(SIGINT, int_handler);
 
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    Sem_init(&head.mutex, 0, 1);

    load_from_txt(&head);

    listenfd = Open_listenfd(argv[1]);
    sbuf_init(&sbuf, SBUFSIZE);
    for(i = 0; i < NTHREADS; i++)
        Pthread_create(&tid,NULL,thread,NULL);
    while (1)
    {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        sbuf_insert(&sbuf, connfd);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
    }

    exit(0);
}
/* $end echoserverimain */

void stock_service(int connfd)
{
    int n, command_args[3];
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
    {
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
            case Exit:
                Rio_writen(connfd, "Connection Closed\n", MAXLINE);
                return;
            default:
                n = sprintf(buf, "No such command\n");
	            Rio_writen(connfd, buf, MAXLINE);
                break;
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
    else if(!strcmp(EXITSTRING, command)){
        command_args[0] = Exit;
    }
    else{
        command_args[0] = Echo;
    }
}

void *thread(void *vargp){
    Pthread_detach(pthread_self());
    while(1){
        int connfd = sbuf_remove(&sbuf);
        stock_service(connfd);
        Close(connfd);
    }
}

void int_handler(int sig){
    save_to_txt(&head);
    exit(0);
}