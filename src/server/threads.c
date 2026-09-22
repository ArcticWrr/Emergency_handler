#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>

#include "../../include/threads.h"

int receiver_thread(void *arg) { //aggiunge a coda priorità richieste valide

    receiver_args_t *params = (receiver_args_t *)arg;
    mqd_t mq = params->mq;
    int num_rescuer_types = params->tot_num_rescuers;
    rescuer_type_t *rescuer_types = params->rescuer_types;
    rescuer_digital_twin_t **emergency_digital_twins = NULL;
    int id = 0; // ID univoco per ogni emergenza

    while (!stop_flag) {
        emergency_request_t request;
        emergency_t valid_emergency;
        ssize_t bytes = mq_receive(mq, (char*)&request, sizeof(request), NULL);
        int curr_element = 0; //modificata in validate_name
        char buffer[MAX_MESSAGE_LENGTH]; //buffer per messaggi errore
        if (bytes >= 0) {
            id++;
            printf("[RECEIVER] Ricevuta: %s (%d,%d)\n",
                   request.emergency_name, request.x, request.y);
            if (validate_name(request.emergency_name, params->emergency_types, params->num_emergency_types, &curr_element) && validate_timestamp(request.timestamp) && validate_coordinates(request.x, request.y, params->grid_height, params->grid_width)) {

                log_valid_emergency_received(request.emergency_name, request.x, request.y, id);
                // richiesta valida
                valid_emergency.type = params->emergency_types[curr_element];
                valid_emergency.status = WAITING; // Stato iniziale dell'emergenza
                valid_emergency.x = request.x;
                valid_emergency.y = request.y;
                valid_emergency.time = request.timestamp;
                valid_emergency.rescuer_count = 0;
                valid_emergency.rescuers_dt = NULL;
                valid_emergency.id = id;
                
                emergency_digital_twins = right_d_twins(valid_emergency, rescuer_types, num_rescuer_types);
                if (!emergency_digital_twins) {
                    fprintf(stderr, "Errore nella creazione dei gemelli digitali per l'emergenza: %s\n", valid_emergency.type.emergency_desc);
                    continue; // Salta questa emergenza se c'è un errore
                }

                valid_emergency.rescuers_dt = emergency_digital_twins;

                priority_queue_push(&internal_queue, valid_emergency);
                printf("[RECEIVER] Emergenza valida, aggiunta alla coda: %s (%d,%d) PRIORITÀ %d\n",
                       valid_emergency.type.emergency_desc, valid_emergency.x, valid_emergency.y, valid_emergency.type.priority);
                       //log
            } else if (!validate_name(request.emergency_name, params->emergency_types, params->num_emergency_types, &curr_element)) {
                snprintf(buffer, sizeof(buffer), " Nome richiesta di emergenza non valida: %s (%d,%d) @ %ld\n",
                       request.emergency_name, request.x, request.y, request.timestamp);
                        log_invalid_emergency_received(request.emergency_name, request.x, request.y, id, buffer);
                        printf("Nome emergenza non valido: %s\n", request.emergency_name);
                       //log
            } else if (!validate_timestamp(request.timestamp)) {
                snprintf(buffer, sizeof(buffer), " Timestamp non valido: %s (%d,%d) @ %ld\n",
                       request.emergency_name, request.x, request.y, request.timestamp);
                       log_invalid_emergency_received(request.emergency_name, request.x, request.y, id, buffer);
                printf("Timestamp non valido: %ld\n", request.timestamp);
            } else if (!validate_coordinates(request.x, request.y, params->grid_height, params->grid_width)) {
                snprintf(buffer, sizeof(buffer), " Coordinate non valide: %s (%d,%d) @ %ld\n",
                       request.emergency_name, request.x, request.y, request.timestamp);
                       log_invalid_emergency_received(request.emergency_name, request.x, request.y, id, buffer);
                printf("Coordinate non valide: (%d,%d) fuori da [%d,%d]\n", request.x, request.y, params->grid_width, params->grid_height);
            }
        } else {
            if (errno == EINTR) break;
            if (errno == EAGAIN) continue;
            perror("mq_receive");
            break;
        }
    }
    printf("[RECEIVER] Terminato.\n");
    return 0;
}

