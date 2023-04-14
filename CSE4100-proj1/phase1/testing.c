#include "csapp.h"

#define N 30

int main(){
    int status, i;
    pid_t pid;

    char buf[MAXLINE];

    // for (i = 0; i < N; i++){
    //     if((pid = Fork()) == 0){
    //         exit(100+i);
    //     }
    // }
    // while((pid = Wait(&status)) > 0){
    //     if(WIFEXITED(status))
    //         printf("child %d terminated normally with exit status=%d\n", pid, WEXITSTATUS(status));
    //     else
    //         printf("child %d terminated abnormally\n", pid);
    // }

    // if(errno != ECHILD)
    //     unix_error("waitpid error");

    FILE* fp;

    fp = fopen(".shell_history", "r");

    while(fgets(buf, MAXLINE, fp) != NULL){
        printf("%s", buf);
    }


    
    return 0;
}