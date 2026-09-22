#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <threads.h>
#include <signal.h>
#include <stdatomic.h>
#include <errno.h>
#include <stdbool.h>

#include "../../include/structures.h"
#include "../../include/macro.h"
#include "../../include/logger.h"
#include "../../include/parse_emergency_types.h"
#include "../../include/parse_env.h"
#include "../../include/digital_twins.h"
#include "../../include/priority_queue.h"
#include "../../include/threads.h"
#include "../../include/support.h"

#define max_message_size 1024

atomic_bool stop_flag = false;
priority_queue_t internal_queue;

void sigint_handler(int sig) {
    (void)sig;
    stop_flag = true;
    cnd_broadcast(&internal_queue.cond); // sveglia dispatcher bloccati
    printf("\n[SERVER] Ricevuto SIGINT, chiusura...\n");
}

int main() {

    // 1. Parsing
    int num_rescuer_types, num_emergency_types;
    rescuer_type_t* rescuer_types = parse_rescuers("conf/rescues.conf", &num_rescuer_types); // crea un array di soccorritori
    emergency_type_t* emergency_types = parse_emergency_types("conf/emergency_types.conf", &num_emergency_types, rescuer_types, num_rescuer_types); // crea un array di emergenze
    env_t* env = parse_env("conf/env.conf"); // parsing ambiente

    // Ricezione messaggi tramite mq

    signal(SIGINT, sigint_handler);
    priority_queue_init(&internal_queue);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(emergency_request_t);
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(env->queue_name, O_CREAT | O_RDONLY | O_NONBLOCK, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("Errore apertura coda");
        exit(EXIT_FAILURE);
    } 

    cnd_t empty;
    cnd_init(&empty);

    receiver_args_t receiver_args = {
        .mq = mq,
        .num_emergency_types = num_emergency_types,
        .emergency_types = emergency_types,
        .grid_height = env->grid_height,
        .grid_width = env->grid_width,
        .tot_num_rescuers = num_rescuer_types,
        .rescuer_types = rescuer_types
    };

    dispatcher_args_t dispatcher_args = {
        .tot_num_rescuers = num_rescuer_types,
        .active_emergency = NULL,
        .empty = &empty
    };

    printf("[SERVER] Avviato. In attesa su %s (Ctrl+C per uscire)...\n", env->queue_name);

    thrd_t t_recv, t_disp;
    thrd_create(&t_recv, receiver_thread, &receiver_args);
    thrd_create(&t_disp, dispatcher_thread, &dispatcher_args);
    
    thrd_join(t_recv, NULL);
    thrd_join(t_disp, NULL);
    
    mq_close(mq);
    mq_unlink(env->queue_name);
    priority_queue_destroy(&internal_queue);

    free(rescuer_types);
    free(emergency_types);
    free(env);

    cnd_destroy(&empty);

    printf("[SERVER] Arrestato correttamente.\n");
    return 0;
}
