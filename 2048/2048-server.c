#define _GNU_SOURCE

#include <stdio.h>
#include <assert.h>
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

/** parse_int
 * @brief Parses an int value
 * @params str The char * that will be parsed
 * @params ptr The int * where the value will be stored
 * @return 1 if str was an int, else 0
 */
static int parse_int(char * str, int * ptr) {
    char * endptr;
    
    *ptr = strtol(str, &endptr, 10);
    
    // todo, function doesn't check all errors
    return (*endptr == '\0'); 
}

int main(int argc, char* argv[]) {
    
    modulname = argv[0];
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
}

