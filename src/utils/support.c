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

#include "../../include/support.h"

int find_correct_index(const char* rescuer_type_name, rescuer_type_t* rescuer_types, int num_rescuer_types) { //trovo indice di un tipo di soccorritore (sua posizione in array di tutti i soccorritori)
    int trovato = 0;
    for (int i = 0; i < num_rescuer_types; i++) {
        if (strcmp(rescuer_types[i].rescuer_type_name, rescuer_type_name) == 0) {
            trovato = 1;
            return i;
        }
    }
    if (!trovato) {
        printf("non trovato\n"); // Tipo di soccorritore non trovato
        return -1;
    }
}

rescuer_digital_twin_t** right_d_twins(emergency_t emergency, rescuer_type_t* rescuer_types, int num_rescuer_types){
    int emergency_index = 0;
    rescuer_digital_twin_t** digital_twins = NULL;
    //printf("num socc.: %d\n", emergency.type.rescuers_req_number);
    MALLOC(digital_twins, sizeof(rescuer_digital_twin_t*), "malloc fallita", emergency.type.rescuers_req_number);

    for (int i = 0; i < emergency.type.rescuers_req_number; i++) {
        if ((emergency_index = find_correct_index(emergency.type.rescuers[i].type->rescuer_type_name, rescuer_types, num_rescuer_types)) < 0) {
            fprintf(stderr, "Tipo di soccorritore non trovato: %s\n", emergency.type.rescuers[i].type->rescuer_type_name);
            return NULL;
        }
        digital_twins[i] = create_digital_twins(rescuer_types, emergency_index, num_rescuer_types, emergency.type.rescuers[i].required_count);
        if (!digital_twins[i]) {
            fprintf(stderr, "Errore nella creazione dei gemelli digitali per il tipo: %s\n", rescuer_types[emergency_index].rescuer_type_name);
            return NULL;
        }
    }
    return digital_twins;
}


bool validate_timestamp(time_t ts) {
    if (ts <= 0) {
        return false; // mai successo o non impostato
    }
    time_t now = time(NULL);
    double diff = difftime(now, ts);
    // troppo vecchio 
    if (diff > 300) {
        return false;
    }
    // troppo nel futuro 
    if (diff < -10) {
        return false;
    }
    return true;
}

bool validate_coordinates(int x, int y, int grid_height, int grid_width) {
    return (x >= 0 && y >= 0 && x < grid_width && y < grid_height);
}

bool validate_name(const char *name, emergency_type_t *emergency_types, int num_emergency_types, int *curr_element) {
    for (int i = 0; i < num_emergency_types; i++) {
        if (strcmp(emergency_types[i].emergency_desc, name) == 0) {
            *curr_element = i;
            return true;
        }
    }
    return false;
}

int estimate_arrival_time(rescuer_digital_twin_t *rescuer_digital_twin_t, emergency_t *em) {
    if (!rescuer_digital_twin_t || !em) return -1.0; // errore

    int x1 = rescuer_digital_twin_t->x;
    int y1 = rescuer_digital_twin_t->y;
    int x2 = em->x;
    int y2 = em->y;

    // distanza Manhattan
    int distance = abs(x1 - x2) + abs(y1 - y2);

    // velocità del soccorritore
    int speed = rescuer_digital_twin_t->rescuer->speed;

    if (speed <= 0) return -1.0; // evita divisione per 0
    return distance / speed;
}

bool can_fulfill(emergency_t *em) {
    for (int i = 0; i < em->type.rescuers_req_number; i++) {
        rescuer_request_t *rr = &em->type.rescuers[i];
        int available = rr->type->quantity - rr->type->occ_rescuers;
        if (rr->required_count > available) {
            return false; // non ci sono abbastanza risorse per questo tipo
        }
    }
    return true; // tutte le richieste soddisfatte
}

int max_rescuer_arriving_time(emergency_t *em) {
    int max_time = 0;
    for (int i = 0; i < em->type.rescuers_req_number; i++) {
        rescuer_request_t *rr = &em->type.rescuers[i];
        for (int j = 0; j < rr->required_count; j++) {
            int arrival = estimate_arrival_time(&em->rescuers_dt[i][j], em);
            if (arrival < 0) {
                fprintf(stderr, "[ERRORE] Calcolo tempo di arrivo fallito per gemello %d tipo %s\n",
                        em->rescuers_dt[i][j].id,
                        em->rescuers_dt[i][j].rescuer->rescuer_type_name);
                continue;
            }
            if (arrival > max_time) {
                max_time = arrival;
            }
        }
    }
    return max_time;
}