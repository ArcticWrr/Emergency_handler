#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../../include/logger.h"

void trim_newline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// Scrive una riga nel file di log
void log_event(const char* id, const char* event, const char* message) {
    FILE* logfile = NULL;
    SNCALL(logfile, fopen(LOG_FILE_PATH, "a"), "Impossibile aprire il file di log");

    time_t now = time(NULL);
    fprintf(logfile, "[%ld] [%s] [%s] %s\n", now, id, event, message);
    FCALL(fclose(logfile), "Errore nella chiusura del file di log");
}

// Eventi specifici per il parsing
void log_file_open(char* filename) {
    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg), "Apertura del file di configurazione: %s", filename);
    log_event(filename, "FILE_PARSING", msg);
}

void log_file_close(char* filename) {
    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg), "Chiusura del file di configurazione: %s", filename);
    log_event(filename, "FILE_PARSING", msg);
}


void log_parsing_line(char* line_content, int id, int num_riga) {
    
    char id_str[20]; // Buffer per contenere la stringa dell'ID

    trim_newline(line_content);

    snprintf(id_str, sizeof(id_str), "%d", id); // Conversione int -> stringa

    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg), "Lettura riga %d: %s", num_riga, line_content);
    log_event(id_str, "FILE_PARSING", msg);
}

void log_parse_error(char* line_content, int id,  int num_riga, const char* error_message) {
    
    char id_str[20]; // Buffer per contenere la stringa dell'ID

    trim_newline(line_content);

    snprintf(id_str, sizeof(id_str), "%d", id); // Conversione int -> stringa

    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg), "%s sulla riga %d: %s",error_message, num_riga, line_content);
    log_event(id_str, "FILE_PARSING_ERROR", msg);
}

void log_parse_resctype_success(char* rescuer_type, int quantity, int speed, int x, int y, int id) {

    char id_str[20]; // Buffer per contenere la stringa dell'ID
    snprintf(id_str, sizeof(id_str), "%d", id); // Conversione int -> stringa

    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg),
             "Soccorritore: %s, Quantità: %d, Velocità: %d, Base: [%d;%d] - Parsing OK",
             rescuer_type, quantity, speed, x, y);
    log_event(id_str, "RESCUER_TYPE_PARSING", msg);
}

void log_parse_emtype_success(char* emergency_type, int priority, int resc_req_number, int id) {

    char id_str[20]; // Buffer per contenere la stringa dell'ID
    snprintf(id_str, sizeof(id_str), "%d", id); // Conversione int -> stringa

    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg),
             "Tipo_Emergenza: %s, Priorità: %d, Numero richieste soccorritori: %d - Parsing OK",
             emergency_type, priority, resc_req_number);
    log_event(id_str, "EMERGENCY_TYPE_PARSING", msg);
}

void log_parse_necessary_rescuers(char* rescuer_name, int qty, int secs, int id) {

    char id_str[20]; // Buffer per contenere la stringa dell'ID
    snprintf(id_str, sizeof(id_str), "%d", id); // Conversione int -> stringa
    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg), "Soccorritore: %s, Quantità: %d, Tempo di gestione: %d", rescuer_name, qty, secs);
    log_event(id_str, "RESCUER_REQUEST_PARSING", msg);
}

void log_valid_emergency_received(const char* emergency_desc, int x, int y, int id) {

     char id_str[20]; // Buffer per contenere la stringa dell'ID
    snprintf(id_str, sizeof(id_str), "%d", id); // Conversione int -> stringa
    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg),
             "Emergenza valida ricevuta: %s (%d,%d)",
             emergency_desc, x, y);
    log_event(id_str, "EMERGENCY_RECEIVED", msg);
}

void log_invalid_emergency_received(const char* emergency_desc, int x, int y, int id, const char* cause) {

     char id_str[20]; // Buffer per contenere la stringa dell'ID
    snprintf(id_str, sizeof(id_str), "%d", id); // Conversione int -> stringa
    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg),
             "Emergenza non valida ricevuta: %s (%d,%d), causa: %s",
             emergency_desc, x, y, cause);
    log_event(id_str, "EMERGENCY_RECEIVED", msg);
}

void log_emergency_status_change(int emergency_id,const char* emergency_desc, const char* new_status) {
    char id_str[20]; // Buffer per contenere la stringa dell'ID
    snprintf(id_str, sizeof(id_str), "%d", emergency_id); // Conversione int -> stringa

    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg), "Stato emergenza %s cambiato in: %s", emergency_desc, new_status);
    log_event(id_str, "EMERGENCY_STATUS_CHANGE", msg);
}

void log_dt_status_change(int emergency_id, int dt_id, const char* rescuer_type_name, const char* new_status) {
    char id_str[20]; // Buffer per contenere la stringa dell'ID
    snprintf(id_str, sizeof(id_str), "%d", emergency_id); // Conversione int -> stringa

    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg), "Gemello digitale con id: %d di tipo %s ha cambiato stato in: %s", dt_id, rescuer_type_name, new_status);
    log_event(id_str, "DIGITAL_TWIN_STATUS_CHANGE", msg);
}

void log_emergency_assignment(int emergency_id, const char* emergency_desc, int dt_id, const char* rescuer_type_name) {
    char id_str[20]; // Buffer per contenere la stringa dell'ID
    snprintf(id_str, sizeof(id_str), "%d", emergency_id); // Conversione int -> stringa

    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg), "gemello digitale con id: %d di tipo %s assegnato con successo a emergenza con id %d (%s).",
             dt_id, rescuer_type_name, emergency_id, emergency_desc);
    log_event(id_str, "EMERGENCY_ASSIGNED", msg);
}

void log_emergency_timeout(int emergency_id, const char* cause) {
    char id_str[20]; // Buffer per contenere la stringa dell'ID
    snprintf(id_str, sizeof(id_str), "%d", emergency_id); // Conversione int -> stringa

    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg), "L'emergenza con id %d ha superato il tempo massimo di attesa e viene annullata. Causa: %s", emergency_id, cause);
    log_event(id_str, "EMERGENCY_TIMEOUT", msg);
}