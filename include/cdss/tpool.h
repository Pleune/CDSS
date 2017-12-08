#ifndef CDSS_TPOOL_H
#define CDSS_TPOOL_H

typedef struct threadpool tpool_t;
typedef void (*tpool_work_t)(void *);

tpool_t *tpool_create(unsigned int threads);
void tpool_destroy(tpool_t *);
void tpool_add(tpool_t *, tpool_work_t, void *arg, int front);
void tpool_pause(tpool_t *);
void tpool_resume(tpool_t *);
void tpool_flush(tpool_t *);

#endif
