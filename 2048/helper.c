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


/** parse_int
 * @brief Parses an int value
 * @params str The char * that will be parsed
 * @params ptr The int * where the value will be stored
 * @return 1 if str was an int, else 0
 */
int parse_int(char * str, int * ptr) {
    char * endptr;
    
    *ptr = strtol(str, &endptr, 10);
    
    // todo, function doesn't check all errors
    return (*endptr == '\0'); 
}

char *get_game_sem(int game_id, int sem_num) {
    char * buffer = malloc(10); 
    sprintf(buffer, "%s%d-%d", SEM_GAME, game_id, sem_num );
    buffer = realloc(buffer, strlen(buffer) + 1);   

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
        bail_out(EXIT_FAILURE, "Error shm_open");
    }

    if (ftruncate(shared_memory_fd, size) == -1) {
        bail_out(EXIT_FAILURE, "Error ftruncate");
    }

    data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
            shared_memory_fd, 0);

    if (data == MAP_FAILED) {
        bail_out(EXIT_FAILURE, "Error mmap");
    }

    if (close(shared_memory_fd) == -1) {
        bail_out(EXIT_FAILURE, "Error close");
    }

    return data;
}

