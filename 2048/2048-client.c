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

#define ENDEBUG

#ifdef ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif

static const char* modulname;

sem_t *s1;
sem_t *s2;

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

static enum game_command get_next_command() {
    char command;

    
    while (true) {
        printf("Enter a command please: ");
        scanf("%c\n",&command);
        switch (command) {
            case 'w':
                return CMD_UP;
                break;
            case 'a':
                return CMD_LEFT;
                break;
            case 's':
                return CMD_DOWN;
                break;
            case 'd': 
                return CMD_RIGHT;
                break;
            case 'q':
                return CMD_QUIT;
                break;
            case 't':
                exit(EXIT_SUCCESS);
            default: 
                printf("wrong command!\n");
                break;
        }
    }
}


int connect() {
    
    struct connect *connection = (struct connect*)create_shared_memory(sizeof *connection, SHM_NAME, O_RDWR);

    if ((s1 = sem_open(SEM_1, 0)) == SEM_FAILED) {
        bail_out("", EXIT_FAILURE, "error sem_open%s\n", strerror(errno));
    }

    if ((s2 = sem_open(SEM_2, 0)) == SEM_FAILED) {
        bail_out("", EXIT_FAILURE, "error sem_open%s\n", strerror(errno));
    }

    // connection->new_game = false; 
    sem_post(s1);
    sem_wait(s2);
    DEBUG("Client: received game_id: %d\n", connection->game_id);
    return connection->game_id; 
}

int main(int argc, char* argv[]) {
    modulname = argv[0];

    int c; 
    bool new_game = false;
    int game_id; 

    if (atexit(free_resources) != 0) {
        bail_out("", EXIT_FAILURE, "Error atexit");
    } 

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

    game_id = connect();

    struct game_data *data = (struct game_data*)create_shared_memory(sizeof *data, SHM_NAME, O_RDWR);

    char *game_name1 = get_game_sem(game_id, 1);
    char *game_name2 = get_game_sem(game_id, 2);

    if ((s1 = sem_open(game_name1, 0)) == SEM_FAILED) {
        bail_out("", EXIT_FAILURE, "error sem_open%s\n", strerror(errno));
    }

    if ((s2 = sem_open(game_name2, 0)) == SEM_FAILED) {
        bail_out("", EXIT_FAILURE, "error sem_open%s\n", strerror(errno));
    }

    while (true) {
        sem_wait(s1); 
        render_field(data->field); 
        data->command = get_next_command(); 
        sem_post(s2);
    }

    return EXIT_SUCCESS;
}

