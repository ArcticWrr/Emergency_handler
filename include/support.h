//support.h
#ifndef SUPPORT_H
#define SUPPORT_H

#include "structures.h"
#include "macro.h"
#include "logger.h"
#include "other_functions.h"
#include "digital_twins.h"
#include "priority_queue.h"
#include "macro.h"

int find_correct_index(const char* rescuer_type_name, rescuer_type_t* rescuer_types, int num_rescuer_types);
rescuer_digital_twin_t** right_d_twins(emergency_t emergency, rescuer_type_t* rescuer_types, int num_rescuer_types);
bool validate_timestamp(time_t ts);
bool validate_coordinates(int x, int y, int grid_height, int grid_width);
bool validate_name(const char *name, emergency_type_t *emergency_types, int num_emergency_types, int *curr_element);
int estimate_arrival_time(rescuer_digital_twin_t *rescuer_digital_twin_t, emergency_t *em);
int max_rescuer_arriving_time(emergency_t *em);
bool can_fulfill(emergency_t *em);
#endif // SUPPORT_H