int digital_twin_arrives(void *arg) {
    emergency_args_t *params = (emergency_args_t *)arg;
    emergency_t *em = params->active_emergency;
    rescuer_request_t *rr = &em->type.rescuers[params->i];
    rescuer_digital_twin_t *dt = &em->rescuers_dt[params->i][params->j];
    int arrival = estimate_arrival_time(dt, em);

    // Soccorritore parte
    printf("[EMERGENZA %d] digital twin: %d, type: %s, assigned to: %s is driving...\n", params->counter_emergenze, em->rescuers_dt[params->i][params->j].id, em->rescuers_dt[params->i][params->j].rescuer->rescuer_type_name, em->type.emergency_desc);
    mtx_lock(params->lock);
    dt->status = EN_ROUTE_TO_SCENE;
    log_dt_status_change(em->id, dt->id, dt->rescuer->rescuer_type_name, rescuer_status_to_string(dt->status));
    log_emergency_assignment(em->id, em->type.emergency_desc, dt->id, dt->rescuer->rescuer_type_name);  
    mtx_unlock(params->lock);
    sleep(arrival);

    mtx_lock(params->lock);
    em->rescuer_count++;
    dt->status = ON_SCENE;
    log_dt_status_change(em->id, dt->id, dt->rescuer->rescuer_type_name, rescuer_status_to_string(dt->status));
    printf("[EMERGENZA %d] digital twin: %d, type: %s, assigned to: %s just arrived on scene..\n", params->counter_emergenze, em->rescuers_dt[params->i][params->j].id, em->rescuers_dt[params->i][params->j].rescuer->rescuer_type_name, em->type.emergency_desc);
    mtx_unlock(params->lock);

    return 0;
}

int digital_twin_goes_away(void *arg) {
    emergency_args_t *params = (emergency_args_t *)arg;
    emergency_t *em = params->active_emergency;
    rescuer_request_t *rr = &em->type.rescuers[params->i];


    rescuer_digital_twin_t *dt = &em->rescuers_dt[params->i][params->j];
    int arrival = estimate_arrival_time(dt, em);

    // Soccorritore parte
    printf("[EMERGENZA %d] digital twin: %d, type: %s, assigned to: %s is returning to base...\n", params->counter_emergenze, em->rescuers_dt[params->i][params->j].id, em->rescuers_dt[params->i][params->j].rescuer->rescuer_type_name, em->type.emergency_desc);
    mtx_lock(params->lock);
    dt->status = RETURNING_TO_BASE;
    log_dt_status_change(em->id, dt->id, dt->rescuer->rescuer_type_name, rescuer_status_to_string(dt->status));
    mtx_unlock(params->lock);
    sleep(arrival);

    mtx_lock(params->lock);
    rr->type->occ_rescuers--;
    em->rescuer_count--;
    dt->status = IDLE;
    log_dt_status_change(em->id, dt->id, dt->rescuer->rescuer_type_name, rescuer_status_to_string(dt->status));
    //printf("[EMERGENZA %d] digital twin %d, type %s, assigned to: %s occ_rescuers: %d ha fatto signal\n", params->counter_emergenze, em->rescuers_dt[params->i][params->j].id, em->rescuers_dt[params->i][params->j].rescuer->rescuer_type_name, em->type.emergency_desc,  rr->type->occ_rescuers);
    cnd_signal(params->empty);
    printf("[EMERGENZA %d] digital twin %d, type %s, assigned to: %s successfully returned to base\n", params->counter_emergenze, em->rescuers_dt[params->i][params->j].id, em->rescuers_dt[params->i][params->j].rescuer->rescuer_type_name, em->type.emergency_desc);
    mtx_unlock(params->lock);

    return 0;
}

