#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include "threadpool.h"
#include "ringbuff.h"
#include "logger.h"
#include "mpool_grow.h"
#include "mpool_static.h"
#include "mpool_dynamic.h"
#include "voxtree.h"
#include "stack.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef int (*work_t)(void);

typedef struct {
    work_t func;
    const char *name;
    int seconds;
    int expected_bad_return;
    int expected_signal;
} test_t;

test_t test_list[] = {
    {&test_tpool_basic,           "Thread Pool - Basic",           1, 0, 0},
    {&test_tpool_max_threads,     "Thread Pool - Max Threads",     1, 0, 0},
    {&test_tpool_invalid_threads, "Thread Pool - Invalid Threads", 1, 0, SIGABRT},
    {&test_ringbuff_basic,        "Ring Buffer",                   1, 0, 0},
    {&test_logger_basic,          "Logger",                        1, 1, 0},
    {&test_mpool_grow_basic,      "Mem Pool (Grow)",               1, 0, 0},
    {&test_mpool_static_basic,    "Mem Pool (Static)",             1, 0, 0},
    {&test_mpool_dynamic_basic,   "Mem Pool (Dynamic)",            9, 0, 0},
    {&test_voxtree_basic,         "Voxtree - Basic",               9, 0, 0},
    {&test_voxtree_noise,         "Voxtree - Noise",               9, 0, 0},
    {&test_voxtree_sphere,        "Voxtree - Sphere",              90, 0, 0},
    {&test_stack_basic,           "Stack - Basic",                 1, 0, 0},
    {&test_stack_advanced,        "Stack - Advanced",              1, 0, 0}
};

int
runtest(test_t test, int log)
{
    pid_t f = fork();

    if(f == 0)
    {//child
        if(log)
        {
            dup2(log, 1);
            dup2(log, 2);
        }

        alarm(test.seconds);

        clock_t before = clock();
        int ret = test.func();
        clock_t after = clock();

        printf("Finished test \"%s\" in %f seconds.\r\n", test.name, (float)(after-before)/CLOCKS_PER_SEC);

        exit(ret);
    } else {
        int status;
        waitpid(f, &status, 0);

        if(WIFSIGNALED(status))
        {
            if(WTERMSIG(status) == test.expected_signal)
                return 0;
            else if(WTERMSIG(status) == SIGALRM)
                return 99; //TIMEOUT
            else if(test.expected_signal != 0)
                return 98; //WRONG SIGNAL
            else
                return 97; //UNEXPECTED SIGNAL
        } else if(test.expected_signal != 0)
        {
            return 96; //EXPECTED SIGNAL
        } else {
            return test.expected_bad_return ?
                !WEXITSTATUS(status) :
                WEXITSTATUS(status);
        }
    }
}

int
main(int argc, char **argv)
{
    int i;
    int exec_test = -1;
    for(i=0; i<argc; i++)
    {
        if(strncmp(argv[i], "-t", MIN(2, strlen(argv[i]))) == 0)
        {
            argv[i] += 2;
            if(strlen(argv[i]) >= 1)
                exec_test = atoi(argv[i]);
        }
    }

    if(exec_test >= 0 && exec_test < (int)(sizeof(test_list)/sizeof(test_list[0])))
        return test_list[exec_test].func();

    int log_fd = open("test.log",
                      O_RDWR | O_TRUNC | O_CREAT,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    const int num_tests = sizeof(test_list)/sizeof(test_list[0]);
    int num_failed = 0;

    for(i=0; i<num_tests; i++)
    {
        printf("[\033[32mSTARTING\033[0m] %i %s...    ", (int) i, test_list[i].name);
        fflush(stdout);

        int status = runtest(test_list[i],
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
