#define _GNU_SOURCE

#include <semaphore.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

#include "helper.h"

static const char* modulname;

static void usage() {
    (void)fprintf(stderr, "USAGE: %s [-n | -i <id>]\n\t-n:\tStart a new game\n\t-i:\tConnect to existing game with the given id",modulname);
    exit(EXIT_FAILURE);
}

static void free_resources(void) {

}

static void render_field(int field[]) {
    for (int y = 0; y < FIELD_SIZE; y++) {
        for (int x = 0; x < FIELD_SIZE; x++) {
            int position = y*FIELD_SIZE + x;
            printf("\t%d", field[position]);
        }
        printf("\n");
    }
}

int connect() {
    
    struct connect *connection = (struct connect*)create_shared_memory(sizeof *connection, SHM_NAME, O_RDWR);

    sem_t *s1 = sem_open(SEM_1, 0);
    sem_t *s2 = sem_open(SEM_2, 0);

    connection->new_game = false; 
    printf("Client: obtaining game_id\n");
    sem_post(s1);
    printf("Client: waiting for server\n");
    sem_wait(s2);
    printf("Client: received game_id: %d\n", connection->game_id);
    return connection->game_id; 
}

int main(int argc, char* argv[]) {
    modulname = argv[0];
    int c; 
    bool new_game = false;
    int game_id; 

    while ((c = getopt (argc, argv, "ni:")) != -1) {
        switch (c) {
            case 'i':
                if (parse_int(optarg, &game_id)) {
                    usage();   
                } 
                break;
            case 'n':
                new_game = true;
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

    //   int example_field[FIELD_SIZE*FIELD_SIZE] = {};
    // render_field(example_field);
    printf("game_id: %d", connect());

    return EXIT_SUCCESS;

}

