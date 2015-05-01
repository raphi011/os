#define _GNU_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>

static const char* modulname;

static void usage() {
    (void)fprintf(stderr, "usage: %s [-s <seconds>] [-f <seconds>] <program> <emergency> <logfile>", modulname);
    exit(EXIT_FAILURE);
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
                // timeframe = optarg;
            case 'f':
                // timeframe_duration  = optarg;
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

    switch (pid = fork()) {
        case -1: bail_out("Fork failed");
                 break;
        case 0:  /* childprocess */

                 break;
        default: /* parent process */
                 int pid = wait(&status);
                 if (WEXITSTATUS(status)) {
                    break;
                 }
                 break;

    }


}
