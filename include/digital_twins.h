// digital_twins.h
#ifndef DIGITAL_TWINS_H
#define DIGITAL_TWINS_H

#include "structures.h"
#include "parse_rescuers.h"
#include "logger.h"

rescuer_digital_twin_t* create_digital_twins(rescuer_type_t* types, int types_curr_index, int num_types, int quantity);

#endif