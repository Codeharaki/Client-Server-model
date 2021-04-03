#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

/* Taille maximum des noms des pipes. */
#define BUFFER_SIZE 50
#define MON_SHM "mon_shm_33"
#define MESSAGE_LENGTH 1024
/* Nombre maximum de message contenu dans notre file. */
#define MESSAGE_NUMBER 10
#define ARGUMENTS_NUMBER 10
#define SHM_SIZE sizeof(fifo)
/* Prefixe le nom des pipes */
#define TUBE_NAME "PROJET_SE_2016_"
/* Suffixe du nom du pipe de sortie */
#define OUTPUT "_OUTPUT"
/* Suffixe du nom du pipe d'entree */
#define INPUT "_INPUT"

#define STR(s) #s
#define XSTR(s) STR(s)

#define CHECK_CONDITION(expr, cause) \
    checkCondition(__FILE__, __LINE__, expr, cause)

#define PERROR(expr, message) \
    checkPerror(expr, message)

typedef struct request request;

struct request {
    pid_t pid;
    char message[MESSAGE_LENGTH];
};

typedef struct fifo fifo;

struct fifo {
    sem_t mutex;
    sem_t empty;
    sem_t full; 
    int read;
    int write;
    int request_number;
    request buffer[MESSAGE_NUMBER];
};

extern void checkCondition(const char *file, int line, bool expr,
    const char *cause);

extern void checkPerror(bool expr, const char *message);
extern void getNamedPipe(pid_t pid, char *output, char* input);

#endif