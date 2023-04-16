/* $begin shellmain */
#include "csapp.h"
#include<errno.h>
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
int getexecpath(char* path_name, char* exec_name);
/* Redirection and Pipe Implementation */
int token_pipe_command(char** piped_commands, char* cmdline);
int run_pipe(char** piped_commands, const int num_piped_commands);
void run_child(int *fd, const char* cmdline, const int idx, const int is_last_command);

int main() 
{
    int save_history_counter = 0; 
    char cmdline[MAXLINE]; /* Command line */

    set_shell_history_location();

    while (1) {
	/* Read */
        printf("CSE4100-MP-P1> ");                   
        fgets(cmdline, MAXLINE, stdin); 
        if (feof(stdin))
            exit(0);
        /* Evaluate */
        eval(cmdline);

        save_history_counter++;
    }

    return 0;
}
/* $end shellmain */
  
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char *argv[MAXARGS];   /* Argument list execve() */
    char buf[MAXLINE];     /* Holds modified command line */
    char pipe_buf[MAXLINE];
    char name[MAXLINE];    /* Holds name of program */
    int bg;                /* Should the job run in bg or fg? */
    int builtin_condition; /*  */
    char *piped_commands[MAXPIPES+1];
    int num_piped_commands;

    pid_t pid;             /* Process id */
    
    builtin_condition = 0;

    do{
        strcpy(buf, cmdline);
        bg = parseline(buf, argv);

        if (argv[0] == NULL)  
            return;   /* Ignore empty lines */
        open_shell_history();
        add_command_to_history(cmdline);
        save_shell_history();

        if(strpbrk(cmdline, "|") != NULL){
            strcpy(pipe_buf, cmdline);

            if((num_piped_commands = token_pipe_command(piped_commands, pipe_buf)) == 0){
                printf("pipe error\n");
                return;
            }

            run_pipe(piped_commands, num_piped_commands);

            return;
        }
        else{
            if (!(builtin_condition = builtin_command(argv))) { //quit -> exit(0), & -> ignore, other -> run
                if ((pid = Fork()) == 0){
                    getexecpath(name, argv[0]);

                    if (execve(name, argv, environ) < 0) {	//ex) /bin/ls ls -al &
                        printf("%s: Command not found.\n", argv[0]);
                        exit(1);
                    }
                }

                if (!bg){ 
                    int status;
                    Waitpid(pid, &status, 0);
                }
                else { //when there is backgrount process!
                    printf("%d %s", pid, cmdline);

                }
            }
            else if(builtin_condition == 2){
                builtin_condition = history_command(argv[0], cmdline);
            }
        }
    } while (builtin_condition == 2);

	/* Parent waits for foreground job to terminate */

    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "quit"))    /* quit command */
        exit(0);
    if (!strcmp(argv[0], "exit"))    /* exit command */
        exit(0);
    if (!strcmp(argv[0], "&"))       /* Ignore singleton & */
	    return 1;
    if (!strcmp(argv[0], "cd")){    /* change directory */
        if(argv[1] == NULL){
            if(chdir(getenv("HOME")))
                unix_error("cd HOME error");
        }
        else{
            if(chdir(argv[1]))
                printf("%s : directory not found\n", argv[1]);
        }

        return 1;
    }
    if (!strcmp(argv[0], "history")){ /* print command history */
        history();
        return 1;
    }
    if (argv[0][0] == '!'){
        return 2;
    }

    return 0;                        /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    char *temp;
    int argc;            /* Number of args */
    int bg;              /* Background job? */
    int condition;

    if((buf[strlen(buf)-1]) == '\n')
        buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	    buf++;

    condition = 0;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        if(*buf == '\"'){
            temp = ++buf;
            while((*temp) != '\"' && (*temp) != '\0'){
                temp++;
            }
            delim = temp;
        }
        if(*buf == '\''){
            temp = ++buf;
            while((*temp) != '\'' && (*temp) != '\0'){
                temp++;
            }
            delim = temp;
        }
	    argv[argc++] = buf;
	    *delim = '\0';
	    buf = delim + 1;
	    while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
	    return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	    argv[--argc] = NULL;

    return bg;
}
/* $end parseline */

/*  */
int token_pipe_command(char** piped_commands, char* cmdline){
    char *strtok_ptr;
    int pipe_idx;

    strtok_ptr = strtok(cmdline, "|");
    pipe_idx = 0;

    while(strtok_ptr != NULL){
        piped_commands[pipe_idx] = strtok_ptr;

        strtok_ptr = strtok(NULL, "|");
        
        pipe_idx++;

        if(pipe_idx > MAXPIPES){
            printf("max piping exceeded : ");
            return 0;
        }
    }

    piped_commands[pipe_idx] = NULL;

    return pipe_idx;
}

int run_pipe(char** piped_commands, const int num_piped_commands){
    int fd[2 * (num_piped_commands-1)];
    int status;
    pid_t pid;

    for(int i = 0; i < 2 * (num_piped_commands-1); i+=2)
        pipe(fd+i);

    for(int idx = 0; idx < num_piped_commands; idx++){
        run_child(fd, piped_commands[idx], idx, num_piped_commands);
    }

    for(int i = 0; i < 2 * (num_piped_commands-1); i++){
        close(fd[i]);
    }

    while(wait(&status) != -1);
}

void run_child(int *fd, const char* cmdline, const int idx, const int num_piped_commands){
    char *argv[MAXARGS];   /* Argument list execve() */
    char buf[MAXLINE];     /* Holds modified command line */
    char name[MAXLINE];    /* Holds name of program */
    pid_t pid;

    strcpy(buf, cmdline);

    parseline(buf, argv);

    if((pid = Fork()) == 0){
        getexecpath(name, argv[0]);

        if(idx != 0){
            Dup2(fd[idx*2 - 2], STDIN_FILENO);
        }
        if(idx != (num_piped_commands-1)){
            Dup2(fd[idx*2 + 1], STDOUT_FILENO);
        }

        for(int i = 0; i < 2 * (num_piped_commands-1); i++){
            close(fd[i]);
        }

        if (execvp(name, argv) < 0) {	//ex) /bin/ls ls -al &
            printf("%s: Command not found.\n", argv[0]);
            exit(1);
        }
    }  
}

int getexecpath(char* path_name, char* exec_name){
    char *path;
    char *token_ptr;

    const char *temp = getenv("PATH");

    if (temp != NULL){
        path =(char*)malloc(strlen(temp)+1);
        if(path == NULL){
            printf("getexecpath malloc fail\n");
            return 0;
        }
        else{
            strcpy(path, temp);
        }
    }

    token_ptr = strtok(path, ":");
    while(token_ptr != NULL){
        strcpy(path_name, token_ptr);
        strcat(path_name, "/");
        strcat(path_name, exec_name);

        if(!access(path_name, X_OK))
            break;

        token_ptr = strtok(NULL, ":");
    }

    free(path);

    if(token_ptr == NULL)
        return 0;

    return 1;
}