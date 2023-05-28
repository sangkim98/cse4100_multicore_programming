/*
 * echoserveri.c - An iterative echo server
 */
/* $begin echoserverimain */
#include "csapp.h"
#include "stock_data_structure.h"

stock_tree_head head;

void echo(int connfd);
void stock_service(int connfd);
void stock_command(char buf[], int command_args[3]);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */ // line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    load_from_txt(&head);

    listenfd = Open_listenfd(argv[1]);
    while (1)
    {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        // echo(connfd);
        stock_service(connfd);
        Close(connfd);
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

        stock_command(buf, command_args);

        switch (command_args[0]){
            case Sell:
                n = sell(&head, command_args[1], command_args[2], buf);
                Rio_writen(connfd, buf, MAXLINE);
                save_to_txt(&head);
                break;
            case Buy:
                n = buy(&head, command_args[1], command_args[2], buf);
                Rio_writen(connfd, buf, MAXLINE);
                save_to_txt(&head);
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
}

void stock_command(char buf[], int command_args[3]){
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