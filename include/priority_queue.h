// priority_queue.h
#include <stdatomic.h>
#include <stdbool.h>

#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H
#include "structures.h"
#include "macro.h"
#include "logger.h"


extern atomic_bool stop_flag; // Flag per terminare i thread
extern priority_queue_t internal_queue; // Coda per le richieste di emergenza

void priority_queue_init(priority_queue_t *queue);
void priority_queue_destroy(priority_queue_t *queue);
void priority_queue_push(priority_queue_t *queue, emergency_t emergency);
int priority_queue_pop(priority_queue_t *queue, emergency_t *emergency);

#endif