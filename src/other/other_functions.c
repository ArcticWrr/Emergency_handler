#include<stdio.h>
#include "../../include/other_functions.h"
// Function to remove all spaces from a given string
void removeSpaces(char *str)
{
    // To keep track of non-space character count
    int count = 0;
    // Traverse the provided string. If the current character is not a space,
    //move it to index 'count++'.
    for (int i = 0; str[i]; i++)
        if (str[i] != ' ')
            str[count++] = str[i]; // here count is incremented
    str[count] = '\0';
}

const char* rescuer_status_to_string(rescuer_status_t status) {
    switch(status) {
        case IDLE: return "IDLE";
        case RETURNING_TO_BASE: return "RETURNING_TO_BASE";
        case EN_ROUTE_TO_SCENE: return "EN_ROUTE_TO_SCENE";
        case ON_SCENE: return "ON_SCENE";
        default: return "UNKNOWN";
    }
}

const char* emergency_status_to_string(emergency_status_t status) {
    switch(status) {
        case WAITING: return "WAITING";
        case ASSIGNED: return "ASSIGNED";
        case IN_PROGRESS: return "IN_PROGRESS";
        case TIMEOUT: return "TIMEOUT";
        case PAUSED: return "PAUSED";
        case COMPLETED: return "COMPLETED";
        case CANCELED: return "CANCELED";
        default: return "UNKNOWN";
    }
}