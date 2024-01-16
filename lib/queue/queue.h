#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_EMPTY (-3)

typedef struct queue *queue_t;

void queue_init(queue_t *queue);
int queue_push(queue_t queue, void *value);
int queue_pop(queue_t queue, void **value);
void queue_destroy(queue_t *queue);

#endif //QUEUE_H
