#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "queue.h"

#define DEFAULT_INDEX -1

struct queue {
    void **buffer;
    size_t size;
    ssize_t front, rear;
    bool full, empty;
    pthread_mutex_t lock;
};

int queue_create(queue_t *queue, size_t size) {
    queue_t tmp_queue = malloc(sizeof(struct queue));
    if (tmp_queue == NULL) {
        return errno;
    }

    if ((tmp_queue->buffer = calloc(size, sizeof(void *))) == NULL) {
        free(tmp_queue);
        return errno;
    }

    int rc;
    if ((rc = pthread_mutex_init(&tmp_queue->lock, NULL)) != EXIT_SUCCESS) {
        free(tmp_queue->buffer);
        free(tmp_queue);
        return rc;
    }

    tmp_queue->size = size;
    tmp_queue->front = DEFAULT_INDEX;
    tmp_queue->rear = DEFAULT_INDEX;
    tmp_queue->full = false;
    tmp_queue->empty = true;
    *queue = tmp_queue;

    return EXIT_SUCCESS;
}

bool queue_full(queue_t queue) {
    pthread_mutex_lock(&queue->lock);
    bool is_full = queue->full;
    pthread_mutex_unlock(&queue->lock);
    return is_full;
}

bool queue_empty(queue_t queue) {
    pthread_mutex_lock(&queue->lock);
    bool is_empty = queue->empty;
    pthread_mutex_unlock(&queue->lock);
    return is_empty;
}

int queue_push(queue_t queue, void *value) {
    pthread_mutex_lock(&queue->lock);
    if (queue->full) {
        return QUEUE_FULL;
    }
    if (queue->empty) {
        queue->front = 0;
        queue->empty = false;
    }
    queue->rear = (queue->rear + 1) % queue->size;
    queue->buffer[queue->rear] = value;
    if (queue->front == queue->rear) {
        queue->full = true;
    }
    pthread_mutex_unlock(&queue->lock);

    return EXIT_SUCCESS;
}

int queue_pop(queue_t queue, void **value) {
    pthread_mutex_lock(&queue->lock);
    if (queue->empty) {
        return QUEUE_EMPTY;
    }
    *value = queue->buffer[queue->front];
    if (queue->full) {
        queue->full = false;
    }
    queue->front = (queue->front + 1) % queue->size;
    if (queue->front == queue->rear) {
        queue->empty = true;
    }
    pthread_mutex_unlock(&queue->lock);

    return EXIT_SUCCESS;
}

int queue_destroy(queue_t *queue) {
    if (queue == NULL || *queue == NULL) {
        return EXIT_SUCCESS;
    }

    int rc;
    if ((rc = pthread_mutex_destroy(&(*queue)->lock)) != EXIT_SUCCESS) {
        return rc;
    }

    free((*queue)->buffer);
    free(*queue);

    return EXIT_SUCCESS;
}
