#include "cdss.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define BUFF_SIZE 8192
#define MAX_MSG_LEN 256

struct message {
    enum plog_level level;
    size_t len;
};

static pthread_mutex_t lock;
static pthread_cond_t notempty;
static pthread_cond_t notfull;
static pthread_t thread;

static int initalized = 0;
static ringbuff_t *msg_buff;

static enum plog_level logger_level = L_INFO;
static FILE *output[2] = {0};

static void *
worker(void *v)
{
    char string[MAX_MSG_LEN];

    while(1)
    {
        pthread_mutex_lock(&lock);

        while(ringbuff_empty(msg_buff))
            pthread_cond_wait(&notempty, &lock);

        struct message m;
        ringbuff_remove(msg_buff, &m, sizeof(m));
        ringbuff_remove(msg_buff, string, m.len);
        string[m.len] = 0;

        pthread_mutex_unlock(&lock);

        size_t i;

        if(m.level >= logger_level)
        for(i=0; i<sizeof(output)/sizeof(output[0]); i++)
        if(output[i])
        switch(m.level)
        {
        case L_DEBUG:
            fprintf(output[i], "[\033[34mDEBUG\033[0m] %s\r\n", string);
            break;
        case L_INFO:
            fprintf(output[i], "[\033[32mINFO \033[0m] %s\r\n", string);
            break;
        case L_WARN:
            fprintf(output[i], "[\033[33mWARN \033[0m] %s\r\n", string);
            break;
        case L_ERROR:
            fprintf(output[i], "[\033[31mERROR\033[0m] %s\r\n", string);
            break;
        case L_FATAL:
            fprintf(output[i], "[\033[35mFATAL\033[0m] %s\r\n", string);
            fflush(output[i]);
            break;
        default:
            fprintf(output[i], "[\033[35mINVALID\033[0m] %s\r\n", string);
            break;
        }

        pthread_mutex_lock(&lock);
        pthread_cond_signal(&notfull);
        pthread_mutex_unlock(&lock);

        if(m.level == L_FATAL)
            exit(1);
    }

    return 0;
}

static void
initalize()
{
    if(output[0] == 0)
        output[0] = stdout;

    pthread_mutex_init(&lock, 0);
    pthread_cond_init(&notempty, 0);
    pthread_cond_init(&notfull, 0);

    msg_buff = ringbuff_create(BUFF_SIZE);

    pthread_create(&thread, 0, &worker, 0);

    initalized = 1;
}

void
plog(enum plog_level l, const char *msg, ...)
{
    if(!initalized)
        initalize();

    va_list args;
    struct message m;
    char string[MAX_MSG_LEN];

    va_start(args, msg);
    vsnprintf(string, MAX_MSG_LEN, msg, args);
    va_end(args);

    m.level = l;
    m.len = strlen(string);

    pthread_mutex_lock(&lock);

    while(sizeof(m) + m.len > ringbuff_remaining(msg_buff))
        pthread_cond_wait(&notfull, &lock);

    ringbuff_put(msg_buff, &m, sizeof(m));
    ringbuff_put(msg_buff, string, m.len);

    pthread_cond_signal(&notempty);
    pthread_mutex_unlock(&lock);
}

void
plog_set_level(enum plog_level l)
{
    logger_level = l;
}

void
plog_set_stream(enum plog_stream stream, FILE *f)
{
    if(stream <= 1)
        output[stream] = f;
    else
        plog(L_ERROR, "plog_set_stream(): invalid stream %i", stream);
}

void
plog_flush()
{
    if(!initalized)
        return;

    pthread_mutex_lock(&lock);
    while(!ringbuff_empty(msg_buff))
        pthread_cond_wait(&notfull, &lock);
    pthread_mutex_unlock(&lock);

    size_t i;
    for(i=0; i<sizeof(output)/sizeof(output[0]); i++)
    if(output[i])
        fflush(output[i]);
}
