// parse_env.h
#ifndef PARSE_ENV_H
#define PARSE_ENV_H

#include "structures.h"
#include "logger.h"
#include "other_functions.h"
#include "macro.h"

#define MAX_LINE_LENGTH 256

env_t* parse_env(char* filename);
    
#endif
