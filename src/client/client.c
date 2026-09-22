#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <threads.h>

#include "../../include/structures.h"
#include "../../include/parse_env.h"
#include "../../include/logger.h"
#include "../../include/macro.h"

#define MAX_LINE 256
#define MAX_THREADS 100

int send_emergency(void *arg) {
    send_args_t *args = (send_args_t *)arg;
    printf("[THREAD] Inizio invio emergenza: %s (delay=%d)\n",
           args->request.emergency_name, args->delay);

    sleep(args->delay);
    args->request.timestamp = time(NULL);

    if (mq_send(args->mq, (char *)&args->request, sizeof(args->request), 0) == -1) {
        perror("Errore invio messaggio");
    } else {
        printf("[THREAD] Inviata emergenza: %s (%d,%d) @ %ld\n",
               args->request.emergency_name, args->request.x, args->request.y, args->request.timestamp);
    }
    free(args);
    return 0;
}

int main(int argc, char *argv[]){

    env_t* env = parse_env("conf/env.conf");
    if (env == NULL) {
        fprintf(stderr, "Errore parsing file di configurazione.\n");
        exit(EXIT_FAILURE);
    }

    if (argc < 5 && !(argc == 3 && strcmp(argv[1], "-f") == 0)) {
        fprintf(stderr, "Uso: %s <nome_emergenza> <coord_x> <coord_y> <delay_in_secs>\n", argv[0]);
        fprintf(stderr, "Oppure: %s -f <file_input>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *queue_name = env->queue_name;

    mqd_t mq = mq_open(queue_name, O_WRONLY);
    if (mq == (mqd_t)-1) {
        perror("Errore apertura coda messaggi");
        exit(EXIT_FAILURE);
    }

    thrd_t tids[MAX_THREADS];
    int num_threads = 0;

    if (strcmp(argv[1], "-f") != 0) {
        // Caso singola emergenza
        emergency_request_t request;
        strncpy(request.emergency_name, argv[1], sizeof(request.emergency_name) - 1);
        request.emergency_name[sizeof(request.emergency_name) - 1] = '\0';
        request.x = atoi(argv[2]);
        request.y = atoi(argv[3]);
        int delay = atoi(argv[4]);

        send_args_t *args = malloc(sizeof(send_args_t));
        args->mq = mq;
        args->request = request;
        args->delay = delay;

        thrd_create(&tids[num_threads++], send_emergency, args);

    } else {
        // Lettura da file
        const char *filename = argv[2];
        FILE *f = NULL;
        SNCALL(f, fopen(filename, "r"), "Errore nell'aprire il file");
        if (!f) {
            perror("Errore apertura file input");
            exit(EXIT_FAILURE);
        }

        char line[MAX_LINE];
        while (fgets(line, sizeof(line), f) && num_threads < MAX_THREADS) {
            if (line[0] == '#' || strlen(line) < 3) continue;

            emergency_request_t request;
            int delay;
            if (sscanf(line, "%63s %d %d %d",
                       request.emergency_name, &request.x, &request.y, &delay) != 4) {
                fprintf(stderr, "Formato riga non valido: %s", line);
                continue;
            }

            send_args_t *args = malloc(sizeof(send_args_t));
            args->mq = mq;
            args->request = request;
            args->delay = delay;

            thrd_create(&tids[num_threads++], send_emergency, args);
        }
        FCALL(fclose(f), "Errore nella chiusura del file");
    }

    // Aspetta che tutti i thread abbiano finito
    for (int i = 0; i < num_threads; i++) {
        thrd_join(tids[i], NULL);
    }

    mq_close(mq);
    return 0;
}
