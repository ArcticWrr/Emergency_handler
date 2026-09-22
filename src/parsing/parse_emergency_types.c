#define _POSIX_C_SOURCE 200809L
#include <stdio.h>      // fopen, fclose, FILE, printf
#include <stdlib.h>     // malloc, free, exit
#include <string.h>     // strdup, strcmp, strtok, sscanf
#include <stdbool.h>    // bool, true, false

#include "../../include/parse_emergency_types.h"

emergency_type_t* parse_emergency_types(const char* filename, int* num_emergency_types_out, rescuer_type_t* rescuer_types, int num_rescuer_types) {
    
    FILE* in = NULL;
    SNCALL(in, fopen(filename, "r"), "Errore nell'aprire il file");

    emergency_type_t* emergency_types = NULL;
    MALLOC(emergency_types, sizeof(emergency_type_t), "malloc fallita", MAX_EMERGENCY_TYPES);

    char riga[MAX_LINE_LENGTH];
    
    int num_type_emergencies = 0; // Contatore righe valide(quindi tipi di emergenze)
    int total_lines_counter = 0; // Contatore righe totali

    log_file_open("../../conf/emergency_types.conf");

    while (fgets(riga, sizeof(riga), in)) { // Leggere il file riga per riga
        removeSpaces(riga); // Rimuove gli spazi bianchi
        total_lines_counter++;
        log_parsing_line(riga, num_type_emergencies, total_lines_counter);
        char name[64];
        char resto[192];
        int priority;
        int salta = 0; // Flag per saltare la riga se ci sono errori
        int parsed = sscanf(riga, "[%63[^]]][%d] %191[^\n]]", name, &priority, resto);
        if (parsed != 3) {
            log_parse_error(riga, num_type_emergencies, total_lines_counter, "Errore di formato");
            //fprintf(stderr, "Errore di formato nella riga: %d del file: %s\n", total_lines_counter, filename);
            continue; // Salta la riga se il formato non è corretto
        }
        if (priority < 0 || priority > 2) {
            log_parse_error(riga, num_type_emergencies, total_lines_counter, "Priorità non valida");
            continue;
        }
        // Alloca e riempi la struttura
        emergency_types[num_type_emergencies].emergency_desc = strdup(name);
        emergency_types[num_type_emergencies].priority = priority;
        emergency_types[num_type_emergencies].rescuers = malloc(sizeof(rescuer_request_t) * MAX_RESCUER_REQUESTS);
        emergency_types[num_type_emergencies].rescuers_req_number = 0;

        // Parsing soccorritori: es. "Pompieri:1,5;Ambulanza:2,3;"
        char* token = strtok(resto, ";");
        if (token == NULL) {
            //fprintf(stderr, "Nessun soccorritore specificato per l'emergenza: %s\n", name);
            log_parse_error(riga, num_type_emergencies, total_lines_counter, "Nessun soccorritore specificato");
            salta = 1; // Segna riga da saltare se non ci sono soccorritori (per sovrascrivere struttura già allocata)
            continue; // Se non ci sono soccorritori, salta la riga
        }
        while (token && emergency_types[num_type_emergencies].rescuers_req_number < MAX_RESCUER_REQUESTS) {
            int emergenza_presente= 0;
            char rescuer_name[64];
            int qty, secs;

            int parsed = sscanf(token, "%63[^:]:%d,%d", rescuer_name, &qty, &secs);
            if (parsed != 3) {
                log_parse_error(token, num_type_emergencies, total_lines_counter, "Errore di formato per il soccorritore");
                //fprintf(stderr, "Formato errato per il soccorritore: %s nella riga: %d del file: %s\n", token, total_lines_counter, filename);
                token = strtok(NULL, ";");
                salta = 1; // Segna riga da saltare a causa del formatto errato
                break; 
            }

            int idx = emergency_types[num_type_emergencies].rescuers_req_number;
            rescuer_request_t* r = &emergency_types[num_type_emergencies].rescuers[idx]; // array emergenze, metto emergenza in posizione idx
            emergency_types[num_type_emergencies].rescuers_req_number++;

            for(int i = 0; i < num_rescuer_types; i++) {
                if (strcmp(rescuer_name, rescuer_types[i].rescuer_type_name) == 0) {
                    r->type = &rescuer_types[i];
                    r->required_count = qty;
                    r->time_to_manage = secs;
                    emergenza_presente = 1; // Il tipo di soccorritore è stato trovato
                    break;
                }
            }
            if (!emergenza_presente) {
                //fprintf(stderr, "Tipo di soccorritore '%s' non trovato per l'emergenza: %s\n nel file: %s\n", rescuer_name, name, filename);
                // Log dell'errore
                salta = 1; // Segna riga da saltare perchè soccorritore non esiste (sovrascrive struttura già allocata)
                log_parse_error(token, num_type_emergencies, total_lines_counter, "Tipo di soccorritore non trovato");
                break; // Se il tipo di soccorritore non esiste, salta la riga
                
            }

            token = strtok(NULL, ";");

            log_parse_necessary_rescuers(rescuer_name, qty, secs, idx);
        }
        if (salta) {
            continue; // Se il tipo di soccorritore non esiste, salta la riga non aumenta il contatore
        }
        log_parse_emtype_success(emergency_types[num_type_emergencies].emergency_desc, emergency_types[num_type_emergencies].priority, emergency_types[num_type_emergencies].rescuers_req_number, num_type_emergencies);
        num_type_emergencies++;
    }
    /*
    for (int i=0; i<num_type_emergencies; i++) {
        printf("Emergenza %d: %s\n", i, emergency_types[i].emergency_desc);
        for (int j=0; j<emergency_types[i].rescuers_req_number; j++) {
            printf("  Soccorritore %d: %s, Quantità: %d, Tempo di gestione: %d\n", j, emergency_types[i].rescuers[j].type->rescuer_type_name, emergency_types[i].rescuers[j].required_count, emergency_types[i].rescuers[j].time_to_manage);
        }
    } */
    log_file_close("../conf/emergency_types.conf");
    FCALL(fclose(in), "Errore nella chiusura del file");

    *num_emergency_types_out = num_type_emergencies;
    return emergency_types;
}