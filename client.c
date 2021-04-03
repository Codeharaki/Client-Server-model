#include "util.h"

void gestionnaire(int signum);
void destroy();
void *run(void *arg);

request *r = NULL;
char *input = NULL;
char *output = NULL;
pthread_t th;
pthread_t *ptrth = NULL;

int main(int argc, char *argv[]) {
    // CREATION ET INITIALISATION DES PIPES
    input = malloc(BUFFER_SIZE * sizeof(char*));
    output = malloc(BUFFER_SIZE * sizeof(char*));
    memset(input, 0, BUFFER_SIZE);
    memset(output, 0, BUFFER_SIZE);
    getNamedPipe(getpid(), input, output);

    PERROR((mkfifo(input, S_IRUSR | S_IWUSR) == -1), "mkfifo");
    PERROR((mkfifo(output, S_IRUSR | S_IWUSR) == -1), "mkfifo");

    // CONSTRUCTION DES SIGNAUX
    struct sigaction action;
    action.sa_handler = gestionnaire;
    action.sa_flags = SA_RESTART;

    PERROR((sigfillset(&action.sa_mask) == -1), "sigfillset");
    PERROR((sigaction(SIGINT, &action, NULL) == -1), "sigaction");
    PERROR((sigaction(SIGTERM, &action, NULL) == -1), "sigaction");
    PERROR((sigaction(SIGTSTP, &action, NULL) == -1), "sigaction");
    PERROR((sigaction(SIGHUP, &action, NULL) == -1), "sigaction");
    PERROR((sigaction(SIGPIPE, &action, NULL) == -1), "sigaction");

    // CREATION D'UN SEGMENT DE MEMOIRE PARTAGE    
    int shm_fd = shm_open(MON_SHM, O_RDWR | O_APPEND, 
        S_IRUSR | S_IWUSR | S_IXUSR);

    PERROR((shm_fd == -1), "shm_fd");
    PERROR((ftruncate(shm_fd, SHM_SIZE) == -1), "shm_truncate");
    
    fifo *result = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, 
        shm_fd, 0);

    PERROR((result == MAP_FAILED), "mmap");

    // CREATION ET INITIALISATION D'UNE REQUETE
    r = malloc(sizeof(*r));
    CHECK_CONDITION(r != NULL, "Erreur Malloc");
    memset(r->message, 0, MESSAGE_LENGTH); 

    switch (argc) {
        case 1:
            printf("Entrer une commande à executer : \n> ");
            char mess[MESSAGE_LENGTH];
            scanf("%" XSTR(MESSAGE_LENGTH) "[^\n]", mess);
            char *verify = strncpy(r->message, mess, MESSAGE_LENGTH);
            CHECK_CONDITION((strcmp(verify, r->message) == 0), "Copy error");
            break;
        default:
            //CONCATENATION DE TOUT LES ARGUMENTS
            for (int i = 1; i < argc; i++) {
                strncat(r->message, argv[i], MESSAGE_LENGTH);
                strncat(r->message, " ", MESSAGE_LENGTH);
            }
            break;
      }
      r->pid = getpid();

    PERROR((sem_wait(&result->empty) == -1), "sem_wait");
    PERROR((sem_wait(&result->mutex) == -1), "sem_wait");
    
    result->buffer[result->write] = *r;
    result->request_number += 1;
    result->write = (result->write + 1) % MESSAGE_NUMBER;
    
    PERROR((sem_post(&result->mutex) == -1), "sem_post");
    PERROR((sem_post(&result->full) == -1), "sem_post");

    // OUVERTURE DU TUBES DE LECTURE
    int fd1 = open(input, O_RDONLY);
    PERROR((fd1 == -1), "open");
    int fd2 = open(output, O_WRONLY);
    PERROR((fd2 == -1), "open");

    // CREATION DU THREAD D'ECRITURE
        
    CHECK_CONDITION((pthread_create(&th, NULL, run, &fd2)) == 0,
        "Erreur Pthread Create");   

    // LECTURE DANS LE PIPE DE LECTURE (Resultat de la commande)
    char buf; 
    while (read(fd1, &buf, 1) > 0) {      
        CHECK_CONDITION((write(STDOUT_FILENO, &buf, 1) == 1), "Ecriture erreur");
    }

    PERROR((close(fd1) == -1), "close");
    PERROR((close(fd2) == -1), "close");

    destroy();
    return EXIT_SUCCESS;
}

void gestionnaire(int signum) {
    CHECK_CONDITION((signum >= 0), "Wrong signal number");

    switch (signum) {
        case SIGINT: 
        case SIGHUP:
        case SIGTERM:
        case SIGTSTP:
        destroy();
        break;
        case SIGPIPE :
        printf("plus de pipe ?\n");
        break;
    }    
}

void destroy() {
    if (input != NULL) {
        unlink(input);
        free(input);
    }
    if (output != NULL) {
        unlink(output);
        free(output);
    }
    if (r != NULL) {
        free(r);
    }

    //On annule le thread de lecture
    if (ptrth != NULL) {
        PERROR((pthread_cancel(*ptrth) != 0), "cancel");
        PERROR((pthread_join(*ptrth, NULL) != 0), "join");
    }

    exit(EXIT_SUCCESS);
}

void *run(void *arg) {
    char buf; 
    // ECRITURE DANS LE PIPE D'ECRITURE DE CE QUI EST LU SUR STDIN
    while (read(STDIN_FILENO, &buf, 1) > 0) {      
        CHECK_CONDITION((write(*(int *)arg, &buf, 1) == 1), "Ecriture erreur");
    }
    return arg;
}

void getNamedPipe(pid_t pid_number, char *output, char *input) {

    char * verify = strncpy(output, TUBE_NAME, 15);  
    CHECK_CONDITION((strcmp(verify, output) == 0), "Copy error");
    
    verify = strncpy(input, TUBE_NAME, 15);
    CHECK_CONDITION((strcmp(verify, input) == 0), "Copy error");
    
    char pid[10];

    CHECK_CONDITION((snprintf(pid, 10, "%d", pid_number) >= 0), "Error pid");
        
    verify = strncat(output, pid, BUFFER_SIZE);
    CHECK_CONDITION((strcmp(verify, output) == 0), "Copy error");
    
    verify = strncat(input, pid, BUFFER_SIZE);
    CHECK_CONDITION((strcmp(verify, input) == 0), "Copy error");

    verify = strncat(output, OUTPUT, 7);
    CHECK_CONDITION((strcmp(verify, output) == 0), "Copy error");

    verify = strncat(input, INPUT, 6);
    CHECK_CONDITION((strcmp(verify, input) == 0), "Copy error");
}

void checkCondition(const char *file, int line, bool expr, const char *cause) {
    if (expr) {
        return;
    }

    fprintf(stderr, "*** Abnormal program termination: %s\n"
                    "*** file: %s, line: %d \n",
                    cause, file, line);
    destroy();
}

void checkPerror(bool expr, const char *message) {
    if (!expr) {
        return;
    }
    
    perror(message);
    destroy();
}