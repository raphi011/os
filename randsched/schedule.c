#define _GNU_SOURCE

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

static const char* modulname;

static void usage() {
    (void)fprintf(stderr, "usage: %s [-s <seconds>] [-f <seconds>] <program> <emergency> <logfile>\n", modulname);
    exit(EXIT_FAILURE);
}

static void free_resources(void) {

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
                 bail_out(EXIT_FAILURE, "error while attempting to execute prog\n");
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

int parseInt(char * str, int * ptr) {
    char * endptr;
    
    *ptr = strtol(str, &endptr, 10);
    
    if (*endptr != '\0') {
        return -1;
    }
    
    return 0;

}

int main(int argc, char* argv[]) {
    
    modulname = argv[0];
    int timeframe = 1;
    int timeframe_duration = 0;
    int index;
    int c; 

    while ((c = getopt (argc, argv, "s:f:")) != -1) {
        switch (c) {
            case 's':
                if (parseInt(optarg, &timeframe)) {
                    usage();   
                }
                break;
            case 'f':
                if (parseInt(optarg, &timeframe_duration)) {
                    usage();   
                } 
                break;
            default: 
                usage(); 
        }
    }

    index = optind;

    if ((argc-index) != 3 ) {
        usage(); 
    }
    
    char* program = argv[index++];
    char* emergency = argv[index++];
    char* logfile = argv[index]; 
    
    pid_t pid;
    int status;

    int fildes[2];

    if (pipe(fildes) != 0) {
        bail_out(EXIT_FAILURE, "Can't create pipe\n"); 
    }

    switch (pid = fork()) {
        case -1: bail_out(EXIT_FAILURE, "Fork failed\n");
                 break;
        case 0:  /* childprocess */
                 close(fildes[0]); 
                 if (dup2(fildes[1], fileno(stdout)) == -1) {
                    bail_out(EXIT_FAILURE, "child dup2 failed\n");
                 } 
                 
                 char string1[]="String for pipe I/O";
                 write(fildes[1],string1,strlen(string1));
                

                 srand(time(NULL));    
    
                 int return_code = 0;

                 do {
                    int sleep_time = timeframe_duration > 0 ? 
                                ((rand() % timeframe_duration) + timeframe)  :
                                timeframe; 
                    (void)sleep(sleep_time); 
                    return_code = execute_and_wait(program); 

                 } while (!return_code); 
                
                 execute_and_wait(emergency);
                 close(fildes[1]);

                 break;
        default: /* parent process */
                 close(fildes[1]); 
                 
                 FILE * file = fopen(logfile, "w");
                 int fd = fileno(file);

                 if (fd == -1) {
                    bail_out(EXIT_FAILURE, strerror(errno));   
                 }
                
/*                 if (dup2(fildes[0], fd) == -1) {
                    bail_out(EXIT_FAILURE, strerror(errno));   
                 } */


                 /* for testing purposes */
                 char reading_buf[3];
                 while(read(fildes[0], reading_buf, 3) > 0) {
                    write(fd, reading_buf, 1); // 1 -> stdout
                 }


                 (void)wait(&status);
                 close(fildes[0]);
                 break;
    }
}

