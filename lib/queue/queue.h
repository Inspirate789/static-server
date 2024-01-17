#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

#define QUEUE_EMPTY (-3)
#define QUEUE_FULL  (-4)

typedef struct queue *queue_t;

int queue_create(queue_t *queue, size_t size);
bool queue_full(queue_t queue);
bool queue_empty(queue_t queue);
int queue_push(queue_t queue, void *value);
int queue_pop(queue_t queue, void **value);
int queue_destroy(queue_t *queue);

#endif //QUEUE_H
