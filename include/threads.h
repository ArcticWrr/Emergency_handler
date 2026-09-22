// threads.h
#ifndef THREADS_H
#define THREADS_H
#include "structures.h"
#include "macro.h"
#include "logger.h"
#include "other_functions.h"
#include "digital_twins.h"
#include "priority_queue.h"
#include "macro.h"
#include "support.h"

//extern rescuer_type_t* rescuer_types;
//extern emergency_type_t* emergency_types;

int receiver_thread(void *arg);
int dispatcher_thread(void *arg);

#endif
