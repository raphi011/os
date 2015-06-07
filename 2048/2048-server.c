/** 
 * @file 2048-server.c
 * @author Raphael Gruber (0828630)
 * @brief The server implementation of 2048
 * @date 07.06.2015
 */

#define _GNU_SOURCE

#include <math.h>
#include <time.h>
#include <sys/mman.h>
#include <signal.h>
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

#define ENDEBUG

/**
 * Calculates the size of an array
 */
#define COUNT_OF(x) (sizeof(x)/sizeof(x[0]))

#ifdef ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif

/** 
 * The name of the program
 */
char* modulname;

/**
 * 1 when a signal was received
 */
volatile sig_atomic_t terminating = 0;

/**
 * count of clients that connected
 */
int connected_clients = 0;

/**
 * shared memory for establishing a connection
 */
struct connect *connection;

/** 
 * fork pid
 */
pid_t pid; 

/** 
 * true if a game has started
 */
bool started;


/**
 * semaphores
 */
sem_t *s1;
sem_t *s2;
sem_t *child_1;
sem_t *child_2;

/**
 * semaphore names
 */
char * game_name1;
char * game_name2; 
char * shm_game; 

/**
 * game data shared memory
 */
struct game_data *data;

/**
 * usage
 * @brief prints the usage message with the correct call syntax and exits
 * with EXIT_FAILURE
 * @details Global parameters: modulname
 */
static void usage(void) {
    (void)fprintf(stderr, "USAGE: %s [-p power_of_two]\n\t-p:\tPlay until 2ˆpower_of_two is reached (default: 11)\n",modulname);
    exit(EXIT_FAILURE);
}

/** 
 * free_resources
 * @brief frees open resources and signal handlers
 */
static void free_resources(void) {
    
    sigset_t blocked_signals;
    (void) sigfillset(&blocked_signals);
    (void) sigprocmask(SIG_BLOCK, &blocked_signals, NULL);

    /* signals need to be blocked here to avoid race */
    if(terminating == 1) {
        return;
    }
    terminating = 1;

    
    if (started && pid == 0) { // child
        DEBUG("Child shutting down\n");

        int sem_clo_1 = sem_close(child_1);
        int sem_clo_2 = sem_close(child_2);

        if (sem_clo_1 || sem_clo_2) {
            (void)fprintf(stderr, "%s: error sem_close\n", modulname);
        }

        if (game_name1) {
            int sem_unl_1 = sem_unlink(game_name1);
            if (sem_unl_1 != 0) {
                (void)fprintf(stderr, "%s: error sem_unlink\n", modulname);
            }
        }
        if (game_name2) {
            int sem_unl_2 = sem_unlink(game_name2);
            if (sem_unl_2 != 0) {
                (void)fprintf(stderr, "%s: error sem_unlink\n", modulname);
            }
        }

        if (data != NULL) {
            if (munmap(data, sizeof (*data)) == -1) {
                (void)fprintf(stderr, "%s: error munmap\n", modulname);
            }
        }

        if (shm_game != NULL) {
            if (shm_unlink(shm_game) == -1) {
                (void)fprintf(stderr, "%s: error shm_unlink\n", modulname);
            }
        }

        (void)free(game_name1);
        (void)free(game_name2);

    } else { // parent 
        DEBUG("Parent shutting down\n");

        int sem_clo_1 = sem_close(s1);
        int sem_clo_2 = sem_close(s2);

        if (sem_clo_1 || sem_clo_2) {
            (void)fprintf(stderr, "%s: error sem_close\n", modulname);
        }

        int sem_unl_1 = sem_unlink(SEM_CON_1);
        int sem_unl_2 = sem_unlink(SEM_CON_2);

        if (sem_unl_1 || sem_unl_2) {
            (void)fprintf(stderr, "%s: error sem_unlink\n", modulname);
        }

        if (munmap(connection, sizeof (*connection)) == -1) {
            (void)fprintf(stderr, "%s: error munmap\n", modulname);
        }

        if (shm_unlink(SHM_CON) == -1) {
            (void)fprintf(stderr, "%s: error shm_unlink\n", modulname);
        }
    }
}

/**
 * signal_handler
 * @brief The signal handler
 * @param sig The signal that occured
 */
void signal_handler(int sig) {
    (void)printf("%s: received signal %d\n",modulname, sig);
    // freeing resources is handled by 'atexit'
    exit(EXIT_SUCCESS);
}

