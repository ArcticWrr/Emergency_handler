#include <stdio.h>      // fopen, fclose, FILE, printf
#include <stdlib.h>     // malloc, free, exit
#include <string.h>     // strdup, strcmp, strtok, sscanf
#include <stdbool.h>    // bool, true, false

#include "../../include/priority_queue.h"

void priority_queue_init(priority_queue_t *pq) {
    pq->head = NULL;
    pq->tail = NULL;
    mtx_init(&pq->mutex, mtx_plain);
    cnd_init(&pq->cond);
}

void priority_queue_push(priority_queue_t *q, emergency_t item) {
    node_t *n = malloc(sizeof(node_t));
    n->data = item;
    n->next = NULL;

    mtx_lock(&q->mutex);

    if (!q->head || item.type.priority > q->head->data.type.priority) {
        // Inserisci in testa
        n->next = q->head;
        q->head = n;
    } else {
        // Cerca posizione
        node_t *curr = q->head;
        while (curr->next && curr->next->data.type.priority >= item.type.priority) {
            curr = curr->next;
        }
        n->next = curr->next;
        curr->next = n;
    }

    cnd_signal(&q->cond); // sveglia dispatcher
    mtx_unlock(&q->mutex);
}

int priority_queue_pop(priority_queue_t *q, emergency_t *out) {
    mtx_lock(&q->mutex);
    while (!q->head && !stop_flag) {
        cnd_wait(&q->cond, &q->mutex);
    }
    if (stop_flag && !q->head) {
        mtx_unlock(&q->mutex);
        return 0;
    }
    node_t *n = q->head;
    *out = n->data;
    q->head = n->next;
    free(n);
    mtx_unlock(&q->mutex);
    return 1;
}

void priority_queue_destroy(priority_queue_t *q) {

    node_t *curr = q->head;
    while (curr) {
        node_t *tmp = curr;
        curr = curr->next;
        free(tmp);
    }
    
    mtx_destroy(&q->mutex);
    cnd_destroy(&q->cond);
}
