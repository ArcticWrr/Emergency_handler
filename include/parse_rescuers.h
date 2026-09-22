// parse_rescuers.h
#ifndef PARSE_RESCUERS_H
#define PARSE_RESCUERS_H

#include "structures.h"
#include "logger.h"
#include "other_functions.h"
#include "macro.h"

#define MAX_LINE_LENGTH 256
#define MAX_RESCUERS 100

rescuer_type_t* parse_rescuers(const char* filename, int* num_rescuers_out);

#endif