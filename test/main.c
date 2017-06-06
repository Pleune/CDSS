#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "threadpool.h"

typedef int (*work_t)(void);

typedef struct {
    work_t func;
    const char *name;
    int seconds;
    int expected_signal;
} test_t;

test_t test_list[] = {
    {&test_tpool_basic,           "Thread Pool - Basic",           1, 0},
    {&test_tpool_max_threads,     "Thread Pool - Max Threads",     1, 0},
    {&test_tpool_invalid_threads, "Thread Pool - Invalid Threads", 1, SIGABRT}
};

int
runtest(work_t func, int seconds, int expected_signal, int log)
{
    pid_t f = fork();

    if(f == 0)
    {//child
        if(log)
        {
            dup2(log, 1);
            dup2(log, 2);
        }

        alarm(seconds);
        exit(func());
    } else {
        int status;
        waitpid(f, &status, 0);

        if(WIFSIGNALED(status))
        {
            if(WTERMSIG(status) == expected_signal)
                return 0;
            else if(WTERMSIG(status) == SIGALRM)
                return 99; //TIMEOUT
            else if(expected_signal != 0)
                return 98; //WRONG SIGNAL
            else
                return 97; //UNEXPECTED SIGNAL
        } else if(expected_signal != 0)
        {
            return 96; //EXPECTED SIGNAL
        } else {
            return WEXITSTATUS(status);
        }
    }
}

int
main(int argc, char **argv)
{
    int log_fd = open("test.log",
                      O_RDWR | O_TRUNC | O_CREAT,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    const int num_tests = sizeof(test_list)/sizeof(test_list[0]);
    int num_failed = 0;

    int i;
    for(i=0; i<num_tests; i++)
    {
        printf("[\033[32mSTARTING\033[0m] %s...    ", test_list[i].name);
        fflush(stdout);

        int status = runtest(test_list[i].func,
                             test_list[i].seconds,
                             test_list[i].expected_signal,
                             log_fd);

        const char *message;
        int fail = 1;

        switch(status)
        {
        case 0:
            message = "\033[32mSuccess\033[0m";
            fail = 0;
            break;
        case 1:
            message = "\033[31mGeneral failure\033[0m";
            break;
        case 96:
            message = "\033[31mExpected signal\033[0m";
            break;
        case 97:
            message = "\033[31mUnexpected signal\033[0m";
            break;
        case 98:
            message = "\033[31mWrong signal\033[0m";
            break;
        case 99:
            message = "\033[31mTimeout\033[0m";
            break;
        default:
            message = "\033[31mInternal test suite error\033[0m";
            break;
        }

        num_failed += fail;

        printf("%s\r\n", message);
    }

    printf("%i tests run. %i success. %i fail\r\n", i, i-num_failed, num_failed);

    if(num_failed > 0)
        return 1;
    else
        return 0;
}
