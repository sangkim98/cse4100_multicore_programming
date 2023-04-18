/* $begin shellmain */
#include "csapp.h"
#include<errno.h>

/* Function prototypes */
int eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
int getexecpath(char* path_name, char* exec_name);
/* Redirection and Pipe Implementation */
int token_pipe_command(char** piped_commands, char* cmdline);
int run_pipe(char** piped_commands, const int num_piped_commands, int bg, sigset_t* prev_addr);
void run_child(int *fd, char* cmdline, const int idx, const int is_last_command, int bg);
void deleteAmperSand(char* cmdline);
char* getFirstNonSpace(char* cmdline);
int checkJobNumberFormat(char** argv);

void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigtstp_handler_shell(int sig);
void sigchld_handler(int sig);

job jobs[MAXPROCESSES+1];
volatile sig_atomic_t sig_pid;

int main()
{
    int is_hist;
    char cmdline[MAXLINE]; /* Command line */
    num_jobs = 0;

    set_shell_history_location();

    while (1) {
	/* Read */
        Signal(SIGINT, sigint_handler);
        Signal(SIGTSTP, sigtstp_handler_shell);

        printf("CSE4100-MP-P1> ");
        fgets(cmdline, MAXLINE, stdin);
        fflush(stdout);
        if (feof(stdin))
            exit(0);
        /* Evaluate */
        while(is_hist = eval(cmdline));
    }

    return 0;
}
/* $end shellmain */
  
