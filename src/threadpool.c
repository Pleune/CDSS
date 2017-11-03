#include "cdss.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

struct work_queue {
    struct work_queue *next;

    tpool_work_t work;
    void *arg;

    int used;
};

struct queue_pool {
    struct work_queue slot[4096];

    struct queue_pool *next;
};

struct threadpool {
    unsigned int num_threads;
    int stop;
    int paused;

    pthread_t threads[256];
    struct work_queue *queue;

    //protects queue
    pthread_mutex_t mut;

    //used to notify worker() of new work
    pthread_cond_t cond;
    pthread_mutex_t cond_mut;

    //used to notify tpool_flush() of work being finished
    pthread_cond_t flush;
    pthread_mutex_t flush_mut;
};

static struct queue_pool pool = {{{0, 0, 0, 0}}, 0};

static void *
worker(void *arg)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
    tpool_t *data = arg;

    while(data->stop == 0)
    {
        if(data->queue == 0)
            while((data->paused || !data->queue) && !data->stop)
            {
                pthread_mutex_lock(&data->cond_mut);
                pthread_cond_wait(&data->cond, &data->cond_mut);
                pthread_mutex_unlock(&data->cond_mut);
            }

        if(data->stop)
            break;

        tpool_work_t func = 0;
        void *arg;

        pthread_mutex_lock(&data->mut);
        if(data->queue != 0)
        {
            func = data->queue->work;
            arg = data->queue->arg;
            data->queue->used = 0;
            data->queue = data->queue->next;
        }
        pthread_mutex_unlock(&data->mut);

        if(func != 0)
            func(arg);

        pthread_mutex_lock(&data->flush_mut);
        pthread_cond_broadcast(&data->flush);
        pthread_mutex_unlock(&data->flush_mut);
    }

    return 0;
}

tpool_t *
tpool_create(unsigned int threads)
{
    assert(threads > 0 && threads <= 256);

    tpool_t *ret = malloc(sizeof(struct threadpool));

    if(ret == 0)
        return 0;

    ret->stop = 0;
    ret->queue = 0;
    ret->paused = 0;

    ret->num_threads = threads;

    int status;

    status = pthread_mutex_init(&ret->mut, 0);
    if(status != 0)
    {
        free(ret);
        return 0;
    }

    status = pthread_cond_init(&ret->cond, 0);
    if(status != 0)
    {
        free(ret);
        pthread_mutex_destroy(&ret->mut);
        return 0;
    }

    status = pthread_mutex_init(&ret->cond_mut, 0);
    if(status != 0)
    {
        free(ret);
        pthread_mutex_destroy(&ret->mut);
        pthread_cond_destroy(&ret->cond);
        return 0;
    }

    status = pthread_cond_init(&ret->flush, 0);
    if(status != 0)
    {
        free(ret);
        pthread_mutex_destroy(&ret->mut);
        pthread_cond_destroy(&ret->cond);
        pthread_mutex_destroy(&ret->cond_mut);
        return 0;
    }

    status = pthread_mutex_init(&ret->flush_mut, 0);
    if(status != 0)
    {
        free(ret);
        pthread_mutex_destroy(&ret->mut);
        pthread_cond_destroy(&ret->cond);
        pthread_mutex_destroy(&ret->cond_mut);
        pthread_cond_destroy(&ret->flush);
        return 0;
    }


    size_t i;
    for(i=0; i<threads; i++)
    {
        int status = pthread_create(&ret->threads[i], 0, &worker, ret);
        if(status != 0)
        {
            size_t j;
            for(j=0; j<i; j++)
                pthread_cancel(ret->threads[i]);

            free(ret);
            pthread_mutex_destroy(&ret->mut);
            pthread_cond_destroy(&ret->cond);
            pthread_mutex_destroy(&ret->cond_mut);
            pthread_cond_destroy(&ret->flush);
            pthread_mutex_destroy(&ret->flush_mut);
            return 0;
        }
    }

    return ret;
}

void
tpool_destroy(tpool_t *t)
{
    t->stop = 1;

    //wake threads to let them stop
    pthread_mutex_lock(&t->cond_mut);
    pthread_cond_broadcast(&t->cond);
    pthread_mutex_unlock(&t->cond_mut);

    size_t i;
    for(i=0; i<t->num_threads; i++)
        pthread_join(t->threads[i], 0);

    pthread_mutex_destroy(&t->mut);

    pthread_cond_destroy(&t->cond);
    pthread_mutex_destroy(&t->cond_mut);

    pthread_cond_destroy(&t->flush);
    pthread_mutex_destroy(&t->flush_mut);

    if(t->queue != 0)
    {
        struct work_queue *q = t->queue;

        while(q != 0)
        {
            q->used = 0;
            q = q->next;
        }
    }

    free(t);
}

void
tpool_add(tpool_t *t, tpool_work_t work, void *arg, int front)
{
    //Find queue obj
    struct queue_pool *p_ptr = &pool;
    size_t i = 0;

    while(p_ptr->slot[i].used != 0)
    {
        i++;

        if(i == 4096)
        {
            i = 0;
            if(p_ptr->next == 0)
            {
                p_ptr->next = malloc(sizeof(struct queue_pool));
                memset(p_ptr->next, 0, sizeof(struct queue_pool));
            }

            p_ptr = p_ptr->next;
        }
    }

    struct work_queue *queue_node = &(p_ptr->slot[i]);

    //Lock mutex while working with queue
    pthread_mutex_lock(&t->mut);

    if(front)
    {
        queue_node->next = t->queue;
        t->queue = queue_node;
    } else {
        struct work_queue **work_queue_place = &(t->queue);
        while(*work_queue_place != 0)
            work_queue_place = &((*work_queue_place)->next);
        *work_queue_place = queue_node;

        queue_node->next = 0;
    }

    queue_node->work = work;
    queue_node->arg = arg;
    queue_node->used = 1;

    pthread_mutex_unlock(&t->mut);

    pthread_mutex_lock(&t->cond_mut);
    pthread_cond_broadcast(&t->cond);
    pthread_mutex_unlock(&t->cond_mut);
}

void
tpool_pause(tpool_t *t)
{
    t->paused = 1;
}

void
tpool_resume(tpool_t *t)
{
    t->paused = 0;

    //notify threads of resume
    pthread_mutex_lock(&t->cond_mut);
    pthread_cond_broadcast(&t->cond);
    pthread_mutex_unlock(&t->cond_mut);
}

void
tpool_flush(tpool_t *t)
{
    while(t->queue != 0)//single read needs no mutex lock.
    {
        //wait for update
        pthread_mutex_lock(&t->flush_mut);
        pthread_cond_wait(&t->flush, &t->flush_mut);
        pthread_mutex_unlock(&t->flush_mut);
    }
}
