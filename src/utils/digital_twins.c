#define _POSIX_C_SOURCE 200809L
#include <stdio.h>      // fopen, fclose, FILE, printf
#include <stdlib.h>     // malloc, free, exit
#include <string.h>     // strdup, strcmp, strtok, sscanf
#include <stdbool.h>    // bool, true, false

#include "../../include/digital_twins.h"

rescuer_digital_twin_t* create_digital_twins(rescuer_type_t* types, int types_curr_index, int num_types, int quantity) {
    if (types_curr_index < 0 || types_curr_index >= num_types) {
        fprintf(stderr, "Indice fuori dai limiti: %d\n", types_curr_index);
        return NULL;
    }
    rescuer_digital_twin_t* twins = NULL;
    MALLOC(twins, sizeof(rescuer_digital_twin_t), "malloc fallita", quantity);
    //printf("quantita:%d\n", quantity);
    for (int i = 0; i < quantity; i++) { 
        twins[i].id = i + 1; // ID univoco del soccorritore
        twins[i].x = types[types_curr_index].x; // Coordinate base del soccorritore
        twins[i].y = types[types_curr_index].y; // Coordinate base del soccorritore
        twins[i].rescuer = &types[types_curr_index]; // Tipo di soccorritore
        twins[i].status = IDLE; // Stato iniziale del soccorritore
    }
    return twins;
}
