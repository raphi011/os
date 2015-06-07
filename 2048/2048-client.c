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

char* modulname;

#define INPUT_BUFFER_SIZE (100)

sem_t *s1;
sem_t *s2;

static void usage() {
    (void)fprintf(stderr, "USAGE: %s [-n | -i <id>]\n\t-n:\tStart a new game\n\t-i:\tConnect to existing game with the given id",modulname);
    exit(EXIT_FAILURE);
}

static void free_resources(void) {
    // todo
}

static void draw_field(int field[]) {
    printf("\n");
    for (int y = 0; y < FIELD_SIZE; y++) {
        for (int x = 0; x < FIELD_SIZE; x++) {
            int position = get_index(x,y);
            printf("\t%d", field[position]);
        }
        printf("\n");
    }
}

static enum game_command get_next_command() {
    char buffer[INPUT_BUFFER_SIZE]; 
    
    while (true) {
        printf("Enter a command please: ");
        fgets(buffer, INPUT_BUFFER_SIZE, stdin);

        if (buffer[1] != '\n') {
            (void)printf("Please enter a command followed by a newline\n"); 
            continue;
        }

        switch (buffer[0]) {
            case 'w':
                printf("received cmd_up\n");
                return CMD_UP;
            case 'a':
                printf("received cmd_left\n");
                return CMD_LEFT;
            case 's':
                printf("received cmd_down\n");
                return CMD_DOWN;
            case 'd': 
                printf("received cmd_right\n");
                return CMD_RIGHT;
            case 'q':
                printf("received cmd_quit\n");
                return CMD_QUIT;
            case 't':
                printf("received cmd_terminate\n");
                return CMD_NONE;
            default: 
                printf("wrong command\n");
                break;
        }
    }
}


int connect() {
    
    struct connect *connection = (struct connect*)create_shared_memory(sizeof *connection, SHM_CON, O_RDWR);

    if ((s1 = sem_open(SEM_CON_1, 0)) == SEM_FAILED) {
        bail_out(EXIT_FAILURE, "error sem_open%s\n", strerror(errno));
    }

    if ((s2 = sem_open(SEM_CON_2, 0)) == SEM_FAILED) {
        bail_out(EXIT_FAILURE, "error sem_open%s\n", strerror(errno));
    }

    sem_post(s1);
    sem_wait(s2);

    int game_id = connection->game_id;
    DEBUG("Client: received game_id: %d\n", game_id);

    if (munmap(connection,sizeof(*connection)) == -1) {
        bail_out(EXIT_FAILURE, "error munmap failed\n");
    }

    return game_id; 
}

int main(int argc, char* argv[]) {
    modulname = argv[0];

    int c; 
    bool new_game = false;
    int game_id = 0;

    if (atexit(free_resources) != 0) {
        bail_out(EXIT_FAILURE, "Error atexit\n");
    } 

    while ((c = getopt (argc, argv, "ni:")) != -1) {
        switch (c) {
            case 'i':
                if (new_game) {
                    usage();
                }
                if (!parse_int(optarg, &game_id)) {
                   usage();   
                } 
                break;
            case 'n':
                if (game_id != 0) {
                    usage();
                }
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

    fflush(stdin);
    fflush(stdout);

    if (game_id == 0) {
        game_id = connect();
    }


    char *game_name1 = get_game_sem(game_id, 1);
    char *game_name2 = get_game_sem(game_id, 2);

    if ((s1 = sem_open(game_name1, 0)) == SEM_FAILED) {
        bail_out(EXIT_FAILURE, "error sem_open%s\n", strerror(errno));
    }

    if ((s2 = sem_open(game_name2, 0)) == SEM_FAILED) {
        bail_out(EXIT_FAILURE, "error sem_open%s\n", strerror(errno));
    }

    sem_wait(s1);

    char * shm_game = get_game_shm(game_id); 

    struct game_data *data = (struct game_data*)create_shared_memory(sizeof *data, shm_game, O_RDWR);

    draw_field(data->field); 

    while (data->state == ST_RUNNING) {

        data->command = get_next_command(); 

        sem_post(s2);

        switch (data->command) {
            case CMD_QUIT:
            case CMD_NONE: 
                return EXIT_SUCCESS; 
            default: break;
        }

        sem_wait(s1); 
        
        switch (data->state) {
            case ST_LOST:
                (void)printf("You've lost");
                return EXIT_SUCCESS;
            case ST_WON:
                (void)printf("You've won");
                return EXIT_SUCCESS;
            default:
                break;
        }

        draw_field(data->field); 
    }

    return EXIT_SUCCESS;
}

