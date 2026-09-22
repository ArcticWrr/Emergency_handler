#define _POSIX_C_SOURCE 200809L
#include <stdio.h>      // fopen, fclose, FILE, printf
#include <stdlib.h>     // malloc, free, exit
#include <string.h>     // strdup, strcmp, strtok, sscanf
#include <stdbool.h>    // bool, true, false

#include "../../include/parse_rescuers.h"


rescuer_type_t* parse_rescuers(const char* filename, int* num_rescuers_out) {
    
    FILE* in = NULL;
    SNCALL(in, fopen(filename, "r"), "Errore nell'aprire il file");
    
    rescuer_type_t* rescuers = NULL;
    MALLOC(rescuers, sizeof(rescuer_type_t), "malloc fallita", MAX_RESCUERS);

    char riga[MAX_LINE_LENGTH];
    
    int num_type_rescuers = 0; // Contatore righe valide(quindi tipi di soccorritori)
    int total_lines_counter = 0; // Contatore righe totali

    log_file_open("../conf/rescuer.conf");

    while (fgets(riga, sizeof(riga), in)) { // Leggere il file riga per riga
        removeSpaces(riga); // Rimuove gli spazi bianchi
        total_lines_counter++;
        log_parsing_line(riga, num_type_rescuers, total_lines_counter);
        char name[64];
        int quantity, speed, x, y;
        int parsed = sscanf(riga, "[%63[^]]][%d][%d][%d;%d]", name, &quantity, &speed, &x, &y);
        if (parsed != 5) {
            log_parse_error(riga, num_type_rescuers, total_lines_counter, "Errore di formato");
            //fprintf(stderr, "Errore di formato nella riga: %d del file: %s\n", total_lines_counter, filename);
            continue; // Salta la riga se il formato non è corretto
        }
        // A questo punto, i dati sono stati analizzati correttamente
        rescuers[num_type_rescuers].rescuer_type_name = strdup(name);
        rescuers[num_type_rescuers].quantity = quantity;
        rescuers[num_type_rescuers].speed = speed;
        rescuers[num_type_rescuers].x = x;
        rescuers[num_type_rescuers].y = y;

        log_parse_resctype_success(name, quantity, speed, x, y,num_type_rescuers);
        num_type_rescuers++;
    }

    log_file_close("../conf/rescuer.conf");
    FCALL(fclose(in), "Errore nella chiusura del file");

    *num_rescuers_out = num_type_rescuers;
    return rescuers;
}
