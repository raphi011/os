/** 
 * @file schedule.c
 * @author Raphael Gruber (0828630)
 * @brief Repeatedly executes a program after a given amount of time,
 * in case of an error an emergency program is executed.
 * The programs output is logged to a given logfile
 * @date 02.05.2015
 */

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

/**
 * Size of the pipe buffer
 */
#define BUFFER_SIZE (30)

/**
 * amount of positional arguments
 */
#define POS_ARG_COUNT (3)

/**
 * Calculates the size of an array
 */
#define COUNT_OF(x) (sizeof(x)/sizeof(x[0]))

/** 
 * The name of the program
 */
static const char* modulname;

/** 
 * Logfile file descriptor 
*/
static int logfd;

/**
 * A struct for easy access to the program parameters 
 */
struct params {
    int timeframe_duration;
    int timeframe;
    char *logfile;
    char *program;
    char *emergency;
} params = {0,1,NULL,NULL,NULL}; /* initialize params */

/**
 * usage
 * @brief prints the usage message with the correct call syntax and exits
 * with EXIT_FAILURE
 * @details Global parameters: modulname
 */
static void usage(void) {
    (void)fprintf(stderr, "usage: %s [-s <seconds>] [-f <seconds>] <program> <emergency> <logfile>\n", modulname);
    exit(EXIT_FAILURE);
}

/** 
 * free_resources
 * @brief frees open resources and signal handlers
 * @details Global parameters: logfd
 */
static void free_resources(void) {
    sigset_t blocked_signals;
    (void) sigfillset(&blocked_signals);
    (void) sigprocmask(SIG_BLOCK, &blocked_signals, NULL);

    if (logfd >= 0) {
        (void) close(logfd); 
    }
}

/**
 * signal_handler
 * @brief The signal handler
 * @param sig The signal that occured
 */
static void signal_handler(int sig) {
    printf("caught signal %d\n", sig);
    free_resources(); 
    exit(EXIT_SUCCESS);
}

/**
 * bail_out
 * @brief Stops the program gracefully, freeing resources and printing an
 * error message to stderr
 * @param eval The return code of the program
 * @param fmt The error message
 * @param ... message format parameters
 */
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

/** 
 * execute_and_wait
 * @brief Forks the process and executes the given program without params
 * waiting for it to exit
 * @return The returncode of the executed program
 */
static int execute_and_wait(char *prog) {
    pid_t pid;
    int status;

    switch (pid = fork()) {
        case -1: bail_out(EXIT_FAILURE, "fork failed\n"); 
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
        bail_out(EXIT_FAILURE, "prog '%s' didn't terminate normally\n", prog);
    }

    return WEXITSTATUS(status); 
}

/** 
* set_signal_handler
* @brief sets the signal handler for SIGINT, SIGQUIT, SIGTERM
*/
static void set_signal_handler(void) {
    sigset_t blocked_signals;

    if (sigfillset(&blocked_signals) < 0) {
        bail_out(EXIT_FAILURE, "sigfillset\n");
    } else {
        const int signals[] = { SIGINT, SIGQUIT, SIGTERM };
        struct sigaction s;
        s.sa_handler = signal_handler;
        (void)memcpy(&s.sa_mask, &blocked_signals, sizeof(s.sa_mask));
        s.sa_flags = SA_RESTART;
        for (size_t i = 0; i < COUNT_OF(signals); i++) {
            if (sigaction(signals[i], &s, NULL) < 0) {
                bail_out(EXIT_FAILURE, "sigaction\n");
            }
        }
    }
}

/** parse_int
 * @brief Parses an int value
 * @params str The char * that will be parsed
 * @params ptr The int * where the value will be stored
 * @return 1 if str was an int, else 0
 */
static int parse_int(char * str, int * ptr) {
    char * endptr;
    
    *ptr = strtol(str, &endptr, 10);
    
    return (*endptr == '\0'); 
}

/**
 * child
 * @brief Executes child code after forking, waits timeframe + (0..timeframe_duration) 
 * seconds and then executes program redirecting its stdout to the parent via an 
 * unnamed pipe, prog will be executed continously  until it returns a value != 0
 * then it runs emergency and stops
 * @param write_pipe The write pipe file descriptor
 * @details Global variables: params
 */
static void child(int write_pipe) {

     int stdout_copy = dup(fileno(stdout)); /* save stdout fd */

     if (dup2(write_pipe, fileno(stdout)) == -1) { /* redirect stdout to pipe */
        bail_out(EXIT_FAILURE, "dup2 failed\n");
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
     
     (void)close(STDOUT_FILENO); /* stop emergency from writing to pipe */ 
    
     if (dup2(stdout_copy, STDOUT_FILENO) == -1) { /* reassign stdout to fd 1 */
        bail_out(EXIT_FAILURE, "dup2 failed\n"); 
     }; 

     (void)close(stdout_copy);

     int ret = execute_and_wait(params.emergency);
     
     char * emergency_log = ret ? "EMERGENCY UNSUCCESSFUL\n" :
                                  "EMERGENCY SUCCESSFUL\n";

     /* write emergency status to pipe */
     if ((int)write(write_pipe, emergency_log, strlen(emergency_log)) == -1) {
        bail_out(EXIT_FAILURE, "write failed\n");        
     } 
    
     (void)close(write_pipe);
}

/** 
 * parent
 * @brief Executes parent code after the fork, reads from the given pipe
 * and writes it to stdout and the logfile.
 * @param read_pipe The read pipe file descriptor
 * @details Global variables: params, logfd
 */
static void parent(int read_pipe) {
     int status;
     FILE * file = fopen(params.logfile, "w");
     logfd = fileno(file);

     if (logfd == -1) {
        bail_out(EXIT_FAILURE, "%s\n", strerror(errno));   
     }

     char buf[BUFFER_SIZE];

     int bytes_read;
     
     do {
        bytes_read = read(read_pipe, buf, BUFFER_SIZE);
        if ((int)write(fileno(stdout), buf, bytes_read) == -1) {
            bail_out(EXIT_FAILURE, "write failed\n");
        }
        if ((int)write(logfd, buf, bytes_read) == -1) {
            bail_out(EXIT_FAILURE, "write failed\n");
        }
         
     } while (bytes_read > 0); 

     if (wait(&status) == -1)  {
        bail_out(EXIT_FAILURE, "wait failed\n");   
     }
     (void)close(read_pipe);
     free_resources();
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

    if ((argc-index) != POS_ARG_COUNT) { /* make sure there are 3 more params */
        usage(); 
    }
    
    params.program = argv[index++];
    params.emergency = argv[index++];
    params.logfile = argv[index]; 
    
    pid_t pid;

    set_signal_handler();

    int fildes[2];

    if (pipe(fildes) != 0) {
        bail_out(EXIT_FAILURE, "can't create pipe\n"); 
    }

    switch (pid = fork()) {
        case -1: bail_out(EXIT_FAILURE, "fork failed\n");
                 break;
        case 0:  /* childprocess */
                 (void)close(fildes[0]); 
                 child(fildes[1]);             
                 break;
        default: /* parent process */
                 (void)close(fildes[1]); 
                 parent(fildes[0]);
                 break;
    }

    return EXIT_SUCCESS;
}

