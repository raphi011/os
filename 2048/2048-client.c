/** 
 * @file 2048-client.c
 * @author Raphael Gruber (0828630)
 * @brief The client implementation of 2048
 * @date 07.06.2015
 */

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

/** 
 * The name of the program
 */
char* modulname;

#define INPUT_BUFFER_SIZE (100)

/**
 * Semaphore for communication with the server
 */
sem_t *s1 = NULL;

/**
 * Semaphore for communication with the server
 */
sem_t *s2 = NULL;

/**
 * Name of the first semaphore 
 */
char *game_name1;
/**
 * Name of the second semaphore 
 */
char *game_name2;

/**
 * Name of the game shared memory
 */
char * shm_game;

/**
 * Shared game data
 */
struct game_data *data;

/**
 * usage
 * @brief prints the usage message with the correct call syntax and exits
 * with EXIT_FAILURE
 * @details Global parameters: modulname
 */
static void usage(void) {
    (void)fprintf(stderr, "USAGE: %s [-n | -i <id>]\n\t-n:\tStart a new game\n\t-i:\tConnect to existing game with the given id",modulname);
    exit(EXIT_FAILURE);
}

/** 
 * free_resources
 * @brief frees open resources and signal handlers
 * @details Global parameters: s1, s2, data, game_name1, game_name2, shm_game, modulname 
 */
static void free_resources(void) {

    if (s1) {
        int sem_clo_1 = sem_close(s1);     
        if (sem_clo_1) {
            (void)fprintf(stderr, "%s: error sem_close\n", modulname);
        }
    }
    if (s2) {
        int sem_clo_2 = sem_close(s2);     
        if (sem_clo_2) {
            (void)fprintf(stderr, "%s: error sem_close\n", modulname);
        }
    }
    
    if (data != NULL) {
        if (munmap(data, sizeof (*data)) == -1) {
            (void)fprintf(stderr, "%s: error munmap\n", modulname);
        }
    }

    (void)free(game_name1);
    (void)free(game_name2);
    (void)free(shm_game);
}

/**
 * draw_field
 * @brief draws the board onto the screen
 * @param field the field
 */
static void draw_field(int field[]) {
    (void)printf("\n");
    for (int y = 0; y < FIELD_SIZE; y++) {
        for (int x = 0; x < FIELD_SIZE; x++) {
            int position = get_index(x,y);
            (void)printf("\t%d", field[position]);
        }
        (void)printf("\n");
    }
}

/**
 * get_next_command 
 * @brief prompts the user for the next command
 */
static enum game_command get_next_command(void) {
    char buffer[INPUT_BUFFER_SIZE]; 
    
    while (true) {
        (void)printf("Enter a command please: ");

        (void)fgets(buffer, INPUT_BUFFER_SIZE, stdin);

        if (buffer[1] != '\n') {
            (void)printf("Please enter a command followed by a newline\n"); 
            continue;
        }

        switch (buffer[0]) {
            case 'w':
                return CMD_UP;
            case 'a':
                return CMD_LEFT;
            case 's':
                return CMD_DOWN;
            case 'd': 
                return CMD_RIGHT;
            case 'q':
                return CMD_QUIT;
            case 't':
                return CMD_NONE;
            default: 
                break;
        }
    }
}



/**
 * connect
 * @brief connects to the server and gets the game id
 * @details Global parameters: SHM_CON  
 */
static int connect(void) {
    

    sem_t *sem_con_1 = sem_open(SEM_CON_1, 0);
    sem_t *sem_con_2 = sem_open(SEM_CON_2, 0); 

    if (sem_con_1 == SEM_FAILED || sem_con_2 == SEM_FAILED) {
        if (errno == ENOENT) {
            bail_out(EXIT_FAILURE, "seems like the server hasn't started yet\n");
        } else {
            bail_out(EXIT_FAILURE, "error sem_open%s\n", strerror(errno));
        }
    }

    struct connect *connection = (struct connect*)create_shared_memory(sizeof *connection, SHM_CON, O_RDWR);

    sem_post_sec(sem_con_1);
    sem_wait_sec(sem_con_2);

    int game_id = connection->game_id;
    DEBUG("%s: received game_id: %d\n", modulname, game_id);

    int sem_clo_1 = sem_close(sem_con_1);     
    int sem_clo_2 = sem_close(sem_con_2);     

    if (sem_clo_1 || sem_clo_2) {
        (void)fprintf(stderr, "%s: error sem_close\n", modulname);
    }

    if (munmap(connection, sizeof(*connection)) == -1) {
        (void)fprintf(stderr, "%s: error munmap\n", modulname);
    }

    (void)printf("Game ID: %d\nCommands: \tw: up, a: left, s: down, d: right\n\t\tq: quit, t: close\n", game_id);

    return game_id; 
}

/**
 * main
 * @brief The main entry point of the program.
 * @param argc The number of command-line parameters in argv.
 * @param argv The array of command-line parameters, argc elements long.
 * @details Global variables: modulname, params
 * @return The exit code of the program. EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int main(int argc, char* argv[]) {
    modulname = argv[0];

    int c; 
    bool new_game = false;
    int game_id = 0;

    if (atexit(free_resources) != 0) {
        bail_out(EXIT_FAILURE, "error atexit\n");
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

    if (game_id == 0) {
        game_id = connect();
    }


    game_name1 = get_game_sem(game_id, 1);
    game_name2 = get_game_sem(game_id, 2);

    s1 = sem_open(game_name1, 0);
    s2 = sem_open(game_name2, 0);

    if (s1 == SEM_FAILED || s2 == SEM_FAILED) {
        if (errno == ENOENT) {
            bail_out(EXIT_FAILURE, "seems like the server hasn't started yet\n");
        } else {
            bail_out(EXIT_FAILURE, "error sem_open%s\n", strerror(errno));
        }
    }

    sem_wait_sec(s1);

    shm_game = get_game_shm(game_id); 

    data = (struct game_data*)create_shared_memory(sizeof *data, shm_game, O_RDWR);

    draw_field(data->field); 

    while (data->state == ST_RUNNING) {

        data->command = get_next_command(); 

        sem_post_sec(s2);

        switch (data->command) {
            case CMD_QUIT:
            case CMD_NONE: 
                return EXIT_SUCCESS; 
            default: break;
        }

        sem_wait_sec(s1); 
        
        switch (data->state) {
            case ST_LOST:
                (void)printf("You've lost\n");
                return EXIT_SUCCESS;
            case ST_WON:
                (void)printf("You've won\n");
                return EXIT_SUCCESS;
            default:
                break;
        }

        draw_field(data->field); 
    }

    return EXIT_SUCCESS;
}

