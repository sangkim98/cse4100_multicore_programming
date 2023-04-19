[system programming lecture]

-project 1 baseline

csapp.{c,h}
        CS:APP3e functions and additional functionalities of shellex

myshell.c
        Simple shell example

README

Phase 1: Run child process

1. Components
        csapp.h
        csapp.c
        Makefile
        myshell.c

2. Build:
        All the components have to be located in the same file in order for make compilation to work.
        After compilation, 'myshell' binary file is created, which launches the shell made for this project.

3. Functionalities:
        Functionalities required for phase1 specification are implemented.
        For more detail refer to the specification document.

        List: cd, ls, mkdir, rmdir, touch, cat, echo, history, exit

3-1. History: !!, !#

3-2. Note for history:
        For history, .shell_history file is created once 'myshell' binary file executes it's first instruction.
        .shell_history file is created in the same directory as 'myshell' binary.