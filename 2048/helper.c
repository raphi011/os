/** 
 * @file helper.c
 * @author Raphael Gruber (0828630)
 * @brief Shared functions for 2048 client / server 
 * @date 07.06.2015
 */

#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "helper.h"


long parse_int(char * str, int * ptr) {
    char * endptr;
    
    *ptr = strtol(str, &endptr, 10);
    
    if (errno == ERANGE) {
        return 0;
    }

    return (*endptr == '\0'); 
}


int get_index(int x, int y) {
    if (x >= FIELD_SIZE || x < 0 ||
        y >= FIELD_SIZE || y < 0) {
        return -1;    
    }

    return y * FIELD_SIZE + x;
}

void sem_post_sec(sem_t *sem) {
    if (sem_post(sem) == -1) {
        bail_out(EXIT_FAILURE, "error sem_post");
    }
}

void sem_wait_sec(sem_t *sem) {
    if (sem_wait(sem) == -1) {
        if (errno != EINTR) {
           bail_out(EXIT_FAILURE, "error sem_wait");  
        }
    }
}

char *get_game_shm(int game_id) {
    char * buffer = malloc(10); 
    if (buffer == NULL) {
        bail_out(EXIT_FAILURE, "error malloc");
    }
    if (sprintf(buffer, "%s%d", SHM_GAME, game_id) < 0 ) {
        bail_out(EXIT_FAILURE, "error sprintf");
    }
    buffer = realloc(buffer, strlen(buffer) + 1);   

    if (buffer == NULL) {
        bail_out(EXIT_FAILURE, "error realloc");
    }

    return buffer;
}

char *get_game_sem(int game_id, int sem_num) {
    char * buffer = malloc(10); 
    if (buffer == NULL) {
        bail_out(EXIT_FAILURE, "error malloc");
    }
    if (sprintf(buffer, "%s%d-%d", SEM_GAME, game_id, sem_num ) < 0 ) {
        bail_out(EXIT_FAILURE, "error sprintf");
    }

    buffer = realloc(buffer, strlen(buffer) + 1);   

    if (buffer == NULL) {
        bail_out(EXIT_FAILURE, "error realloc");
    }

    return buffer;
}


void bail_out(int eval, const char *fmt, ...) {
    va_list ap;

    (void) fprintf(stderr, "%s: ", modulname);
    if (fmt != NULL) {
        va_start(ap, fmt);
        (void) vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    if (errno != 0) {
        (void) fprintf(stderr, ": %s", strerror(errno));
    }
    (void) fprintf(stderr, "\n");

    exit(eval);
}

void *create_shared_memory(size_t size, char * name, int oflag) {
    void * data;
    int shared_memory_fd; 
 
    if ((shared_memory_fd = shm_open(name, oflag, PERMISSION)) == -1) {
        bail_out(EXIT_FAILURE, "error shm_open");
    }

    if (ftruncate(shared_memory_fd, size) == -1) {
        bail_out(EXIT_FAILURE, "error ftruncate");
    }

    data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
            shared_memory_fd, 0);

    if (data == MAP_FAILED) {
        bail_out(EXIT_FAILURE, "error mmap");
    }

    if (close(shared_memory_fd) == -1) {
        bail_out(EXIT_FAILURE, "error close");
    }

    return data;
}

