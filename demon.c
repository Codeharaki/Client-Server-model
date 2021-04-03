#include "util.h"

void *run(void * arg);
void gestionnaire(int signum);
void destroy();

int main(void) {
    printf("Démon en cours d'execution...\n");
    int shm_fd = shm_open(MON_SHM, O_CREAT | O_RDWR, 
        S_IRUSR | S_IWUSR | S_IXUSR);

    PERROR(shm_fd == -1, "shm_open");    
    PERROR((ftruncate(shm_fd, SHM_SIZE) == -1), "shm_truncate");

    // PROJECTION DE NOTRE FILE */
    fifo *result = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, 
        shm_fd, 0);

    PERROR(result == MAP_FAILED, "mmap");

    // INITIALISATION DE NOTRE FILE 
    result->read = 0;
    result->write = 0;
    result->request_number = 0;

    // INITIALISATION DES SEMAPHORES //
    PERROR((sem_init(&result->mutex, 1, 1) == -1), "sem_init");
    PERROR((sem_init(&result->empty, 1, MESSAGE_NUMBER) == -1), "sem_init");
    PERROR((sem_init(&result->full, 1, 0) == -1), "sem_init");    

    // CONSTRUCTION DES SIGNAUX
    struct sigaction action;
    action.sa_handler = gestionnaire;
    action.sa_flags = SA_RESTART;

    PERROR((sigfillset(&action.sa_mask) == -1), "sigfillset");
    PERROR((sigaction(SIGTERM, &action, NULL) == -1), "sigaction");
    PERROR((sigaction(SIGINT, &action, NULL) == -1), "sigaction");
    PERROR((sigaction(SIGTSTP, &action, NULL) == -1), "sigaction");
    PERROR((sigaction(SIGHUP, &action, NULL) == -1), "sigaction");
    PERROR((sigaction(SIGCHLD, &action, NULL) == -1), "sigaction");
    PERROR((sigaction(SIGPIPE, &action, NULL) == -1), "sigaction");

    while (true) {
        PERROR((sem_wait(&result->full) == -1), "sem_wait");

        request r = result->buffer[result->read];
        result->read = (result->read + 1) % MESSAGE_NUMBER;
        result->request_number -= 1;

        pthread_t th;
        
        CHECK_CONDITION((pthread_create(&th, NULL, run, &r)) == 0,
            "Erreur Pthread Create");

        // On detache le thread
        CHECK_CONDITION(pthread_detach(th) == 0, "Erreur de détachement"); 
        
        PERROR((sem_post(&result->empty)), "sem_post"); 
    }
    
        
    return EXIT_SUCCESS;
}

void *run(void * arg) {
    if (arg == NULL) {
        return NULL;
    }
    request *r = (request *)arg;
    switch (fork()) {
        case -1:
        PERROR(true, "fork");
        case 0: {

        char input[BUFFER_SIZE];
        memset(input, 0, BUFFER_SIZE);
        char output[BUFFER_SIZE];
        memset(output, 0, BUFFER_SIZE);
        getNamedPipe(r->pid, output, input);

        int fd1 = open(output, O_WRONLY);
        int fd2 = open(input, O_RDONLY);

        PERROR((fd1 == -1), "fd1");  
        PERROR((fd2 == -1), "fd2"); 

        // REDIRECTION DES ENTREES / SORTIES STANDARD 
        PERROR((dup2(fd1, STDOUT_FILENO) == -1), "dup2");
        PERROR((dup2(fd2, STDIN_FILENO) == -1), "dup2");

        PERROR((close(fd1) == -1), "close");
        PERROR((close(fd2) == -1), "close");

        // RECUPERATION DES ARGUMENTS DE LA COMMANDE A EXECUTER
        char *arguments[ARGUMENTS_NUMBER] = {NULL};
        char *token = NULL;

        token = strtok(r->message, " ");
        int i = 0;

        while (token != NULL && i < ARGUMENTS_NUMBER) {
            arguments[i] = malloc(sizeof(char *) * MESSAGE_LENGTH);
            CHECK_CONDITION(arguments[i] != NULL, "memory stack");
            char *verify = strncpy(arguments[i], token, MESSAGE_LENGTH);
            CHECK_CONDITION((strcmp(verify, arguments[i]) == 0), "Copy error");
            token = strtok(NULL, " ");
            ++i;
        }
        execvp(arguments[0], arguments);
        
        perror("execlp");
        }
        
    }
    return NULL;
}

void gestionnaire(int signum) {
    CHECK_CONDITION((signum >= 0), "Wrong signal number");

    switch (signum) {
        case SIGINT:
        case SIGTERM:
        case SIGTSTP:
        case SIGHUP:
        destroy();
        break;
        case SIGPIPE:
        printf("plus de pipe \n");
        break;
        case SIGCHLD: {
        int r; 
            do {
                r = waitpid(-1, NULL, WNOHANG);
            } while (r > 0);
        }
        break;
    }
}

// Libération de la mémoire.
void destroy() {    
    PERROR((shm_unlink(MON_SHM) == -1), "shm_unlink");
    exit(EXIT_SUCCESS);
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