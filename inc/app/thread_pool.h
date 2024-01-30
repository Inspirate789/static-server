#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stdlib.h>

typedef void *thread_pool_task_t;
typedef struct thread_pool *thread_pool_t;

int thread_pool_create(thread_pool_t *pool, size_t capacity);
int thread_pool_start(thread_pool_t pool, void *(*worker_thread)(void *));
int thread_pool_submit(thread_pool_t pool, thread_pool_task_t task);
int thread_pool_take_task(thread_pool_task_t *task, thread_pool_t pool);
void thread_pool_cleanup_handler(void *pool);
int thread_pool_stop(thread_pool_t pool);
void thread_pool_destroy(thread_pool_t *pool);

#endif //THREAD_POOL_H