/* $begin eval */
/* eval - Evaluate a command line */
int eval(char *cmdline) 
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
    sigset_t mask, prev;

    Sigemptyset(&mask);
    Sigaddset(&mask, SIGCHLD);

    Signal(SIGCHLD, sigchld_handler);

    builtin_condition = 0;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);

    if (argv[0] == NULL)  
        return 0;   /* Ignore empty lines */
    
    open_shell_history();
    add_command_to_history(cmdline);
    save_shell_history();
    if (!(builtin_condition = builtin_command(argv))) { //quit -> exit(0), & -> ignore, other -> run
        Sigprocmask(SIG_BLOCK, &mask, &prev);

        if(strpbrk(cmdline, "|") != NULL){

            strcpy(pipe_buf, cmdline);

            if((num_piped_commands = token_pipe_command(piped_commands, pipe_buf)) == 0){
                printf("pipe error\n");
                exit(1);
            }

            run_pipe(piped_commands, num_piped_commands, bg, &prev);

        }
        else{
            Signal(SIGTSTP, sigtstp_handler_shell);
            if ((pid = Fork()) == 0){
                Signal(SIGINT, SIG_DFL);
                Signal(SIGTSTP, SIG_DFL);

                if(!getexecpath(name, argv[0]))
                    strcpy(name, argv[0]);

                if (execve(name, argv, environ) < 0) {	//ex) /bin/ls ls -al &
                    printf("%s: Command not found.\n", argv[0]);
                    exit(1);
                }
            }
            sig_pid = 0;
            if (bg){
                deleteAmperSand(cmdline);
                cmdline = getFirstNonSpace(cmdline);
                addJob(jobs, pid, JOB_RUNNING, cmdline);
            }
            else{
                addJob(jobs, pid, JOB_FOREGROUND, cmdline);
                while(!sig_pid){
                    Sigsuspend(&prev);
                }
            }
        }
        Sigprocmask(SIG_SETMASK, &prev, NULL);
        Signal(SIGINT, SIG_IGN);
        Signal(SIGTSTP, SIG_IGN);
    }
    else if(builtin_condition == 2){
        history_command(argv[0], cmdline);
        return 1;
    }

	/* Parent waits for foreground job to terminate */

    return 0;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv)
{
    int jobID;

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
    if (!strcmp(argv[0], "jobs")){
        printAllJobs(jobs);
        return 1;
    }
    if (!strcmp(argv[0], "kill")){
        jobID = checkJobNumberFormat(argv);
        killJob(jobs, jobID);
        return 1;
    }
    if (!strcmp(argv[0], "bg")){
        jobID = checkJobNumberFormat(argv);
        bg(jobs, jobID);
        return 1;
    }
    if (!strcmp(argv[0], "fg")){
        jobID = checkJobNumberFormat(argv);
        fg(jobs, jobID);
        return 1;
    }

    return 0;
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
    
    temp = buf+strlen(buf)-1;
    while (*temp == ' '){
        temp--;
    }
    if(bg = (*temp == '&'))
        *temp = ' ';

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

int run_pipe(char** piped_commands, const int num_piped_commands, int bg, sigset_t *prev_addr){
    int fd[2 * (num_piped_commands-1)];
    int status;

    for(int i = 0; i < 2 * (num_piped_commands-1); i+=2)
        pipe(fd+i);

    for(int idx = 0; idx < num_piped_commands; idx++){
        run_child(fd, piped_commands[idx], idx, num_piped_commands, bg);
    }

    for(int i = 0; i < 2 * (num_piped_commands-1); i++){
        close(fd[i]);
    }

    if(!bg){
        while(!sig_pid)
            Sigsuspend(prev_addr);
    }
}

void run_child(int *fd, char* cmdline, const int idx, const int num_piped_commands, int bg){
    char *argv[MAXARGS];   /* Argument list execve() */
    char buf[MAXLINE];     /* Holds modified command line */
    char name[MAXLINE];    /* Holds name of program */
    pid_t pid;

    strcpy(buf, cmdline);

    if(parseline(buf, argv))
        printf("%s\n", cmdline+strlen(cmdline)-1);

    if((pid = Fork()) == 0){
        Signal(SIGINT, SIG_DFL);
        Signal(SIGTSTP, SIG_DFL);

        if(!getexecpath(name, argv[0]))
            strcpy(name, argv[0]);

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
    Signal(SIGINT, SIG_IGN);
    Signal(SIGTSTP, SIG_IGN);
    sig_pid = 0;
    
    if(bg){
        deleteAmperSand(cmdline);
        cmdline = getFirstNonSpace(cmdline);
        addJob(jobs, pid, JOB_RUNNING, cmdline);
    }
    else{
        addJob(jobs, pid, JOB_FOREGROUND, cmdline);
    }
}

int getexecpath(char* path_name, char* exec_name){
    char *path;
    char *token_ptr;

    const char *temp = getenv("PATH");

    strcpy(path_name, exec_name);

    if(!access(path_name, X_OK))
        return 0;

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

void sigchld_handler(int sig){
    int status, jobID;
    int en = errno;

	while ((sig_pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        jobID = findJobID(jobs, sig_pid);
        if(WIFEXITED(status)){
            if(jobID){
                if(jobs[jobID].state == JOB_FOREGROUND){
                    deleteForeGroundJob(jobs, jobID);
                }
                if(jobs[jobID].state == JOB_RUNNING){
                    deleteJob(jobs, jobID);
                }
            }
            break;
        }
        if(WIFSIGNALED(status)){
            if(WTERMSIG(status) == SIGINT)
                Sio_puts("\n");
            if(jobID){
                if(jobs[jobID].state == JOB_RUNNING)
                    continue;
            }
            deleteJob(jobs, jobID);
            break;
        }
		if(WIFSTOPPED(status)) {
			Sio_puts("[CTRL+Z]\n");
			if (jobID) {
				jobs[jobID].state = JOB_SUSPENDED;
			}
			break;
		}
	}

	errno = en;

    return;
}

void sigint_handler(int sig){
    Sio_puts("\n");
    Sio_puts("CSE4100-MP-P1> ");
    fflush(stdin);

    return;
}

void sigtstp_handler_shell(int sig){
    Sio_puts("\b\b  \b\b");

    fflush(stdin);

    return;
}

void deleteAmperSand(char* cmdline){
    int cmd_len, idx;

    idx = strlen(cmdline)-1;

    if(cmdline[idx] == '\n')
        idx--;

    while(cmdline[idx] == ' ' || cmdline[idx] == '&' || cmdline[idx] == '\n'){
        cmdline[idx--] = ' ';
    }

}

char* getFirstNonSpace(char* cmdline){
    int idx = 0;

    while(cmdline[idx] == ' '){
        idx++;
    }

    return cmdline+idx;
}

int checkJobNumberFormat(char** argv){
    int jobNum;

    if(argv[1][0] == '%'){
        if(jobNum = atoi(argv[1]+1))
            return jobNum;
    }

    return 0;
}