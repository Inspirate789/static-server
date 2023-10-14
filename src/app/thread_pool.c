#include "../../inc/app/thread_pool.h"
#include "../../lib/log/log.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

pthread_mutex_t queue_mutex;
pthread_cond_t queue_cond;

struct thread_pool {
    size_t size;
    size_t capacity;
    thread_pool_task_t *tasks;
    pthread_t *threads;
};

int thread_pool_create(thread_pool_t *pool, size_t capacity) {
    if (pool == NULL) {
        log_error("thread pool pointer is NULL");
        return EXIT_FAILURE;
    } else if (*pool != NULL) {
        log_warn("thread pool is already initialized");
        return EXIT_FAILURE;
    }

    if (((*pool) = malloc(sizeof(struct thread_pool))) == NULL) {
        log_error("thread_pool_init malloc() thread_pool: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    (*pool)->size = 0;
    (*pool)->capacity = capacity;
    if (((*pool)->tasks = malloc(capacity * sizeof(thread_pool_task_t))) == NULL) { // TODO: tmp
        log_error("thread_pool_init malloc() thread_pool_task: %s", strerror(errno));
        free(*pool);
        return EXIT_FAILURE;
    }
    if (((*pool)->threads = malloc(capacity * sizeof(pthread_t))) == NULL) {
        log_error("thread_pool_init malloc() pthread_t: %s", strerror(errno));
        free((*pool)->tasks);
        free(*pool);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int thread_pool_start(thread_pool_t pool, void *(*worker_thread)(void *)) {
    if (pool == NULL) {
        log_error("pool pointer is NULL");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < pool->capacity; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            log_error("pthread_create(): %s", strerror(errno));
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int thread_pool_submit(thread_pool_t pool, thread_pool_task_t task) {
    if (pool == NULL) {
        log_error("pool pointer is NULL");
        return EXIT_FAILURE;
    }

    log_debug("try to put task to pool...");
    pthread_mutex_lock(&queue_mutex);

    log_debug("wait for place in pool to put task...");
    while (pool->size >= pool->capacity) {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    pool->tasks[pool->size++] = task;

    log_debug("task added to pool");
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

    return EXIT_SUCCESS;
}

int thread_pool_take_task(thread_pool_task_t *task, thread_pool_t pool) {
    if (pool == NULL) {
        log_error("pool pointer is NULL");
        return EXIT_FAILURE;
    }

    log_debug("try to take task from pool...");
    pthread_mutex_lock(&queue_mutex);

    log_debug("wait for any task pool...");
    while (pool->size == 0) {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    thread_pool_task_t t = pool->tasks[--pool->size];

    log_debug("task taken from pool");
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

    *task = t;

    return EXIT_SUCCESS;
}

int thread_pool_stop(thread_pool_t pool) {
    if (pool == NULL) {
        log_error("pool pointer is NULL");
        return EXIT_FAILURE;
    }

    log_info("stop threads in pool...");
    for (int i = 0; i < pool->capacity; i++) {
        int rc = EXIT_SUCCESS;
        log_debug("wait for thread %d...", i);
        if (pthread_join(pool->threads[i], (void **)&rc) != 0) {
            log_error("pthread_join(): %s", strerror(errno));
            return EXIT_FAILURE;
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
    free((*pool)->tasks);
    free(*pool);
    *pool = NULL;
}
