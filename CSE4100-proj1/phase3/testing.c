#include "csapp.h"

void sigint_handler(int sig){
    printf("Caught SIGINT\n");
    exit(0);
}

int main(){
    // pid_t pid;

    // if (Signal(SIGINT, sigint_handler) == SIG_ERR){
    //     unix_error("signal error");
    // }

    // pause();
    Sleep(5);

    return 0;
}