/**
 *  get_empty_field
 * @brief looks for an empty field
 * @param field the game field
 * @return returns the index for an empty field or -1 if none are remaining
 */
static int get_empty_field(int field[]) {
    int free_fields[FIELD_SIZE * FIELD_SIZE];
    int free_count = 0;

    for (int x = 0; x < FIELD_SIZE; x++) {
        for (int y = 0; y < FIELD_SIZE; y++) {
            int index = get_index(x,y);            
            if (field[index] == 0) {
                free_fields[free_count++] = index;
            }
        }
    }

    if (free_count == 0) {
        return -1;
    } else {
        int r = rand() % free_count;
        return free_fields[r]; 
    }
}

/**
 *  get_prev_index
 * @brief returns previous index depending on movement direction
 * @param index the current index
 * @param cmd the movement direction
 * @return previous index
 */
static int get_prev_index(int index, enum game_command cmd) {

    switch (cmd) {
         case CMD_UP: 
             index -= FIELD_SIZE;
             break;
         case CMD_LEFT:
             index -= 1;
             break;
         case CMD_DOWN:
             index += FIELD_SIZE; 
             break;
         case CMD_RIGHT:
             index += 1;  
             break;
         default: 
             assert(0);
    } 

    return index;
}

/**
 * move_available
 * @brief checks if there are game moves remaining
 * @param field the game field
 * @return returns true if there are game moves remaining
 */
