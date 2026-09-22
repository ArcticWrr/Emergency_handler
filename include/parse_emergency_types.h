// parse_emergency_types.h
#ifndef PARSE_EMERGENCY_TYPES_H
#define PARSE_EMERGENCY_TYPES_H

#include "structures.h"
#include "logger.h"
#include "other_functions.h"
#include "macro.h"

#define MAX_LINE_LENGTH 256
#define MAX_EMERGENCY_TYPES 100
#define MAX_RESCUER_REQUESTS 10

emergency_type_t* parse_emergency_types(const char* filename, int* num_emergency_types_out, rescuer_type_t* rescuer_types, int num_rescuer_types);

#endif