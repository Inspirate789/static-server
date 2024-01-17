#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "thread_pool.h"
#include "queue.h"
#include "log.h"

pthread_mutex_t queue_mutex;
pthread_cond_t queue_cond;

struct thread_pool {
    size_t size;
    queue_t tasks;
    pthread_t *threads;
};

int thread_pool_create(thread_pool_t *pool, size_t size) {
    thread_pool_t tmp_pool = malloc(sizeof(struct thread_pool));
    if (tmp_pool == NULL) {
        log_error("thread_pool_create malloc() thread_pool: %s", strerror(errno));
        return errno;
    }

    int rc;
    if ((rc = queue_create(&tmp_pool->tasks, size)) != EXIT_SUCCESS) {
        log_error("thread_pool_create queue_init(): %s", strerror(errno));
        free(tmp_pool);
        return rc;
    }

    if ((tmp_pool->threads = malloc(size * sizeof(pthread_t))) == NULL) {
        log_error("thread_pool_create malloc() pthread_t: %s", strerror(errno));
        free(tmp_pool->tasks);
        free(tmp_pool);
        return EXIT_FAILURE;
    }

    tmp_pool->size = size;
    *pool = tmp_pool;

    return EXIT_SUCCESS;
}

int thread_pool_start(thread_pool_t pool, void *(*worker_thread)(void *)) {
    for (int i = 0; i < pool->size; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            log_error("pthread_create(): %s", strerror(errno));
            return errno;
        }
    }

    return EXIT_SUCCESS;
}

int thread_pool_submit(thread_pool_t pool, thread_pool_task_t task) {
    log_debug("try to put task to pool...");
    pthread_mutex_lock(&queue_mutex);

    log_debug("wait for place in pool to put task...");
    while (queue_full(pool->tasks)) {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    int rc;
    if ((rc = queue_push(pool->tasks, task)) != EXIT_SUCCESS) {
        log_error("queue_push(): %d", rc);
        return rc;
    }

    log_debug("task added to pool");
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

    return EXIT_SUCCESS;
}

int thread_pool_take_task(thread_pool_task_t *task, thread_pool_t pool) {
    log_debug("try to take task from pool...");
    pthread_mutex_lock(&queue_mutex);

    log_debug("wait for any task pool...");
    while (queue_empty(pool->tasks)) {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    thread_pool_task_t tmp_task;
    int rc;
    if ((rc = queue_pop(pool->tasks, &tmp_task)) != EXIT_SUCCESS) {
        log_error("queue_push(): %d", rc);
        return rc;
    }

    log_debug("task taken from pool");
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

    *task = tmp_task;

    return EXIT_SUCCESS;
}

int thread_pool_stop(thread_pool_t pool) {
    log_info("stop threads in pool...");
    for (int i = 0; i < pool->size; i++) {
        int rc = EXIT_SUCCESS;
        log_debug("wait for thread %d...", i);
        if (pthread_join(pool->threads[i], (void **)&rc) != 0) {
            log_error("pthread_join(): %s", strerror(errno));
            return errno;
        }
        if (rc != EXIT_SUCCESS) {
            log_warn("error: thread from pool exited with code %d", rc);
        } else {
            log_debug("thread from pool exited with code %d", rc);
        }
    }
    log_info("thread pool stopped");

    return EXIT_SUCCESS;
}

void thread_pool_destroy(thread_pool_t *pool) {
    if (pool == NULL || *pool == NULL) {
        return;
    }
    free((*pool)->threads);
    queue_destroy(&(*pool)->tasks);
    free(*pool);
    *pool = NULL;
}
