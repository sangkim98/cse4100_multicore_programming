#include "csapp.h"

int main(){
    pid_t pid;

    if ((pid = Fork()) == 0){
        Sleep(50);
        printf("control should not reach here\n");
        exit(0);
    }

    Sleep(5);
    Kill(pid, SIGKILL);
    exit(0);

    return 0;
}