static bool move_available(int field[]) {

    for (int x = 0; x < FIELD_SIZE; x++) {
        for (int y = 0; y < FIELD_SIZE; y++) {

        int index = get_index(x,y);

        for (int direction = 0; direction < 4; direction++) {
            int neighbour_index;

            switch (direction) {
                case 0: // up
                    if ((neighbour_index = get_index(x,y-1)) != -1 &&
                        field[index] == field[neighbour_index]) {
                        return true;
                    }
                case 1: // right
                    if ((neighbour_index = get_index(x+1,y)) != -1 &&
                        field[index] == field[neighbour_index]) {
                        return true;
                    }
                case 2: // down
                    if ((neighbour_index = get_index(x,y+1)) != -1 &&
                        field[index] == field[neighbour_index]) {
                        return true;
                    }
                case 3: // left
                    if ((neighbour_index = get_index(x-1,y)) != -1 &&
                        field[index] == field[neighbour_index]) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}
            
/**
 * add_random_value
 * @brief looks for an empty spot and sets it to 2 or 4 
 * @param data the game data
 */
static void add_random_value(struct game_data *data) {
    int r = rand() % 3;
    int new_value = (r == 0 ? 2 : 4);
    int next_empty_field = get_empty_field(data->field);   

    data->field[next_empty_field] = new_value;
   

    if (next_empty_field == -1) {
        data->state = ST_LOST;
        return;
    }
}

/**
 * make_game_move 
 * @brief makes a game move
 * @param data the game data
 * @param power_of_two power of two when game is won 
 * @return true if movement has occured`
 */
static bool make_game_move(struct game_data *data, int power_of_two) {
    enum game_command cmd = data->command;
    int *field = data->field;
    int highest_exp = 0;  
    bool movement = false;
    
    // true if movement is done on x axis
    bool direction_x = (cmd == CMD_LEFT || cmd == CMD_RIGHT);

    for (int a = 0; a < FIELD_SIZE; a++) {
        int y = cmd == CMD_UP ? a : (FIELD_SIZE - a - 1);

        for (int b = 0; b < FIELD_SIZE; b++) {

            int x = cmd == CMD_LEFT ? b : (FIELD_SIZE - b - 1);
            int cur_index = get_index(x,y); 
            int possible_movements = direction_x ? b : a;

            for (int mov = 0; mov < possible_movements; mov++) {
                int prev_index = get_prev_index(cur_index, data->command); 

                // 2 0 -> 2
                if (field[prev_index] == 0 && field[cur_index] != 0) {
                    movement = true;
                    field[prev_index] = field[cur_index]; 
                    field[cur_index] = 0;
                    cur_index = prev_index;
                    continue;        
                } else if (field[prev_index] == field[cur_index]) { // 2 2 -> 4
                    movement = true;
                    field[prev_index] *= 2;
                    field[cur_index] = 0;
                } 
                // 0 & 0 or e.g. 2 & 4
                break;
            }

            int exp = log2(field[get_index(x,y)]);

            if (exp > highest_exp) {
                highest_exp = exp;
            }
        }
    }

    if (get_empty_field(data->field) == -1 && !move_available(data->field)) {
        data->state = ST_LOST;
    } else if (highest_exp == power_of_two) {
        data->state = ST_WON;
    }

    return movement;
}

/**
 * new_game 
 * @brief  creates a new game
 * @param game_id the game id
 * @param power_of_two power of two when game is won 
 */
static void new_game(int game_id, int power_of_two) {
    DEBUG("Server: created new game with id %d\n", game_id);

    shm_game = get_game_shm(game_id);

    data = (struct game_data*)create_shared_memory(sizeof *data, shm_game, O_RDWR | O_CREAT); 
    
    add_random_value(data);
    add_random_value(data);

    while (true) {

        sem_post_sec(child_1); 

        switch (data->state) {
            case ST_LOST: 
            case ST_WON:
                exit(EXIT_SUCCESS);
            default: 
                break;
        }

        sem_wait_sec(child_2);

        if (data->command == CMD_QUIT) {
            exit(EXIT_SUCCESS);
        }

        if (data->command != CMD_NONE && make_game_move(data, power_of_two)) {
            add_random_value(data);
        }
    }
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
    started = false;

    srand(time(NULL));

    sigset_t blocked_signals;

    if (atexit(free_resources) != 0) {
        bail_out(EXIT_FAILURE, "Error atexit\n");
    }
    
    int c; 
    int power_of_two = 11;

    while ((c = getopt (argc, argv, "p:")) != -1) {
        switch (c) {
            case 'p':
                if (!parse_int(optarg, &power_of_two)) {
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
    
    /* setup signal handlers */
    if(sigfillset(&blocked_signals) < 0) {
        bail_out(EXIT_FAILURE, "sigfillset\n");
    } else {
        const int signals[] = { SIGINT, SIGQUIT, SIGTERM };
        struct sigaction s;
        s.sa_handler = signal_handler;
        (void) memcpy(&s.sa_mask, &blocked_signals, sizeof(s.sa_mask));
        s.sa_flags = SA_RESTART;
        for (unsigned int i = 0; i < COUNT_OF(signals); i++) {
            if (sigaction(signals[i], &s, NULL) < 0) {
                bail_out(EXIT_FAILURE, "sigaction\n");
            }
        }
    }

    s1 = sem_open(SEM_CON_1, O_CREAT | O_EXCL, PERMISSION, 0);
    s2 = sem_open(SEM_CON_2, O_CREAT | O_EXCL, PERMISSION, 0);

    struct connect *connection = (struct connect*)create_shared_memory(sizeof *connection, SHM_CON, O_RDWR | O_CREAT); 

    while (true) {
        sem_wait_sec(s1);
        started = true;

        connection->game_id = ++connected_clients;

        game_name1 = get_game_sem(connection->game_id, 1);
        game_name2 = get_game_sem(connection->game_id, 2);
                
        if ((child_1 = sem_open(game_name1, O_CREAT | O_EXCL, 0600, 0)) == SEM_FAILED) {
            bail_out(EXIT_FAILURE, "error sem_open%s\n", strerror(errno));
        }
        if ((child_2 = sem_open(game_name2, O_CREAT | O_EXCL, 0600, 0)) == SEM_FAILED) {
            bail_out(EXIT_FAILURE, "error sem_open%s\n", strerror(errno));
        }

        switch (pid = fork()) {
            case -1: bail_out(EXIT_FAILURE, "can't fork\n");
                     break;
                     // child process
            case 0:  
                     if (sem_close(s1) == -1) {
                         fprintf(stderr, "sem_close failed\n");
                     }
                     if (sem_close(s2) == -1) {
                         fprintf(stderr, "sem_close failed\n");
                     }
                     new_game(connection->game_id, power_of_two); 
                     break; 
            default: // parent process
                     if (sem_close(child_1) == -1) {
                         fprintf(stderr, "sem_close failed\n");
                     }
                     if (sem_close(child_2) == -1) {
                         fprintf(stderr, "sem_close failed\n");
                     }
                     break; 
        }
        
        sem_post_sec(s2);
    }

    return EXIT_SUCCESS;
}

