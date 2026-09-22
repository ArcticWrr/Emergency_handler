#define _POSIX_C_SOURCE 200809L
#include <stdio.h>      // fopen, fclose, FILE, printf
#include <stdlib.h>     // malloc, free, exit
#include <string.h>     // strdup, strcmp, strtok, sscanf
#include <stdbool.h>    // bool, true, false

#include "../../include/parse_env.h"

env_t *parse_env(char *filename){

    FILE* in = NULL;
    SNCALL(in, fopen(filename, "r"), "Errore nell'aprire il file");

    env_t *env = NULL;
    MALLOC(env, sizeof(env_t), "malloc fallita", 1);

    char riga[MAX_LINE_LENGTH];
    int total_lines_counter = 0;

    log_file_open(filename);

    while (fgets(riga, sizeof(riga), in) && total_lines_counter <= 2) {
        total_lines_counter++;
        log_parsing_line(riga, 0, total_lines_counter);

         // Rimuovi newline finale
        riga[strcspn(riga, "\r\n")] = 0;

        // Cerca il separatore '='
        char* equal_sign = strchr(riga, '=');
        if (!equal_sign) {
            fprintf(stderr, "Formato errato nella riga: %s\n", riga);
            continue;
        }

        // Dividi in chiave e valore
        *equal_sign = '\0';
        char* key = riga;
        char* value = equal_sign + 1;

        if (strcmp(key, "queue") == 0) {
            strncpy(env->queue_name, value, MAX_QUEUE_NAME_LENGTH-1);
            env->queue_name[MAX_QUEUE_NAME_LENGTH-1] = '\0';
            memmove(env->queue_name + 1, env->queue_name, strlen(env->queue_name) + 1);
            env->queue_name[0] = '/'; // Aggiungi '/' all'inizio
            //printf("Coda: %s\n", env->queue_name);

        } else if (strcmp(key, "height") == 0) {
            env->grid_height = atoi(value);
        } else if (strcmp(key, "width") == 0) {
            env->grid_width = atoi(value);
        } else {
            fprintf(stderr, "Chiave sconosciuta: %s\n", key);
            log_parse_error(riga, 0, total_lines_counter, "Chiave sconosciuta");
            continue;
        }
    }
    log_file_close(filename);
    FCALL(fclose(in), "Errore nella chiusura del file");
    return env;
}