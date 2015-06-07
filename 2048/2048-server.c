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

#define COUNT_OF(x) (sizeof(x)/sizeof(x[0]))

#ifdef ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif

char* modulname;

volatile sig_atomic_t terminating = 0;

static int connected_clients = 0;

static struct connect *connection;

static pid_t pid; 

static sem_t *s1;
static sem_t *s2;
static sem_t *child_1;
static sem_t *child_2;

static char * game_name1;
static char * game_name2; 
static char * shm_game; 

static struct game_data *data;

static void usage() {
    (void)fprintf(stderr, "USAGE: %s [-p power_of_two]\n\t-p:\tPlay until 2ˆpower_of_two is reached (default: 11)\n",modulname);
    exit(EXIT_FAILURE);
}

static void free_resources(void) {
    
    sigset_t blocked_signals;
    (void) sigfillset(&blocked_signals);
    (void) sigprocmask(SIG_BLOCK, &blocked_signals, NULL);

    /* signals need to be blocked here to avoid race */
    if(terminating == 1) {
        return;
    }
    terminating = 1;

    
    if (pid == 0) { // child
        DEBUG("Child shutting down\n");

        if (sem_close(child_1) == -1) {
            fprintf(stderr, "sem_close child_1 failed\n");
        }

        if (sem_close(child_2) == -1) {
            fprintf(stderr, "sem_close child_2 failed\n");
        }

        if (sem_unlink(game_name1) == -1) {
            fprintf(stderr, "sem_unlink failed\n");
        }
        
        if (sem_unlink(game_name2) == -1) {
            fprintf(stderr, "sem_unlink failed\n");
        }

        if (munmap(data, sizeof (*data)) == -1) {
            fprintf(stderr, "game_data\n");
        }

        if (shm_unlink(shm_game) == -1) {
            fprintf(stderr, "shm_unlink failed\n");
        }

        (void)free(game_name1);
        (void)free(game_name2);

    } else { // parent 

        if (sem_close(s1) == -1) {
            fprintf(stderr, "sem_close failed\n");
        }
        if (sem_close(s2) == -1) {
            fprintf(stderr, "sem_close failed\n");
        }

        if (sem_unlink(SEM_CON_1)) {
            fprintf(stderr, "sem_unlink failed\n");
        }
        if (sem_unlink(SEM_CON_2)) {
            fprintf(stderr, "sem_unlink failed\n");
        }

        if (munmap(connection, sizeof (*connection)) == -1) {
            fprintf(stderr, "munmap failed\n");
        }

        if (shm_unlink(SHM_CON) == -1) {
            fprintf(stderr, "shm_unlink failed\n");
        }
    }
}

static void signal_handler()
{
    // freeing resources is handled by 'atexit'
    exit(EXIT_SUCCESS);
}

int get_empty_field(int field[]) {
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

int get_prev_index(int index, enum game_command cmd) {

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

void add_random_value(struct game_data *data) {
    int r = rand() % 3;
    int new_value = (r == 0 ? 2 : 4);
    int next_empty_field = get_empty_field(data->field);   

    data->field[next_empty_field] = new_value;

    if (next_empty_field == -1) {
        data->state = ST_LOST;
        return;
    }
}

bool make_game_move(struct game_data *data, int power_of_two) {
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

    if (get_empty_field(data->field) == -1) {
        data->state = ST_LOST;
    } else if (highest_exp == power_of_two) {
        data->state = ST_WON;
    }

    return movement;
}

void new_game(int game_id, int power_of_two) {
    DEBUG("Server: created new game with id %d\n", game_id);

    shm_game = get_game_shm(game_id);

    data = (struct game_data*)create_shared_memory(sizeof *data, shm_game, O_RDWR | O_CREAT); 
    
    add_random_value(data);
    add_random_value(data);

    while (true) {

        sem_post(child_1); 
        sem_wait(child_2);

        if (data->command == CMD_QUIT) {
            exit(EXIT_SUCCESS);
        }

        if (data->command != CMD_NONE && make_game_move(data, power_of_two)) {
            add_random_value(data);
        }

        switch (data->state) {
            case ST_LOST: 
            case ST_WON:
                exit(EXIT_SUCCESS);
            default: 
                break;
        }

    }
}

int main(int argc, char* argv[]) {
    modulname = argv[0];

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
        // wait for a new connection
        if (sem_wait(s1) == -1) {
            bail_out(EXIT_FAILURE, "error sem_wait: %s\n", strerror(errno));
        }

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
        
        if (sem_post(s2) == -1) {
            bail_out(EXIT_FAILURE, "error sem_post: %s\n", strerror(errno));
        }
    }

    return EXIT_SUCCESS;
}