int handle_emergency(void *arg) {
    dispatcher_args_t*params = (dispatcher_args_t *)arg;
    emergency_t *em = params->active_emergency;
    printf("[GESTORE EMERGENZA %d] Gestisco: %s (%d,%d) PRIORITÀ %d\n", em->id,
               em->type.emergency_desc, em->x, em->y, em->type.priority);

        int created_threads_digital_twin_arrives = 0;  //per vedere num thread creati
        int created_threads_digital_twin_goes_away = 0;  //per vedere num thread creati
        int max_arriving_time = 0; //per vedere tempo di arrivo per ogni soccorritore
        int max_threads = 0; //per vedere num max di thread che posso creare
        mtx_t lock;

        params->active_emergency->status = ASSIGNED;
        log_emergency_status_change(em->id,em->type.emergency_desc, emergency_status_to_string(em->status));

        mtx_init(&lock, mtx_plain);

        time_t now = time(NULL);
        time_t deadline;

        if (em->type.priority == 0) deadline = LONG_MAX;
        else if (em->type.priority == 1) deadline = now + 30;
        else deadline = now + 10;

        for (int i = 0; i < em->type.rescuers_req_number; i++) {
            max_threads += em->type.rescuers[i].required_count;
        }

        thrd_t *dt_arrives_threads = malloc(sizeof(thrd_t) * max_threads); //array di thread descriptors
        thrd_t *dt_goes_away_threads = malloc(sizeof(thrd_t) * max_threads); //array di thread descriptors
        emergency_args_t **thread_params = malloc(sizeof(emergency_args_t*) * max_threads);  //struttura dove mi salvo puntatori di p per fare free successivo
        if (!dt_arrives_threads || !dt_goes_away_threads || !thread_params) {
            fprintf(stderr, "Errore malloc array thread/params\n");
            free(dt_arrives_threads);
            free(dt_goes_away_threads);
            free(thread_params);
            return -1;
        }

        for (int i = 0; i < em->type.rescuers_req_number; i++) { //se emergenza richiede più soccorritori di quelli presenti nel sistema abortisci gestione emergenza
            rescuer_request_t *rr = &em->type.rescuers[i]; 
            if(rr->type->quantity < rr->required_count) {
                fprintf(stderr, "[GESTORE EMERGENZA %d] ERRORE: risorse nel sistema insufficienti per soccorritore %s\n",
                    em->id, rr->type->rescuer_type_name);
                em->status = CANCELED;
                log_emergency_status_change(em->id, em->type.emergency_desc,
                                            emergency_status_to_string(em->status));
                free(dt_arrives_threads);
                free(dt_goes_away_threads);
                free(thread_params);
                return -1;  // abortisci la gestione di questa emergenza
            }
        }

        max_arriving_time = max_rescuer_arriving_time(em);                 
        
        if (now + max_arriving_time >= deadline) {       //controllo che le emergenze vengano gestite tempestivamente
            em->status = TIMEOUT;   
            log_emergency_timeout(em->id, "Distanza troppo elevata, impossibile gestire entro il tempo limite"); 
            printf("[GESTORE EMERGENZA %d] Timeout: %s\n",
                em->id, em->type.emergency_desc);
            free(dt_arrives_threads);
            free(dt_goes_away_threads);
            free(thread_params);
            return -1;
        }

        mtx_lock(&lock);  // 🔹 Controllo se ci sono risorse sufficienti per gestire l'emergenza
        while (!can_fulfill(em)) {
            now = time(NULL);
            if (now >= deadline) {
                em->status = TIMEOUT;
                log_emergency_timeout(em->id, "Mancanza risorse, impossibile gestire entro il tempo limite");
                printf("[GESTORE EMERGENZA %d] Timeout: %s\n",
                    em->id, em->type.emergency_desc);
                free(dt_arrives_threads);
                free(dt_goes_away_threads);
                free(thread_params);
                mtx_unlock(&lock);
                return -1;
            }

            printf("[GESTORE EMERGENZA %d] Risorse insufficienti, attendo...\n",em->id);
            cnd_wait(params->empty, &lock);   // 🔹 aspetto che un soccorritore liberi risorse
        }
        for (int i = 0; i < em->type.rescuers_req_number; i++) { //se puo essere preso occupo subito tuti soccorritori dentro sezione critica
            rescuer_request_t *rr = &em->type.rescuers[i]; 
            rr->type->occ_rescuers += rr->required_count;
            printf("[GESTORE EMERGENZA %d] Soccorritore %s occupato, occ_rescuers: %d\n",
                em->id, rr->type->rescuer_type_name, rr->type->occ_rescuers);
        }
        mtx_unlock(&lock);

        // Controllo tutti i soccorritori richiesti
        for (int i = 0; i < em->type.rescuers_req_number; i++) {
            rescuer_request_t *rr = &em->type.rescuers[i];
            for (int j = 0; j < rr->required_count; j++) {

                // Parametri per il thread
                emergency_args_t *p = malloc(sizeof(emergency_args_t));
                if (!p) continue;
                p->active_emergency = em;
                p->i = i;
                p->j = j;
                p->tot_num_rescuers = params->tot_num_rescuers;
                p->lock = &lock;
                p->empty = params->empty;
                p->counter_emergenze = params->counter_emergenze;
                // Salva puntatore per uso successivo
                thread_params[created_threads_digital_twin_arrives] = p;
                if (thrd_create(&dt_arrives_threads[created_threads_digital_twin_arrives], digital_twin_arrives, p) != thrd_success) {
                    fprintf(stderr, "Errore creazione thread arrivo digital twin\n");
                    free(p);
                    continue;
                }
                created_threads_digital_twin_arrives++;
            }
        }

        if (created_threads_digital_twin_arrives < max_threads) {  // se non sono riuscito a crearli tutti abortisci gestione emergenza
            fprintf(stderr, "[GESTORE EMERGENZA %d] ERRORE: creati solo %d/%d thread\n",
                    em->id, created_threads_digital_twin_arrives, max_threads);
            for (int k = 0; k < created_threads_digital_twin_arrives; k++) {
                thrd_join(dt_arrives_threads[k], NULL);
                free(thread_params[k]);
            }
            free(thread_params);
            free(dt_arrives_threads);
            free(dt_goes_away_threads);

            em->status = CANCELED;
            log_emergency_status_change(em->id, em->type.emergency_desc,
                                        emergency_status_to_string(em->status));
            return -1;  // abortisci la gestione di questa emergenza
        }

        // 🔹 Attendi che tutti i thread abbiano finito
        for (int i = 0; i < created_threads_digital_twin_arrives; i++) {
            thrd_join(dt_arrives_threads[i], NULL);
        }
        
        // Verifica che ci siano tutti gemelli necessari
        int valid = true;
        for (int i = 0; i < em->type.rescuers_req_number; i++) {
            rescuer_request_t *rr = &em->type.rescuers[i];
            for (int j = 0; j < rr->required_count; j++) {
                rescuer_digital_twin_t *dt = &params->active_emergency->rescuers_dt[i][j];
                if (dt->status != ON_SCENE) {
                    valid = false;
                }
            }
        }
        
        if (valid) {  // Se sono arrivato qui emergenza può essere gestita
            em->status = IN_PROGRESS;
            log_emergency_status_change(em->id,em->type.emergency_desc, emergency_status_to_string(em->status));

            printf("[GESTORE EMERGENZA %d] Emergenza %s is being managed ✅\n", em->id, em->type.emergency_desc);
            int max_managing_time = 0;
            for (int i = 0; i < em->type.rescuers_req_number; i++) {
                rescuer_request_t *rr = &em->type.rescuers[i];
                int curr_rescuer_managing_time = rr->time_to_manage;
                if (curr_rescuer_managing_time > max_managing_time) {
                    max_managing_time = curr_rescuer_managing_time;
                }
            }
            printf("[GESTORE EMERGENZA %d] Tempo di gestione: %d\n", em->id, max_managing_time);
            sleep(max_managing_time); //simula tempo che serve ai soccorritori per gestire l'emergenza
            printf("[GESTORE EMERGENZA %d] Emergenza %s gestita ✅✅\n", em->id, em->type.emergency_desc);
            em->status = COMPLETED;
            log_emergency_status_change(em->id,em->type.emergency_desc, emergency_status_to_string(em->status));
        } else {
            em->status = CANCELED;
            log_emergency_status_change(em->id,em->type.emergency_desc, emergency_status_to_string(em->status));
            printf("[GESTORE EMERGENZA %d] Timeout: %s\n", em->id, em->type.emergency_desc);
        }

        //faccio tornare indietro digital twins
        for (int i = 0; i < em->type.rescuers_req_number; i++) {
            rescuer_request_t *rr = &em->type.rescuers[i];
            for (int j = 0; j < rr->required_count; j++) {
                if (thrd_create(&dt_goes_away_threads[created_threads_digital_twin_goes_away], digital_twin_goes_away, thread_params[created_threads_digital_twin_goes_away]) != thrd_success) {
                    fprintf(stderr, "Errore creazione thread arrivo digital twin\n");
                    free(thread_params[created_threads_digital_twin_goes_away]);
                    continue;
                }
                created_threads_digital_twin_goes_away++;

            }
        }

        for (int i = 0; i < created_threads_digital_twin_goes_away; i++) {
            thrd_join(dt_goes_away_threads[i], NULL);
            free(thread_params[i]);
        }
        free(thread_params);
        free(dt_arrives_threads);
        free(dt_goes_away_threads);

        mtx_destroy(&lock);

    return 0;
}

int dispatcher_thread(void *arg) {
    dispatcher_args_t *params = (dispatcher_args_t *)arg;
    emergency_t em;
    int counter_emergenze = 0;

    while (priority_queue_pop(&internal_queue, &em)) {
        counter_emergenze++;
        // Copia dinamica emergenza
        emergency_t *req_copy = malloc(sizeof(emergency_t));
        if (!req_copy) continue;
        *req_copy = em;

        // Copia dei parametri del dispatcher per questo thread
        dispatcher_args_t *em_params = malloc(sizeof(dispatcher_args_t));
        if (!em_params) {
            free(req_copy);
            continue;
        }
        *em_params = *params;  // copia tutti i campi (inclusi lock globali, se servono)
        em_params->active_emergency = req_copy;
        em_params->counter_emergenze = counter_emergenze;

        thrd_t em_thread;
        if (thrd_create(&em_thread, handle_emergency, em_params) != thrd_success) {
            fprintf(stderr, "[DISPATCHER] Errore creazione thread emergenza %s\n", em.type.emergency_desc);
            free(req_copy);
            continue;
        }
        thrd_detach(em_thread);
    }

    printf("[DISPATCHER] Terminato.\n");
    return 0;
} 