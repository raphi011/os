#define _GNU_SOURCE

#include <sys/mman.h>
#include <fcntl.h>

#include <semaphore.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "helper.h"

static const char* modulname;

int connected_clients = 3;


static void usage() {
    (void)fprintf(stderr, "USAGE: %s [-p power_of_two]\n\t-p:\tPlay until 2ˆpower_of_two is reached (default: 11)\n",modulname);
    exit(EXIT_FAILURE);
}

static void free_resources(void) {
    
    // (void)munmap(shared, sizeof *shared);
    (void)shm_unlink(SHM_NAME);

    // (void) sem_close();
    // (void sem_unlink();

}


// int create_semaphore(int game_id) {
// 
// }


int wait_for_connection() {
    int connecting_process = -1;

    struct connect *connection = (struct connect*)create_shared_memory(sizeof *connection, SHM_NAME, O_RDWR | O_CREAT);

    sem_t *s1 = sem_open(SEM_1, O_CREAT | O_EXCL, 0600, 0);
    sem_t *s2 = sem_open(SEM_2, O_CREAT | O_EXCL, 0600, 0);
    
    printf("Server: waiting for client\n");
    sem_wait(s1);
    printf("Server: assigning game_id\n");
    connection->game_id = connected_clients;
    printf("Server: assigned game_id: %d\n", connection->game_id);
    sem_post(s2);
    
    return connected_clients++;
}

int main(int argc, char* argv[]) {

    if (atexit(free_resources) != 0) {
        // bail_out(modulname, EXIT_FAILURE, "Error atexit");
        bail_out("", EXIT_FAILURE, "Error atexit");
    }
    
    modulname = argv[0];
    int c; 
    int power_of_two;

    while ((c = getopt (argc, argv, "p:")) != -1) {
        switch (c) {
            case 'p':
                if (parse_int(optarg, &power_of_two)) {
                    usage();   
                } 
                break;
            case '?':
                usage();   
            default: 
                assert(0);
        }
    }

    if (argc != optind) {
        usage(); 
    }
    
    (void) wait_for_connection(); 
//     (void) create_shared_memory();

    return EXIT_SUCCESS;
}

