#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "thread_pool.h"
#include "log.h"

struct thread_pool {
    size_t size;
    size_t capacity;
    thread_pool_task_t *tasks;
    pthread_t *threads;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
};

int thread_pool_create(thread_pool_t *pool, size_t capacity) {
    thread_pool_t tmp_pool = malloc(sizeof(struct thread_pool));
    if (tmp_pool == NULL) {
        log_error("thread_pool_init malloc() thread_pool: %s", strerror(errno));
        return errno;
    }

    tmp_pool->size = 0;
    tmp_pool->capacity = capacity;
    if ((tmp_pool->tasks = malloc(capacity * sizeof(thread_pool_task_t))) == NULL) {
        log_error("thread_pool_init malloc() thread_pool_task: %s", strerror(errno));
        free(tmp_pool);
        return errno;
    }
    if ((tmp_pool->threads = malloc(capacity * sizeof(pthread_t))) == NULL) {
        log_error("thread_pool_init malloc() pthread_t: %s", strerror(errno));
        free(tmp_pool->tasks);
        free(tmp_pool);
        return errno;
    }

    int rc;
    if ((rc = pthread_mutex_init(&tmp_pool->queue_mutex, NULL)) != 0) {
        log_error("thread_pool_init pthread_mutex_init(): %s", strerror(rc));
        free(tmp_pool->threads);
        free(tmp_pool->tasks);
        free(tmp_pool);
        return rc;
    }
    if ((rc = pthread_cond_init(&tmp_pool->queue_cond, NULL)) != 0) {
        log_error("thread_pool_init pthread_cond_init(): %s", strerror(rc));
        free(tmp_pool->threads);
        free(tmp_pool->tasks);
        free(tmp_pool);
        return rc;
    }

    *pool = tmp_pool;

    return EXIT_SUCCESS;
}

int thread_pool_start(thread_pool_t pool, void *(*worker_thread)(void *)) {
    for (size_t i = 0; i < pool->capacity; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            log_error("pthread_create(): %s", strerror(errno));
            return errno;
        }
    }

    return EXIT_SUCCESS;
}

int thread_pool_submit(thread_pool_t pool, thread_pool_task_t task) {
    log_debug("try to put task to pool...");
    pthread_mutex_lock(&pool->queue_mutex);

    log_debug("wait for place in pool to put task...");
    while (pool->size >= pool->capacity) {
        pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
    }

    pool->tasks[pool->size++] = task;

    log_debug("task added to pool");
    pthread_cond_signal(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);

    return EXIT_SUCCESS;
}

int thread_pool_take_task(thread_pool_task_t *task, thread_pool_t pool) {
    log_debug("try to take task from pool...");
    pthread_mutex_lock(&pool->queue_mutex);

    log_debug("wait for any task pool...");
    while (pool->size == 0) {
        pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
    }

    thread_pool_task_t t = pool->tasks[--pool->size];

    log_debug("task taken from pool");
    pthread_cond_signal(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);

    *task = t;

    return EXIT_SUCCESS;
}

void thread_pool_cleanup_handler(void *pool) {
    pthread_mutex_unlock(&((thread_pool_t)pool)->queue_mutex);
}

int thread_pool_stop(thread_pool_t pool) {
    int rc;
    log_info("stop threads in pool...");
    for (size_t i = 0; i < pool->capacity; i++) {
        log_debug("send cancellation request to thread %d...", i);
        if ((rc = pthread_cancel(pool->threads[i])) != 0) {
            log_error("pthread_cancel(): %s", strerror(rc));
            return rc;
        }
    }
    for (size_t i = 0; i < pool->capacity; i++) {
        void *thread_rc;
        log_debug("wait for thread %d...", i);
        if ((rc = pthread_join(pool->threads[i], &thread_rc)) != 0) {
            log_error("pthread_join(): %s", strerror(rc));
            continue; //return rc;
        }
        if (thread_rc != PTHREAD_CANCELED) {
            log_warn("error: thread %lu exited with code %d", i, thread_rc);
        } else {
            log_debug("thread %lu was canceled", i);
        }
    }
    log_info("thread pool stopped");

    return EXIT_SUCCESS;
}

void thread_pool_destroy(thread_pool_t *pool) {
    if (pool == NULL || *pool == NULL) {
        return;
    }
    int rc;
    if ((rc = pthread_cond_destroy(&(*pool)->queue_cond)) != 0) {
        log_error("pthread_cond_destroy(): %s", strerror(rc));
    }
    if ((rc = pthread_mutex_destroy(&(*pool)->queue_mutex)) != 0) {
        log_error("pthread_mutex_destroy(): %s", strerror(rc));
    }
    free((*pool)->threads);
    free((*pool)->tasks);
    free(*pool);
    *pool = NULL;
}
