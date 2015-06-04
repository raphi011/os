#define _GNU_SOURCE

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

static const char* modulname;

volatile sig_atomic_t running = 1;
volatile sig_atomic_t terminating = 0;

int connected_clients = 0;

struct connect *connection;
struct running_game *games;

pid_t pid; 

sem_t *s1;
sem_t *s2;

sem_t *child_1;
sem_t *child_2;

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
    
    (void)sem_close(s1);
    (void)sem_close(s2);
    (void)sem_close(child_1);
    (void)sem_close(child_2); 

    if (pid != 0) {
        (void)sem_unlink(SEM_1); 
        (void)sem_unlink(SEM_2);
        (void)sem_unlink(get_game_sem(connection->game_id, 1));
        (void)sem_unlink(get_game_sem(connection->game_id, 2));
    }
}

static void signal_handler(int sig)
{
    DEBUG("Received signal! %d\n", sig);
    /* signals need to be blocked by sigaction */
    exit(EXIT_SUCCESS);
}

void make_game_move(struct game_data *data) {
    data->field[0] = data->command;
}

void new_game(int game_id) {

    DEBUG("Server: created new game with id %d\n", game_id);

    struct game_data *data= (struct game_data*)create_shared_memory(sizeof *data, SHM_NAME, O_RDWR | O_CREAT); 


    while (data->state == ST_RUNNING) {
        make_game_move(data); 
        
        sem_post(child_1); 
        sem_wait(child_2);
    }

    exit(EXIT_SUCCESS);
}

/*void remove_game(int game_id) {

} */

int main(int argc, char* argv[]) {
    modulname = argv[0];

    sigset_t blocked_signals;

    if (atexit(free_resources) != 0) {
        bail_out("", EXIT_FAILURE, "Error atexit");
    }
    
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
    
    /* setup signal handlers */
    if(sigfillset(&blocked_signals) < 0) {
        bail_out("", EXIT_FAILURE, "sigfillset");
    } else {
        const int signals[] = { SIGINT, SIGQUIT, SIGTERM };
        struct sigaction s;
        s.sa_handler = signal_handler;
        (void) memcpy(&s.sa_mask, &blocked_signals, sizeof(s.sa_mask));
        s.sa_flags = SA_RESTART;
        for (unsigned int i = 0; i < COUNT_OF(signals); i++) {
            if (sigaction(signals[i], &s, NULL) < 0) {
                bail_out("", EXIT_FAILURE, "sigaction");
            }
        }
    }

    s1 = sem_open(SEM_1, O_CREAT | O_EXCL, 0600, 0);
    s2 = sem_open(SEM_2, O_CREAT | O_EXCL, 0600, 0);

    
    struct connect *connection = (struct connect*)create_shared_memory(sizeof *connection, SHM_NAME, O_RDWR | O_CREAT); 

    while (true) {
        sem_wait(s1);

        connection->game_id = ++connected_clients;
        DEBUG("Server: assigned game_id: %d\n", connection->game_id);

        char *game_name1 = get_game_sem(connection->game_id, 1);
        char *game_name2 = get_game_sem(connection->game_id, 2);
                

        child_1 = sem_open(game_name1, O_CREAT | O_EXCL, 0600, 0);
        child_2 = sem_open(game_name2, O_CREAT | O_EXCL, 0600, 0);
    
        switch (pid = fork()) {
            case -1: bail_out("", EXIT_FAILURE, "can't fork\n");
                     break;
                     // child process
            case 0:  new_game(connection->game_id); 
                     break; 
                     // parent process
            default: // maybe save pid for later?
                     break; 
        }
        
        sem_post(s2);
}

return EXIT_SUCCESS;
}

