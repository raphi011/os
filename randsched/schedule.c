#define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>

#define BUFFER_SIZE (30)

#define COUNT_OF(x) (sizeof(x)/sizeof(x[0]))

static const char* modulname;


static int logfd;

struct params {
    int timeframe_duration;
    int timeframe;
    char *logfile;
    char *program;
    char *emergency;
} params = {0,1,NULL,NULL,NULL}; /* initialize params */


static void usage(void) {
    (void)fprintf(stderr, "usage: %s [-s <seconds>] [-f <seconds>] <program> <emergency> <logfile>\n", modulname);
    exit(EXIT_FAILURE);
}

static void free_resources(void) {
    sigset_t blocked_signals;
    (void) sigfillset(&blocked_signals);
    (void) sigprocmask(SIG_BLOCK, &blocked_signals, NULL);

    if (logfd >= 0) {
        (void) close(logfd); 
    }
}

static void signal_handler(int sig) {
    printf("caught signal %d\n", sig);
    free_resources(); 
    exit(EXIT_SUCCESS);
}

static void bail_out(int eval, const char *fmt, ...)
{
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

    free_resources();
    exit(eval);
}

static int execute_and_wait(char *prog) {
    pid_t pid;
    int status;

    switch (pid = fork()) {
        case -1: bail_out(EXIT_FAILURE, "Fork failed\n"); 
                 break;
        case 0:  
                 execl(prog, prog, (char *)NULL);
                 bail_out(EXIT_FAILURE, "execl error executing '%s'\n", prog);
                 break;
        default: 
                 (void)wait(&status); 
                 break;
    }
    
    if (!WIFEXITED(status)) {
        bail_out(EXIT_FAILURE, "didn't terminate normally");
    }

    return WEXITSTATUS(status); 
}

static void set_signal_handler(void) {
    sigset_t blocked_signals;

    if (sigfillset(&blocked_signals) < 0) {
        bail_out(EXIT_FAILURE, "sigfillset");
    } else {
        const int signals[] = { SIGINT, SIGQUIT, SIGTERM };
        struct sigaction s;
        s.sa_handler = signal_handler;
        (void)memcpy(&s.sa_mask, &blocked_signals, sizeof(s.sa_mask));
        s.sa_flags = SA_RESTART;
        for (size_t i = 0; i < COUNT_OF(signals); i++) {
            if (sigaction(signals[i], &s, NULL) < 0) {
                bail_out(EXIT_FAILURE, "sigaction");
            }
        }
    }
}

static int parse_int(char * str, int * ptr) {
    char * endptr;
    
    *ptr = strtol(str, &endptr, 10);
    
    return (*endptr == '\0'); 
}

static void child(int write_pipe) {

     int stdout_copy = dup(fileno(stdout)); /* save stdout fd */

     if (dup2(write_pipe, fileno(stdout)) == -1) { /* redirect stdout to pipe */
        bail_out(EXIT_FAILURE, "child dup2 failed\n");
     } 
     
     srand(time(NULL));    

     int return_code = 0;

     do {
        int sleep_time = params.timeframe_duration > 0 ? 
                    ((rand() % params.timeframe_duration) + params.timeframe)  :
                    params.timeframe; /* avoid division by zero */ 
        (void)sleep(sleep_time); 
        return_code = execute_and_wait(params.program); 

     } while (!return_code); /* while program runs without an error */ 
     
     (void)close(fileno(stdout)); /* stop emergency from writing to pipe */ 
    
     (void)dup2(stdout_copy, STDOUT_FILENO); /* reassign stdout to fd 1 */
     (void)close(stdout_copy);

     int ret = execute_and_wait(params.emergency);
     
     char * emergency_log = ret ? "EMERGENCY UNSUCCESSFUL\n" :
                                  "EMERGENCY SUCCESSFUL\n";

     /* write emergency status to pipe */
     (void)write(write_pipe, emergency_log, strlen(emergency_log)); 
    
     (void)close(write_pipe);
}

static void parent(int read_pipe) {
     int status;
     FILE * file = fopen(params.logfile, "w");
     logfd = fileno(file);

     if (logfd == -1) {
        bail_out(EXIT_FAILURE, strerror(errno));   
     }

     char buf[BUFFER_SIZE];

     int bytes_read;
     
     do {
        bytes_read = read(read_pipe, buf, BUFFER_SIZE);
        write(fileno(stdout), buf, bytes_read);
        write(logfd, buf, bytes_read); 
     } while (bytes_read > 0); 

     (void)wait(&status);
     (void)close(read_pipe);
     free_resources();
}

int main(int argc, char* argv[]) {
    
    modulname = argv[0];
    int index;
    int c; 

    while ((c = getopt (argc, argv, "s:f:")) != -1) {
        switch (c) {
            case 's':
                if (!parse_int(optarg, &params.timeframe)) {
                    usage();   
                }
                break;
            case 'f':
                if (!parse_int(optarg, &params.timeframe_duration)) {
                    usage();   
                } 
                break;
            case '?':
                usage();
            default: 
                assert(0);
        }
    }

    index = optind;

    if ((argc-index) != 3 ) {
        usage(); 
    }
    
    params.program = argv[index++];
    params.emergency = argv[index++];
    params.logfile = argv[index]; 
    
    pid_t pid;

    set_signal_handler();

    int fildes[2];

    if (pipe(fildes) != 0) {
        bail_out(EXIT_FAILURE, "Can't create pipe\n"); 
    }

    switch (pid = fork()) {
        case -1: bail_out(EXIT_FAILURE, "Fork failed\n");
                 break;
        case 0:  /* childprocess */
                 close(fildes[0]); 
                 child(fildes[1]);             
                 break;
        default: /* parent process */
                 close(fildes[1]); 
                 parent(fildes[0]);
                 break;
    }
}

