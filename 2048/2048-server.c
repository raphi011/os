#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

static const char* modulname;

static void usage() {
    (void)fprintf(stderr, "USAGE: %s [-p power_of_two]\n\t-p:\tPlay until 2ˆpower_of_two is reached (default: 11)\n",modulname);
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

int main(int argc, char* argv[]) {
    
    modulname = argv[0];
    int index;
    int c; 

    while ((c = getopt (argc, argv, "s:f:")) != -1) {
        switch (c) {
            case 's':
                /* if (parseInt(optarg, &timeframe)) {
                    usage();   
                } */
                break;
            case 'f':
                /* if (parseInt(optarg, &timeframe_duration)) {
                    usage();   
                }  */
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
    

}

