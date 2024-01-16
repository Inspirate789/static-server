#include <stdlib.h>
#include <errno.h>
#include "queue.h"

typedef struct node {
    void *value;
    struct node *next;
} node_t;

struct queue {
    node_t *head;
    node_t *tail;
};

void queue_init(queue_t *queue) {
    (*queue)->head = NULL;
    (*queue)->tail = NULL;
}

int queue_push(queue_t queue, void *value) {
    node_t *new_node = malloc(sizeof(node_t));
    if (new_node == NULL) {
        return errno;
    }

    new_node->value = value;
    new_node->next = NULL;

    if (queue->head == NULL) {
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }

    return EXIT_SUCCESS;
}

int queue_pop(queue_t queue, void **value) {
    if (queue->head == NULL) {
        return QUEUE_EMPTY;
    }

    *value = queue->head->value;
    node_t *tmp_node = queue->head->next;
    free(queue->head);
    queue->head = tmp_node;

    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    return EXIT_SUCCESS;
}

void queue_destroy(queue_t *queue) {
    int rc;
    void *value;
    do {
        rc = queue_pop((*queue), &value);
    } while (rc != QUEUE_EMPTY);
}
