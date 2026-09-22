#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdlib.h>     // malloc, free, NULL
#include <string.h>     // strdup, strcpy, strlen
#include <time.h>       // time_t
#include <threads.h>    // mtx_t, cnd_t (C11 threads)
#include <mqueue.h>     // mqd_t
#include <stdbool.h>      // bool, true, false

#define EMERGENCY_NAME_LENGTH 64
#define MAX_QUEUE_NAME_LENGTH 100

// Stato dei soccorritori
typedef enum {
    IDLE,
    EN_ROUTE_TO_SCENE,
    ON_SCENE,
    RETURNING_TO_BASE
} rescuer_status_t;

// Tipo di soccorritore
typedef struct {
    char *rescuer_type_name; // Nome del tipo di soccorritore
    int quantity; // Numero di soccorritori di questo tipo
    int speed; // Velocità del soccorritore
    int x; // Coordinate base del soccorritore
    int y; // Coordinate base del soccorritore
    int occ_rescuers; // Numero di soccorritori attualmente occupati
} rescuer_type_t;

// Gemello digitale di un soccorritore (uno per ogni istanza di soccorritore)
typedef struct {
    int id; // ID univoco del soccorritore
    int x; // Coordinate attuali del soccorritore
    int y; // Coordinate attuali del soccorritore
    rescuer_type_t *rescuer; // Tipo di soccorritore
    rescuer_status_t status; // Stato attuale del soccorritore
} rescuer_digital_twin_t;



// Richiesta di soccorritore per un'emergenza
typedef struct {
    rescuer_type_t *type; // Tipo di soccorritore richiesto
    int required_count; // Numero di soccorritori di quel tipo richiesti
    int time_to_manage; // Tempo stimato per gestire la richiesta
} rescuer_request_t;

// Tipo di emergenza
typedef struct {
    short priority; // Priorità dell'emergenza
    char *emergency_desc; // Nome dell'emergenza
    rescuer_request_t *rescuers; // Array di entità rescuer_request_t
    int rescuers_req_number; // Numero soccorritori diversi richiesti
    int *assigned_rescuers_count; // Array che tiene traccia del numero di soccorritori assegnati per ogni tipo richiesto
} emergency_type_t;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Stato di un'emergenza
typedef enum {
    WAITING,
    ASSIGNED,
    IN_PROGRESS,
    PAUSED,
    COMPLETED,
    CANCELED,
    TIMEOUT
} emergency_status_t;

// Richiesta di emergenza ricevuta dal client
typedef struct {
    char emergency_name[EMERGENCY_NAME_LENGTH]; // Nome dell'emergenza
    int x; // Coordinata X dell'emergenza
    int y; // Coordinata Y dell'emergenza
    time_t timestamp; // Timestamp della richiesta
} emergency_request_t;

// Istanza di emergenza generata in caso di validità della richiesta
typedef struct {
    int id; // ID univoco dell'emergenza
    emergency_type_t type;
    emergency_status_t status;
    int x;
    int y;
    time_t time;
    int rescuer_count; // Numero di soccorritori assegnati all'emergenza
    rescuer_digital_twin_t **rescuers_dt; // puntatore a array di gemelli digitali dei soccorritori assegnati
} emergency_t;

typedef struct {
    char queue_name[MAX_QUEUE_NAME_LENGTH]; // Nome della coda
    int grid_height; // Altezza della griglia
    int grid_width; // Larghezza della griglia
} env_t;

// Nodo per la coda interna
typedef struct node {
    emergency_t data;
    struct node *next;
} node_t;

// Coda interna con mutex + condition
typedef struct {
    node_t *head;
    node_t *tail;
    mtx_t mutex;
    cnd_t cond;
} priority_queue_t;

typedef struct {
    mqd_t mq;
    int num_emergency_types;
    emergency_type_t *emergency_types; // array di tipi validi
    int grid_height;
    int grid_width;
    int tot_num_rescuers;
    rescuer_type_t *rescuer_types; // array di tipi di soccorritori validi
} receiver_args_t;

typedef struct {
    int tot_num_rescuers; // Numero totale di tipi di soccorritori
    int counter_emergenze; // Contatore delle emergenze gestite
    emergency_t *active_emergency; // Puntatore all’emergenza che il dispatcher sta gestendo
    cnd_t *empty; // Condizione per segnalare quando la coda è vuota
} dispatcher_args_t;

typedef struct {
    emergency_t *active_emergency;
    int i; // indice tipo soccorritore richiesto
    int j; // indice del soccorritore specifico
    int tot_num_rescuers;
    int counter_emergenze; // Contatore delle emergenze gestite
    bool *resources_taken; // Indica se le risorse per questo tipo di soccorritore sono già state prese
    mtx_t *lock; // Mutex per proteggere lo stato dell'emergenza
    cnd_t *empty; // Condizione per segnalare quando ci sono gemelli digitali disponibili
} emergency_args_t;

typedef struct {
    mqd_t mq;
    emergency_request_t request;
    int delay;
} send_args_t;

#endif // STRUCTURES_H
