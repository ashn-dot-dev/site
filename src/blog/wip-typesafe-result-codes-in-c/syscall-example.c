#define _GNU_SOURCE

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/syscall.h>


int
main(void)
{
    long res;

    #define HELLO "Hello, World!\n"
    res = syscall(SYS_write, STDOUT_FILENO, HELLO, sizeof(HELLO)-1);
    if (res < 0) {
        perror(NULL);
        exit(EXIT_FAILURE);
    }

    #define GOODBY "Goodby, World!\n"
    res = syscall(SYS_write, 0xBAD, GOODBY, sizeof(GOODBY)-1);
    if (res < 0) {
        perror(NULL);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

