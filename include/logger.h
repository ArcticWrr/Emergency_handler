#ifndef LOGGER_H
#define LOGGER_H

#define LOG_FILE_PATH "logs/system.log"
#define MAX_EVENT_LENGTH 64
#define MAX_MESSAGE_LENGTH 256

#include "macro.h"

// Rimuove il carattere di newline da una stringa
void trim_newline(char* str);

// Scrive una riga generica nel file di log
void log_event(const char* id, const char* event, const char* message);

// Eventi specifici per il parsing dei file
void log_file_open(char* filename);
void log_file_close(char* filename);
void log_parsing_line(char* line_content, int id, int num_riga);
void log_parse_error(char* line_content, int id,  int num_riga, const char* error_message);
void log_parse_resctype_success(char* rescuer_type, int quantity, int speed, int x, int y, int id);
void log_parse_emtype_success(char* emergency_type, int priority, int resc_req_number, int id);
void log_parse_necessary_rescuers(char* rescuer_name, int qty, int secs, int id);
void log_valid_emergency_received(const char* emergency_desc, int x, int y, int id);
void log_invalid_emergency_received(const char* emergency_desc, int x, int y, int id, const char* cause);
void log_emergency_status_change(int emergency_id, const char* emergency_desc, const char* new_status);
void log_dt_status_change(int emergency_id, int dt_id, const char* rescuer_type_name, const char* new_status);
void log_emergency_timeout(int emergency_id, const char* cause);
void log_emergency_assignment(int emergency_id, const char* emergency_desc, int dt_id, const char* rescuer_type_name);

#endif // LOGGER_H